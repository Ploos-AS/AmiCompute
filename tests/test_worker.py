import socket
import struct
import threading
import unittest
from worker.server import Server, Handler, invert_pgm


class WorkerTests(unittest.TestCase):
    def test_pixels(self):
        self.assertEqual(invert_pgm(b"P5\n2 1\n255\n\x00\xff"),
                         b"P5\n2 1\n255\n\xff\x00")

    def test_bad_image(self):
        with self.assertRaises(ValueError):
            invert_pgm(b"garbage")

    def test_roundtrip(self):
        server = Server(("127.0.0.1", 0), Handler)
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        try:
            payload = b"P5\n2 1\n255\n\x00\x7f"
            with socket.create_connection(server.server_address, timeout=2) as s:
                s.sendall(b"ACM0\x01" + struct.pack(">I", len(payload)) + payload)
                header = read_exact(s, 9)
                body = read_exact(s, struct.unpack(">I", header[5:])[0])
            self.assertEqual(header[:5], b"ACR0\x00")
            self.assertEqual(body, b"P5\n2 1\n255\n\xff\x80")
        finally:
            server.shutdown()
            server.server_close()
            thread.join(timeout=2)


def read_exact(sock, size):
    result = b""
    while len(result) < size:
        chunk = sock.recv(size - len(result))
        if not chunk:
            raise EOFError()
        result += chunk
    return result


if __name__ == "__main__":
    unittest.main()
