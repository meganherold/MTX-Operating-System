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
  int has_pipe = 0, pid;
  int local_pipe[2];
  char *head = "", *tail = "";

  if(pipe_descriptor) //if a pipe was passed in, we're WRITER on pipe pd
  {
    close(pipe_descriptor[0]);
    dup2(pipe_descriptor[1], 1);
    close(pipe_descriptor[1]);
  }

  has_pipe = scan(commandline, &head, &tail);
  //printf("do_pipe: head=%s tail=%s\n", head, tail);

  if(has_pipe)
  {
    //create a local pipe
    pipe(local_pipe);
    pid = fork();

    if(pid) //parent as reader
    {
      close(local_pipe[1]);
      dup2(local_pipe[0], 0);
      close(local_pipe[0]);
      do_command(tail);
    }

    else //we have more piping to do! recursive call.
      do_pipe(head, local_pipe);
  }

  else //no pipe here, just a regular executable thing
    do_command(commandline);

}

int do_command(char *commandline)
{
  int num_tokens = 0, i, redirect_location, in, out;
  char line_copy[300], new_command[300], *line_tokens[10], *command;
  char* redirect = "";

  //printf("megan's do_command\n");

  //first, break commandline up into tokens
  strcpy(line_copy, commandline);

  line_tokens[num_tokens] = strtok(line_copy, " ");
  while(line_tokens[num_tokens] != 0 && num_tokens < 10)
  {
    //printf("sh: token = %s\n", line_tokens[num_tokens]);
    num_tokens++;
    line_tokens[num_tokens] = strtok(0, " "); //line_tokens will end in null
  }

  command = line_tokens[0];

  //look for IO redirection symbols
  for(i = 0; i < num_tokens; i++)
  {
    //we know we'll only ever have one per do_command call, because of do_pipe
    if(strcmp(line_tokens[i], "<") == 0   //take input from a thing
    || strcmp(line_tokens[i], ">") == 0   // write output to a thing
    || strcmp(line_tokens[i], ">>") == 0) //append output to a thing
    {
      redirect_location = i;
      redirect = line_tokens[i];
      printf("redirect found: %s at %d\n", redirect, redirect_location);
      break;
    }
  }

  //do IO redirections
  if(strcmp(redirect, "<") == 0) //take input from a thing
  {
    //printf("attempting to open %s for reading.\n", line_tokens[redirect_location + 1]);
    close(0);   //close stdin
    in = open(line_tokens[redirect_location + 1], O_RDONLY); //open the thing as 0

    if(in < 0)
    {
      printf("failed to open file %s for reading.\n", line_tokens[redirect_location + 1]);
      exit(0);
    }

    printf("opened %s for reading.\n", line_tokens[redirect_location + 1]);
  }

  else if(strcmp(redirect, ">") == 0) //write output to a thing
  {
    //printf("attempting to open %s for writing.\n", line_tokens[redirect_location + 1]);
    close(1);   //close stdout

    //open for writing OR create the thing if it doesn't exist
    out = open(line_tokens[redirect_location + 1], O_WRONLY | O_CREAT);

    if(out < 0)
    {
      printf("failed to open/create file %s for writing.\n", line_tokens[redirect_location + 1]);
      exit(0);
    }
    printf("opened %s for writing.\n", line_tokens[redirect_location + 1]);
  }

  else if(strcmp(redirect, ">>") == 0) //append output to a thing
  {
    //printf("attempting to open %s for writing.\n", line_tokens[redirect_location + 1]);
    close(1);   //close stdout

    //open for writing OR appending OR create the thing if it doesn't exist
    out = open(line_tokens[redirect_location + 1], O_WRONLY | O_APPEND | O_CREAT);

    if(out < 0)
    {
      printf("failed to open/create file %s for appending.\n", line_tokens[redirect_location + 1]);
      exit(0);
    }
    printf("opened %s for appending.\n", line_tokens[redirect_location + 1]);
  }

  //now build a command line without redirection symbols
  //(because we just took care of those) and send it to exec
  strcpy(new_command, line_tokens[0]);
  for(i = 1; i < num_tokens; i++)
  {
    if(i != redirect_location)
    {
      strcat(new_command, " ");
      strcat(new_command, line_tokens[i]);
    }
  }

  //printf("do_command: excecuting %s\n", new_command);
  exec(new_command);
  return 1;
}

int scan(char *commandline, char **head, char **tail)
{
  //divide command line into head, tail by rightmost pipe symbol
  char *pointer = commandline;

  //printf("scanning...\n");
  //set pointer to the end of the command line
  while(*pointer)
    pointer++;

  //now go backwards looking for the first "|"
  while(pointer != commandline && *pointer != '|')
    pointer--;

  //did we find any | ?
  if(pointer == commandline)
  {
    //printf("scan: no | were found.\n");
    return 0; //if we've reached the beginning, no | were found.
  }

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
  //printf("hello, this is sh.c!\n");

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
    //printf("bottom of while loop\n");
  }
}
