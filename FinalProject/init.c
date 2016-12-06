//initial init.c creates only one login process on console=/dev/tty0
//your init.c must also create login processes on serial ports
// /dev/ttyS0 and /dev/ttyS1

int pid, status;
int child, tty0, ttys0, ttys1;
int stdin, stdout;

#include "ucode.c"  //<========== AS POSTED on class website

main(int argc, char *argv[])
{
  int i = 0;

  // open /dev/tty0 as 0 (READ) and 1 (WRTIE) in order to display messages
  open("/dev/tty0", 0);
  open("/dev/tty0", 1);

  // Now we can use printf, which calls putc(), which writes to stdout
  printf("megan's 460 final project, initializing...\n");

  //p2 on tty0, p3 on ttys0, and p4 on ttys1
  ttys1 = fork();
  if (!ttys1)
  {
    login(2);
  }

  ttys0 = fork();
  if (!ttys0)
  {
    login(1);
  }

  tty0 = fork();
  if (!tty0)
  {
    login(0);
  }
  
  parent();

}

int login(int terminal)
{
  if(terminal == 0)
    exec("login /dev/tty0");

  if(terminal == 1)
    exec("login /dev/ttyS0");

  if(terminal == 2)
    exec("login /dev/ttyS1");
}

int parent()
{
  while(1){
    printf("megan's init : waiting .....\n");

    pid = wait(&status);
    //printf("pid: %d\n", pid);

    if (pid == tty0)
    {
      tty0 = fork();
      if (!tty0)
        login(0);
    }
    if(pid == ttys0)
    {
      ttys0 = fork();
      if(!ttys0)
        login(1);
    }
    if(pid == ttys1)
    {
      ttys1 = fork();
      if(!ttys1)
        login(2);
    }
    else
      printf("megan's init : buried an orphan child %d\n", pid);
  }
}
