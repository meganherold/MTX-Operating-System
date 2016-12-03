/*************************************************************************
  usp  1   2   3   4   5   6   7   8   9  10   11   12    13  14  15  16
----------------------------------------------------------------------------
 |uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/

/****************** syscall handler in C ***************************/
//p. 148 in textbook

/******* PROC status ********/
/*#define FREE     0
#define READY    1
#define RUNNING  2
#define STOPPED  3
#define SLEEP    4
#define ZOMBIE   5 */

#define AX 8
#define PA 13

int kcinth()
{
   u16    segment, offset;
   int    a,b,c,d, r;

   segment = running->uss;
   offset = running->usp;

   a = get_word(segment, offset + 2 * 13);
   b = get_word(segment, offset + 2 * 14);
   c = get_word(segment, offset + 2 * 15);
   d = get_word(segment, offset + 2 * 16);

   switch(a){
       case 0 : r = kgetpid();        break;
       case 1 : r = kps();            break;
       case 2 : r = kchname(b);       break;
       case 3 : r = kmode();          break;
       case 4 : r = ktswitch();       break;
       case 5 : r = kkwait(b);        break;
       case 6 : r = kkexit(b);        break;

       case 7 : r = fork();           break;
       case 8 : r = exec(b);          break;
       case 9 : r = hop(b);           break;

       case 10: r = pipe(b);           break;
       case 11: r = pfd();            break;
       case 12: r = close(b);         break;
       case 13: r = read(b, c, d);          break;
       case 14: r = write(b, c, d);         break;

       case 15: r = get_segment();    break;
       case 99: kkexit(b);

       default: printf("invalid syscall # : %d\n", a);
   }
   //==> WRITE CODE to let r be the return value to Umode
   put_word(r, segment, offset + 2*AX);
}

//============= WRITE C CODE FOR syscall functions ======================

int kgetpid()
{
    return running->pid;
}

int kps()
{
    //WRITE C code to print PROC information
   int i = 0;
   printf("----------------------- process statuses -----------------------------\n");
   for(i = 0; i < NPROC; i++)
   {
        printf("process: %d    status: ", proc[i].pid);
        switch(proc[i].status)
        {
          case 0: printf("free");         break;
          case 1: printf("ready");        break;
          case 2: printf("running");      break;
          case 3: printf("stopped");      break;
          case 4: printf("asleep");       break;
          case 5: printf("zombified");    break;
          default: printf("unknown status"); break;
        }
        printf("      name: %s\n", proc[i].name);
   }

}

int kchname(char *name)
{

    //WRITE C CODE to change running's name string;
    char byte;
    int i;

    for(i = 0; i < 32; i++)
    {
        byte = get_byte(running->uss, name + i);
        running->name[i] = byte;
        if (byte == '\0')
        {
          break;
        }
    }

}

int kkfork()
{
  //use your kfork() in kernel
  return kfork("/bin/u1");

  /*if(p)
  {
    enqueue(&readyQueue, p->pid);
    put_word(p->pid, running->uss, running->usp);
  }

  else
  {
    put_word(-1, running->uss, running->usp);
  }*/
  //return child pid or -1 to Umode!!!
}

int ktswitch()
{
    running->status = READY;
    return tswitch();
}

int kkwait(int *status)
{

  //use YOUR kwait() in LAB3;
  //return values to Umode!!!
  int exitCode;
  int pid = kwait(&exitCode);

  put_word(exitCode, running->uss, status);
  return pid;
}

int kkexit(int value)
{
    return kexit(value);
}


int kmode()
{
  body();
}

int kgetc()
{
  return getc();
}

int kputc(char c)
{
  return getc(c);
}
