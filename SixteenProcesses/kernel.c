/*****************kernel.c file:***********************/


#include "queue.c"
#include <string.h>

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
	    p->kstack[SSIZE-1] = (int)goUmode;

	    // top of the stack
	    p->ksp = &(p->kstack[SSIZE - 9]);

      if(filename)
      {
        segment = (p->pid + 1) * 0x800 + 0x1000;
        p->uss = segment;
	      p->usp = -24;                     //DS, top of ustack

        //load filename to child segment as its Umode image
        if(!load(filename, segment))
          return -1;

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

int copyImage(u16 pseg, u16 cseg, u16 size)
{
  u16 i;
  printf("copying parent image\n");
  for (i=0; i<size; i++)
    put_word(get_word(pseg, 2*i), cseg, 2*i);
}

int fork()
{
  int pid; u16 segment;
  PROC *p;

  printf("megan's fork: ");

  p = kfork(0); // kfork() a child, do not load image file
  //printf("back from kfork, child pid = %d\n", p->pid);

  if (p==0)
    return -1; // kfork failed

  segment = (p->pid + 1) * 0x800 + 0x1000; // child segment
  copyImage(running->uss, segment, 16*1000); // copy 16K words
  p->uss = segment; // child’s own segment
  p->usp = running->usp; // same as parent's usp

  printf("fork: segment = %x offset = %d\n", segment, p->usp);

  //*** change uDS, uES, uCS, AX in ustack ****
  put_word(segment, segment, p->usp);       // uDS=segment
  put_word(segment, segment, p->usp+2);     // uES=segment
  put_word(0, segment, p->usp+2*8);         // uax=0
  put_word(segment, segment, p->usp+2*10);  // uCS=segment

  //enqueue(&readyQueue, p);

  return p->pid;
}

int exec(char *y) // y points at whole command line in Umode space
{
  int i, len;
  u16 segment, offset, HIGH;
  char line[64], *cp, c;
  char filename[32], f2[16],*cq;

  segment = running->uss;

  /* get command line from U space */
  cp = line;
  while( (*cp=get_byte(segment, y)) != 0 ){
        y++; cp++;
  }
  cp = line;        cq = f2;

  while (*cp != ' ' && *cp != 0){
       *cq = *cp;
        cq++; cp++;
  }
  *cq = 0;

  strcpy(filename, f2);

  printf("proc %d execing to %s in segment %x\n",
         running->pid, filename, segment);

  if (!load(filename, segment))
   return -1;

  len = strlen(line) + 1;
  if (len % 2)   // odd ==> must pad a byte
   len ++;

  offset = -len;
  for (i=0; i<len; i++){
     put_byte(line[i], segment, offset+i);
  }

  HIGH = offset - 2;  /* followed by INT stack frame */

  /* *s pointing at the command line string */
  put_word(offset, segment, HIGH);

  /* zero out U mode registers in ustack di to ax */
  for (i=1; i<=12; i++){
   put_word(0, segment, HIGH-2*i);
  }

  // must set PROC.uss to our own segment
  running->uss = segment;
  running->usp = HIGH-2*12;

  put_word(running->uss, segment, HIGH-2*12);   // uDS=uSS
  put_word(running->uss, segment, HIGH-2*11);   // uES=uSS
  put_word(0,            segment, HIGH-2*3);    // uPC=0
  put_word(segment,      segment, HIGH-2*2);    // uCS=segment
  put_word(0x0200,       segment, HIGH-2*1);    // flag
}

int get_segment()
{
  return running->uss;
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

    printf("proc %d[%d] running: segment=%x\n",
	   running->pid, running->priority, running->uss);

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
