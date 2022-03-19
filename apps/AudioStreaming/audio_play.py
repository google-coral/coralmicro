import argparse
import socket
import struct

import pyaudio  # sudo apt-get install python3-pyaudio

def main():
    parser = argparse.ArgumentParser(description='Play audio from Valiant mic')
    parser.add_argument('--host', type=str, default='10.10.10.1')
    parser.add_argument('--port', '-p', type=int, default=33000)
    parser.add_argument('--sample_rate_hz', '-r', type=int,
                        choices=(16000, 48000), default=16000)
    parser.add_argument('--dma_buffer_size_ms', '-b', type=int, default=50)
    parser.add_argument('--num_dma_buffers', '-n', type=int, default=6)
    args = parser.parse_args()

    sock = socket.socket()
    sock.connect((args.host, args.port))
    sock.send(struct.pack('<iii', args.sample_rate_hz,
                                  args.dma_buffer_size_ms,
                                  args.num_dma_buffers))

    audio = pyaudio.PyAudio()
    stream = audio.open(format=pyaudio.paInt32,
                        channels=1,
                        rate=args.sample_rate_hz,
                        output=True)

    samples_per_buffer = args.dma_buffer_size_ms * (args.sample_rate_hz // 1000)
    bytes_per_buffer = 4 * samples_per_buffer
    while True:
        samples = sock.recv(bytes_per_buffer // 2)
        if not samples:
            break
        stream.write(samples)

if __name__ == '__main__':
   main()
