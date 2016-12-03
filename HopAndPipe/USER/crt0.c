//main0
int main0(char *s)
{
  int argc = 0, i;
  char *argv[32];
  char *token;
  char *cp = s;
  
  printf("megan's main0: s=%s\n", s);
  
  while (*cp != 0){
       while (*cp == ' ') *cp++ = 0;        
       if (*cp != 0)
           argv[argc++] = cp;         
       while (*cp != ' ' && *cp != 0) cp++;                  
       if (*cp != 0)   
           *cp = 0;                   
       else 
            break; 
       cp++;
  }
  argv[argc] = 0;
  
  printf("megan's main0: argc = %d\n", argc);
  main(argc, argv);
}
