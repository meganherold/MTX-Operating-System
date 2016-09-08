/*********** t.c file of a multitasking program **************/

#define NPROC 9 // number of PROCs
#define SSIZE 1024 // proc kstack size = 2KB

typedef struct proc
{
	struct proc *next;
	int *ksp;
	int pid; // add pid as proc's ID
	int kstack[SSIZE]; // proc stack area
}PROC;

PROC proc[NPROC], *running; // define NPROC proc structures
int procSize = sizeof(PROC); // PROC size, needed in assembly code

int body()
{ 
	char c;
	int pid = running->pid;
	
	printf("proc %d resumes to body()\n", pid);
	while(1)
	{
		printf("proc %d running, enter a key:\n", pid); c=getc();
		tswitch();
	}
}

int init() // initialize PROC structures
{
	PROC *p; int i, j;

	for (i=0; i<NPROC; i++) // initialize all PROCs
	{
		p = &proc[i];
		p->pid = i; // pid = 0,1,2,..NPROC-1
		p->next = &proc[i+1]; // point to next PROC
		if (i) // for PROCs other than P0
		{
			p->kstack[SSIZE-1]=(int)body; // entry address of body()
			
			for (j=2; j<10; j++) // all saved registers = 0
				p->kstack[SSIZE-j] = 0;

			p->ksp = &(p->kstack[SSIZE-9]);// saved sp in PROC.ksp
		}
	}

	proc[NPROC-1].next = &proc[0]; // all PROCs form a circular list
	running = &proc[0]; // P0 is running;

	printf("init complete\n");
}

int scheduler() // scheduler() function
{ 
	running = running->next; 
}

main() // main() function
{
	init();
	while(1)
	{
		printf("P0 running\n");
		tswitch();
	}
} 
