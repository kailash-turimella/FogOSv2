// user/compress.c — Run-Length Encoding (RLE) compressor/decompressor for FogOSv2
// Format: stream of pairs [count:1 byte][value:1 byte], repeated to EOF.
// Runs >255 are split into multiple pairs.

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define INBUF  512
#define OUTBUF 128

static void usage(void) {
  printf(
    "usage:\n"
    "  compress -c [in] [out]   # compress\n"
    "  compress -d [in] [out]   # decompress\n"
    "If in/out omitted, stdin/stdout are used.\n"
  );
}

static int open_in(const char *p) {
  if (!p) return 0; // stdin
  int fd = open(p, O_RDONLY);
  if (fd < 0) {
    printf("compress: cannot open %s\n", p);
  }
  return fd;
}

static int open_out(const char *p) {
  if (!p) return 1; // stdout
  int fd = open(p, O_CREATE | O_WRONLY);
  if (fd < 0) {
    printf("compress: cannot create %s\n", p);
  }
  return fd;
}

/* -------------------- RLE compress -------------------- */
static int rle_compress_fd(int in, int out) {
  uchar buf[INBUF];
  int have_prev = 0;
  uchar prev = 0;
  int cnt = 0;

  for (;;) {
    int n = read(in, buf, sizeof(buf));
    if (n < 0) { printf("compress: read error\n"); return -1; }
    if (n == 0) break;

    for (int i = 0; i < n; i++) {
      uchar b = buf[i];
      if (!have_prev) {
        prev = b; cnt = 1; have_prev = 1;
      } else if (b == prev && cnt < 255) {
        cnt++;
      } else {
        uchar pair[2] = { (uchar)cnt, prev };
        if (write(out, pair, 2) != 2) { printf("compress: write error\n"); return -1; }
        prev = b; cnt = 1;
      }
    }
  }

  if (have_prev) {
    uchar pair[2] = { (uchar)cnt, prev };
    if (write(out, pair, 2) != 2) { printf("compress: write error\n"); return -1; }
  }
  return 0;
}

/* ------------------ RLE decompress -------------------- */
static int rle_decompress_fd(int in, int out) {
  uchar buf[INBUF];
  int carry = -1;      // -1 = no carry; otherwise holds a pending count byte
  uchar outbuf[OUTBUF];

  for (int k = 0; k < OUTBUF; k++) outbuf[k] = 0; // init once

  for (;;) {
    int n = read(in, buf, sizeof(buf));
    if (n < 0) { printf("compress: read error\n"); return -1; }
    if (n == 0) break;

    int i = 0;

    // If we had an odd byte (count) from the previous read, pair it now.
    if (carry != -1) {
      if (n < 1) { printf("compress: corrupt input (missing value)\n"); return -1; }
      int cnt = carry;
      uchar val = buf[0];
      if (cnt <= 0) { printf("compress: corrupt input (zero/neg count)\n"); return -1; }

      // emit cnt copies of val
      for (int j = 0; j < OUTBUF; j++) outbuf[j] = val;
      int left = cnt;
      while (left > 0) {
        int chunk = left < OUTBUF ? left : OUTBUF;
        if (write(out, outbuf, chunk) != chunk) { printf("compress: write error\n"); return -1; }
        left -= chunk;
      }
      i = 1;       // consumed one byte (value)
      carry = -1;  // cleared
    }

    // Process full pairs from this buffer
    for (; i + 1 < n; i += 2) {
      int cnt = buf[i];
      uchar val = buf[i+1];
      if (cnt <= 0) { printf("compress: corrupt input (zero/neg count)\n"); return -1; }

      for (int j = 0; j < OUTBUF; j++) outbuf[j] = val;
      int left = cnt;
      while (left > 0) {
        int chunk = left < OUTBUF ? left : OUTBUF;
        if (write(out, outbuf, chunk) != chunk) { printf("compress: write error\n"); return -1; }
        left -= chunk;
      }
    }

    // If there's one byte left, it's a dangling count; carry it to next read
    if (i < n) {
      carry = buf[n-1];
    }
  }

  if (carry != -1) { // ended with odd number of bytes → corrupt
    printf("compress: corrupt input (odd byte count)\n");
    return -1;
  }
  return 0;
}

/* ------------------------ main ------------------------ */
int main(int argc, char *argv[]) {
  if (argc < 2) { usage(); exit(1); }

  int mode = 0;             // 1 = compress, 2 = decompress
  const char *inpath = 0;   // NULL -> stdin
  const char *outpath = 0;  // NULL -> stdout

  // Parse: first flag (-c/-d), then up to 2 positional args
  for (int i = 1; i < argc; i++) {
    char *a = argv[i];
    if (strcmp(a, "-c") == 0) {
      mode = 1;
    } else if (strcmp(a, "-d") == 0) {
      mode = 2;
    } else if (!inpath) {
      inpath = a;
    } else if (!outpath) {
      outpath = a;
    } else {
      printf("compress: too many arguments\n");
      usage();
      exit(1);
    }
  }

  if (mode == 0) {
    printf("compress: must specify -c or -d\n");
    usage();
    exit(1);
  }

  int in = open_in(inpath);
  if (in < 0) exit(1);
  int out = open_out(outpath);
  if (out < 0) { if (in > 1) close(in); exit(1); }

  int rc = (mode == 1) ? rle_compress_fd(in, out) : rle_decompress_fd(in, out);

  if (in > 1) close(in);
  if (out > 1) close(out);
  if (rc < 0) exit(1);
  exit(0);
}
