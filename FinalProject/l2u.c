#include "ucode.c"
#include "util.c"

int main(int argc, char *argv[])
{
  int in, out, bytes_read;
  char character;

  //if no arguments, read from stdin and write to stdout
  if(argc == 1)
  {
    in = dup(0);
    out = dup(1);
  }

  //if 2 arguments, read from argv[1] write to argv[2]
  else if(argc == 3)
  {
    in = open(argv[1], O_RDONLY);
    out = open(argv[2], O_WRONLY);

    if(in < 0)
    {
      printf("failed to open file %s for reading.\n");
      return 0;
    }

    if(out < 0)
    {
      printf("failed to open file %s for writing.\n");
      return 0;
    }
  }

  //read from in, convert to upper, write to out
  bytes_read = read(in, &character, 1);
  while(bytes_read > 0)
  {
    //one character at a time
    if(character >= 97 && character <= 122)
      character = character - 32;

    write(out, &character, 1);
    bytes_read = read(in, &character, 1);
  }

  close(in);
  close(out);

  return 0;
}
