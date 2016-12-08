#include "ucode.c"

int main (int argc, char *argv[])
{
  char character;
  int file_descriptor;
  printf("megan's cute cat meow ^.^\n");

  if(argc > 1) //we're reading from a file
  {
    file_descriptor = open(argv[1], O_RDONLY);
    if(file_descriptor < 0)
    {
      printf("cat: could not open the file %s\n", argv[1]);
      exit(0);
    }
  }
  else //we're reading from stdin
    file_descriptor = 0;

  //printf("file_descriptor = %d\n", file_descriptor);

  //read until the end of the file/stdin
  //IF STDIN, CTRL+D BREAKS YOU OUT OF THIS LOOP
  while(read(file_descriptor, &character, 1) > 0)
  {
    putc(character);

    //if we're reading from a file and we find an \n
    if(file_descriptor != 0 && character == '\n')
      putc('\r'); //write a \r to produce the right visual effects

    //if we're reading from stdin and we find an \r
    if(file_descriptor == 0 && character == '\r')
    {
      putc('\n');   //write a \n and a \r
      putc('\r');   //why?? idk
    }
  }

  //cool, we're all done now
  close(file_descriptor);
  //printf("file closed.\n");
  return 0;
}
