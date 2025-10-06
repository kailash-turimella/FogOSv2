# compress — Run-Length Encode / Decode files

## Usage

`compress` provides simple Run-Length Encoding (RLE) for any file. It can read from **stdin** and write to **stdout** if file paths are omitted.

* `-c` : **Compress** input → RLE output
* `-d` : **Decompress** RLE input → original bytes
* `-h` : Show help/usage

> RLE format: a stream of 2-byte pairs `[count][value]` with `count` in 1–255. Long runs are split. Random data may grow in size.

### Examples

```sh
# File → file
$ compress -c README.md README.rle
$ compress -d README.rle README.out

# Stream mode (stdin/stdout + pipes/redirection)
$ cat README.md | compress -c > foo.rle
$ cat foo.rle    | compress -d > out.txt
```

---

## Testing

An automated test runner `/compress_test` is included. It creates sample files, runs compress/decompress, and checks round-trip equality.

### To run tests

1. Boot the OS and open the shell.
2. Run:

   ```
   /compress_test
   ```
3. You should see `PASS`/`FAIL` lines and a summary, e.g.:

   ```
   [PASS] small text
   [PASS] empty file
   [PASS] long run 1000x'A'
   [PASS] binary 0x00..0xFF

   All tests PASSED 
   ```

If a test fails, the message indicates which stage (compress/decompress/compare) had an issue.
