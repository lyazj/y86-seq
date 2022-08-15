#include "arch.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
  int argi = setopt(argc, argv);
  if(argi != argc - 1)
  {
    fprintf(stderr, "usage: %s [-[i][e][s][c]] <pathname>\n", argv[0]);
    return 1;
  }
  run(argv[argi]);
  return 0;
}
