/****************** kernel.c file *************************/
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

PROC *kfork()
{
      PROC *p = get_proc(&freeList);
      int i;
      
      if (p==0)
      {
         printf("no more PROC, kfork() failed\n");
         return 0;
      }
      
      p->status = READY;
      p->priority = 1;          // priority = 1 for all proc except P0
      p->ppid = running->pid;   // parent = running
      p->parent = running;
      
      for (i=1; i<10; i++)
	      p->kstack[i]= 0 ;
	      
	    p->kstack[SSIZE - 10] = 0x1000;
	    p->kstack[SSIZE - 11] = 0x1000;
	    
      p->kstack[SSIZE-1] = (int)body;
	    
      p->ksp = &(p->kstack[SSIZE-11]);
      enqueue(&readyQueue, p);  // enter p into readyQueue by priority
      
      nproc++;
      return p;                 // return child PROC pointer
}         

int do_tswitch()
{
  printf("proc %d tswitch()\n", running->pid);
  tswitch();
  printf("proc %d resumes\n", running->pid);
}

int do_kfork()
{
  PROC *p;
  printf("proc %d kfork a child\n", running->pid);
  p = kfork();
  if (p==0)
    printf("kfork failed\n");
  else
    printf("child pid = %d\n", p->pid);
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

    printf("enter a char [s|f|w|q] : ");
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
    }
  }
}
