// ucode.c file


char *cmd[]={"getpid", "ps", "chname", "kmode", "switch", "wait", "exit", "fork", "exec", "hop", "pipe", "pfd", "close", "read", "write", 0};

int pd[2];

int show_menu()
{
   printf("********************* Menu ***************************\n");
   printf("*  ps  chname  kmode  switch  wait  exit  fork exec  *\n");
   /*         1     2      3      4    5     6   7     8   9   */
   printf("*   hop    pipe    pfd    close    read    write     *\n");
   //          10    11  12    13   14
   printf("******************************************************\n");
}

int find_cmd(char *name)
{
   int i;   char *p;
   i = 0;   p = cmd[0];
   while (p){
         if (strcmp(p, name)==0)
            return i;
         i++;  p = cmd[i];
   }
   return(-1);
}


int getpid()
{
   return syscall(0,0,0);
}

int ps()
{
   syscall(1, 0, 0);
}

int chname()
{
    char s[64];
    printf("input new name : ");
    gets(s);
    syscall(2, s, 0);
}

int kmode()
{
    printf("kmode : enter Kmode via INT 80\n");
    printf("proc %d going K mode ....\n", getpid());
        syscall(3, 0, 0);
    printf("proc %d back from Kernel\n", getpid());
}

int kswitch()
{
    printf("proc %d enter Kernel to switch proc\n", getpid());
        syscall(4,0,0);
    printf("proc %d back from Kernel\n", getpid());
}

int wait()
{
    int child, exitValue;
    printf("proc %d enter Kernel to wait for a child to die\n", getpid());
    child = syscall(5, &exitValue, 0);
    printf("proc %d back from wait, dead child=%d", getpid(), child);
    if (child>=0)
        printf("exitValue=%d", exitValue);
    printf("\n");
    return child;
}

int exit()
{
   char exitValue;
   printf("enter an exitValue (0-9) : ");
   exitValue=(getc()&0x7F) - '0';
   printf("enter kernel to die with exitValue=%d\n",exitValue);
   _kexit(exitValue);
}

int _kexit(int exitValue)
{
  syscall(6,exitValue,0);
}

int fork()
{
  int child;
  child = syscall(7,0,0,0);
  if (child)
    printf("parent %d return form fork, child=%d\n", getpid(), child);
  else
    printf("child %d return from fork, child=%d\n", getpid(), child);
}

int exec()
{
  int r;
  char filename[32];
  printf("enter exec command : ");
  gets(filename);
  return syscall(8,filename,0,0);
  printf("exec failed\n");
}

int hop()
{
  char segment[32];
  printf("enter segment to hop to : ");
  gets(segment);
  printf("entering kmode for hop : %s\n", segment);
  return syscall(9, segment, 0, 0);
  printf("hop failed\n");
}


int pipe()
{
  printf("entering kernel to create a pipe\n");
  syscall(10, pd, 0, 0);
  printf("returned to umode, created a pipe with fd = [%d, %d]\n", pd[0], pd[1]);
}

int pfd()
{
  return syscall(11, 0, 0, 0);
}

int close()
{
  char fd;
  printf("[0 = read, 1 = write]\n");
  printf("choose a pipe descriptor to close: ");
  fd=(getc()&0x7F) - '0';
  _kclose(fd);
}

int _kclose(int fd)
{
  printf("kclosing fd %d\n", fd);
  return syscall(12, fd, 0, 0);
}

int read()
{
  char buffer[1024], command[32];
  int fd;
  int nbytes, recieved_bytes;

  printf("read [file descriptor] [nbytes] :");
  gets(command);
  sscanf(command, "%d %d", &fd, &nbytes);
  recieved_bytes = syscall(13, fd, buffer, nbytes);  //sleep here

  printf("\nprocess %d is back in umode, read %d bytes\n", getpid(), recieved_bytes);
  if(recieved_bytes > 0)
  {
    buffer[recieved_bytes] = 0;
    printf("contents: %s\n", buffer);
  }
}

int write()
{
  char buffer[1024], command[32];
  int fd;
  int nbytes, written_bytes;

  printf("write [file descriptor] [contents] :");
  gets(command);
  sscanf(command, "%d %s", &fd, &buffer);
  nbytes = strlen(buffer);

  written_bytes = syscall(14, fd, buffer, nbytes);

  printf("process %d is back in umode, wrote %d bytes\n", getpid(), written_bytes);
}

int get_segment()
{
  return syscall(15, 0, 0, 0);
}

int invalid(char *name)
{
    printf("Invalid command : %s\n", name);
}
