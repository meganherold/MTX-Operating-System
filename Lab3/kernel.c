/*****************kernel.c file:***********************/

//YOUR kernel.c file as in LAB3

//MODIFY kfork() to enter kernel to kfork a chile with /bin/u1 is Umode image

#include "queue.c"

int goUmode(); //tell C compiler goUmode is function _goUmode in assembly code
 
PROC *kfork(char *filename)
{
      PROC *p = get_proc(&freeList);
      int i, segment;
      
      if (p==0)
      {
         printf("no more PROC, kfork() failed\n");
         return 0;
      }
      
      printf("kfork: proc %d taken from freeList\n", p->pid);
      
      p->status = READY;
      p->priority = 1;          // priority = 1 for all proc except P0
      p->ppid = running->pid;   // parent = running
      p->parent = running;
      
      for (i=1; i<10; i++) 
      {
		    p->kstack[SSIZE-i] = 0;
	    }
 
	    // this is the resume point
	    p->kstack[SSIZE-1] = (int)body;
	    
	    // top of the stack
	    p->ksp = &(p->kstack[SSIZE - 9]);
      
      if(filename)
      {
        segment = (p->pid + 1) * 0x1000;
        p->uss = segment;
	      p->usp = -24;                     //DS, top of ustack
	      
        //load filename to child segment as its Umode image
        load(filename, segment);
        
        //now we'll set up the child's ustack
        //CPU registers are all 0
        for (i=4; i<10; i++)
	        put_word(0, segment, i*-2);
	       
	      
	      //set flag, uCS, uPC, uES, and uDS
	      put_word(0x0200, segment, -2);    //flag 
	      put_word(segment, segment, -4);   //CS
	      //put_word(0, segment, -6);         //PC
	      put_word(segment, segment, -22);  //ES
	      put_word(segment, segment, -24);  //DS
	      
      }
      
      enqueue(&readyQueue, p);  // enter p into readyQueue by priority
      
      nproc++;
      return p;                 // return child PROC pointer
}

int rflag, body();

int atoi(char *s)
{
  int v = 0;
  while(*s){
    v = v*10 + (*s-'0');
    s++;
  }
  return v;
}

int geti()
{
  char s[16];
  gets(s);
  return atoi(s);
}

int do_tswitch()
{
  printf("proc %d tswitch()\n", running->pid);
  running->status = READY;
  tswitch();
  //running->status = RUNNING;
  printf("proc %d resumes\n", running->pid);
}

int do_kfork()
{
  PROC *p;
  printf("proc %d kfork a child\n", running->pid);
  p = kfork("/bin/u1");
  if (p==0)
  {
    printf("kfork failed\n");
    return -1;
  }
  else
    printf("child pid = %d\n", p->pid);
  return p->pid;
}

int do_stop()
{
  printf("proc %d stop running\n", running->pid);
  running->status = STOPPED;
  tswitch();
  printf("proc %d resume from stop\n", running->pid);
}

int do_continue()
{
  PROC *p;
  int pid;

  printf("enter pid to resume : ");
  pid = (getc()&0x7F) - '0';

  if (pid < 1 || pid >= NPROC){
    printf("invalid pid\n", pid);
    return 0;
  }
  p = &proc[pid];
  if (p->status == STOPPED){
    p->status = READY;
    enqueue(&readyQueue, p);
    return 1;
  }
  return 0;
}

int do_sleep()
{
  int event;
  printf("enter an event value to sleep on: ");
  event = geti();
  ksleep(event);
  printf("proc %d resumes after sleep\n", running->pid);
}

int do_wakeup()
{
  int event;
  printf("enter an event value to wake up: ");
  event = geti();
  kwakeup(event);
}

int do_exit()
{
  int exitValue;
  if (running->pid == 1 && nproc > 2){
      printf("other procs still exist, P1 can't die yet !%c\n",007);
      return -1;
  }
  printf("enter an exitValue (0-9) : ");
  exitValue = (getc()&0x7F) - '0'; 
  printf("%d\n", exitValue);
  kexit(exitValue);
}

int do_wait()
{
  int child, status;
  child = kwait(&status);
  if (child<0){
    printf("proc %d wait error : no child\n", running->pid);
    return -1;
  }
  printf("proc %d found a ZOMBIE child %d exitValue=%d\n", 
	   running->pid, child, status);
  return child;
}
int reschedule()
{
  PROC *p, *tempQ = 0;

  while ( (p=dequeue(&readyQueue)) ){
        enqueue(&tempQ, p);
  }
  readyQueue = tempQ;

  rflag = 0;
  if (running->priority < readyQueue->priority)
    rflag = 1;
}

int chpriority(int pid, int pri)
{
  PROC *p; int i, ok=0, reQ=0;

  if (pid == running->pid){
     running->priority = pri;

     if (pri < readyQueue->priority) 
        rflag = 1;
    return 1;
  }
  // not for running; for both READY and SLEEP procs
  for (i=1; i<NPROC; i++){
    p = &proc[i];
    if (p->pid == pid && p->status != FREE){
      p->priority = pri;
      ok = 1;
      if (p->status == READY)  // in readyQueue==> redo readyQueue
	reQ = 1;
    }
  }
  if (!ok){
    printf("chpriority failed\n");
    return -1;
  }
  if (reQ)
     reschedule(p);
}

int do_chpriority()
{
  int pid, pri;
  char s[16];
  printf("input pid " );
  pid = geti();
  printf("input new priority " );
  pri = geti();
  if (pri<1) 
      pri = 1;
  chpriority(pid, pri);
}



int body()
{
  char c;
  printf("proc %d resumes to body()\n", running->pid);
  while(1){
    //printf("-----------------------------------------\n");
    printList("freelist  ", freeList);
    printQueue("readyQueue", readyQueue);
    printSleepList("sleepList ", sleepList);
    printf("----------------------------------------------------------------------\n");

    printf("proc %d[%d] running: parent=%d\n",
	   running->pid, running->priority, running->ppid);
	   
	  running->status = RUNNING;

    printf("enter a char [s|f|w|q|u] : ");
    c = getc(); printf("%c\n", c);
    switch(c){
       case 's' : do_tswitch();   break;
       case 'f' : do_kfork();     break;
       case 't' : do_stop();      break;
       case 'c' : do_continue();  break;
       case 'z' : do_sleep();     break;
       case 'a' : do_wakeup();    break;
       case 'p' : do_chpriority();    break;
       case 'w' : do_wait();      break;
       case 'q' : do_exit();      break;
       case 'u' : goUmode();     break;
    }
  }
}
