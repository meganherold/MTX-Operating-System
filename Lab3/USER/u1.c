#include "ucode.c"
#include <stdio.h>

int color;

/*int main()
{
  printf("I am in Umode segment = %s\n", getcs());
  while(1);
} */

int main()
{ 
  char name[64]; 
  int pid, cmd;

  while(1){
    pid = getpid();
    color = 0x0C;
       
    printf("------------------------------megan-----------------------------------\n");
    printf("I am proc %d in U mode: running segment=%x\n",getpid(), getcs());
    show_menu();
    printf("Command ? ");
    gets(name);
    
    if (name[0]==0) 
        continue;

    cmd = find_cmd(name);
    switch(cmd){
           case 0 : getpid();   break;
           case 1 : ps();       break;
           case 2 : chname();   break;
           case 3 : kfork();    break;
           case 4 : kswitch();  break;
           case 5 : wait();     break;
           case 6 : exit();     break;

           default: invalid(name); break;
    }
  }
} 
