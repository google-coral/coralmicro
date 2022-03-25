import argparse
import socket
import struct

import pyaudio  # sudo apt-get install python3-pyaudio

def main():
    parser = argparse.ArgumentParser(description='Play audio from Valiant mic')
    parser.add_argument('--host', type=str, default='10.10.10.1')
    parser.add_argument('--port', type=int, default=33000)
    parser.add_argument('--sample_rate_hz', '-r', type=int,
                        choices=(16000, 48000), default=48000)
    parser.add_argument('--sample_format', '-f', type=str,
                        choices=('S16_LE', 'S32_LE'), default='S32_LE')
    parser.add_argument('--dma_buffer_size_ms', '-b', type=int, default=100)
    parser.add_argument('--num_dma_buffers', '-n', type=int, default=10)
    args = parser.parse_args()

    sample_format = {
        'S16_LE': {'id': 0, 'bytes': 2, 'pyaudio': pyaudio.paInt16},
        'S32_LE': {'id': 1, 'bytes': 4, 'pyaudio': pyaudio.paInt32},
    }[args.sample_format]

    sock = socket.socket()
    sock.connect((args.host, args.port))
    sock.send(struct.pack('<iiii', args.sample_rate_hz,
                                   sample_format['id'],
                                   args.dma_buffer_size_ms,
                                   args.num_dma_buffers))

    audio = pyaudio.PyAudio()

    stream = audio.open(format=sample_format['pyaudio'],
                        channels=1,
                        rate=args.sample_rate_hz,
                        output=True)

    samples_per_buffer = args.dma_buffer_size_ms * (args.sample_rate_hz // 1000)
    bytes_per_buffer = sample_format['bytes'] * samples_per_buffer
    while True:
        samples = sock.recv(bytes_per_buffer // 2)
        if not samples:
            break
        stream.write(samples)

if __name__ == '__main__':
   main()
