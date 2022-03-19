import argparse
import socket
import struct
import wave

def copy_data(sock, write, chunk_size=2048):
    num_bytes = 0
    try:
        while True:
            samples = sock.recv(chunk_size)
            if not samples:
                break
            write(samples)
            num_bytes += len(samples)
    except KeyboardInterrupt:
        pass
    return num_bytes

def main():
    parser = argparse.ArgumentParser(
        description='Save audio from Valiant mic to file')
    parser.add_argument('--host', type=str, default='10.10.10.1')
    parser.add_argument('--port', '-p', type=int, default=33000)
    parser.add_argument('--output', '-o', type=str, required=True)
    parser.add_argument('--sample_rate_hz', '-r', type=int,
                        choices=(16000, 48000), default=16000)
    parser.add_argument('--dma_buffer_size_ms', '-b', type=int, default=50)
    parser.add_argument('--num_dma_buffers', '-n', type=int, default=6)
    parser.add_argument('--raw', action='store_true')
    args = parser.parse_args()

    sock = socket.socket()
    sock.connect((args.host, args.port))
    sock.send(struct.pack('<iii', args.sample_rate_hz,
                                  args.dma_buffer_size_ms,
                                  args.num_dma_buffers))

    if args.raw:
        with open(args.output, 'wb') as f:
            num_bytes = copy_data(sock, f.write)
    else:
        with wave.open(args.output, 'wb') as f:
            f.setnchannels(1)
            f.setsampwidth(4)
            f.setframerate(args.sample_rate_hz)
            num_bytes = copy_data(sock, f.writeframes)

    print('File:', args.output)
    print('Format:', 'RAW' if args.raw else 'WAV')
    print('Sample Rate:', args.sample_rate_hz)
    print('Sample Count:', num_bytes / 4)
    print('Duration (ms):', 1000.0 * num_bytes / (4 * args.sample_rate_hz))

if __name__ == '__main__':
   main()
