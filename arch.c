#include "arch.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

const uint64_t instr_size[16] = {
  [IHALT  ] = 1,
  [INOP   ] = 1,
  [IRRMOVQ] = 2,
  [IIRMOVQ] = 10,
  [IRMMOVQ] = 10,
  [IMRMOVQ] = 10,
  [IOPQ   ] = 2,
  [IJXX   ] = 9,
  [ICALL  ] = 9,
  [IRET   ] = 1,
  [IPUSHQ ] = 2,
  [IPOPQ  ] = 2,
  [IEND   ] = 1,
};

const uint8_t instr_srcA[16] = {
  [IHALT  ] = RNONE,
  [INOP   ] = RNONE,
  [IRRMOVQ] = RRA,
  [IIRMOVQ] = RNONE,
  [IRMMOVQ] = RRA,
  [IMRMOVQ] = RNONE,  // modified
  [IOPQ   ] = RRA,
  [IJXX   ] = RNONE,
  [ICALL  ] = RNONE,
  [IRET   ] = RRSP,   // modified
  [IPUSHQ ] = RRA,
  [IPOPQ  ] = RRSP,
  [IEND   ] = RNONE,
};

const uint8_t instr_srcB[16] = {
  [IHALT  ] = RNONE,
  [INOP   ] = RNONE,
  [IRRMOVQ] = RNONE,  // modified
  [IIRMOVQ] = RNONE,
  [IRMMOVQ] = RRB,
  [IMRMOVQ] = RRB,
  [IOPQ   ] = RRB,
  [IJXX   ] = RNONE,
  [ICALL  ] = RRSP,
  [IRET   ] = RRSP,
  [IPUSHQ ] = RRSP,
  [IPOPQ  ] = RRSP,
  [IEND   ] = RNONE,
};

const uint8_t instr_dstE[16] = {
  [IHALT  ] = RNONE,
  [INOP   ] = RNONE,
  [IRRMOVQ] = RRB,    // before execute
  [IIRMOVQ] = RRB,
  [IRMMOVQ] = RNONE,
  [IMRMOVQ] = RNONE,
  [IOPQ   ] = RRB,
  [IJXX   ] = RNONE,
  [ICALL  ] = RRSP,
  [IRET   ] = RRSP,
  [IPUSHQ ] = RRSP,
  [IPOPQ  ] = RRSP,
  [IEND   ] = RNONE,
};

const uint8_t instr_dstM[16] = {
  [IHALT  ] = RNONE,
  [INOP   ] = RNONE,
  [IRRMOVQ] = RNONE,
  [IIRMOVQ] = RNONE,
  [IRMMOVQ] = RNONE,
  [IMRMOVQ] = RRA,
  [IOPQ   ] = RNONE,
  [IJXX   ] = RNONE,
  [ICALL  ] = RNONE,
  [IRET   ] = RNONE,  // modified
  [IPUSHQ ] = RNONE,
  [IPOPQ  ] = RRA,
  [IEND   ] = RNONE,
};


const uint64_t instr_aluA[16] = {
  [IHALT  ] = V0,
  [INOP   ] = V0,
  [IRRMOVQ] = VA,
  [IIRMOVQ] = VC,
  [IRMMOVQ] = VC,
  [IMRMOVQ] = VC,
  [IOPQ   ] = VA,
  [IJXX   ] = V0,
  [ICALL  ] = VN8,
  [IRET   ] = VP8,
  [IPUSHQ ] = VN8,
  [IPOPQ  ] = VP8,
  [IEND   ] = V0,
};

const uint64_t instr_aluB[16] = {
  [IHALT  ] = V0,
  [INOP   ] = V0,
  [IRRMOVQ] = V0,
  [IIRMOVQ] = V0,
  [IRMMOVQ] = VB,
  [IMRMOVQ] = VB,
  [IOPQ   ] = VB,
  [IJXX   ] = V0,
  [ICALL  ] = VB,
  [IRET   ] = VB,
  [IPUSHQ ] = VB,
  [IPOPQ  ] = VB,
  [IEND   ] = V0,
};

const uint8_t instr_memio[16] = {
  [IHALT  ] = MNONE,
  [INOP   ] = MNONE,
  [IRRMOVQ] = MNONE,
  [IIRMOVQ] = MNONE,
  [IRMMOVQ] = MWR,
  [IMRMOVQ] = MRD,
  [IOPQ   ] = MNONE,
  [IJXX   ] = MNONE,
  [ICALL  ] = MWR,
  [IRET   ] = MRD,
  [IPUSHQ ] = MWR,
  [IPOPQ  ] = MRD,
  [IEND   ] = MNONE,
};

const uint64_t instr_memaddr[16] = {
  [IHALT  ] = V0,
  [INOP   ] = V0,
  [IRRMOVQ] = V0,
  [IIRMOVQ] = V0,
  [IRMMOVQ] = VB,
  [IMRMOVQ] = VB,
  [IOPQ   ] = V0,
  [IJXX   ] = V0,
  [ICALL  ] = VB,
  [IRET   ] = VA,
  [IPUSHQ ] = VB,
  [IPOPQ  ] = VA,
  [IEND   ] = V0,
};

const uint8_t instr_syntax[16][TOKNUM_MAX] = {
  [IHALT  ] = {'\t', TOKINS, '\n'},
  [INOP   ] = {'\t', TOKINS, '\n'},
  [IRRMOVQ] = {'\t', TOKINS, '\t', TOKRA, ',', ' ', TOKRB, '\n'},
  [IIRMOVQ] = {'\t', TOKINS, '\t', TOKIMD, ',', ' ', TOKRB, '\n'},
  [IRMMOVQ] = {'\t', TOKINS, '\t', TOKRA, ',', ' ', TOKADR, '(', TOKRB, ')', '\n'},
  [IMRMOVQ] = {'\t', TOKINS, '\t', TOKADR, '(', TOKRB, ')', ',', ' ', TOKRA, '\n'},
  [IOPQ   ] = {'\t', TOKINS, '\t', TOKRA, ',', ' ', TOKRB, '\n'},
  [IJXX   ] = {'\t', TOKINS, '\t', TOKADR, '\n'},
  [ICALL  ] = {'\t', TOKINS, '\t', TOKADR, '\n'},
  [IRET   ] = {'\t', TOKINS, '\n'},
  [IPUSHQ ] = {'\t', TOKINS, '\t', TOKRA, '\n'},
  [IPOPQ  ] = {'\t', TOKINS, '\t', TOKRA, '\n'},
  [IEND   ] = {'\t', TOKINS, '\n'},
};

const char *instr_name[16][16] = {
  [IHALT  ] = {[FNONE] = "halt"},
  [INOP   ] = {[FNONE] = "nop"},
  [IRRMOVQ] = {
    [CNONE] = "rrmovq",
    [CLE  ] = "cmovle",
    [CL   ] = "cmovl",
    [CE   ] = "cmove",
    [CNE  ] = "cmovne",
    [CGE  ] = "cmovge",
    [CG   ] = "cmovg",
  },
  [IIRMOVQ] = {[FNONE] = "irmovq"},
  [IRMMOVQ] = {[FNONE] = "rmmovq"},
  [IMRMOVQ] = {[FNONE] = "mrmovq"},
  [IOPQ   ] = {
    [FADDQ ] = "addq",
    [FSUBQ ] = "subq",
    [FANDQ ] = "andq",
    [FXORQ ] = "xorq",
    [FIMULQ] = "imulq",
    [FIDIVQ] = "idivq",
  },
  [IJXX   ] = {
    [CNONE] = "jmp",
    [CLE  ] = "jle",
    [CL   ] = "jl",
    [CE   ] = "je",
    [CNE  ] = "jne",
    [CGE  ] = "jge",
    [CG   ] = "jg",
  },
  [ICALL  ] = {[FNONE] = "call"},
  [IRET   ] = {[FNONE] = "ret"},
  [IPUSHQ ] = {[FNONE] = "pushq"},
  [IPOPQ  ] = {[FNONE] = "popq"},
  [IEND   ] = {
    [ETEXT] = "endtext",
    [EDATA] = "enddata",
  },
};

const char *reg_name[16] = {
  "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
  "r8", "r9", "r10", "r11", "r12", "r13", "r14",
};

const char *stat_name[16] = {
  [SEND] = "SEND",
  [SAOK] = "SAOK",
  [SADR] = "SADR",
  [SINS] = "SINS",
  [SHLT] = "SHLT",
};

const uint8_t direc_syntax[16][TOKNUM_MAX] = {
  [DIMD  ] = {'\t', TOKDIR, '\t', TOKADR, '\n'},
  [DPOS  ] = {'\t', TOKDIR, '\t', TOKADR, '\n'},
  [DALIGN] = {'\t', TOKDIR, ' ', TOKADR, '\n'},
};

const char *direc_name[16][16] = {
  [DIMD  ] = {
    [DBYTE] = "byte",
    [DWORD] = "word",
    [DLONG] = "long",
    [DQUAD] = "quad",
  },
  [DPOS  ] = {[DNONE] = "pos"},
  [DALIGN] = {[DNONE] = "align"},
};

pstat_t pstat;
instr_t instr;
char *mem;
uint64_t mem_size;
opt_t opt;
lab_t lab[LABNUM_MAX];
lab_t ref[LABNUM_MAX];
size_t labsiz;
size_t labpnd;
size_t refsiz;
size_t align;

static void imem_error(void);
static void invalid_instr(void);
static void dmem_error(void);
static void halt(void);
static void end(void);
static const char **strsnstr(const char **strs, const char *str, size_t n);

void pstat_init(void)
{
  memset(&pstat, 0, sizeof pstat);
  pstat.zf = 1;
  pstat.stat = SAOK;
}

void mem_load(const char *pathname)
{
  int r, fd;
  struct stat sbuf;

  r = stat(pathname, &sbuf);
  if(r < 0)
  {
    perror(pathname);
    exit(EXIT_FAILURE);
  }

  fd = open(pathname, O_RDONLY);
  if(fd < 0)
  {
    perror(pathname);
    exit(EXIT_FAILURE);
  }

  mem = (char *)mmap(NULL, sbuf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if(mem == MAP_FAILED)
  {
    perror(pathname);
    exit(EXIT_FAILURE);
  }
  mem_size = sbuf.st_size;

  r = close(fd);
  if(r < 0)
  {
    perror(pathname);
    exit(EXIT_FAILURE);
  }
}

void instr_fetch(void)
{
  uint64_t fetch_size, fetch_base, fetch_pos;

  if(opt.print_stg)
    fprintf(stderr, "    [stage  change]\t%s\n", __func__);
  if(pstat.stat != SAOK)
    return;
  fetch_base = pstat.pc;
  fetch_pos = fetch_base;

  if(fetch_pos >= mem_size)
    return imem_error();
  memcpy(&instr, &mem[fetch_pos++], 1);
  if(!instr_name[instr.icode][instr.ifunc])
    return invalid_instr();

  fetch_size = instr_size[instr.icode];
  if(mem_size < fetch_size - 1 || fetch_pos > mem_size - fetch_size + 1)
    return imem_error();
  instr.stat = SAOK;

  if(fetch_size & 0x2)
    memcpy((char *)&instr + 1, &mem[fetch_pos++], 1);
  if(fetch_size & 0x8)
    memcpy(&instr.valC, &mem[fetch_pos], 8);  // modified

  instr.valP = fetch_base + fetch_size;
  if(opt.print_ins)
    print_instr(stderr);
  if(instr.icode == IHALT)
    halt();
  else if(instr.icode == IEND && instr.ifunc == ETEXT)
    end();
}

void instr_decode(void)
{
  uint8_t srcA, srcB, dstE, dstM;

  if(opt.print_stg)
    fprintf(stderr, "    [stage  change]\t%s\n", __func__);
  if(instr.stat != SAOK)
    return;

  srcA = instr_srcA[instr.icode];
  srcA = srcA & 0x10 ? instr.rA : srcA;

  srcB = instr_srcB[instr.icode];
  srcB = srcB & 0x10 ? instr.rB : srcB;

  dstE = instr_dstE[instr.icode];
  dstE = dstE & 0x10 ? instr.rB : dstE;

  dstM = instr_dstM[instr.icode];
  dstM = dstM & 0x10 ? instr.rA : dstM;

  instr.srcA = srcA;
  instr.srcB = srcB;
  instr.dstE = dstE;
  instr.dstM = dstM;

  if(instr.srcA != RNONE)
    instr.valA = pstat.reg[instr.srcA];
  if(instr.srcB != RNONE)
    instr.valB = pstat.reg[instr.srcB];
  if(instr.icode == ICALL)
    instr.valA = instr.valP;
}

void instr_execute(void)
{
  uint64_t aluA, aluB, valE;
  uint8_t zf, sf, of;

  if(opt.print_stg)
    fprintf(stderr, "    [stage  change]\t%s\n", __func__);
  if(instr.stat != SAOK)
    return;

  aluA = instr_aluA[instr.icode];
  aluA = aluA & 0x1 ? aluA - 1 : *(uint64_t *)((char *)&instr + aluA);

  aluB = instr_aluB[instr.icode];
  aluB = aluB & 0x1 ? aluB - 1 : *(uint64_t *)((char *)&instr + aluB);

  switch(instr.icode == IOPQ ? instr.ifunc : 0)
  {
  case FADDQ:
    valE = aluB + aluA; of = (int64_t)aluA > 0 && (int64_t)valE < (int64_t)aluB; break;
  case FSUBQ:
    valE = aluB - aluA; of = (int64_t)aluA > 0 && (int64_t)valE > (int64_t)aluB; break;
  case FANDQ:
    valE = aluB & aluA; of = 0; break;
  case FXORQ:
    valE = aluB ^ aluA; of = 0; break;
  case FIMULQ:
    {
      __int128_t __valE = (__int128_t)aluB * (__int128_t)aluA;
      of = !!(__valE >> 64);
      valE = __valE;
      break;
    }
  case FIDIVQ:
    valE = aluB / aluA; of = 0; break;
  default:
    fprintf(stderr, "%s: invalid ifunc value: %hhu\n", __func__, instr.ifunc);
    exit(EXIT_FAILURE);
  }
  instr.valB = valE;
  zf = !valE;
  sf = !!(valE & INT64_MIN);

  if(instr.icode == IOPQ)
  {
    if(opt.print_chg && pstat.zf != zf)
      fprintf(stderr, "    [status change]\tZF: %hhu -> %hhu\n", pstat.zf, zf);
    pstat.zf = zf;
    if(opt.print_chg && pstat.sf != sf)
      fprintf(stderr, "    [status change]\tSF: %hhu -> %hhu\n", pstat.sf, sf);
    pstat.sf = sf;
    if(opt.print_chg && pstat.of != of)
      fprintf(stderr, "    [status change]\tOF: %hhu -> %hhu\n", pstat.of, of);
    pstat.of = of;
  }
  else if(instr.icode == IRRMOVQ || instr.icode == IJXX)
  {
    switch(instr.ifunc)
    {
    case CNONE:
      instr.cnd = 1; break;
    case CLE:
      instr.cnd = pstat.zf | (pstat.sf ^ pstat.of); break;
    case CL:
      instr.cnd = pstat.sf ^ pstat.of; break;
    case CE:
      instr.cnd = pstat.zf; break;
    case CNE:
      instr.cnd = !pstat.zf; break;
    case CGE:
      instr.cnd = !(pstat.sf ^ pstat.of); break;
    case CG:
      instr.cnd = !pstat.zf & !(pstat.sf ^ pstat.of); break;
    default:
      fprintf(stderr, "%s: invalid ifunc value: %hhu\n", __func__, instr.ifunc);
      exit(EXIT_FAILURE);
    }
    if(instr.icode == IRRMOVQ && !instr.cnd)
      instr.dstE = RNONE;
  }
}

void instr_memory(void)
{
  uint8_t memio;
  uint64_t memaddr;

  if(opt.print_stg)
    fprintf(stderr, "    [stage  change]\t%s\n", __func__);
  if(instr.stat != SAOK)
    return;

  memio = instr_memio[instr.icode];
  if(memio == MNONE)
    return;

  memaddr = instr_memaddr[instr.icode];
  memaddr = *(uint64_t *)((char *)&instr + memaddr);
  if(mem_size < 8 || memaddr > mem_size - 8)
    return dmem_error();
  if(memio == MRD)
    memcpy(&instr.valA, &mem[memaddr], 8);
  else
  {
    uint64_t valM = *(uint64_t *)&mem[memaddr];

    if(opt.print_chg && valM != instr.valA)
      fprintf(stderr, "    [memory change]\t0x%016zx: 0x%zx -> 0x%zx\n", memaddr, valM, instr.valA);
    memcpy(&mem[memaddr], &instr.valA, 8);
  }
}

void instr_writeback(void)
{
  uint64_t pc;

  if(opt.print_stg)
    fprintf(stderr, "    [stage  change]\t%s\n", __func__);
  if(opt.print_chg && pstat.stat != instr.stat)
    fprintf(stderr, "    [status change]\tSTAT: %s -> %s\n", stat_name[pstat.stat], stat_name[instr.stat]);
  pstat.stat = instr.stat;
  if(instr.stat != SAOK)
    return;

  if(instr.dstE != RNONE)
  {
    if(opt.print_chg && pstat.reg[instr.dstE] != instr.valB)
      fprintf(stderr, "    [status change]\t%%%s: 0x%zx -> 0x%zx\n", reg_name[instr.dstE], pstat.reg[instr.dstE], instr.valB);
    pstat.reg[instr.dstE] = instr.valB;
  }
  if(instr.dstM != RNONE)
  {
    if(opt.print_chg && pstat.reg[instr.dstM] != instr.valA)
      fprintf(stderr, "    [status change]\t%%%s: 0x%zx -> 0x%zx\n", reg_name[instr.dstM], pstat.reg[instr.dstM], instr.valA);
    pstat.reg[instr.dstM] = instr.valA;
  }

  if(instr.icode == IRET)
    pc = instr.valA;
  else if(instr.icode == ICALL || (instr.icode == IJXX && instr.cnd))
    pc = instr.valC;
  else
    pc = instr.valP;

  // if(opt.print_chg && pstat.pc != pc)
  //   fprintf(stderr, "    [status change]\tPC: 0x%zx -> 0x%zx\n", pstat.pc, pc);
  pstat.pc = pc;
}

void instr_process(void)
{
  instr_fetch();
  instr_decode();
  instr_execute();
  instr_memory();
  instr_writeback();
}

void process_loop(void)
{
  while(pstat.stat == SAOK)
    instr_process();
}

void print_loop(FILE *stream)
{
  while(pstat.stat == SAOK)
  {
    instr_fetch();
    if(instr.stat == SHLT)
      instr.stat = SAOK;
    if((pstat.stat = instr.stat) != SAOK)
      break;
    pstat.pc = instr.valP;
    print_instr(stream);
  }
  if(pstat.stat == SEND)
  {
    print_instr(stream);
  }
  else if(pstat.stat == SINS)
  {
    fprintf(stderr, "%s: invalid instruction at: 0x%zx\n", __func__, pstat.pc);
    exit(EXIT_FAILURE);
  }
  else if(pstat.stat == SADR)
  {
    fprintf(stderr, "%s: invalid memory access at: 0x%zx\n", __func__, pstat.pc);
    exit(EXIT_FAILURE);
  }
  else
  {
    fprintf(stderr, "%s: unknown error status: %hhu\n", __func__, pstat.stat);
    exit(EXIT_FAILURE);
  }
}

void run(const char *pathname)
{
  mem_load(pathname);
  pstat_init();
  process_loop();
}

void disas(const char *pathname, FILE *stream)
{
  mem_load(pathname);
  pstat_init();
  print_loop(stream);
}

void imem_error(void)
{
  fprintf(stderr, "%s: error detacted\n", __func__);
  instr.stat = SADR;
}

void invalid_instr(void)
{
  fprintf(stderr, "%s: error detacted\n", __func__);
  instr.stat = SINS;
}

void dmem_error(void)
{
  fprintf(stderr, "%s: error detacted\n", __func__);
  instr.stat = SADR;
}

static void halt(void)
{
  fprintf(stderr, "%s: halt detacted, return code: 0x%zx\n", __func__, pstat.reg[RRAX]);
  instr.stat = SHLT;
}

static void end(void)
{
  fprintf(stderr, "%s: end of text detacted\n", __func__);
  instr.stat = SEND;
}

int setopt(int argc, char *argv[])
{
  int r;
  const char *optstr = "iesc";

  memset(&opt, 0, sizeof opt);
  optind = 1;
  while((r = getopt(argc, argv, optstr)) != -1)
  {
    switch(r)
    {
    case '?':
      exit(EXIT_FAILURE);
    case 'i':
      opt.print_ins = 1; break;
    case 'e':
      opt.print_enc = 1; break;
    case 's':
      opt.print_stg = 1; break;
    case 'c':
      opt.print_chg = 1; break;
    default:
      fprintf(stderr, "%s: unknown error\n", __func__);
      exit(EXIT_FAILURE);
    }
  }
  return optind;
}

void print_instr(FILE *stream)
{
  const uint8_t *syntax = instr_syntax[instr.icode];

  print_instr_addr(stream);
  fputc(':', stream);
  if(opt.print_enc)
    print_instr_enc(stream);
  for(size_t i = 0; i < TOKNUM_MAX && syntax[i] != TOKNONE; ++i)
  {
    if(!(syntax[i] & 0x80))
    {
      fputc(syntax[i], stream);
      continue;
    }
    switch(syntax[i])
    {
    case TOKANY:
      break;
    case TOKINS:
      fputs(instr_name[instr.icode][instr.ifunc], stream); break;
    case TOKRA:
      fprintf(stream, "%%%s", reg_name[instr.rA]); break;
    case TOKRB:
      fprintf(stream, "%%%s", reg_name[instr.rB]); break;
    case TOKIMD:
      fprintf(stream, "$0x%zx", instr.valC); break;
    case TOKADR:
      fprintf(stream, "0x%zx", instr.valC); break;
    default:
      fprintf(stderr, "%s: invalid token value: %hhu\n", __func__, syntax[i]);
      exit(EXIT_FAILURE);
    }
  }
}

void print_instr_addr(FILE *stream)
{
  uint64_t valP = instr.valP - instr_size[instr.icode];
  fprintf(stream, "0x%016zx", valP);
}

void print_instr_enc(FILE *stream)
{
  uint64_t fetch_size = instr_size[instr.icode];

  fprintf(stream, "\t%02hhx", *(uint8_t *)&instr);
  if(fetch_size & 0x2)
    fprintf(stream, " %02hhx", *((uint8_t *)&instr + 1));
  else
    fprintf(stream, "   ");
  if(fetch_size & 0x8)
  {
    fprintf(stream, " ");
    for(size_t i = 0; i < 8; ++i)
      fprintf(stream, "%02hhx", *((uint8_t *)&instr.valC + i));
  }
  else
  {
    fprintf(stream, " ");
    for(size_t i = 0; i < 8; ++i)
      fprintf(stream, "  ");
  }
}

int get_line(FILE *stream, char *line)
{
  char *buf;
  size_t len;

  buf = fgets(line, LINE_MAX, stream);
  if(buf == NULL)
    return EOF;
  len = strlen(buf);
  if(!len)
  {
    fprintf(stderr, "%s: unknown error\n", __func__);
    exit(EXIT_FAILURE);
  }
  if(len == LINE_MAX - 1 && line[len - 1] != '\n')
  {
    fprintf(stderr, "%s: line too long\n", __func__);
    exit(EXIT_FAILURE);
  }
  if((buf = strchr(line, '#')))
  {
    *buf = 0;
    return buf - line;
  }
  if(line[len - 1] == '\n')
    line[--len] = 0;
  return len;
}

int istokal(int ch)
{
  return isalnum(ch) || ch == '+' || ch == '-' || ch == '_' || ch == '.';
}

uint8_t scan_tok_any(const char **pcur, const char **pend)
{
  const char *cur = *pcur;

  while(isspace(*cur))
    ++cur;
  *pcur = cur;
  if(!*cur)
  {
    *pend = cur;
    return TOKNONE;
  }

  if(!istokal(*cur))
  {
    *pend = cur + 1;
    return *cur;
  }
  while(istokal(*++cur));
  *pend = cur;
  return TOKANY;
}

uint8_t scan_tok(const char **pcur, const char **pend)
{
  const char *cur = *pcur;

  while(isspace(*cur))
    ++cur;
  *pcur = cur;
  if(!*cur)
  {
    *pend = cur;
    return TOKNONE;
  }

  if(isdigit(*cur) || *cur == '+' || *cur == '-')
  {
    uint8_t tok;

    tok = scan_tok_any(pcur, pend);
    if(tok != TOKANY)
    {
      fprintf(stderr, "%s: unknown error\n", __func__);
      exit(EXIT_FAILURE);
    }

    return TOKADR;
  }

  if(*cur == '%' || *cur == '$')
  {
    uint8_t tok, ret = *cur;

    ++cur;
    tok = scan_tok_any(&cur, pend);
    if(tok != TOKANY)
    {
      *pend = *pcur;
      return TOKERR;
    }

    *pcur = cur;
    return ret == '%' ? TOKREG : TOKIMD;
  }

  return scan_tok_any(pcur, pend);
}

uint8_t scan_line(const char *line)
{
  const char *cur[TOKNUM_MAX];
  const char *end[TOKNUM_MAX];
  uint8_t tok;
  char name[TOKLEN_MAX];

  cur[0] = line;
  tok = scan_tok(&cur[0], &end[0]);
  if(tok == TOKNONE)
    return LEMPTY;
  if(tok != TOKANY)
  {
    fprintf(stderr, "%s: expect instruction/directive/label name: %s\n", __func__, line);
    return LERROR;
  }

  cur[1] = end[0];
  tok = scan_tok(&cur[1], &end[1]);

  if(tok == ':')
  {
    cur[2] = end[1];
    tok = scan_tok(&cur[2], &end[2]);
    if(tok != TOKNONE)
    {
      fprintf(stderr, "%s: junk after label: %s\n", __func__, line);
      return LERROR;
    }
    tokcpy(name, cur[0], end[0]);
    pend_lab(name);
    return LLABEL;
  }

  if(cur[0][0] == '.')
  {
    ++cur[0];
    return scan_direc(line, cur, end);
  }

  return scan_instr(line, cur, end);
}

int pend_lab(const char *name)
{
  if(labsiz == LABNUM_MAX)
  {
    fprintf(stderr, "%s: too many labels\n", __func__);
    exit(EXIT_FAILURE);
  }

  if(retrieve_lab(name))
  {
    fprintf(stderr, "%s: redefined label: %s\n", __func__, name);
    exit(EXIT_FAILURE);
  }
  strcpy(lab[labsiz].name, name);
  ++labpnd;
  return labsiz++;
}

void write_lab(uint64_t addr)
{
  for(size_t i = labsiz - labpnd; i < labsiz; ++i)
    lab[i].addr = addr;
  labpnd = 0;
}

int insert_ref(const char *name, uint64_t addr)
{
  if(refsiz == LABNUM_MAX)
  {
    fprintf(stderr, "%s: too many label references\n", __func__);
    exit(EXIT_FAILURE);
  }

  strcpy(ref[refsiz].name, name);
  ref[refsiz].addr = addr;
  return refsiz++;
}

const lab_t *retrieve_lab(const char *name)
{
  for(size_t i = 0; i < labsiz; ++i)
    if(lab[i].name && !strcmp(name, lab[i].name))
      return &lab[i];
  return NULL;
}

uint8_t scan_direc(const char *line, const char **cur, const char **end)
{
  char name[TOKLEN_MAX];
  const char **pname;
  uint8_t id, tok;
  uint64_t valC;
  char *ptr;

  tokcpy(name, cur[0], end[0]);
  pname = strsnstr(direc_name[0], name, 256);
  if(!pname)
  {
    fprintf(stderr, "%s: unknown directive: .%s\n", __func__, name);
    return LERROR;
  }
  id = pname - direc_name[0];

  cur[1] = end[0];
  tok = scan_tok(&cur[1], &end[1]);
  if(tok != TOKADR)
  {
    fprintf(stderr, "%s: expect integer literal after .%s: %s\n", __func__, name, line);
    return LERROR;
  }
  valC = strtoll(cur[1], &ptr, 0);
  if(ptr != end[1])
  {
    tokcpy(name, cur[1], end[1]);
    fprintf(stderr, "%s: invalid integer literal: %s\n", __func__, name);
    return LERROR;
  }

  switch(id >> 4)
  {
  case DIMD:
    insert_imd(id & 0xf, valC); break;
  case DPOS:
    goto_pos(valC); break;
  case DALIGN:
    if(valC != 1 && valC != 2 && valC != 4 && valC != 8 && valC != 16)
    {
      fprintf(stderr, "%s: invalid align: %zu\n", __func__, valC);
      return LERROR;
    }
    set_align(valC); break;
  }
  return LDIREC;
}

void tokcpy(char *tok, const char *cur, const char *end)
{
  size_t n = end - cur;

  if(n >= TOKLEN_MAX)
  {
    fprintf(stderr, "%s: token too long\n", __func__);
    exit(EXIT_FAILURE);
  }
  strncpy(tok, cur, n);
  tok[n] = 0;
}

const char **strsnstr(const char **strs, const char *str, size_t n)
{
  for(size_t i = 0; i < n; ++i)
    if(strs[i] && !strcmp(strs[i], str))
      return &strs[i];
  return NULL;
}

void insert_imd(size_t size, uint64_t imd)
{
  apply_align(size, &pstat.pc);
  expand_for(size);
  write_lab(pstat.pc);
  memcpy(&mem[pstat.pc], &imd, size);
}

void goto_pos(uint64_t pos)
{
  write_lab(pstat.pc);
  if(pos > pstat.pc)
    expand_for(pos - pstat.pc);
  pstat.pc = pos;
}

void set_align(size_t size)
{
  align = size;
}

void apply_align(size_t size, uint64_t *pos)
{
  int i;

  size = align ? align : size ? size : 1;
  for(i = -1; size; ++i)
    size >>= 1;
  i = (1 << i) - 1;
  *pos = (*pos + i) & ~i;
  align = 0;
}

void expand_for(size_t size)
{
  if(pstat.pc + size > mem_size)
  {
    size_t org_size = mem_size;

    mem_size = pstat.pc + size;
    if(mem_size > MEM_MAX)
    {
      fprintf(stderr, "%s: too large memory\n", __func__);
      exit(EXIT_FAILURE);
    }
    if(!(mem = realloc(mem, mem_size)))
    {
      fprintf(stderr, "%s: error allocating memory\n", __func__);
      exit(EXIT_FAILURE);
    }
    memset(mem + org_size, 0, mem_size - org_size);
  }
}

uint8_t scan_instr(const char *line, const char **cur, const char **end)
{
  char name[TOKLEN_MAX];
  const char **pname;
  uint8_t id, tok;
  const uint8_t *syntax;
  size_t i, j, size;
  char *ptr;

  tokcpy(name, cur[0], end[0]);
  pname = strsnstr(instr_name[0], name, 256);
  if(!pname)
  {
    fprintf(stderr, "%s: unknown instruction: %s\n", __func__, name);
    return LERROR;
  }
  id = pname - instr_name[0];
  syntax = instr_syntax[id >> 4];
  size = instr_size[id >> 4];
  apply_align(1, &pstat.pc);
  expand_for(size);
  write_lab(pstat.pc);

  memset(&instr, id, 1);
  memset((char *)&instr + 1, -1, 1);
  j = 0;

  for(i = 3; i < TOKNUM_MAX && syntax[i] != TOKNONE; ++i)
  {
    if(isspace(syntax[i]))
      continue;
    cur[i] = end[j];
    tok = scan_tok(&cur[i], &end[i]);
    if(!(syntax[i] & 0x80))
    {
      if(tok == syntax[i])
      {
        j = i;
        continue;
      }
      fprintf(stderr, "%s: expect %c: %s\n", __func__, syntax[i], line);
      return LERROR;
    }
    switch(syntax[i])
    {
    case TOKRA:
    case TOKRB:
      if(tok != TOKREG)
      {
        fprintf(stderr, "%s: expect register specifier: %s\n", __func__, line);
        return LERROR;
      }
      tokcpy(name, cur[i], end[i]);
      pname = strsnstr(reg_name, name, 16);
      if(!pname)
      {
        fprintf(stderr, "%s: unknown register: %%%s\n", __func__, name);
        return LERROR;
      }
      id = pname - reg_name;
      if(syntax[i] == TOKRA)
        instr.rA = id;
      else
        instr.rB = id;
      break;

    case TOKADR:
      if(tok == TOKANY)
      {
        tokcpy(name, cur[i], end[i]);
        insert_ref(name, pstat.pc + size - 8);
        instr.valC = 0;
        break;
      }
      if(tok != syntax[i])
      {
        fprintf(stderr, "%s: expect address: %s\n", __func__, line);
        return LERROR;
      }
      instr.valC = strtoll(cur[i], &ptr, 0);
      if(ptr != end[i])
      {
        tokcpy(name, cur[i], end[i]);
        fprintf(stderr, "%s: invalid integer literal: %s\n", __func__, name);
        return LERROR;
      }
      break;

    case TOKIMD:
      if(tok != syntax[i])
      {
        fprintf(stderr, "%s: expect immediate: %s\n", __func__, line);
        return LERROR;
      }
      if(isalpha(*cur[i]) || *cur[i] == '_' || *cur[i] == '.')
      {
        tokcpy(name, cur[i], end[i]);
        insert_ref(name, pstat.pc + size - 8);
        instr.valC = 0;
        break;
      }
      instr.valC = strtoll(cur[i], &ptr, 0);
      if(ptr != end[i])
      {
        tokcpy(name, cur[i], end[i]);
        fprintf(stderr, "%s: invalid integer literal: %s\n", __func__, name);
        return LERROR;
      }
      break;

    default:
      fprintf(stderr, "%s: unknown error\n", __func__);
      exit(EXIT_FAILURE);
    }
    j = i;
  }

  memcpy(&mem[pstat.pc++], &instr, 1);
  if(size & 0x2)
    memcpy(&mem[pstat.pc++], (char *)&instr + 1, 1);
  if(size & 0x8)
  {
    memcpy(&mem[pstat.pc], &instr.valC, 8);
    pstat.pc += 8;
  }

  return LINSTR;
}

void scan_loop(FILE *stream)
{
  int r;
  uint8_t l;
  char line[LINE_MAX];

  while((r = get_line(stream, line)) != EOF)
  {
    l = scan_line(line);
    if(l == LERROR)
      exit(EXIT_FAILURE);
  }
  write_lab(pstat.pc);
  for(size_t i = 0; i < refsiz; ++i)
    resolve_ref(&ref[i]);
}

void as(const char *pathname, FILE *stream)
{
  int r;
  FILE *file;

  mem = malloc((mem_size = 0));
  pstat_init();
  scan_loop(stream);
  file = fopen(pathname, "wb");
  if(!file)
  {
    perror(pathname);
    exit(EXIT_FAILURE);
  }
  r = fwrite(mem, 1, mem_size, file);
  if(r != (int)mem_size)
  {
    fprintf(stderr, "%s: error writing file\n", pathname);
    exit(EXIT_FAILURE);
  }
  r = fclose(file);
  if(r < 0)
  {
    perror(pathname);
    exit(EXIT_FAILURE);
  }
}

void resolve_ref(const lab_t *pref)
{
  const lab_t *plab = retrieve_lab(pref->name);

  if(!plab)
  {
    fprintf(stderr, "%s: undefined reference to: %s\n", __func__, pref->name);
    exit(EXIT_FAILURE);
  }
  memcpy(&mem[pref->addr], &plab->addr, 8);
}
