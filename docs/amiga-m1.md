# Native Amiga client: M1 prototype

The native client is in `client/amiga/amicompute_amiga.c`. It uses `bsdsocket.library` v4+ and a numeric IPv4 address. Unlike the host POSIX client, this is written against the AmigaOS socket ABI.

## Build (requires actual Amiga SDK)
Use an m68k AmigaOS-targeting compiler (vbcc or Bebbo GCC) and the matching NDK/bsdsocket headers and link libraries. An example **illustrative** invocation for a configured Bebbo GCC toolchain:

```sh
m68k-amigaos-gcc -O2 -Wall -o AmiCompute client/amiga/amicompute_amiga.c
```

Toolchain naming and include/library paths vary by installation. This command is not exercised by the Linux CI job. Verify on AmigaOS 2.x/3.x and a TCP/IP stack such as Roadshow or Miami before marking M1 complete.

## Test
1. Start worker on a trusted LAN IP: `python3 -m worker.server --host 192.168.1.10`.
2. Generate `examples/sample.pgm` on the host and transfer it to the Amiga.
3. On the Amiga run: `AmiCompute 192.168.1.10 6050 sample.pgm result.pgm`.
4. Compare output against an inverted input; for the sample, pixel bytes should be FF 80 7F 00.

## Limits
- No TLS/authentication: private LAN only; restrict network access.
- One synchronous job per connection, no progress, cancellation or ARexx yet.
- Only P5 PGM without comments, 8-bit maximum 255.
- No real Amiga compilation or physical hardware test has been performed yet.
