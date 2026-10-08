# AmiCompute

Native Amiga front end, modern compute back end.

AmiCompute M0 implements a small, inspectable TCP job protocol and a Linux worker (ARM64/x86-64) with a native C client intended for AmigaOS/m68k. Jobs run on a trusted LAN; no arbitrary shell execution.

## M0 demo
The first operation is `invert`: send a binary PGM (P5) grayscale image, invert pixels on the worker, and retrieve the result. The program running on the Amiga remains a native Amiga executable.

### Worker
```sh
python3 -m worker.server --host 127.0.0.1 --port 6050
python3 -m unittest discover -s tests -v
```
Python 3.10+ standard library only. For another machine on your trusted LAN, bind the worker to its LAN IP and open port 6050 only to the Amiga.

### Client
```sh
cc -std=c99 -Wall -Wextra -O2 client/amicompute.c -o amicompute
./amicompute 127.0.0.1 6050 examples/sample.pgm result.pgm
```
Native Amiga compilation requires an Amiga-targeting C compiler and compatible BSD-socket SDK; the portable POSIX build is a host-side smoke test. Amiga-specific socket initialization remains an M1 deliverable.

## Protocol v0 (experimental)
All integers are unsigned big-endian. Request: ASCII `ACM0`, one-byte operation (1=invert P5), 4-byte payload length, raw payload (max 8 MiB). Response: ASCII `ACR0`, one-byte status (0=success; 1=invalid request; 2=internal error), 4-byte length, result or UTF-8 error message. Exactly one request/response per TCP connection. No authentication or TLS in M0; **never expose the worker to the Internet**.

## Roadmap
- **M0:** protocol, Linux worker, portable C demo client, deterministic image processing and tests.
- **M1:** AmigaOS socket initialization/cleanup, cross-toolchain CI, native m68k smoke test.
- **M2:** async jobs, IDs, polling, cancellation, ARexx bridge.
- **M3:** AmiRender/AmiTerrain/Ami3D adapters, scheduling and heterogeneous nodes.
- **M4:** secured remote access, GPU plugins and job isolation.

Software license: MIT. Copyright Ploos AS.
