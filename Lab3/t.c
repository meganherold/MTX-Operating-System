#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc = 0;

int body();
char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter", 
               "Saturn", "Uranus", "Neptune" };

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
**************************************************/
/* #include "bio.c" */
/* #include "queue.c" */
/* #include "loader.c" */

#include "wait.c"             // YOUR wait.c   file
#include "kernel.c"           // YOUR kernel.c file
#include "int.c"              // YOUR int.c    file
int color;

int init()
{
    PROC *p; int i;
    color = 0x0C;
    printf("init ....");
    for (i=0; i<NPROC; i++){   // initialize all procs
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0;  
        strcpy(proc[i].name, pname[i]);
        p->next = &proc[i+1];
    }
    freeList = &proc[0];      // all procs are in freeList
    proc[NPROC-1].next = 0;
    readyQueue = sleepList = 0;

    /**** create P0 as running ******/
    p = get_proc(&freeList);
    p->status = RUNNING;
    p->ppid   = 0;
    p->parent = p;
    running = p;
    nproc = 1;
    printf("done\n");
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

int scheduler()
{
    printf("scheduler: inside scheduler\n");
    if (running->status == READY)
    {
      printf("scheduler: enqueueing the running proc\n");
        enqueue(&readyQueue, running);
     }
     running = dequeue(&readyQueue);
     color = running->pid + 0x0A;
}

int int80h(); //tell C compiler int80h is function _int80h in assembly code

int set_vector(u16 vector, u16 handler)
{
     //initialize vector 80 area
    //vector80 = [PC, CS] = [int80h, kernel_segment] (kernel segment = 0x1000)
     put_word(handler, 0x0000, vector<<2); 
     put_word(0x1000,  0x0000,(vector<<2) + 2); //text p. 137
}
            
int main(void)
{
    printf("MTX starts in main()\n");
    init();      // initialize and create P0 as running
    
    //set vector 80
    set_vector(80, int80h);
    
    kfork("/bin/u1");     // P0 kfork() P1

    while(1){
      printf("P0 running\n");
      while(!readyQueue);
      printf("P0 switch process\n");
      do_tswitch();         // P0 switch to run P1
   }
   
   return 0;
}
