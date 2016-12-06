#include "ucode.c"

//IO redirection is based on manipulations of file descriptors

int is_built_in(char* command, char* arg1)
{
  if(strcmp(command, "logout") == 0)
  {
    printf("logging you out now. goodbye.\n");
    exit(0);
  }

  else if(strcmp(command, "cd") == 0)
  {
    if(arg1 != 0)
      chdir(arg1);
    else
      chdir("/");
  }

  else if (strcmp(command, "pwd") == 0)
    pwd();

  else
    return 0;
}

int do_pipe(char *commandline, int *pipe_descriptor)
{
  int has_pipe = 0;
  char *head, *tail;

  if(pipe_descriptor) //if a pipe was passed in, we're WRITER on pipe pd
  {
    close(pipe_descriptor[0]);
    dup2(pipe_descriptor[1], 1);
    close(pipe_descriptor[1]);
  }

  has_pipe = scan(commandline, &head, &tail);
  printf("do_pipe: head=%s tail=%s\n", head, tail);

  if(has_pipe)
  {
    
  }

}

int scan(char *commandline, char **head, char **tail)
{
  //divide command line into head, tail by rightmost pipe symbol
  char *pointer = commandline;

  //set pointer to the end of the command line
  while(*pointer)
    pointer++;

  //now go backwards looking for the first "|"
  while(pointer != commandline && *pointer != '|')
    pointer--;

  //did we find any | ?
  if(pointer == commandline)
    return 0; //if we've reached the beginning, no | were found.

  //we found one! there's a pipe here
  *pointer = 0;          //turn the | into NULL so that we can set commandline to head
  *head = commandline;

  pointer++; //move past the new null
  while(*pointer == ' ') //and past any spaces
    pointer++;

  *tail = pointer;
  //printf("scan: head=%s tail=%s\n", *head, *tail);
  return 1;
}


int main(int argc, char *argv[])
{
  int num_tokens = 0, pid, status;
  char line[300], line_copy[300], *line_tokens[10], *command;
  printf("hello, this is sh.c!\n");

  while(1)
  {
    //get a command line; eg. cmdline = cmd | cmd2 | ... | cmdline
    printf("command: ");
    gets(line);
    line[strlen(line)] = 0;
    //printf("command: %s\n", line);

    //get cmd token from command line
    strcpy(line_copy, line);
    //printf("copy: %s\n", line_copy);

    line_tokens[num_tokens] = strtok(line_copy, " ");
    while(line_tokens[num_tokens] != 0 && num_tokens < 10)
    {
      //printf("sh: token = %s\n", line_tokens[num_tokens]);
      num_tokens++;
      line_tokens[num_tokens] = strtok(0, " "); //line_tokens will end in null
    }

    command = line_tokens[0];

    if(!is_built_in(command, line_tokens[1]))
    {
      //must be an executable. fork a child
      pid = fork();
      if(pid)
        pid = wait(&status);
      else
        do_pipe(line, 0);
    }

  }
}
