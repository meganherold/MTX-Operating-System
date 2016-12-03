/************** syscall routing table ***********/
int kcinth()
{
  u16 x, y, z, w, r;
  u16 seg, off;

  seg = running->uss; off = running->usp;

  x = get_word(seg, off+13*2);
  y = get_word(seg, off+14*2);
  z = get_word(seg, off+15*2);
  w = get_word(seg, off+16*2);

   switch(x){
       case 0 : r = running->pid;    break;
       case 1 : r = kps();           break;
       case 2 : r = chname(y);       break;
       case 3 : r = kmode();         break;
       case 4 : r = tswitch();       break;
       case 5 : r = kwait();         break;
       case 6 : r = kexit();         break;
       case 7 : r = fork();          break;
       case 8 : r = exec(y);        break;


       // FOCUS on ksin() nd ksout()
       case 9 : r = ksout(y);        break;
       case 10: r = ksin(y);         break;

       case 99: r = kexit();         break;

       default: printf("invalid syscall # : %d\n", x);

   }
   put_word(r, seg, off+2*8);
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

int chname(char *name)
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

int kmode()
{
  body();
}

int ksin(char *y)
{
  // get a line from serial port 0; write line to Umode
  char line[128], *c;
  //printf("ksin\n");
  sgetline(line);
  c = line;

  //printf("putting bytes of line %s...\n", line);
  while(*c != 0 && *c != '\n')
  {
    put_byte(*c, running->uss, y);
    c++;
    y++;
  }
  //printf("end of ksin\n");
  return 0;
}

int ksout(char *y)
{
  // get line from Umode; write line to serial port 0
  char line[128], *c;

  c = line;
  *c = get_byte(running->uss, y);   //get the first char of the line
  while(*c != 0 && *c != '\n')
  {
    c++;
    y++;
    *c = get_byte(running->uss, y);  //keep getting chars from the line
  }
  printf("line = %s\n", line);
  sputline(line);
  lock();
  return 0;
}
