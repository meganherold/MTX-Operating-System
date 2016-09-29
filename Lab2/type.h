/*************** type.h file *****************/
#define NPROC 9
#define SSIZE 1024
#define FREE 0
#define READY 1
#define RUNNING 2 // for clarity only, not needed or used
#define STOPPED 3
#define SLEEP 4
#define ZOMBIE 5

typedef struct proc{
  struct proc *next;
  int *ksp;
  int pid; // process ID number
  int status; // status = FREE|READY|RUNNING|SLEEP|ZOMBIE
  int ppid; // parent pid
  struct proc *parent; // pointer to parent PROC
  int priority;
  int event; // sleep event
  int exitCode; // exit code
  int kstack[SSIZE];
}PROC;
