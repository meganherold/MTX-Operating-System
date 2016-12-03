struct semaphore{
  int value;    //initial value of semaphore
  PROC *queue;  //queue of BLOCKed processes
};

int enterCR()
{
  return int_off(); //disable interrupts, return the status register
}

void exitCR(int status_register)
{
  int_on(status_register);  //restore status register to original value
}

//blocks the process in the semaphore's queue
/*void BLOCK(struct semaphore *s)
{
  running->status = BLOCK;
  enqueue(&s->queue, running);
  tswitch();  //switch process
} */

//unblocks the first process in the semaphore's queue
void SIGNAL(struct semaphore *s)
{
  PROC *p;
  p = dequeue(&s->queue);   //get a process from semaphore's queue
  p->status = READY;        //set it to ready
  enqueue(&readyQueue, p);  //put it in the ready queue
}

int P(struct semaphore *s)
{
  //in order to change the semaphore, we have to be in the same critical region
  int status_register = enterCR();

  s->value--;
  if(s->value < 0)
  {
    running->status = BLOCK;
    enqueue(&s->queue, running);
    tswitch();  //switch process
  }
  else
    exitCR(status_register);
}

int V(struct semaphore *s)
{
  //again, same critical region
  int status_register = enterCR();
  //printf("V\n");

  s->value++;
  if(s->value <= 0)
  {
    PROC *p;
    p = dequeue(&s->queue);   //get a process from semaphore's queue
    p->status = READY;        //set it to ready
    enqueue(&readyQueue, p);  //put it in the ready queue
  }

  exitCR(status_register);
  //printf("end of V\n");
}
