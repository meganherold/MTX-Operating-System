int get_line(int fd, char buffer[])
{
  int n = 1, i = 0;
  char character = 0;

  while(character != '\n' && n > 0)
  {
    n = read(fd, &character, 1);
    buffer[i] = character;
    i++;
  }

  if(n <= 0)
    n = 0;

  return n;
}

//this is all wrong
void clear_buffer(char* buffer, int length)
{
  int i;
  for(i = 0; i < length; i++)
  {
    *buffer = 0;
  }
}
