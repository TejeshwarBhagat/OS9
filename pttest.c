#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

int
main(void)
{
  int n, pad;
  uint oldpages, newpages;
  volatile char *touch;
  int i;

  printf(1, "pttest: initial address space\n");
  vmprint();

  // Align the break so a 4-page extension adds exactly four mappings.
  n = (int)sbrk(0);
  pad = (PGSIZE - (n % PGSIZE)) % PGSIZE;
  if(pad && sbrk(pad) == (char*)-1){
    printf(2, "pttest: alignment sbrk failed\n");
    exit();
  }
  printf(1, "pttest: before four-page extension\n");
  vmprint();
  oldpages = getuserpages();
  if(sbrk(4 * PGSIZE) == (char*)-1){
    printf(2, "pttest: sbrk failed\n");
    exit();
  }
  // Touch one byte in each new page; required after lazy sbrk is enabled.
  touch = (volatile char*)sbrk(0) - 4 * PGSIZE;
  for(i = 0; i < 4; i++) touch[i * PGSIZE] = 1;
  printf(1, "pttest: after sbrk(4 * PGSIZE) and touching its pages\n");
  vmprint();
  newpages = getuserpages();
  printf(1, "pttest: added user pages = %d (expected 4)\n", newpages-oldpages);
  exit();
}
