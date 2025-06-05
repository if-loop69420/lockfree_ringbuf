#include "ring.h"


rbuf*  rbuf_init(size_t size) {
  rbuf* r_buf = (rbuf*)malloc(sizeof(rbuf));
  if (!r_buf) {
    return NULL;
  }

  void* buf = malloc(size);
  if (!buf) {
    return NULL;
  }
  memset(buf, 0, size);

  void* read_ptr = buf;
  void* write_ptr = buf;

  r_buf->buf = buf;
  r_buf->read_ptr = read_ptr;
  r_buf->write_ptr = write_ptr;
  r_buf->size = size;
  return r_buf;
}


int rbuf_write(rbuf* rbuf, void* data, size_t size) {
  if(size > rbuf->size) {
    return SIZE_EXCEEDS_BUF_ERR;
  }
  const char* original_buf = rbuf->buf;
  size_t original_size = rbuf->size;
  char* buf_check_copy = malloc(original_size);
  char* buf_work_copy = malloc(original_size);
  if (!buf_check_copy || !buf_work_copy) {
    return ALLOC_ERR;
  }
  memcpy(buf_check_copy, original_buf, original_size);
  memcpy(buf_work_copy, original_buf, original_size);
  
  char* write_ptr_check_copy = atomic_load(rbuf->write_ptr);
  // Translate to write_ptr_work_copy
  size_t offset = (char*)write_ptr_check_copy - (char*)original_buf;
  char* write_ptr_work_copy = buf_work_copy + offset;

  // If work_ptr + size goes past the boundary (wraparound in the buffer) write first from write_ptr to the end of the buffer 
  // and then from the start of the buffer to size - (size-write_ptr) 
  if (write_ptr_work_copy + size > buf_work_copy + original_size) {
    size_t size_to_end = (char*)original_buf + original_size - (char*)write_ptr_check_copy;
    size_t remaining_size = size - size_to_end;
    memcpy(write_ptr_work_copy,data, size_to_end);
    // Reset write_ptr_work_copy to buf_work_copy
    write_ptr_work_copy = buf_work_copy;
    memcpy(write_ptr_work_copy, (char*)data+size_to_end, remaining_size);
    write_ptr_work_copy += size;
  }
  // Else just write from write_ptr to write_ptr + size 
  else {
    memcpy(write_ptr_work_copy, data, size);
    write_ptr_work_copy += size;
  }


  int compared_memory = memcmp(buf_check_copy, original_buf,original_size);

  if (compared_memory == 0 && write_ptr_check_copy == rbuf->write_ptr) {
    // Actually write the changes here
    memcpy((char*)original_buf, buf_work_copy, original_size);
    // Translate the write_ptr_work_copy back to the original buffer
    rbuf->write_ptr = (_Atomic char*)(original_buf + (write_ptr_work_copy - buf_work_copy));
    return SUCCESS;
  } else {
    return DATA_CHANGED_ERR;
  }
}

int rbuf_read(rbuf* rbuf, size_t size, void* data) {
    char* buf_start = (char*)rbuf->buf;
    size_t buf_size = rbuf->size;

    char* read_ptr = (char*)rbuf->read_ptr;
    char* write_ptr = (char*)rbuf->write_ptr;

    // Calculate how much data is available
    size_t available;
    if (write_ptr >= read_ptr) {
        available = write_ptr - read_ptr;
    } else {
        available = (buf_start + buf_size - read_ptr) + (write_ptr - buf_start);
    }

    if (size > available) {
        return SIZE_EXCEEDS_BUF_ERR;  // not enough data to read
    }

    char* dst = (char*)data;

    // Normal read or wraparound read
    if (read_ptr + size <= buf_start + buf_size) {
        memcpy(dst, read_ptr, size);
        read_ptr += size;
        if (read_ptr == buf_start + buf_size) {
            read_ptr = buf_start;  // wraparound
        }
    } else {
        // Wraparound: read to end, then from beginning
        size_t size_to_end = buf_start + buf_size - read_ptr;
        size_t remaining_size = size - size_to_end;

        memcpy(dst, read_ptr, size_to_end);
        memcpy(dst + size_to_end, buf_start, remaining_size);

        read_ptr = buf_start + remaining_size;
    }

    rbuf->read_ptr = (_Atomic char*)read_ptr;
    return SUCCESS;
}

void rbuf_deinit(rbuf* rbuf) {
  free((void*)rbuf->buf);
  free(rbuf);
}
