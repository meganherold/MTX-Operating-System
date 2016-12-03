#include "ucode.c"
main()
{
  char name[64]; int cmd;
  int pid;
  while(1){
       pid = getpid();
       printf("==============================================\n");
       printf("Das ist prozess %d im Umode segment=%x\n", pid, getcs());
       show_menu();
       printf("Command ? ");
       gets(name);
       if (name[0]==0)
           continue;

       cmd = find_cmd(name);
       switch(cmd){
           case 0 : getpid();  break;
           case 1 : ps();       break;
           case 2 : chname();   break;
           case 3 : kmode();    break;
           case 4 : kswitch();  break;
           case 5 : mywait();   break;
           case 6 : myexit();   break;
           case 7 : ufork();    break;
           case 8 : myexec("/bin/u1"); break;
           case 9 : sin(); break;
           case 10: sout(); break;
          default: invalid(name); break;
       }
  }
}
