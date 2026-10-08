/* AmiCompute M0 host-side POSIX demo client; native Amiga socket port in M1. */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_PAYLOAD (8u * 1024u * 1024u)

static int transfer(int fd, unsigned char *p, size_t n, int sending) {
    while (n) {
        ssize_t k = sending ? send(fd, p, n, 0) : recv(fd, p, n, 0);
        if (k <= 0) return -1;
        p += k; n -= (size_t)k;
    }
    return 0;
}
static uint32_t read_be(const unsigned char *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | p[3];
}
static void write_be(unsigned char *p, uint32_t n) {
    p[0] = n >> 24; p[1] = n >> 16; p[2] = n >> 8; p[3] = n;
}
int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s HOST PORT INPUT.pgm OUTPUT.pgm\n", argv[0]);
        return 2;
    }
    char *end;
    long port = strtol(argv[2], &end, 10);
    if (*end || port < 1 || port > 65535) return 2;
    FILE *in = fopen(argv[3], "rb");
    if (!in) { perror("input"); return 1; }
    if (fseek(in, 0, SEEK_END) || ftell(in) < 0) { fclose(in); return 1; }
    long size = ftell(in);
    if (size > MAX_PAYLOAD || fseek(in, 0, SEEK_SET)) { fclose(in); return 1; }
    unsigned char *data = malloc((size_t)size + 1);
    if (!data) { fclose(in); return 1; }
    if (fread(data, 1, (size_t)size, in) != (size_t)size) {
        fclose(in); free(data); return 1;
    }
    fclose(in);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); free(data); return 1; }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; addr.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, argv[1], &addr.sin_addr) != 1 ||
        connect(fd, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("connect"); close(fd); free(data); return 1;
    }
    unsigned char header[9] = {'A','C','M','0',1,0,0,0,0};
    write_be(header + 5, (uint32_t)size);
    if (transfer(fd, header, 9, 1) || transfer(fd, data, (size_t)size, 1) ||
        transfer(fd, header, 9, 0)) {
        fprintf(stderr, "Transfer failure\n"); close(fd); free(data); return 1;
    }
    free(data);
    uint32_t length = read_be(header + 5);
    if (memcmp(header, "ACR0", 4) || length > MAX_PAYLOAD) {
        fprintf(stderr, "Invalid response\n"); close(fd); return 1;
    }
    data = malloc((size_t)length + 1);
    if (!data || transfer(fd, data, length, 0)) {
        fprintf(stderr, "Response read failed\n"); free(data); close(fd); return 1;
    }
    close(fd);
    if (header[4]) {
        data[length] = 0; fprintf(stderr, "Worker: %s\n", data); free(data); return 1;
    }
    FILE *out = fopen(argv[4], "wb");
    if (!out) { perror("output"); free(data); return 1; }
    int ok = fwrite(data, 1, length, out) == length && fclose(out) == 0;
    free(data);
    return ok ? 0 : 1;
}
