# Run with a configured m68k-amigaos-gcc toolchain and Amiga NDK.
CC ?= m68k-amigaos-gcc
CFLAGS ?= -O2 -Wall -Wextra
TARGET ?= AmiCompute
SRC = amicompute_amiga.c

.PHONY: all clean
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
