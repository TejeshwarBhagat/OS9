# xv6 x86 kernel integration (classic 32-bit xv6-public)

These changes target the classic x86 tree with `walkpgdir`, `deallocuvm`, `copyuvm`, `sys_sbrk`, and `T_PGFLT`. The exact syscall numbers can differ by fork; use unused numbers and update every syscall table consistently.

## `proc.h`

Add to `struct proc`:

```c
uint faults; // Number of lazy user pages allocated after a page fault.
```

Set `p->faults = 0` in `allocproc()` in `proc.c`, so a reused proc slot does not retain the previous process's count.

## `vm.c`: page-table printer and sparse address spaces

Add the following routines. `getuserpages()` is a small helper for the assignment test; both routines use the same present-user-page definition.

```c
int
getuserpages(pde_t *pgdir)
{
  uint va, count = 0;
  pte_t *pte;
  for(va = 0; va < KERNBASE; va += PGSIZE){
    pte = walkpgdir(pgdir, (char*)va, 0);
    if(pte && (*pte & (PTE_P|PTE_U)) == (PTE_P|PTE_U)) count++;
  }
  return count;
}

int
vmprint(pde_t *pgdir)
{
  uint va, pages = 0, ptpages = 0;
  pte_t *pte;
  pde_t *pde;
  for(va = 0; va < KERNBASE; va += PGSIZE){
    pde = &pgdir[PDX(va)];
    if((*pde & PTE_P) == 0) continue;
    pte = walkpgdir(pgdir, (char*)va, 0);
    if(pte && (*pte & (PTE_P|PTE_U)) == (PTE_P|PTE_U)){
      cprintf("va 0x%x pa 0x%x P=%d W=%d U=%d\\n", va,
              PTE_ADDR(*pte), !!(*pte&PTE_P), !!(*pte&PTE_W), !!(*pte&PTE_U));
      pages++;
    }
  }
  // Count allocated page-table pages in the user half of the directory.
  for(va = 0; va < KERNBASE; va += (1 << PDXSHIFT))
    if(pgdir[PDX(va)] & PTE_P) ptpages++;
  cprintf("user pages: %d; user page-table pages: %d\\n", pages, ptpages);
  return 0;
}
```

Add `int vmprint(pde_t*); int getuserpages(pde_t*);` to `defs.h`. If your compiler rejects `!!`, replace each with `(condition ? 1 : 0)`.

In `deallocuvm()`, preserve the normal page-table traversal but skip absent PTEs instead of panicking. In the loop, after `pte = walkpgdir(pgdir, (char*)a, 0)`, do:

```c
if(pte == 0) { a = (char*) (PGADDR(PDX(a)+1, 0, 0) - PGSIZE); continue; }
if((*pte & PTE_P) == 0) continue;
```

Then retain the normal `pa = PTE_ADDR(*pte)`, `kfree`, and clear-PTE code. The jump avoids scanning a whole absent 4 MB region. In `copyuvm()`, replace the `panic("copyuvm: pte should exist")` branch with a `continue` when `pte == 0 || (*pte & PTE_P) == 0`; only allocate/copy/map present pages. Be sure a missing PTE in a directory causes a 4 MB skip, or at minimum verify the loop safely handles it.

## `sysproc.c`: lazy `sbrk` and count syscalls

Keep negative `sbrk` behavior eager so shrinking frees pages. Replace the positive branch in `sys_sbrk()` with:

```c
if(argint(0, &n) < 0) return -1;
addr = myproc()->sz;
if(n > 0){
  if(addr + n >= KERNBASE) return -1;
  myproc()->sz += n;
}else if(n < 0){
  if((addr = deallocuvm(myproc()->pgdir, addr, addr+n)) == 0) return -1;
  myproc()->sz = addr;
}
return addr;
```

Use the existing local declarations for `n` and `addr`. Add:

```c
int
sys_getfaults(void)
{
  return myproc()->faults;
}

int
sys_vmprint(void)
{
  return vmprint(myproc()->pgdir);
}

int
sys_getuserpages(void)
{
  return getuserpages(myproc()->pgdir);
}
```

## `trap.c`: lazy page-fault handler

Add `extern char* kalloc(void);` only if the declaration is not already visible through `defs.h`. In the `T_PGFLT` case, handle only user-mode faults and keep the kernel's existing default panic for kernel-mode faults:

```c
case T_PGFLT: {
  uint va = rcr2();
  struct proc *p = myproc();
  char *mem;
  if(p && (tf->cs & 3) == DPL_USER){
    if(va < p->sz){
      mem = kalloc();
      if(mem){
        memset(mem, 0, PGSIZE);
        if(mappages(p->pgdir, (char*)PGROUNDDOWN(va), PGSIZE,
                    V2P(mem), PTE_W|PTE_U) == 0){
          p->faults++;
          break; // return to retry the faulting instruction
        }
        kfree(mem);
      }
    }
    cprintf("pid %d: invalid page fault at 0x%x\\n", p->pid, va);
    p->killed = 1;
    break;
  }
  if(p) p->killed = 1;
  break;
}
```

Confirm `V2P` and `PGROUNDDOWN` are available in this tree. Existing xv6 `trap()` checks `myproc()->killed` after the switch and exits the process; keep that logic intact. If `T_PGFLT` is currently the default panic path, remove its duplicate case.

## Syscall plumbing

For three new calls (`vmprint`, `getfaults`, `getuserpages`), use three unused syscall numbers. Add `SYS_*` values to `syscall.h`, `extern int sys_* (void);` declarations and table entries to `syscall.c`, declarations to `user.h`, and stubs to `usys.S` via `SYSCALL(name)`. Example table entries:

```c
[SYS_vmprint] sys_vmprint,
[SYS_getfaults] sys_getfaults,
[SYS_getuserpages] sys_getuserpages,
```

Add the user programs to `UPROGS` in the Makefile. Ensure `PGSIZE` is exposed to user code via the tree's `param.h`/`memlayout.h` convention; classic xv6-public user programs commonly include `param.h`.

## Important fork/exec detail

With sparse pages, `copyuvm()` must preserve holes while copying allocated pages. `exec()` builds a dense image, but after it replaces the old address space its process fault counter should remain meaningful for faults since process start (or reset it deliberately and document that choice). Check every other loop that assumes all pages below `sz` are present, especially `uva2ka()`/`copyout()` if a test uses a lazy page as a syscall buffer; those paths should fail cleanly rather than panic.
