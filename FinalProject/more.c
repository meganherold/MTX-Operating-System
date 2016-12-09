#include "ucode.c"
#include "util.c"

int file_descriptor;

int print_page()
{
  int bytes_read, i = 0, j = 0;
  char buffer[200];

  bytes_read = get_line(file_descriptor, buffer);
  while(i < 24 && bytes_read > 0)
  {
    printf("%s\r", buffer);

    //clear out the buffer so we can use it again
    for(j = 0; j < 200; j++)
    {
      buffer[j] = 0;
    }

    //get the next line and keep going
    bytes_read = get_line(file_descriptor, buffer);
    i++;
  }

  return bytes_read;
}

int main(int argc, char* argv[])
{
  int bytes_read, line_number, i = 0;
  char buffer[200], user_input;

  //open the file we'll be reading (or stdin)
  if(argc == 1)
    file_descriptor = dup(0);

  if(argc == 2)
  {
    file_descriptor = open(argv[1], O_RDONLY);
    if(file_descriptor < 0)
    {
      printf("failed to open file %s for reading.\n");
      return 0;
    }
  }

  //first, write one page worth of lines to the screen
  print_page();

  //then, write another line every '\n' or page every ' '
  bytes_read = get_line(file_descriptor, buffer);
  while(bytes_read > 0)
  {
    user_input = getc();
    if(user_input == ' ') //print a whole page
      bytes_read = print_page();

    else                  //print one line
    {
      printf("%s\r", buffer);

      //clear out the buffer so we can use it again
      for(i = 0; i < 200; i++)
      {
        buffer[i] = 0;
      }

      bytes_read = get_line(file_descriptor, buffer);
    }
  }

  //close it up
  close(file_descriptor);
  return 0;
}
