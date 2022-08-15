#include "arch.h"

#include <string.h>

int main(int argc, char *argv[])
{
  FILE *filein;
  size_t pathin_len;
  int pathin_dots;
  char pathout[FILENAME_MAX + 1];
  int argi = setopt(argc, argv);

  if(argi < argc - 2 || argi == argc)
  {
    fprintf(stderr, "usage: %s [-[i][e][s][c]] <asm> [<obj>]\n", argv[0]);
    return 1;
  }
  filein = fopen(argv[argi], "r");
  if(!filein)
  {
    perror(argv[argi]);
    return 1;
  }

  pathin_len = strlen(argv[argi]);
  pathin_dots = pathin_len >= 2 && !strcmp(&argv[argi][pathin_len - 2], ".s");
  if(pathin_len > FILENAME_MAX || (pathin_len > FILENAME_MAX - 2 && !pathin_dots))
  {
    fprintf(stderr, "%s: filename too long", argv[argi]);
    return 1;
  }
  strcpy(pathout, argv[argi]);
  strcpy(&pathout[pathin_len - (pathin_dots << 1)], ".o");
  as(pathout, filein);

  return 0;
}
