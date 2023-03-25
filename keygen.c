#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <time.h>

void main(int argc, char *argv[])
{
  const char validChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  srand(time(NULL));

  if (argv[1] == NULL)
  {
    fprintf(stderr, "keygen: no length input\n");
    exit(1);
  }

  int length = atoi(argv[1]);
  
  int i = 0;
  char key[length+1];
  int index;

  for (i = 0; i < length; i++)
  {
    index = rand() % 27;
    key[i] = validChars[index];
  }
  key[i] = '\0';

  fprintf(stdout, "%s\n", key);

  return;
}