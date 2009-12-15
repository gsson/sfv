#include <stdio.h>
#include "crc.h"

const char *tv[] = {
  "012345678",
  " 012345678",
  "  012345678",
  "   012345678",
  "    012345678",
  "     012345678",
  "      012345678",
  "       012345678" };

const uint32_t v = 0x37fad1baul;

int
main(int argc, char **argv) {
  int i;
  for (i = 0; i < 8; i++) {
    uint32_t result = crc32((void *)(tv[i] + i), 0xfffffffful, 9) ^ 0xfffffffful;
    if (result != v) {
      printf("%d %08x != %08x\n", i, result, v);
      return 1;
    }
  }
  return 0;
}
