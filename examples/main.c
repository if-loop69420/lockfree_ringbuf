#include "../ring.h"
#include <stdio.h>

int main(int argc, char** argv) {
  rbuf* ring = rbuf_init(sizeof(int)*10);
  int a = 12;
  int write_res = rbuf_write(ring, &a, sizeof(int));
  if (write_res == SUCCESS){
    printf("Successfull write!\r\n");
  } else {
    printf("Failed with code %i\r\n", write_res);
  }
  int b;
  int read_res = rbuf_read(ring, sizeof(int), &b);
  if (read_res == SUCCESS){
    printf("Successfull read. Value of b: %i!\r\n", b);
  } else {
    printf("Failed with code %i\r\n", read_res);
  }

  for(int i = 0; i < 12; i++) {
    rbuf_write(ring, &i, sizeof(int));
  }
  for(int i = 0; i < 12; i++) {
    int read;
    rbuf_read(ring,sizeof(int),&read);
    printf("Read back %i from Ring Buffer\r\n", read);
  }
  printf("Actual contents of the buffer:\r\n");

  for(int i =0; i < 10; i++) {
    printf("%i,", *((char*)ring->buf + sizeof(int) * i));
  }
  printf("\r\n");

  rbuf_deinit(ring);
}
