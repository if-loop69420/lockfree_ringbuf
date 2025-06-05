#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>

enum rbuf_return_codes {
  SUCCESS = 0,
  DATA_CHANGED_ERR = -1,
  ALLOC_ERR = -2,
  SIZE_EXCEEDS_BUF_ERR = -3,
  READ_EXCEEDS_WRITE_ERR = -4,
};

// Can't read beyond write pointer
// Can write beyond read_pointer
// Before writing changes copy the things you're going to change and check if they're equal before writing changes to the datastructure
// If things haven't been changed -> write
// If things have been changed -> return -1 (or some error code)
typedef struct {
  const char* buf;
  _Atomic char* read_ptr;
  _Atomic char* write_ptr;
  _Atomic size_t size;
} rbuf;

rbuf*  rbuf_init(size_t size);
int rbuf_write(rbuf* rbuf, void* data, size_t size);
int rbuf_read(rbuf* rbuf, size_t size, void* buf);
void rbuf_deinit(rbuf* rbuf);
