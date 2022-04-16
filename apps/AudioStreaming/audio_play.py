import argparse
import collections
import contextlib
import socket
import struct
import threading
import wave

# Third party libraries
import pyaudio  # sudo apt-get install python3-pyaudio


class Overflow(Exception):
    """Exception raised when ring buffer does not have enough space to write."""
    pass


class Underflow(Exception):
    """Exception raised when ring buffer does not have enough data to read."""
    pass


class RingBuffer:
    """Simple ring buffer implementation.
    https://en.wikipedia.org/wiki/Circular_buffer
    """

    def __init__(self, buf):
        self._buf = buf
        self._r = 0
        self._size = 0

    def __len__(self):
        return len(self._buf)

    def __str__(self):
        return str(self._buf)

    @property
    def read_size(self):
        return self._size

    @property
    def write_size(self):
        return len(self) - self.read_size

    def read_only(self, buf):
        size = len(buf)
        if size == 0:
            return
        if size > self.read_size:
            raise Underflow
        f = self._r
        l = (f + size) % len(self)
        if f < l:
            buf[:] = self._buf[f:l]
        else:
            n = len(self) - f
            buf[:n] = self._buf[f:]
            buf[n:] = self._buf[:l]

    def remove_only(self, size):
        if size < 0:
            raise ValueError("'size' must be a non-negative number")
        if size > self.read_size:
            raise Underflow
        self._r = (self._r + size) % len(self)
        self._size -= size

    def read(self, buf):
        self.read_only(buf)
        self.remove_only(len(buf))

    def write(self, buf):
        size = len(buf)
        if size == 0:
            return
        if size > self.write_size:
            raise Overflow
        f = (self._r + self._size) % len(self)
        l = (f + size) % len(self)
        if f < l:
            self._buf[f:l] = buf
        else:
            n = len(self) - f
            self._buf[f:] = buf[:n]
            self._buf[:l] = buf[n:]
        self._size += size


class ConcurrentRingBuffer:
    """Blocking ring buffer for concurrent access from multiple threads."""

    def __init__(self, buf):
        self._rb = RingBuffer(buf)
        self._lock = threading.Lock()
        self._overflow = threading.Condition(self._lock)
        self._underflow = threading.Condition(self._lock)

    def __str__(self):
        return str(self._rb)

    def write(self, buf, block=True, timeout=None):
        if len(buf) > len(self._rb):
            raise ValueError("'buf' is too big")

        with self._lock:
            if block and not self._overflow.wait_for(
                    lambda: len(buf) <= self._rb.write_size, timeout):
                raise Overflow
            self._rb.write(buf)
            self._underflow.notify()

    def read(self, buf, remove_size=None, block=True, timeout=None):
        if len(buf) > len(self._rb):
            raise ValueError("'buf' is too big")

        if remove_size is not None:
            if remove_size < 0:
                raise ValueError("'remove_size' must be non-negative")
            if remove_size > len(buf):
                raise ValueError("'remove_size' must not exceed 'len(buf)'")

        with self._lock:
            if block and not self._underflow.wait_for(
                    lambda: len(buf) <= self._rb.read_size, timeout):
                raise Underflow
            self._rb.read_only(buf)
            self._rb.remove_only(
                len(buf) if remove_size is None else remove_size)
            self._overflow.notify()


@contextlib.contextmanager
def PyAudioStream(**kwargs):
    audio = pyaudio.PyAudio()
    try:
        stream = audio.open(**kwargs)
        if not kwargs.get('start', True):
            stream.start_stream()
        try:
            yield stream
        finally:
            stream.stop_stream()
            stream.close()
    finally:
        audio.terminate()


@contextlib.contextmanager
def CallbackMonoPlayer(sample_format, sample_rate_hz, frames_per_callback_ms):
    frames_per_buffer = (sample_rate_hz // 1000) * frames_per_callback_ms
    rb = ConcurrentRingBuffer(bytearray(10 * frames_per_buffer))
    buf = bytearray(frames_per_buffer * sample_format.bytes)

    def callback(in_data, frame_count, time_info, status):
        assert(frame_count == frames_per_buffer)
        rb.read(buf)
        return bytes(buf), pyaudio.paContinue

    with PyAudioStream(format=sample_format.pyaudio,
                       frames_per_buffer=frames_per_buffer,
                       channels=1,
                       rate=sample_rate_hz,
                       stream_callback=callback,
                       start=False,
                       output=True) as stream:
        try:
            yield rb.write
        finally:
            rb.write(bytes(len(buf)))  # push zeros to unblock the callback


@contextlib.contextmanager
def BlockingMonoPlayer(sample_format, sample_rate_hz, frames_per_callback_ms):
    with PyAudioStream(format=sample_format.pyaudio,
                       channels=1,
                       rate=sample_rate_hz,
                       output=True) as stream:
        yield stream.write


@contextlib.contextmanager
def WaveFileWriter(filename, sample_format, sample_rate_hz):
    with wave.open(filename, 'wb') as f:
        f.setnchannels(1)
        f.setsampwidth(sample_format.bytes)
        f.setframerate(sample_rate_hz)
        yield f.writeframes


@contextlib.contextmanager
def Null(*args, **kwargs):
    yield lambda *largs: None


SampleFormat = collections.namedtuple(
    'SampleFormat', ['name', 'id', 'bytes', 'pyaudio'])
SAMPLE_FORMATS = {f.name: f for f in
    [SampleFormat(name='S16_LE', id=0, bytes=2, pyaudio=pyaudio.paInt16),
     SampleFormat(name='S32_LE', id=1, bytes=4, pyaudio=pyaudio.paInt32)]
}

PLAYERS = {
    'blocking': BlockingMonoPlayer,
    'callback': CallbackMonoPlayer,
    'no': Null
}


def main():
    parser = argparse.ArgumentParser(description='Play audio from Valiant mic')
    parser.add_argument('--host', type=str, default='10.10.10.1')
    parser.add_argument('--port', type=int, default=33000)
    parser.add_argument('--sample_rate_hz', '-r', type=int,
                        choices=(16000, 48000), default=48000)
    parser.add_argument('--sample_format', '-f', type=SAMPLE_FORMATS.get,
                        choices=SAMPLE_FORMATS.values(),
                        metavar='{%s}' % ','.join(SAMPLE_FORMATS.keys()),
                        default='S32_LE')
    parser.add_argument('--dma_buffer_size_ms', '-b', type=int, default=100,
                        metavar='MS')
    parser.add_argument('--num_dma_buffers', '-n', type=int, default=10,
                        metavar='NUM')
    parser.add_argument('--drop_first_samples_ms', type=int, default=0,
                        metavar='MS')
    parser.add_argument('--player',  type=str,
                        choices=PLAYERS.keys(),
                        default='callback')
    parser.add_argument('--output', '-o', type=str, default=None,
                        metavar='FILENAME')
    args = parser.parse_args()

    sock = socket.socket()
    sock.connect((args.host, args.port))
    sock.send(struct.pack('<iiiii', args.sample_rate_hz,
                          args.sample_format.id,
                          args.dma_buffer_size_ms,
                          args.num_dma_buffers,
                          args.drop_first_samples_ms))

    Player = PLAYERS[args.player]
    FileWriter = WaveFileWriter if args.output else Null
    with FileWriter(filename=args.output,
                    sample_format=args.sample_format,
                    sample_rate_hz=args.sample_rate_hz) as write, \
        Player(sample_format=args.sample_format,
               sample_rate_hz=args.sample_rate_hz,
               frames_per_callback_ms=max(args.dma_buffer_size_ms, 50)) as play:

        while True:
            samples = sock.recv(4096)
            if not samples:
                break
            play(samples)
            write(samples)


if __name__ == '__main__':
    main()
