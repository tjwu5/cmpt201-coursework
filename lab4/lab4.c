#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <errno.h>
#include <stdint.h>
#include <stdio.h>  // snprintf, perror
#include <stdlib.h> // exit
#include <string.h> // memset
#include <unistd.h> // sbrk, write

#define BUF_SIZE 256
#define BLOCK_SIZE 128

struct header {
  uint64_t size;
  struct header *next;
};

static void handle_error(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

static void print_out(const char *format, void *data, size_t data_size) {
  char buf[BUF_SIZE];
  ssize_t len;

  if (data_size == sizeof(uint64_t)) {
    len = snprintf(buf, BUF_SIZE, format, *(uint64_t *)data);
  } else {
    len = snprintf(buf, BUF_SIZE, format, *(void **)data);
  }

  if (len < 0) {
    handle_error("snprintf");
  }
  if (write(STDOUT_FILENO, buf, (size_t)len) != len) {
    handle_error("write");
  }
}

int main(void) {
  void *base = sbrk(BLOCK_SIZE * 2);
  if (base == (void *)-1) {
    handle_error("sbrk");
  }

  void *block1_addr = base;
  void *block2_addr = (void *)((char *)base + BLOCK_SIZE);

  struct header *first_block = (struct header *)block1_addr;
  struct header *second_block = (struct header *)block2_addr;

  first_block->size = BLOCK_SIZE;
  first_block->next = NULL;

  second_block->size = BLOCK_SIZE;
  second_block->next = first_block;

  size_t header_bytes = sizeof(struct header);
  size_t data_bytes = BLOCK_SIZE - header_bytes;

  unsigned char *first_data =
      (unsigned char *)((char *)block1_addr + header_bytes);
  unsigned char *second_data =
      (unsigned char *)((char *)block2_addr + header_bytes);

  memset(first_data, 0, data_bytes);
  memset(second_data, 1, data_bytes);

  print_out("first block:       %p\n", &block1_addr, sizeof(block1_addr));
  print_out("second block:      %p\n", &block2_addr, sizeof(block2_addr));

  print_out("first block size:  %lu\n", &first_block->size, sizeof(uint64_t));
  print_out("first block next:  %p\n", &first_block->next,
            sizeof(first_block->next));

  print_out("second block size: %lu\n", &second_block->size, sizeof(uint64_t));
  print_out("second block next: %p\n", &second_block->next,
            sizeof(second_block->next));

  for (size_t i = 0; i < data_bytes; i++) {
    uint64_t v = first_data[i];
    print_out("%lu\n", &v, sizeof(uint64_t));
  }
  for (size_t i = 0; i < data_bytes; i++) {
    uint64_t v = second_data[i];
    print_out("%lu\n", &v, sizeof(uint64_t));
  }

  return 0;
}
