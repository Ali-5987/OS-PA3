#include <stdio.h>
#include <unistd.h>

int
main ()
{
  
  printf ("Hello, World!\n");
  int pid = fork ();
  if (pid == 0) {
    // Child process
    printf ("Hello from the child process! PID\n");
    while (1) {}
  } else if (pid > 0) {
    // Parent process
    printf ("Hello from the parent process!\n");
    while (1) {}
  } else {
    // Fork failed
    printf ("Fork failed!\n");
  }
  return 0;
}