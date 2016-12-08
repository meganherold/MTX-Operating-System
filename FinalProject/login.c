#include "ucode.c"
#include "util.c"

char *tty;
int in, out, err, file, bytes_read;
char username[64], password[64], filebuffer[1024];
char line[300], *username_from_file, *password_from_file;

void prompt_user()
{
  //1. show login to stdout
  printf("\nusername: ");
  //2. read user name from stdin
  gets( username);
  username[strlen(username)] = 0;
  //3. show passwd:
  printf("password: ");
  //4. read user passwd
  gets(password);
  password[strlen(password)] = 0;
}

int is_user_valid()
{
  //open /etc/passwd file for reading
  file = open("/etc/passwd", 0);

  //verify user name and passwd from /etc/passwd file
  while(get_line(file, line) > 0)
  {
    //tokenize account line to get username and password
    //printf("line: %s\n", line);

    username_from_file = strtok(line, ":"); //this gets the username
    password_from_file = strtok(0, ":"); //this gets the password

    if(strcmp(username, username_from_file) == 0 && strcmp(password, password_from_file) == 0)
    {
      printf("login success! welcome, %s!\n", username);
      return 1;
    }
  }
  printf("login failed :( try again.\n");
  return 0;
}

int init_user_profile()
{
  int uid, gid;
  char *home, *program;

  //change uid, gid to user's uid, gid
  gid = atoi(strtok(0, ":"));
  uid = atoi(strtok(0, ":"));
  chuid(uid, gid);

  //change cwd to user's home directory
  strtok(0, ":");
  home = strtok(0, ":");
  chdir(home);

  //close opened password file
  close(file);

  //exec to program in user's account
  program = strtok(0, ":");
  program[strlen(program)] = 0;
  printf("going to %s\n", program);
  //exec(program);
}

main(int argc, char *argv[])   // invoked by exec("login /dev/ttyxx")
{
  tty =  argv[1];

  //1. close(0); close(1); close(2); // login process may run on different terms
  close(0);
  close(1);
  close(2);

  //2. // open the tty passed in as stdin, stdout, stderr
  open(tty, O_RDONLY);    //in
  open(tty, O_WRONLY);   //out
  open(tty, O_WRONLY);   //err

  settty(tty);   // store tty string in PROC.tty[] for putc()

  // NOW we can use printf, which calls putc() to our tty

  printf("-----megan's login-----\n");

  signal(2,1);  // ignore Control-C interrupts so that
                // Control-C KILLs other procs on this tty but not the main sh

  while(1)
  {
    prompt_user();

    if(is_user_valid())
    {
      init_user_profile();
      break;
    }
  }
  exec("sh");
}
