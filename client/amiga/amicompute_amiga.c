/* AmiCompute native AmigaOS m68k client (M1 prototype).
 * Requires an AmigaOS SDK with bsdsocket.library headers and a TCP/IP stack.
 * This is intentionally separate from the POSIX smoke-test client.
 */
#include <exec/types.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <proto/bsdsocket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Library *SocketBase = NULL;
#define MAX_PAYLOAD (8UL * 1024UL * 1024UL)

static int xfer(LONG fd, UBYTE *buf, ULONG n, int sending)
{
    while (n) {
        LONG count = n > 32768UL ? 32768 : (LONG)n;
        LONG k = sending ? send(fd, (char *)buf, count, 0)
                         : recv(fd, (char *)buf, count, 0);
        if (k <= 0) return -1;
        buf += k;
        n -= (ULONG)k;
    }
    return 0;
}
static ULONG read32(const UBYTE *p)
{
    return ((ULONG)p[0] << 24) | ((ULONG)p[1] << 16) |
           ((ULONG)p[2] << 8) | (ULONG)p[3];
}
static void write32(UBYTE *p, ULONG n)
{
    p[0] = (UBYTE)(n >> 24); p[1] = (UBYTE)(n >> 16);
    p[2] = (UBYTE)(n >> 8); p[3] = (UBYTE)n;
}
int main(int argc, char **argv)
{
    FILE *input = NULL, *output = NULL;
    UBYTE *buf = NULL;
    UBYTE hdr[9] = {'A','C','M','0',1,0,0,0,0};
    LONG fd = -1, size;
    ULONG len;
    struct sockaddr_in addr;
    long port;
    char *end;
    int rc = 1;

    if (argc != 5) {
        fprintf(stderr, "Usage: AmiCompute HOST_IPV4 PORT INPUT.pgm OUTPUT.pgm\n");
        return 2;
    }
    port = strtol(argv[2], &end, 10);
    if (*end || port < 1 || port > 65535) return 2;
    input = fopen(argv[3], "rb");
    if (!input) { perror("input"); goto cleanup; }
    if (fseek(input, 0, SEEK_END)) goto cleanup;
    size = ftell(input);
    if (size < 0 || (ULONG)size > MAX_PAYLOAD || fseek(input, 0, SEEK_SET))
        goto cleanup;
    buf = malloc((ULONG)size + 1);
    if (!buf || fread(buf, 1, (ULONG)size, input) != (ULONG)size)
        goto cleanup;
    fclose(input); input = NULL;

    SocketBase = OpenLibrary("bsdsocket.library", 4);
    if (!SocketBase) {
        fprintf(stderr, "bsdsocket.library v4+ unavailable\n");
        goto cleanup;
    }
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) goto cleanup;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((UWORD)port);
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    if (addr.sin_addr.s_addr == INADDR_NONE) {
        fprintf(stderr, "Use numeric IPv4 address\n");
        goto cleanup;
    }
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        goto cleanup;
    write32(hdr + 5, (ULONG)size);
    if (xfer(fd, hdr, 9, 1) || xfer(fd, buf, (ULONG)size, 1) ||
        xfer(fd, hdr, 9, 0)) goto cleanup;
    free(buf); buf = NULL;
    if (memcmp(hdr, "ACR0", 4) != 0) goto cleanup;
    len = read32(hdr + 5);
    if (len > MAX_PAYLOAD) goto cleanup;
    buf = malloc(len + 1);
    if (!buf || xfer(fd, buf, len, 0)) goto cleanup;
    if (hdr[4] != 0) {
        buf[len] = 0;
        fprintf(stderr, "Worker: %s\n", (char *)buf);
        goto cleanup;
    }
    output = fopen(argv[4], "wb");
    if (!output) goto cleanup;
    if (fwrite(buf, 1, len, output) != len) goto cleanup;
    rc = 0;
cleanup:
    if (output && fclose(output)) rc = 1;
    if (input) fclose(input);
    if (fd >= 0) CloseSocket(fd);
    if (SocketBase) CloseLibrary(SocketBase);
    free(buf);
    if (rc) fprintf(stderr, "AmiCompute failed\n");
    return rc;
}
