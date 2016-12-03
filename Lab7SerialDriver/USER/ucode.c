
char *cmd[]={"getpid", "ps", "chname", "kmode", "switch", "wait", "eixt",
             "fork", "exec", "sin", "sout", 0};

int show_menu()
{
   printf("************************* Menu ***********************\n");
   printf("* ps chname kmode switch wait exit fork exec sin sout *\n");
   /*        1    2      3     4     5     6   7    8    9   10    */
   printf("******************************************************\n");
}

int find_cmd(name) char *name;
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
        syscall(3, 0, 0);
    printf("TASK %d back from Kernel\n", getpid());
}

int kswitch()
{
    printf("enter Kernel to switch task\n");
        syscall(4,0,0);
    printf("back again from Kerenl\n");
}

int mywait()
{
    int child;
    printf("enter Kernel to wait for a child to die\n");
    child = syscall(5,0,0);
    printf("back from wait, dead child = %d\n", child);
}

int myexit()
{
   printf("enter kernel to die\n");
   syscall(6,0,0);
}

int ufork()
{
   int child;
   printf("enter Kernel to fork an IDENTICAL child task\n");
       child = syscall(7, 0, 0);
   printf("back from Kernel, child_pid = %d\n", child);
}

int sout()
{
  syscall(9, "serial line from Umode\r\n");
}

int sin()
{
  char uline[64];
  syscall(10, uline, 0);
  printf("uline = %s\n", uline);
}

int invalid(char *name)
{
  printf("Invalid command : %s\n",name);
}

int myexec(char *file)
{
   printf("exec syscall to change Umode image\n");
         syscall(8, file, 0);
   printf("should not see this at all\n");
}
