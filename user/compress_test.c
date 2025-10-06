// It creates small input files, runs `compress -c` then `compress -d` via
// fork/exec, and verifies the round-trip output matches the original.

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define IN_CHUNK 512

static int write_file(const char *path, const void *buf, int len) {
  int fd = open(path, O_CREATE | O_WRONLY);
  if (fd < 0) { printf("compress_test: open(O_CREATE) %s failed\n", path); return -1; }
  const char *p = (const char*)buf;
  int left = len;
  while (left > 0) {
    int n = write(fd, p, left);
    if (n < 0) { printf("compress_test: write %s failed\n", path); close(fd); return -1; }
    left -= n; p += n;
  }
  close(fd);
  return 0;
}

static int files_equal(const char *a, const char *b) {
  int fa = open(a, O_RDONLY);
  int fb = open(b, O_RDONLY);
  if (fa < 0 || fb < 0) { if (fa>=0) close(fa); if (fb>=0) close(fb); return 0; }

  char ba[IN_CHUNK], bb[IN_CHUNK];
  for (;;) {
    int na = read(fa, ba, sizeof(ba));
    int nb = read(fb, bb, sizeof(bb));
    if (na < 0 || nb < 0) { close(fa); close(fb); return 0; }
    if (na != nb) { close(fa); close(fb); return 0; }
    if (na == 0) { close(fa); close(fb); return 1; } // both EOF
    for (int i = 0; i < na; i++) if (ba[i] != bb[i]) { close(fa); close(fb); return 0; }
  }
}

static int run_prog(char *prog, char *argv[]) {
  int pid = fork();
  if (pid < 0) { printf("compress_test: fork failed\n"); return -1; }
  if (pid == 0) {
    // child
    exec(prog, argv);
    printf("compress_test: exec %s failed\n", prog);
    exit(127);
  }
  int status = 0;
  if (wait(&status) < 0) {
    // Some FogOS variants support wait(&status); others allow wait(0).
    // If your toolchain complains, change to: wait(0);
  }
  return status;
}

static int do_roundtrip(const char *label, const char *infile,
                        const char *rlefile, const char *outfile)
{
  // compress -c infile rlefile
  char *argv_c[] = { "compress", "-c", (char*)infile, (char*)rlefile, 0 };
  int st1 = run_prog("compress", argv_c);
  if (st1 != 0) { printf("[FAIL] %s: compress step failed (status=%d)\n", label, st1); return -1; }

  // decompress -d rlefile outfile
  char *argv_d[] = { "compress", "-d", (char*)rlefile, (char*)outfile, 0 };
  int st2 = run_prog("compress", argv_d);
  if (st2 != 0) { printf("[FAIL] %s: decompress step failed (status=%d)\n", label, st2); return -1; }

  // compare
  if (!files_equal(infile, outfile)) {
    printf("[FAIL] %s: round-trip mismatch\n", label);
    return -1;
  }

  printf("[PASS] %s\n", label);
  return 0;
}

static void make_repeating(char *buf, int len, char byte) {
  for (int i = 0; i < len; i++) buf[i] = byte;
}

int
main(void)
{
  int rc = 0;

  // --- Test 1: small text ---
  const char *t1_in  = "t1_in.txt";
  const char *t1_rle = "t1.rle";
  const char *t1_out = "t1_out.txt";
  const char *msg = "Hello RLE!\nHello RLE!\nAAABBBCCCDDD\n";
  if (write_file(t1_in, msg, strlen(msg)) < 0) rc = -1;
  if (rc == 0 && do_roundtrip("small text", t1_in, t1_rle, t1_out) < 0) rc = -1;

  // --- Test 2: empty file ---
  const char *t2_in  = "t2_in.bin";
  const char *t2_rle = "t2.rle";
  const char *t2_out = "t2_out.bin";
  if (rc == 0 && write_file(t2_in, "", 0) < 0) rc = -1;
  if (rc == 0 && do_roundtrip("empty file", t2_in, t2_rle, t2_out) < 0) rc = -1;

  // --- Test 3: long run (>255) ---
  const char *t3_in  = "t3_in.bin";
  const char *t3_rle = "t3.rle";
  const char *t3_out = "t3_out.bin";
  char big[1000];
  make_repeating(big, sizeof(big), 'A');
  if (rc == 0 && write_file(t3_in, big, sizeof(big)) < 0) rc = -1;
  if (rc == 0 && do_roundtrip("long run 1000x'A'", t3_in, t3_rle, t3_out) < 0) rc = -1;

  // --- Test 4: binary with zero bytes ---
  const char *t4_in  = "t4_in.bin";
  const char *t4_rle = "t4.rle";
  const char *t4_out = "t4_out.bin";
  char bin[256];
  for (int i = 0; i < 256; i++) bin[i] = (char)i;       // 0x00..0xFF once
  if (rc == 0 && write_file(t4_in, bin, sizeof(bin)) < 0) rc = -1;
  if (rc == 0 && do_roundtrip("binary 0x00..0xFF", t4_in, t4_rle, t4_out) < 0) rc = -1;

  if (rc == 0) {
    printf("\nAll tests PASSED\n");
    exit(0);
  } else {
    printf("\nSome tests FAILED\n");
    exit(1);
  }
}
