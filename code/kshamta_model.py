import sys
import os
import queue
import json
import sounddevice as sd
from vosk import Model, KaldiRecognizer
from RF24 import RF24, RF24_PA_LOW

# ========== VOSK SETUP ==========
model_path = '/home/pi/models/vosk-model-small-en-us-0.15'
model = Model(model_path)
samplerate = 16000
device = None  # Use default mic

q = queue.Queue()

# ========== NRF24L01 SETUP ==========
# Initialize radio: CE pin = GPIO 25, CSN = SPI CE0 (pin 24)
radio = RF24(25, 0)
radio.begin()

# Configure radio
radio.setPALevel(RF24_PA_LOW)
radio.setChannel(0x76)  # Channel 118
radio.setDataRate(RF24_250KBPS)
radio.setPayloadSize(32)

# Set transmit address (must match receiver)
transmit_address = [0xE0, 0xE0, 0xF1, 0xF1, 0xE0]
radio.openWritingPipe(transmit_address)
radio.stopListening()  # Put in transmit mode

print("nRF24L01 initialized and ready to transmit")
radio.printDetails()

# ========== AUDIO CALLBACK ==========
def callback(indata, frames, time, status):
    q.put(bytes(indata))

# ========== TRANSMIT FUNCTION ==========
def transmit_text(text):
    """Transmit entire text string via nRF24L01"""
    try:
        # nRF24L01 max payload is 32 bytes
        # Split long messages into chunks if needed
        chunk_size = 32
        
        # If text is longer than 32 chars, send in chunks
        if len(text) <= 32:
            # Send as single message
            message = list(text.ljust(32))
            success = radio.write(message)
            
            if success:
                print(f"✓ Transmitted: '{text}'")
            else:
                print(f"✗ Transmission failed for: '{text}'")
        else:
            # Split into chunks and send
            print(f"Sending '{text}' in chunks...")
            chunks = [text[i:i+chunk_size] for i in range(0, len(text), chunk_size)]
            
            for i, chunk in enumerate(chunks):
                message = list(chunk.ljust(32))
                success = radio.write(message)
                
                if success:
                    print(f"✓ Chunk {i+1}/{len(chunks)}: '{chunk.strip()}'")
                else:
                    print(f"✗ Chunk {i+1}/{len(chunks)} failed")
                
                # Small delay between chunks
                import time
                time.sleep(0.01)
                
    except Exception as e:
        print(f"Error transmitting: {e}")

# ========== MAIN LOOP ==========
with sd.RawInputStream(samplerate=samplerate, blocksize=8000, dtype='int16',
                       channels=1, callback=callback, device=device):
    print("Speak into the mic...")
    rec = KaldiRecognizer(model, samplerate)
    
    while True:
        data = q.get()
        
        if rec.AcceptWaveform(data):
            # Final result - complete phrase
            result_json = rec.Result()
            result = json.loads(result_json)
            text = result.get('text', '')
            
            print(f"Recognized: {text}")
            
            # Transmit entire text
            if text:
                transmit_text(text)
                
        else:
            # Partial result - word being spoken
            print(rec.PartialResult(), end='
')