import socket
import struct
import threading
import unittest

from worker.server import Server, Handler


def read_exact(sock, n):
    data = b""
    while len(data) < n:
        chunk = sock.recv(n - len(data))
        if not chunk:
            raise EOFError("short response")
        data += chunk
    return data


class ProtocolErrorTests(unittest.TestCase):
    def setUp(self):
        self.server = Server(("127.0.0.1", 0), Handler)
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()

    def tearDown(self):
        self.server.shutdown()
        self.server.server_close()
        self.thread.join(timeout=2)

    def request(self, magic, operation, payload, claimed_length=None):
        with socket.create_connection(self.server.server_address, timeout=2) as sock:
            sock.settimeout(2)
            sock.sendall(magic + bytes([operation]) +
                         struct.pack(">I", len(payload) if claimed_length is None else claimed_length) +
                         payload)
            header = read_exact(sock, 9)
            body = read_exact(sock, struct.unpack(">I", header[5:])[0])
        return header, body

    def test_invalid_magic(self):
        hdr, msg = self.request(b"BAD!", 1, b"")
        self.assertEqual(hdr[:5], b"ACR0\x01")
        self.assertIn(b"magic", msg)

    def test_unsupported_operation(self):
        hdr, msg = self.request(b"ACM0", 99, b"")
        self.assertEqual(hdr[:5], b"ACR0\x01")
        self.assertIn(b"Unsupported", msg)

    def test_oversized_payload_rejected_before_read(self):
        hdr, msg = self.request(b"ACM0", 1, b"", 8 * 1024 * 1024 + 1)
        self.assertEqual(hdr[:5], b"ACR0\x01")
        self.assertIn(b"large", msg)

    def test_malformed_pgm(self):
        hdr, msg = self.request(b"ACM0", 1, b"P5\n1 1\n255\n")
        self.assertEqual(hdr[:5], b"ACR0\x01")
        self.assertIn(b"length", msg)


if __name__ == "__main__":
    unittest.main()
