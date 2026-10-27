#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

#define MAXREF 128
#define MAXFRAME 32

static int refs[MAXREF];

static void
show(int *frames, int n)
{
  int i;
  printf(1, "[");
  for(i = 0; i < n; i++){
    if(i) printf(1, " ");
    if(frames[i] < 0) printf(1, "-");
    else printf(1, "%d", frames[i]);
  }
  printf(1, "]");
}

static int
runfifo(int nframes, int nrefs)
{
  int f[MAXFRAME], i, j, next, hit, faults;
  for(i = 0; i < nframes; i++) f[i] = -1;
  next = faults = 0;
  printf(1, "FIFO (%d frames)\n", nframes);
  for(i = 0; i < nrefs; i++){
    hit = 0;
    for(j = 0; j < nframes; j++) if(f[j] == refs[i]) hit = 1;
    if(!hit){ f[next] = refs[i]; next = (next + 1) % nframes; faults++; }
    printf(1, "%d ", refs[i]); show(f, nframes);
    printf(1, " %s\n", hit ? "hit" : "fault");
  }
  printf(1, "FIFO faults: %d\n", faults);
  return faults;
}

static int
runlru(int nframes, int nrefs)
{
  int f[MAXFRAME], last[MAXFRAME], i, j, victim, hit, faults, tick;
  for(i = 0; i < nframes; i++){ f[i] = -1; last[i] = -1; }
  faults = tick = 0;
  printf(1, "LRU (%d frames)\n", nframes);
  for(i = 0; i < nrefs; i++, tick++){
    hit = 0;
    for(j = 0; j < nframes; j++) if(f[j] == refs[i]){ hit = 1; last[j] = tick; }
    if(!hit){
      victim = -1;
      for(j = 0; j < nframes; j++) if(f[j] < 0){ victim = j; break; }
      if(victim < 0){ victim = 0; for(j = 1; j < nframes; j++) if(last[j] < last[victim]) victim = j; }
      f[victim] = refs[i]; last[victim] = tick; faults++;
    }
    printf(1, "%d ", refs[i]); show(f, nframes);
    printf(1, " %s\n", hit ? "hit" : "fault");
  }
  printf(1, "LRU faults: %d\n", faults);
  return faults;
}

int
main(int argc, char **argv)
{
  int nframes, nrefs, i, ff, lf;
  if(argc < 3){
    printf(2, "usage: pagerepl frames ref1 ref2 ...\n");
    exit();
  }
  nframes = atoi(argv[1]);
  nrefs = argc - 2;
  if(nframes < 1 || nframes > MAXFRAME || nrefs > MAXREF){
    printf(2, "pagerepl: frames must be 1..%d and refs at most %d\n", MAXFRAME, MAXREF);
    exit();
  }
  for(i = 0; i < nrefs; i++) refs[i] = atoi(argv[i+2]);
  ff = runfifo(nframes, nrefs);
  lf = runlru(nframes, nrefs);
  printf(1, "summary: FIFO faults=%d hit ratio=%d/%d; LRU faults=%d hit ratio=%d/%d\n",
         ff, nrefs-ff, nrefs, lf, nrefs-lf, nrefs);
  exit();
}
