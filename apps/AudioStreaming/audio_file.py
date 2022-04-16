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
    parser.add_argument('--port', type=int, default=33000)
    parser.add_argument('--sample_rate_hz', '-r', type=int,
                        choices=(16000, 48000), default=48000)
    parser.add_argument('--sample_format', '-f', type=str,
                        choices=('S16_LE', 'S32_LE'), default='S32_LE')
    parser.add_argument('--dma_buffer_size_ms', '-b', type=int, default=100)
    parser.add_argument('--num_dma_buffers', '-n', type=int, default=10)
    parser.add_argument('--output', '-o', type=str, required=True)
    parser.add_argument('--raw', action='store_true')
    parser.add_argument('--drop_first_samples_ms', type=int, default=0)

    args = parser.parse_args()

    sample_format = {
        'S16_LE': {'id': 0, 'bytes': 2},
        'S32_LE': {'id': 1, 'bytes': 4},
    }[args.sample_format]

    sock = socket.socket()
    sock.connect((args.host, args.port))
    sock.send(struct.pack('<iiiii', args.sample_rate_hz,
                          sample_format['id'],
                          args.dma_buffer_size_ms,
                          args.num_dma_buffers,
                          args.drop_first_samples_ms))

    if args.raw:
        with open(args.output, 'wb') as f:
            num_bytes = copy_data(sock, f.write)
    else:
        with wave.open(args.output, 'wb') as f:
            f.setnchannels(1)
            f.setsampwidth(sample_format['bytes'])
            f.setframerate(args.sample_rate_hz)
            num_bytes = copy_data(sock, f.writeframes)

    num_samples = num_bytes / sample_format['bytes']
    print('File [%s]: %s' % ('RAW' if args.raw else 'WAV', args.output))
    print('Sample Rate:', args.sample_rate_hz)
    print('Sample Format:', args.sample_format)
    print('Sample Count:', num_samples)
    print('Duration (ms):', 1000.0 * num_samples / args.sample_rate_hz)


if __name__ == '__main__':
    main()
