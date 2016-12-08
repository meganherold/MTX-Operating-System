#include "ucode.c"
#include "util.c"

//usage: grep pattern [filename optional]
//if filename is not provided, use stdin instead

int main(int argc, char *argv[])
{
  int file_descriptor, bytes_read, line_number, i;
  char buffer[300], *pointer;

  if(argc == 2) //grep and pattern, using stdin
    file_descriptor = 0;

  else if(argc == 3)
  {
    file_descriptor = open(argv[2], O_RDONLY);
    if(file_descriptor < 0)
    {
      printf("failed to open file %s for reading.\n");
      return 0;
    }
  }

  else  //user obviously doesn't know how to grep
  {
    printf("grep searches for patterns in a file or stdin.\n");
    printf("usage: grep pattern [optional filename]\n");
    return 0;
  }

  //get one line at a time?
  bytes_read = nget_line(file_descriptor, buffer);
  line_number = 1;
  while(bytes_read > 0)
  {
    pointer = buffer;

    //move through the line and look for the pattern
    while(*pointer != '\n')
    {
      if(strncmp(pointer, argv[1], strlen(argv[1])) == 0)
      {
        printf("%d: %s\r", line_number, buffer);
        break;
      }
      pointer++;
    }

    //clear out the buffer so we can use it again
    for(i = 0; i < 300; i++)
    {
      buffer[i] = 0;
    }

    //get the next line from the file
    bytes_read = nget_line(file_descriptor, buffer);
    line_number++;
  }

  //all done
  close(file_descriptor);
  return 0;
}
