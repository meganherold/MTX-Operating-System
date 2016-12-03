#include "type.h"

PROC proc[NPROC];
PROC *freeList, *running, *readyQueue, *sleepList;
int procSize = sizeof(PROC);

char *pname[]={"Sun","Mercury", "Venus", "Earth",  "Mars", "Jupiter",
               "Saturn",  "Uranus","Neptune"};
int nproc;

#include "wait.c"
#include "kernel.c"
#include "int.c"
#include "pv.c"
#include "serial.c"

int body();

int init()
{
  int i,j,fd; PROC *p;
   for (i=0; i<NPROC; i++){
     p = &proc[i];
       p->pid = i;
       p->status = FREE;
       p->next = &proc[i+1];
       strcpy(p->name, pname[i]);
       printf("%d %s ", p->pid, proc[i % 4].name);
   }
   printf("\n");

   proc[NPROC-1].next = 0;
   freeList = &proc[0];
   readyQueue = 0;
   sleepList = 0;

   p = get_proc(&freeList);
   //p->pri = 0;
   p->status = READY;
   p->parent = p;
   //p->inkmode = 1;
   running = p;
   nproc = 1;

   lock();
}

int printSleepList(char *name, PROC *p)
{
   int i = 0;
   printf("-------------------------- %s --------------------------------\n", name);
   for(i = 0; i < NPROC; i++)
   {
      if(proc[i].status == SLEEP)
        printf("%d[e=%d] -> ", proc[i].pid, proc[i].event);
   }
   printf("NULL\n");
}

int int80h(), s0inth(), s1inth();

int set_vec(u16 vector, u16 addr)
{
    put_word(addr,  0, vector << 2);
    put_word(0x1000,0, (vector << 2) + 2);
}

main()
{
      int i, pid, status;  char c, d;
      printf("Welcome to the 460 Multitasking System\n");

      printf("initializing ......\n");
        init();
      printf("initialization complete\n");

      set_vec(80, int80h);

      kfork("/bin/u1");

      set_vec(12, s0inth);
      sinit();

      while(1){
        if (readyQueue)
           tswitch();
      }
      printf("all dead, happy ending\n");
}

int scheduler()
{
    if (running->status == READY)
       enqueue(&readyQueue,running);
    running = dequeue(&readyQueue);
}
