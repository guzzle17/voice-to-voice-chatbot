#!/usr/bin/env python3
"""
Real-time interactive voice assistant pipeline

Flow:
1. Record audio from microphone (press key to stop)
2. Transcribe with Whisper.cpp
3. Generate response with LLaMA (streaming)
4. Speak response with Piper TTS (streaming)
5. Repeat!
"""

import argparse
import os
import subprocess
import time
import tempfile
import sys
import threading
import queue
from llama_cpp import Llama

# Piper Python API (required for streaming)
try:
    from piper import PiperVoice
    PIPER_PYTHON_AVAILABLE = True
except ImportError:
    PIPER_PYTHON_AVAILABLE = False
    print("❌ Piper Python API required. Install with: pip install piper-tts")
    sys.exit(1)

# Model paths - UPDATE THESE TO MATCH YOUR SYSTEM
WHISPER_MODEL_PATH = "/home/dung/whisper.cpp/models/ggml-base.en.bin"
WHISPER_EXECUTABLE = "/home/dung/whisper.cpp/build/bin/whisper-cli"
LLAMA_MODEL_PATH = "/home/dung/llama.cpp/models/Llama-3.1-8B-Instruct-Q4_K_M.gguf"
PIPER_MODEL_PATH = "/home/dung/piper/en_US-kristin-medium.onnx"

# Global model instances
llama_model = None
piper_voice = None

# Conversation history for context
conversation_history = []

def load_piper_voice():
    """Load Piper voice model for streaming"""
    global piper_voice
    
    if piper_voice is None:
        try:
            print(f"Loading Piper voice from {PIPER_MODEL_PATH}...")
            piper_voice = PiperVoice.load(PIPER_MODEL_PATH)
            print("✓ Piper voice loaded")
            return True
        except Exception as e:
            print(f"❌ Failed to load Piper: {e}")
            sys.exit(1)
    return True

def load_llama_model():
    """Load LLaMA model once at startup"""
    global llama_model
    
    print("[Setup] Loading LLaMA model...")
    
    try:
        default_cpu = os.cpu_count() or 1
    except Exception:
        default_cpu = 1
    
    n_threads = int(os.getenv("LLAMA_N_THREADS", str(min(8, default_cpu))))
    n_gpu_layers = int(os.getenv("LLAMA_N_GPU_LAYERS", "0"))
    n_ctx = int(os.getenv("LLAMA_N_CTX", "4096"))
    
    print(f"[Setup] Config: n_ctx={n_ctx}, threads={n_threads}, gpu_layers={n_gpu_layers}")
    
    llama_model = Llama(
        model_path=LLAMA_MODEL_PATH,
        n_ctx=n_ctx,
        n_threads=n_threads,
        n_gpu_layers=n_gpu_layers,
        use_mlock=True,
        use_mmap=True,
        verbose=False,
    )
    print("[Setup] ✓ LLaMA model loaded")

def record_audio(output_path: str, duration: int = None):
    """Record audio from microphone using arecord or sox"""
    print("\n🎤 Recording audio...")
    
    if duration:
        print(f"   Recording for {duration} seconds...")
    else:
        print("   Press Ctrl+C to stop recording...")
    
    try:
        # Try arecord first (Linux)
        cmd = ["arecord", "-f", "S16_LE", "-r", "16000", "-c", "1", output_path]
        if duration:
            cmd.extend(["-d", str(duration)])
        
        subprocess.run(cmd, check=True)
        print("✓ Recording complete!")
        return True
        
    except FileNotFoundError:
        # Try sox/rec as fallback
        try:
            cmd = ["rec", "-r", "16000", "-c", "1", output_path]
            if duration:
                cmd.extend(["trim", "0", str(duration)])
            
            subprocess.run(cmd, check=True)
            print("✓ Recording complete!")
            return True
            
        except FileNotFoundError:
            print("❌ Error: Neither arecord nor sox is installed.")
            print("   Install: sudo apt install alsa-utils sox")
            return False
    except KeyboardInterrupt:
        print("\n✓ Recording stopped!")
        return True
    except Exception as e:
        print(f"❌ Recording failed: {e}")
        return False

def transcribe_audio(audio_path: str) -> str:
    """Transcribe audio using Whisper.cpp"""
    print("🗣️  Transcribing audio...")
    
    start_time = time.time()
    
    try:
        result = subprocess.run(
            [WHISPER_EXECUTABLE, "-m", WHISPER_MODEL_PATH, "-f", audio_path, "--no-timestamps"],
            capture_output=True,
            text=True,
            check=True
        )
        
        # Parse output
        text = result.stdout.strip()
        lines = [line.strip() for line in text.split('\n') if line.strip() and not line.startswith('[')]
        text = ' '.join(lines)
        
        elapsed = time.time() - start_time
        
        print(f"✓ Transcription: \"{text}\"")
        print(f"  ({elapsed:.2f}s)")
        return text
        
    except subprocess.CalledProcessError as e:
        print(f"❌ Whisper failed: {e.stderr}")
        return ""
    except FileNotFoundError:
        print(f"❌ Whisper executable not found at: {WHISPER_EXECUTABLE}")
        return ""

def generate_reply_with_streaming_speech(prompt: str, use_context: bool = True) -> str:
    """
    Generate reply and speak simultaneously with streaming TTS
    Uses Piper Python API for native streaming
    """
    print("🤖 Thinking and speaking...")
    
    start_time = time.time()
    return _generate_with_native_streaming(prompt, use_context, start_time)

def _generate_with_native_streaming(prompt: str, use_context: bool, start_time: float) -> str:
    """TRUE streaming: Tokens → Piper API → Audio player in real-time"""
    
    # Audio output setup
    try:
        import pyaudio
        from contextlib import contextmanager
        
        @contextmanager
        def suppress_stderr():
            """Temporarily suppress stderr to hide ALSA warnings"""
            devnull = os.open(os.devnull, os.O_WRONLY)
            old_stderr = os.dup(2)
            sys.stderr.flush()
            os.dup2(devnull, 2)
            os.close(devnull)
            try:
                yield
            finally:
                os.dup2(old_stderr, 2)
                os.close(old_stderr)
        
        # Initialize PyAudio with suppressed warnings
        with suppress_stderr():
            p = pyaudio.PyAudio()
        
        audio_stream = None
        first_chunk = True
        
        # Audio playback queue (non-blocking)
        audio_queue = queue.Queue()
        audio_thread = None
        audio_active = threading.Event()
        audio_lock = threading.Lock()
        
        def audio_player_worker():
            """Background thread that plays audio chunks"""
            nonlocal audio_stream, first_chunk
            
            try:
                while True:
                    item = audio_queue.get()
                    
                    if item is None:
                        audio_queue.task_done()
                        break
                    
                    audio_chunk = item
                    
                    if first_chunk:
                        try:
                            with audio_lock, suppress_stderr():
                                audio_stream = p.open(
                                    format=p.get_format_from_width(audio_chunk.sample_width),
                                    channels=audio_chunk.sample_channels,
                                    rate=audio_chunk.sample_rate,
                                    output=True,
                                    frames_per_buffer=2048
                                )
                            audio_stream.start_stream()
                            first_chunk = False
                            audio_active.set()
                        except Exception as e:
                            print(f"\n⚠️  Failed to initialize audio: {e}", flush=True)
                            audio_queue.task_done()
                            continue
                    
                    if audio_stream:
                        try:
                            with audio_lock:
                                if audio_stream and audio_stream.is_active():
                                    audio_stream.write(audio_chunk.audio_int16_bytes)
                        except Exception as e:
                            print(f"\n⚠️  Audio playback error: {e}", flush=True)
                    
                    audio_queue.task_done()
            except Exception as e:
                print(f"\n⚠️  Audio thread crashed: {e}", flush=True)
        
        audio_thread = threading.Thread(target=audio_player_worker, daemon=True)
        audio_thread.start()
        
    except ImportError:
        print("❌ pyaudio is required for streaming. Install with: pip install pyaudio")
        sys.exit(1)
    
    # Build prompt
    if use_context and conversation_history:
        context = "\n".join(conversation_history[-6:])
        full_prompt = f"""<|im_start|>system
You are a helpful AI assistant. Keep your responses suitable for text-to-speech synthesis:
- Use only plain text, no emojis, emoticons, or special characters
- Avoid asterisks, markdown formatting, or bullet points
- Write out numbers and abbreviations (e.g., "Doctor" not "Dr.", "2" as "two")
- Use natural, conversational language that sounds good when spoken aloud
{context}<|im_end|>
<|im_start|>user
{prompt}<|im_end|>
<|im_start|>assistant
"""
    else:
        full_prompt = f"""<|im_start|>system
You are a helpful AI assistant. Keep your responses suitable for text-to-speech synthesis:
- Use only plain text, no emojis, emoticons, or special characters
- Avoid asterisks, markdown formatting, or bullet points
- Write out numbers and abbreviations (e.g., "Doctor" not "Dr.", "2" as "two")
- Use natural, conversational language that sounds good when spoken aloud<|im_end|>
<|im_start|>user
{prompt}<|im_end|>
<|im_start|>assistant
"""
    
    max_tokens = int(os.getenv("LLAMA_MAX_TOKENS", "-1"))
    temperature = float(os.getenv("LLAMA_TEMPERATURE", "0.2"))
    
    response_text = ""
    current_buffer = ""
    
    print("🤖 ", end="", flush=True)
    
    try:
        for output in llama_model(
            full_prompt,
            max_tokens=max_tokens,
            temperature=temperature,
            stop=["<|im_end|>", "<|endoftext|>"],
            echo=False,
            stream=True
        ):
            chunk = output["choices"][0]["text"]
            response_text += chunk
            current_buffer += chunk
            
            print(chunk, end="", flush=True)
            
            # Smart synthesis with sliding window approach
            buffer_stripped = current_buffer.strip()
            word_count = len(buffer_stripped.split())
            
            # Priority 1: Complete sentence (best quality)
            sentence_complete = (
                buffer_stripped.endswith('. ') or 
                buffer_stripped.endswith('! ') or 
                buffer_stripped.endswith('? ') or
                buffer_stripped.endswith('.\n') or 
                buffer_stripped.endswith('!\n') or 
                buffer_stripped.endswith('?\n')
            )
            
            # Priority 2: Natural phrase break with enough content
            phrase_breaks = [', ', '; ', ': ', ' and ', ' or ', ' but ', ' - ']
            has_phrase_break = any(buffer_stripped.endswith(p) for p in phrase_breaks)
            
            text_to_synth = None
            
            if sentence_complete:
                # Best case: complete sentence
                text_to_synth = buffer_stripped
                current_buffer = ""
                
            elif word_count >= 8 and has_phrase_break:
                # Good case: natural phrase boundary
                text_to_synth = buffer_stripped
                current_buffer = ""
                
            elif word_count >= 12:
                # Fallback: buffer too long, find best split point
                best_break = -1
                break_len = 0
                
                all_breaks = ['. ', '! ', '? ', ', ', '; ', ': ', ' and ', ' or ', ' but ']
                for break_char in all_breaks:
                    pos = buffer_stripped.rfind(break_char)
                    if pos > best_break and pos > len(buffer_stripped) * 0.4:
                        best_break = pos
                        break_len = len(break_char)
                
                if best_break > 0:
                    text_to_synth = buffer_stripped[:best_break + break_len].strip()
                    current_buffer = buffer_stripped[best_break + break_len:]
                else:
                    last_space = buffer_stripped.rfind(' ', 0, len(buffer_stripped) // 2)
                    if last_space > 0:
                        text_to_synth = buffer_stripped[:last_space].strip()
                        current_buffer = buffer_stripped[last_space:]
            
            if text_to_synth:
                try:
                    for audio_chunk in piper_voice.synthesize(text_to_synth):
                        audio_queue.put(audio_chunk)
                        
                        if not audio_active.is_set():
                            print(" 🔊", end="", flush=True)
                except Exception as e:
                    if not audio_active.is_set():
                        print(f"\n⚠️  Audio error: {e}", flush=True)
        
        # Flush remaining buffer
        if current_buffer.strip():
            try:
                for audio_chunk in piper_voice.synthesize(current_buffer.strip()):
                    audio_queue.put(audio_chunk)
            except:
                pass
        
        print()
        
    except Exception as e:
        print(f"\n⚠️  Error: {e}")
    finally:
        try:
            audio_queue.put(None)
            audio_queue.join()
            if audio_thread and audio_thread.is_alive():
                audio_thread.join(timeout=2)
            
            with audio_lock:
                if audio_stream:
                    try:
                        if audio_stream.is_active():
                            audio_stream.stop_stream()
                        audio_stream.close()
                        audio_stream = None
                    except:
                        pass
            
            try:
                with suppress_stderr():
                    p.terminate()
            except:
                pass
        except Exception as e:
            print(f"\n⚠️  Cleanup error: {e}", flush=True)
    
    elapsed = time.time() - start_time
    print(f"  ({elapsed:.2f}s total)")
    
    # Update conversation history
    if use_context:
        conversation_history.append(f"<|im_start|>user\n{prompt}<|im_end|>")
        conversation_history.append(f"<|im_start|>assistant\n{response_text.strip()}<|im_end|>")
    
    return response_text.strip()

def interactive_mode():
    """Run interactive voice assistant loop"""
    print("\n" + "=" * 60)
    print("🎙️  INTERACTIVE VOICE ASSISTANT")
    print("=" * 60)
    print("\nCommands:")
    print("  - Press Enter to start recording")
    print("  - Press Ctrl+C while recording to stop")
    print("  - Type 'quit' or 'exit' to end session")
    print("  - Type 'clear' to reset conversation history")
    print("=" * 60 + "\n")
    
    while True:
        try:
            # Wait for user to press Enter
            user_input = input("\n👤 Press Enter to speak (or type command): ").strip().lower()
            
            if user_input in ['quit', 'exit', 'q']:
                print("\n👋 Goodbye!")
                break
            
            if user_input == 'clear':
                conversation_history.clear()
                print("✓ Conversation history cleared")
                continue
            
            # Record audio
            with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as f:
                audio_path = f.name
            
            if not record_audio(audio_path):
                continue
            
            # Transcribe
            text = transcribe_audio(audio_path)
            if not text:
                print("⚠️  No speech detected, try again")
                os.unlink(audio_path)
                continue
            
            # Generate response with streaming speech
            response = generate_reply_with_streaming_speech(text, use_context=True)
            
            if not response:
                print("⚠️  Failed to generate response")
                os.unlink(audio_path)
                continue
            
            # Cleanup (skip speak_text call since streaming already played audio)
            os.unlink(audio_path)
            
        except KeyboardInterrupt:
            print("\n\n👋 Goodbye!")
            break
        except Exception as e:
            print(f"\n❌ Error: {e}")
            continue

def single_shot_mode(audio_file: str, output_file: str = None, no_play: bool = False):
    """Process a single audio file and optionally save output"""
    print("\n" + "=" * 60)
    print("🎙️  SINGLE-SHOT MODE")
    print("=" * 60 + "\n")
    
    # Transcribe
    text = transcribe_audio(audio_file)
    if not text:
        print("❌ Transcription failed")
        return
    
    # Generate response with streaming (but don't play if no_play is set)
    if no_play and output_file:
        # Generate without audio playback
        print("🤖 Generating response (no audio playback)...")
        response = generate_reply_with_streaming_speech(text, use_context=False)
    else:
        # Generate with streaming audio
        response = generate_reply_with_streaming_speech(text, use_context=False)
    
    if not response:
        print("❌ Response generation failed")
        return
    
    # Save transcript if output file specified
    if output_file:
        transcript_file = output_file.replace('.wav', '_transcript.txt')
        with open(transcript_file, 'w') as f:
            f.write(f"Input: {text}\n\nResponse: {response}\n")
        print(f"\n✅ Transcript saved to: {transcript_file}")
    
    print("\n" + "=" * 60)
    print("✅ Pipeline complete!")
    print("=" * 60)

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description="Real-time Voice Assistant Pipeline")
    parser.add_argument("--interactive", "-i", action="store_true",
                       help="Run in interactive mode (default)")
    parser.add_argument("--audio", help="Input audio file (single-shot mode)")
    parser.add_argument("--output", help="Output transcript file (single-shot mode)")
    parser.add_argument("--no-play", action="store_true",
                       help="Don't play audio, just generate response")
    args = parser.parse_args()
    
    # Load models
    print("\n" + "=" * 60)
    print("🎙️  Initializing Voice Assistant...")
    print("=" * 60)
    load_llama_model()
    load_piper_voice()
    print("=" * 60)
    
    # Choose mode
    if args.audio:
        # Single-shot mode
        single_shot_mode(args.audio, args.output, args.no_play)
    else:
        # Interactive mode (default)
        interactive_mode()

if __name__ == "__main__":
    main()

