#include "type.h"

void printList(char* string, PROC *list)
{
  PROC *current_proc = list;

  printf("-------------------------- %s --------------------------------\n", string);
  while(current_proc != 0)
  {
    printf("%d -> ", current_proc->pid);
    current_proc = current_proc->next;
  }

  printf("NULL\n");
}

int printQueue(char *name, PROC *p)
{
   printf("-------------------------- %s --------------------------------\n", name);
   while(p){
     printf("%d -> ", p->pid);
       p = p->next;
   }
   printf("NULL\n");
}



int put_proc(PROC **list, PROC *p)   // return a PROC to freeList
{
    p->status = FREE;
    p->next = *list;
    *list = p;
}

PROC* get_proc(PROC **queue) //allocate a proc from the given queue
{
  PROC *current = *queue;

  //find the first FREE process
  while(current != 0)
  {
    if(current->status == FREE)
    {
      *queue = current->next;
      return current;
    }

    current = current->next;
  }

  printf("get_proc returning null.\n");
  return 0;
}

//enters p into a queue by priority
//if there are same priority, use first in first out
//low number = first priority
int enqueue(PROC **queue, PROC *p)
{
	PROC *previous = 0;
	PROC *next = *queue;

	//find a good place for this p
	while(next != 0)
	{
		previous = next;
		next = next->next;
	}

	if(previous == 0)//if p is in front
	{
		p->next = next;
		*queue = p;
	}

	//if p is in the middle or last
	else
	{
		previous->next = p;
		p->next = next;
	}

}

//remove and return the first PROC from the queue.
//running = dequeue(&readyQueue)
PROC* dequeue(PROC **queue)
{
  PROC* first_proc = *queue;

  if(*queue == 0)
  {
    printf("dequeue: empty queue. returning null.\n");
    return 0;
  }

  *queue = first_proc->next;
  return first_proc;
}
