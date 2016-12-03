
PIPE pipes[1];
OFT ends[2];

int pipe(int *pd_in_umode)
{
  PIPE *pipe;
  OFT *readOFT, *writeOFT;

  printf("creating a pipe\n");

  //1. allocate a a pipe object
  pipe = &pipes[0];
  pipe->head = 0;
  pipe->tail = 0;
  pipe->data = 0;
  pipe->room = PSIZE;
  pipe->nreaders = 1;
  pipe->nwriters = 1;

  //2. allocate 2 OFTs
  readOFT = &ends[0];
  readOFT->mode = READ_PIPE;
  readOFT->refcount = 1;
  readOFT->pipe_ptr = pipe;

  writeOFT = &ends[1];
  writeOFT->mode = WRITE_PIPE;
  writeOFT->refcount = 1;
  writeOFT->pipe_ptr = pipe;

  //3. assign OFTs to the fd array
  running->fd[0] = readOFT;
  running->fd[1] = writeOFT;

  //4. write 0 to pd[0] and 1 to pd[1] in uspace
  put_word(0, running->uss, pd_in_umode);
  put_word(1, running->uss, pd_in_umode + 1);

  //5. return 0 for ok
  printf("created you a pipe! file descriptors = [0, 1]\n");
  return 0;
}

int pfd()
{
  int i;

  printf("********* process %d's open file descriptors *********\n", running->pid);

  for(i = 0; i < NFD; i++)
  {
    if(running->fd[i] != 0)
    {
      if(running->fd[i]->mode == READ_PIPE)
        printf("  %d           read mode          refcount = %d\n", i, running->fd[i]->refcount);

      else if(running->fd[i]->mode == WRITE_PIPE)
        printf("  %d           write mode         refcount = %d\n", i, running->fd[i]->refcount);
    }
  }

  return 1;
}

int close(int fd)
{
  //a process can be a reader or a writer, but not both
  int i;
  PIPE *nullpipe;

  //1. validate to make sure it's an opened file descriptor
  if(fd < 0 || fd > NFD || running->fd[fd] == 0)
  {
    printf("that's not a valid file descriptor!\n");
    return -1;
  }

  //2. decrement refcount
  running->fd[fd]->refcount--;

  //3. if closing writer...
  if(running->fd[fd]->mode == WRITE_PIPE)
  {
    running->fd[fd]->pipe_ptr->nwriters--;

    //is this the end of the pipe?
    if(running->fd[fd]->pipe_ptr->nreaders == 0
    && running->fd[fd]->pipe_ptr->nwriters == 0)
        nullpipe = &pipes[0];

    kwakeup(&(running->fd[fd]->pipe_ptr)->data);
    //apparently this ^ wakes up the readers? idk why
  }

  //4. if closing reader...
  if(running->fd[fd]->mode == READ_PIPE)
  {
    running->fd[fd]->pipe_ptr->nreaders--;

    //is this the end of the pipe?
    if(running->fd[fd]->pipe_ptr->nreaders == 0
    && running->fd[fd]->pipe_ptr->nwriters == 0)
        nullpipe = &pipes[0];

    kwakeup(&(running->fd[fd]->pipe_ptr)->room);
    //apparently this ^ wakes up the writers? idk this one either
  }

  //5. clear caller's fd and return
  running->fd[fd] = 0;
  return 1;
}

void display_pipe(PIPE *pipe)
{
    char *cp = pipe->buf;
    cp[strlen(pipe->buf)] = 0;

    printf("******************************************************\n");
    printf("    nreaders: %d               nwriters: %d           \n", pipe->nreaders, pipe->nwriters);
    printf("        data: %d                   room: %d           \n", pipe->data, pipe->room);
    printf("    contents: %s                                      \n", cp);
    printf("******************************************************\n");
}

int read(int fd, char *buf, int nbytes)
{
  int bytes_read = 0;
  OFT *openend;
  PIPE *pipe;

  if(nbytes <= 0) //nothing to read :(
  {
    printf("can't read 0 bytes\n");
    return 0;
  }

  //validate fd
  if(fd < 0 || fd > NFD || running->fd[fd] == 0)
  {
    printf("that's not a valid file descriptor!\n");
    return -1;
  }

  //get the things
  openend = running->fd[fd];
  pipe = openend->pipe_ptr;

  while(nbytes > 0)
  {
    printf("               pipe before reading\n");
    display_pipe(pipe);

    while(pipe->data > 0) //while there is data to be read
    {
      //read a byte from the pipe(kernel space) to the buf passed in (user space)
      put_byte(pipe->buf[pipe->tail], running->uss, buf);   //byte, segment, offset
      pipe->buf[pipe->tail] = 0;  //remove the byte we just read from the pipe
      nbytes--;       bytes_read++;
      pipe->data--;   pipe->room++;
      pipe->tail++;

      if(nbytes = 0) //we read all the bytes
        break;
    }

    if(bytes_read > 0) //we read some things
    {
      printf("               pipe after reading\n");
      display_pipe(pipe);

      kwakeup(&pipe->room);
      return bytes_read;
    }

    if(pipe->nwriters > 0)  //read no data and there are still writers
    {
      printf("               pipe before reader sleeps\n");
      display_pipe(pipe);

      kwakeup(&pipe->room);  //wakeup the writers
      ksleep(&pipe->data);   //wait for them to write data
    }

    //I guess try reading again?
    while(pipe->data > 0) //while there is data to be read
    {
      //read a byte from the pipe(kernel space) to the buf passed in (user space)
      put_byte(pipe->buf[pipe->tail], running->uss, buf + bytes_read);   //byte, segment, offset
      pipe->buf[pipe->tail] = 0;  //remove the byte we just read from the pipe
      nbytes--;       bytes_read++;
      pipe->data--;   pipe->room++;
      pipe->tail++;

      if(nbytes = 0) //we read all the bytes
        break;
    }

    if(bytes_read > 0) //we read some things
    {
      printf("               pipe after reading\n");
      display_pipe(pipe);

      kwakeup(&pipe->room);
      return bytes_read;
    }

    //this pipe has no writers and no data
    printf("this pipe has no writers and no data.\n");
    return 0;
  }
}

int write(int fd, char *buf, int nbytes)
{
  int bytes_written = 0;
  OFT *openend;
  PIPE *pipe;

  if(nbytes <= 0) //nothing to write :(
    return 0;

  //validate the file descriptor
  if(fd < 0 || fd > NFD || running->fd[fd] == 0)
  {
    printf("that's not a valid file descriptor!\n");
    return -1;
  }

  //get the things
  openend = running->fd[fd];
  pipe = openend->pipe_ptr;

  while(nbytes > 0)
  {
    //printf("write: pipe->nreaders = %d\n", pipe->nreaders);
    if(pipe->nreaders == 0)  //there are no readers
    {
      printf("this is a broken pipe :(\n");
      kexit(5);  //I chose 5 for no real reason
    }

    printf("               pipe before writing\n");
    display_pipe(pipe);

    while(pipe->room > 0) //while there is space to be written to
    {
      //write a byte from the buf passed in (uspace) to pipe(kernel space)
      pipe->buf[pipe->head] = get_byte(running->uss, buf + bytes_written); //segment, offset
      nbytes--;       bytes_written++;
      pipe->data++;   pipe->room--;
      pipe->head++;

      if(nbytes == 0) //we wrote all the bytes_written
        break;
    }

    kwakeup(&pipe->data); //wake up the readers
    if(nbytes == 0)
    {
      //not getting here
      printf("wrote %d bytes to pipe!\n", bytes_written);

      printf("               pipe after writing\n");
      display_pipe(pipe);

      return bytes_written;     //all done writing
    }

    //if we've gotten here, there's still data to write, but no room
    printf("               pipe before sleeping\n");
    display_pipe(pipe);
    ksleep(&pipe->room);
  }

  return 1;
}
