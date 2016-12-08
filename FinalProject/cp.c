#include "ucode.c"

int main(int argc, char *argv[])
{
  //usage: cp source destination

  int source, destination, bytes_read;
  char buffer[300];

  //open the source and destination files
  source = open(argv[1], O_RDONLY);
  destination = open(argv[2], O_WRONLY);

  //check to see if they were both opened successfully
  if(source < 0)
  {
    printf("failed to open the source file %s\n", argv[1]);
    return 0;
  }

  if(destination < 0)
  {
    printf("failed to open the destination file %s\n", argv[2]);
    return 0;
  }

  //read from source into buffer, write from buffer into destination
  bytes_read = read(source, buffer, 300);
  while(bytes_read > 0 )
  {
    write(destination, buffer, bytes_read);
    bytes_read = read(source, buffer, 300);
  }

  //all done! close the files
  close(source);
  close(destination);

}
