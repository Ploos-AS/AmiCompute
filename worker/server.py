"""AmiCompute M0 TCP worker: bounded single-request image operation."""
import argparse
import socketserver
import struct

MAX_PAYLOAD = 8 * 1024 * 1024


def invert_pgm(data: bytes) -> bytes:
    # Minimal deterministic P5 subset: fixed header and 8-bit pixels.
    if not data.startswith(b"P5\n"):
        raise ValueError("Expected binary PGM P5")
    pos = 3
    dims_end = data.find(b"\n", pos, pos + 64)
    if dims_end < 0:
        raise ValueError("Missing dimensions")
    dims = data[pos:dims_end].split()
    if len(dims) != 2 or any(not token.isdigit() for token in dims):
        raise ValueError("Invalid dimensions")
    width, height = map(int, dims)
    if not (1 <= width <= 4096 and 1 <= height <= 4096):
        raise ValueError("Dimensions out of range")
    if data[dims_end + 1:dims_end + 5] != b"255\n":
        raise ValueError("Only 8-bit maxval 255 supported")
    header_end = dims_end + 5
    if len(data) - header_end != width * height:
        raise ValueError("Pixel length mismatch")
    return data[:header_end] + bytes(255 - pixel for pixel in data[header_end:])


class Handler(socketserver.BaseRequestHandler):
    def handle(self):
        self.request.settimeout(10)
        try:
            header = recv_exact(self.request, 9)
            if header[:4] != b"ACM0":
                raise ValueError("Bad magic")
            operation = header[4]
            length = struct.unpack(">I", header[5:])[0]
            if length > MAX_PAYLOAD:
                raise ValueError("Payload too large")
            payload = recv_exact(self.request, length)
            if operation != 1:
                raise ValueError("Unsupported operation")
            result = invert_pgm(payload)
            send_response(self.request, 0, result)
        except (ValueError, EOFError) as error:
            send_response(self.request, 1, str(error).encode("utf-8"))
        except (OSError, TimeoutError):
            return
        except Exception:
            send_response(self.request, 2, b"Internal worker error")


def recv_exact(sock, length):
    chunks = []
    while length:
        chunk = sock.recv(min(length, 65536))
        if not chunk:
            raise EOFError("Truncated request")
        chunks.append(chunk)
        length -= len(chunk)
    return b"".join(chunks)


def send_response(sock, status, data):
    sock.sendall(b"ACR0" + bytes([status]) + struct.pack(">I", len(data)) + data)


class Server(socketserver.ThreadingTCPServer):
    allow_reuse_address = True
    daemon_threads = True


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=6050)
    args = parser.parse_args()
    with Server((args.host, args.port), Handler) as server:
        print(f"AmiCompute M0 listening on {args.host}:{args.port}", flush=True)
        server.serve_forever()


if __name__ == "__main__":
    main()
