from pathlib import Path

Path('examples/sample.pgm').write_bytes(b'P5\n2 2\n255\n' + bytes([0, 127, 128, 255]))
print('Wrote examples/sample.pgm')
