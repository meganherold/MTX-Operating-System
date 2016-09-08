// Write your OWN gets(char s[ ]) fucntion to get a string.
// Write your OWN printf(char *fmt, ...) as in CS360

// Then, include YOUR io.c file in bc.c. 
// Do NOT use KCW's io.o in linking step.

#define MAXLEN 128



int BASE = 10;
char *table = "0123456789ABCDEF";

char *gets(char s[])
{
	char c, *t = s;
	int length = 0;

	while( (c = getc()) != '\r' && length < MAXLEN - 1)
	{
		*t++ = c;
		putc(c);
		length++;
 	}

	*t = 0;
	return s;
}

//function prints an unsigned int (u32)x (credit KC)
int printu(unsigned int x)
{
  if (x==0)
     putc('0');
  else
     rpu(x);
  putc(' ');
}

//helper function for printu(u32 x) (credit KC)
int rpu(unsigned int x)
{
  char c;
  if (x){
     c = table[x % BASE];
     rpu(x / BASE);
     putc(c);
  }
} 

//function uses putc() to print a string
int prints (char *s)
{
  char i = *s;
  while(i != 0)
  {
    i = *(s++);
    putc(i);
  }
}

//function uses putc() to print an integer (which may be negative)
int printd(int x)
{
  int neg = 0;
  BASE = 10;
  if (x == 0) //case x = 0
     putc('0');
  else if (x > 0) //case x is positive
     rpu(x);
  else if (x < 0) //case x is negative
  {
    putc('-'); 
    neg = 0 - x; //get positive version of x
    rpu(neg);
  }
  putc(' ');
}

int printo(unsigned int x) //print x in octal as 0...
{
  BASE = 8;
  putc('0');
  printu(x);
  BASE = 10;
}

int printx(unsigned int x) //print x in octal as 0x...
{
  BASE = 16;
  putc('0');
  putc('x');
  printu(x);
  BASE = 10;
}

int rpl(u32 x)
{
	char c;
	if(x)
	{
		c = table[x % BASE];
		rpl(x / BASE);
		putc(c);
	}
}

int printl(u32 x)
{
	if (x==0)
     putc('0');
  else
     rpl(x);
	putc(' ');
}

int printf(char *format, ...)
{ 
	char *copy = format;						//cp points to the fmt string
	u16 *ip = (u16 *)&format + 1;		//ip points to the first item
	u32 *up;												//for accessing long params on stack
	
	while(*copy)											//scan the format string
	{
		if(*copy != '%')
		{
			putc(*copy);

			if(*copy == '\n')
				putc('\r');
			
			copy++;
			continue;
		}

		copy++;
		switch(*copy)
		{
			case 'c' : putc(*ip);			break;
			case 's' : prints(*ip); 	break;
			case 'u' : printu(*ip);		break;
			case 'd' : printd(*ip);  	break;
			case 'x' : printx(*ip);		break;
			case 'l' : printl(*(u32 *)ip++); break;
  	}
		
		copy++;
		ip++;
	}
}

