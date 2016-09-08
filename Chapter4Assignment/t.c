/*********** MTX4.2 kernel t.c file **********************/
#define NPROC 9
#define SSIZE 1024

#include "myio.c" // include I/O functions
#include "queue.c" // implement your own queue functions

/******* PROC status ********
#define FREE 0
#define READY 1
#define STOP 2
#define DEAD 3
*****************************

typedef struct proc
{
	struct proc *next;
	int *ksp;
	int pid; // add pid for identify the proc
	int ppid; // parent pid;
	int status; // status = FREE|READY|STOPPED|DEAD, etc
	int priority; // scheduling priority
	int kstack[SSIZE]; // proc stack area
}PROC;*/


PROC proc[NPROC], *running, *freeList, *readyQueue;
int procSize = sizeof(PROC);


void printList(char* string, PROC *list)
{
  PROC *current_proc = list;
  
  printf("-------------------------- %s --------------------------------\n", string);
  while(current_proc->next != 0)
  {
    printf("%d[%d] -> ", current_proc->pid, current_proc->priority);
    current_proc = current_proc->next;
  }
  
  printf("NULL\n");
}

int body()
{
	char c;
	printf("proc %d starts from body()\n", running->pid);

	while(1)
	{
		printList("freelist ", freeList); // optional: show the freeList
		printList("readyQueue", readyQueue); // show the readyQueue
		printf("----------------------------------------------------------------------\n");
		printf("proc %d running: parent=%d\n",running->pid,running->ppid);
		printf("enter a char [s|f] : ");
		c = getc(); 
		printf("%c\n", c);
		//system("pause");

		switch(c)
		{
			case 'f' : do_kfork(); break;
			case 's' : do_tswitch(); break;
		}
	}
}

PROC *kfork() // create a child process, begin from body()
{
	int i;
	PROC *p = get_proc(&freeList);
	printf("inside kfork\n");

	if (!p)
	{
		printf("no more PROC, kfork() failed\n");
		return 0;
	}

	p->status = READY;
	p->priority = 1; // priority = 1 for all proc except P0
	p->ppid = running->pid; // parent = running
	
	/* initialize new proc's kstack[ ] */
	for (i=1; i<10; i++) // saved CPU registers
		p->kstack[SSIZE-i]= 0 ; // all 0's

	p->kstack[SSIZE-1] = (int)body; // resume point=address of body()
	p->ksp = &p->kstack[SSIZE-9]; // proc saved sp
	enqueue(&readyQueue, p); // enter p into readyQueue by priority
	return p; // return child PROC pointer
}

int init()
{
	PROC *p; 
	int i;

	printf("initializing\n");

	for (i=0; i<NPROC; i++) // initialize all procs
	{
		p = &proc[i];
		p->pid = i;
		p->status = FREE;
		p->priority = 0;
		p->next = &proc[i+1];
	}

	proc[NPROC-1].next = 0;
	freeList = &proc[0]; // all procs are in freeList
	printf("all proceses are in the freeList.\n");
	readyQueue = 0;

	/**** create P0 as running ******/
	p = get_proc(&freeList); // allocate a PROC from freeList
	p->ppid = 0; // P0’s parent is itself
	p->status = READY;
	running = p; // P0 is now running
}

int scheduler()
{
	if (running->status == READY) // if running is still READY
		enqueue(&readyQueue, running); // enter it into readyQueue
	running = dequeue(&readyQueue); // new running
}

int main()
{
	printf("\nMTX starts in main()\n");
	init(); // initialize and create P0 as running
	do_kfork(); // P0 creates child P1
	
	while(1) // P0 switches if readyQueue not empty
	{
		if (readyQueue)
			tswitch();
		else
		  break;
	}
	return 0;
}

/*************** kernel command functions ****************/
int do_kfork( )
{
 	PROC *p = kfork();

 	if (p == 0)
	{ 
		printf("kfork failed\n"); 
		return -1; 
	}

	printf("PROC %d kfork a child %d\n", running->pid, p->pid);
	return p->pid;
}

int do_tswitch()
{ 
	tswitch(); 
}
