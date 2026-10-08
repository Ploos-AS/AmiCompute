"""Dependency-free static checks; NOT a substitute for m68k compilation."""
from pathlib import Path

source = Path("client/amiga/amicompute_amiga.c").read_text()
required = [
    'OpenLibrary("bsdsocket.library", 4)',
    'CloseLibrary(SocketBase)',
    'CloseSocket(fd)',
    'connect(fd,',
    'send(fd,',
    'recv(fd,',
    'MAX_PAYLOAD',
]
missing = [part for part in required if part not in source]
if missing:
    raise SystemExit("Missing native socket integration: " + ", ".join(missing))
print("Amiga source static checks passed (cross-compilation still required)")
