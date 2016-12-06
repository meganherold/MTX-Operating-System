

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
    return 0;
  else
    return 1;
}
