# M1 acceptance criteria

M1 is not complete until all of the following are demonstrated:

- [ ] m68k AmigaOS binary produced with a pinned, reproducible toolchain
- [ ] Binary runs on AmigaOS 2.x and 3.x (emulator first)
- [ ] bsdsocket.library opens and closes cleanly with a supported TCP/IP stack
- [ ] Valid PGM request produces byte-exact inverted output from an ARM worker
- [ ] Bad IP, missing worker, truncated response and malformed PGM handled without crashing
- [ ] Physical Amiga-to-SBC test, with reproducible instructions and captured evidence
- [ ] ARexx command interface added and tested (or explicitly moved to M2)

## Suggested test matrix

| Client | Network | Worker | State |
|---|---|---|---|
| Linux POSIX C | loopback | Linux x86-64 | automated CI |
| AmigaOS 3.x m68k emulator | bsdsocket | Linux x86-64 | pending |
| AmigaOS 2.x m68k emulator | bsdsocket | ARM SBC | pending |
| Physical Amiga | Ethernet/TCP-IP | Orange Pi | pending |

Static source checks alone do not validate Amiga compilation or runtime behavior.
