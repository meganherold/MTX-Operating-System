// serial.c file for SERIAL LAB ASSIGNEMNT
/**************** CONSTANTS ***********************/
#define BUFLEN      64
#define NULLCHAR     0

#define NR_STTY      2    /* number of serial ports */

/* offset from serial ports base */
#define DATA         0   /* Data reg for Rx, Tx   */
#define DIVL         0   /* When used as divisor  */
#define DIVH         1   /* to generate baud rate */
#define IER          1   /* Interrupt Enable reg  */
#define IIR          2   /* Interrupt ID rer      */
#define LCR          3   /* Line Control reg      */
#define MCR          4   /* Modem Control reg     */
#define LSR          5   /* Line Status reg       */
#define MSR          6   /* Modem Status reg      */

/**** The serial terminal data structure ****/

//each serial port is represented by an stty structure
struct stty {
   /* input buffer */
   char inbuf[BUFLEN]; //do_rx puts lines here
   int inhead, intail;
   struct semaphore inchars;

   /* output buffer */
   char outbuf[BUFLEN];
   int outhead, outtail;
   struct semaphore outroom;
   int tx_on;

   /* Control section */
   char echo;   /* echo inputs */
   char ison;   /* on or off */
   char erase, kill, intr, quit, x_on, x_off, eof;

   /* I/O port base address */
   int port;
} stty[NR_STTY];


/********  bgetc()/bputc() by polling *********/
int bputc(int port, int c)
{
    while ((in_byte(port+LSR) & 0x20) == 0);
    out_byte(port+DATA, c);
}

int bgetc(int port)
{
    while ((in_byte(port+LSR) & 0x01) == 0);
    return (in_byte(port+DATA) & 0x7F);
}

int enable_irq(u8 irq_nr)
{
   out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));
}


/************ serial ports initialization ***************/
char *p = "\n\rSerial Port Ready\n\r\007";

int sinit()
{
  int i;
  struct stty *t;
  char *q;

  /* initialize stty[] and serial ports */
  for (i = 0; i < NR_STTY; i++)
  {
    q = p;

    printf("sinit : port #%d\n",i);

      t = &stty[i];

      /* initialize data structures and pointers */
      if (i==0)
          t->port = 0x3F8;    /* COM1 base address */
      else
          t->port = 0x2F8;    /* COM2 base address */

      t->inchars.value  = 0;  t->inchars.queue = 0;
      //got rid of some things here that maybe didn't belong or maybe did
      t->outroom.value = BUFLEN; t->outroom.queue = 0;

      t->inhead = t->intail = 0;
      t->outhead =t->outtail = 0;

      t->tx_on = 0;

      // initialize control chars; NOT used in MTX but show how anyway
      t->ison = t->echo = 1;   /* is on and echoing */
      t->erase = '\b';
      t->kill  = '@';
      t->intr  = (char)0177;  /* del */
      t->quit  = (char)034;   /* control-C */
      t->x_on  = (char)021;   /* control-Q */
      t->x_off = (char)023;   /* control-S */
      t->eof   = (char)004;   /* control-D */

    lock();  // CLI; no interrupts

      //out_byte(t->port+MCR,  0x09);  /* IRQ4 on, DTR on */
      out_byte(t->port+IER,  0x00);  /* disable serial port interrupts */

      out_byte(t->port+LCR,  0x80);  /* ready to use 3f9,3f8 as divisor */
      out_byte(t->port+DIVH, 0x00);
      out_byte(t->port+DIVL, 12);    /* divisor = 12 ===> 9600 bauds */

      /******** term 9600 /dev/ttyS0: 8 bits/char, no parity *************/
      out_byte(t->port+LCR, 0x03);

      /*******************************************************************
        Writing to 3fc ModemControl tells modem : DTR, then RTS ==>
        let modem respond as a DCE.  Here we must let the (crossed)
        cable tell the TVI terminal that the "DCE" has DSR and CTS.
        So we turn the port's DTR and RTS on.
      ********************************************************************/

      out_byte(t->port+MCR, 0x0B);  /* 1011 ==> IRQ4, RTS, DTR on   */
      out_byte(t->port+IER, 0x01);  /* Enable Rx interrupt, Tx off */

    unlock();

    enable_irq(4-i);  // COM1: IRQ4; COM2: IRQ3

    /* show greeting message */
    while (*q){
      bputc(t->port, *q);
      q++;
    }

    //clear out inbuf
    for(i = 0; i < BUFLEN; i++)
      t->inbuf[i] = '\0';
  }
}

//======================== LOWER HALF ROUTINES ===============================
int s0handler()
{
  shandler(0);
}
int s1handler()
{
  shandler(1);
}

//serial port interrupt handler //0 for COM1, 1 for COM2
int shandler(int port)
{
   struct stty *t;
   int IntID, LineStatus, ModemStatus, intType, c;

   t = &stty[port];            // IRQ 4 interrupt : COM1 = stty[0] */

   //read InterruptID register to determine interrupt cause
   IntID = in_byte(t->port+IIR);

   //read line and modem status to check for errors
   LineStatus = in_byte(t->port+LSR);
   ModemStatus = in_byte(t->port+MSR);

   intType = IntID & 7;     /* mask out all except the lowest 3 bits */

   //handle the interrupt!
   switch(intType){
      case 6 : do_errors(t);  break;   // 110 = errors
      case 4 : do_rx(t);      break;   // 100 = rx interrupt - input
      case 2 : do_tx(t);      break;   // 010 = tx interrupt - output
      case 0 : do_modem(t);   break;   // 000 = modem interrupt
   }
   out_byte(0x20, 0x20);     /* reenable the 8259 controller */
}

int do_errors()
{ printf("assume no error\n"); }

int do_modem()
{  printf("don't have a modem\n"); }


/* The following show how to enable and disable Tx interrupts */

enable_tx(struct stty *t)
{
  lock();
  out_byte(t->port+IER, 0x03);   /* 0011 ==> both tx and rx on */
  t->tx_on = 1;
  unlock();
}

disable_tx(struct stty *t)
{
  lock();
  out_byte(t->port+IER, 0x01);   /* 0001 ==> tx off, rx on */
  t->tx_on = 0;
  unlock();
}

// ========================== Input Driver =========================

//take input from a serial port tty
int do_rx(struct stty *tty)   /* interrupts already disabled */
{
  int c;
  c = in_byte(tty->port) & 0x7F;  /* read the ASCII char from port */

  //this happens on every character input from the terminal
  //printf("port %x interrupt:c=%c ", tty->port, c,c);
  //printf("%c\n", c);

  ///if inbuf is full
  if(tty->inchars.value >= BUFLEN)
  {
    out_byte(tty, 0x7);
    return;
  }

  tty->inbuf[tty->inhead++] = c;  //put the char into the buffer
  //printf("rx: %c at %d in inbuf\n", tty->inbuf[tty->inhead - 1], tty->inhead - 1);
  tty->inhead %= BUFLEN;          //advance the buffer

  if(c=='\r')
  {
    bputc(tty->port, '\n');
  }

  bputc(tty->port, c);
  V(&tty->inchars);               //we're done, increment semaphore
}

//get a char from serial port
int sgetc(struct stty *tty)
{
  char c;
  //printf("sgetc\n");
  P(&tty->inchars);   //ask to use this port. wait if there's nothing yet.
  lock();             //disable interrupts

  c = tty->inbuf[tty->intail++];  //get the char from the port's inbuf
  //printf("sgetc: %c at %d in inbuf\n", tty->inbuf[tty->intail - 1], tty->intail - 1);
  printf("%c", c);
  tty->intail %= BUFLEN;          //incrememnt the buffer

  unlock();           //enable interrupts again
  //printf("sgetc: returning %c\n", c);
  return(c);
}

//get a whole line from the serial port
int sgetline(char *line)
{
   struct stty *tty = &stty[0];
   char c;
   int i = 0;

   //printf("sgetline\n");
   //P(&tty->outroom);
   lock();
   //printf("getting chars...\n");

   c = sgetc(tty);
   while(c != '\r')
   {
     line[i] = c;
     //printf("line = %s\n", line);
     i++;
     c = sgetc(tty);
     //printf("sgetline again\n");
   }
   line[i] = 0;
   //printf("all chars gotten.\n");
   V(&tty->outroom);
   unlock();
   //printf("end of sgetline\n");
   return strlen(line);
}


/*********************************************************************/
int do_tx(struct stty *tty)
{
  int c;
  printf("tx interrupt "); //happens for every letter of "serial line from umode"

  if (tty->outroom.value == BUFLEN){ // nothing to do
     disable_tx(tty);                 // turn off tx interrupt
     return;
  }

  //output the next char
  c = tty->outbuf[tty->outtail++]; //get the char
  tty->outtail %= BUFLEN;               //increment spot in outbuf
  out_byte(tty->port, c);               //write the char to the port
  V(&tty->outroom);                    //we're done, increment semaphore
}


int sputc(struct stty *tty, int c)
{
  P(&tty->outroom);      //wait for space available in outbuf[]
  lock();                 //disable interrupts

  printf("sputc: c = %c outhead = %d\n", c, tty->outhead);
  tty->outbuf[tty->outhead++] = c;  //put c in serial port's outbuf
  tty->outhead %= BUFLEN;           //increment spot in outbuf
  tty->outhead += 1;

  if(!tty->tx_on)         //if serial port's tx interrupt isn't on
    enable_tx(tty);       //turn it on

  unlock();
  return;
}

int sputline(char *line)
{
  struct stty *tty = &stty[0];

  printf("sputc line = %s\n", line);
  P(&tty->outroom);
  lock();

  /*while(*line != NULL)
  {
    printf("sputc: *line = %c\n", *line);
    sputc(tty, *line);
    line++;
  } */

  while (*line != NULL){
    bputc(tty->port, *line);
    line++;
  }

  unlock();
  V(&tty->outroom);
}
