#include "arch.h"

int main(int argc, char *argv[])
{
  FILE *fileout;
  int argi = setopt(argc, argv);

  if(argi < argc - 2 || argi == argc)
  {
    fprintf(stderr, "usage: %s [-[i][e][s][c]] <obj> [<dmp>]\n", argv[0]);
    return 1;
  }
  if(argi == argc - 1)
    fileout = stdout;
  else
  {
    fileout = fopen(argv[argc - 1], "w");
    if(!fileout)
    {
      perror(argv[argc - 1]);
      return 1;
    }
  }

  opt.print_ins = 0;
  disas(argv[argi], fileout);
  return 0;
}
