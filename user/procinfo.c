#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

struct procinfo {
  int pid;
  int ppid;
  int state;
  uint64 sz;
  char name[16];
};

char*
statename(int state)
{
  switch(state){
    case 0: return "UNUSED";
    case 1: return "USED";
    case 2: return "SLEEPING";
    case 3: return "RUNNABLE";
    case 4: return "RUNNING";
    case 5: return "ZOMBIE";
    default: return "UNKNOWN";
  }
}

int
main(int argc, char *argv[])
{
  struct procinfo info;
  int pid;

  if(argc < 2){
    pid = getpid();
  } else {
    pid = atoi(argv[1]);
  }

  if(procinfo(pid, &info) == 0){
    printf("Process: %s\n", info.name);
    printf("PID: %d, PPID: %d\n", info.pid, info.ppid);
    printf("State: %s\n", statename(info.state));
    printf("Memory: %ld bytes\n", info.sz);
  } else {
    printf("procinfo: process %d not found\n", pid);
  }

  exit(0);
}