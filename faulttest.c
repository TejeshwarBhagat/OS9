#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

int
main(void)
{
  int i, pid, status;
  char *base;
  volatile char *p;

  base = sbrk(32 * PGSIZE);
  if(base == (char*)-1){
    printf(2, "faulttest: sbrk failed\n");
    exit();
  }
  p = (volatile char*)base;
  for(i = 0; i < 5; i++)
    p[i * 3 * PGSIZE] = (char)i; // Five distinct pages, each touched once.
  printf(1, "faulttest: handled page faults = %d (expected 5)\n", getfaults());

  // A bad user address must kill only this child, leaving the test alive.
  pid = fork();
  if(pid < 0){
    printf(2, "faulttest: fork failed\n");
    exit();
  }
  if(pid == 0){
    volatile char value;
    value = *((volatile char*)(base + 33 * PGSIZE)); // Above process sz.
    printf(1, "faulttest: unexpected read succeeded: %d\n", value);
    exit();
  }
  wait(&status);
  printf(1, "faulttest: invalid-address child exited; parent survived\n");
  exit();
}
