#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
  uint64_t  reg[15];
  uint64_t  pc;
  uint8_t   zf   : 1;
  uint8_t   sf   : 1;
  uint8_t   of   : 1;
  uint8_t   stat : 5;
} pstat_t;

typedef struct {
  uint8_t   ifunc : 4;
  uint8_t   icode : 4;
  uint8_t   rB    : 4;
  uint8_t   rA    : 4;
  uint8_t   srcB  : 4;
  uint8_t   srcA  : 4;
  uint8_t   dstM  : 4;
  uint8_t   dstE  : 4;
  uint8_t   cnd   : 1;
  uint8_t   stat  : 7;
  uint64_t  valC;
  uint64_t  valP;
  uint64_t  valB;  // valE
  uint64_t  valA;  // valM
} instr_t;

typedef struct {
  uint8_t   print_ins : 1;
  uint8_t   print_enc : 1;
  uint8_t   print_stg : 1;
  uint8_t   print_chg : 1;
} opt_t;

#define IHALT   ((uint8_t)0x0)
#define INOP    ((uint8_t)0x1)
#define IRRMOVQ ((uint8_t)0x2)
#define IIRMOVQ ((uint8_t)0x3)
#define IRMMOVQ ((uint8_t)0x4)
#define IMRMOVQ ((uint8_t)0x5)
#define IOPQ    ((uint8_t)0x6)
#define IJXX    ((uint8_t)0x7)
#define ICALL   ((uint8_t)0x8)
#define IRET    ((uint8_t)0x9)
#define IPUSHQ  ((uint8_t)0xa)
#define IPOPQ   ((uint8_t)0xb)
#define IEND    ((uint8_t)0xf)

#define DNONE   ((uint8_t)0x0)
#define DIMD    ((uint8_t)0x0)
#define DBYTE   ((uint8_t)0x1)
#define DWORD   ((uint8_t)0x2)
#define DLONG   ((uint8_t)0x4)
#define DQUAD   ((uint8_t)0x8)
#define DPOS    ((uint8_t)0x1)
#define DALIGN  ((uint8_t)0x2)
#define DEND    ((uint8_t)0xf)

#define RNONE   ((uint8_t)0xf)
#define RRAX    ((uint8_t)0x0)
#define RRSP    ((uint8_t)0x4)
#define RRA     ((uint8_t)0x10)
#define RRB     ((uint8_t)0x11)

#define SEND    ((uint8_t)0x0)
#define SAOK    ((uint8_t)0x1)
#define SADR    ((uint8_t)0x2)
#define SINS    ((uint8_t)0x3)
#define SHLT    ((uint8_t)0x4)

#define FNONE   ((uint8_t)0x0)
#define FADDQ   ((uint8_t)0x0)
#define FSUBQ   ((uint8_t)0x1)
#define FANDQ   ((uint8_t)0x2)
#define FXORQ   ((uint8_t)0x3)
#define FIMULQ  ((uint8_t)0x4)
#define FIDIVQ  ((uint8_t)0x5)

#define CNONE   ((uint8_t)0x0)
#define CLE     ((uint8_t)0x1)
#define CL      ((uint8_t)0x2)
#define CE      ((uint8_t)0x3)
#define CNE     ((uint8_t)0x4)
#define CGE     ((uint8_t)0x5)
#define CG      ((uint8_t)0x6)

#define ETEXT   ((uint8_t)0x0)
#define EDATA   ((uint8_t)0x1)

#define V0      ((uint64_t) 0 + 1)
#define VP8     ((uint64_t)+8 + 1)
#define VN8     ((uint64_t)-8 + 1)
#define VA      ((uint64_t)&((instr_t *)NULL)->valA)
#define VB      ((uint64_t)&((instr_t *)NULL)->valB)
#define VC      ((uint64_t)&((instr_t *)NULL)->valC)

#define MNONE   ((uint8_t)0)
#define MRD     ((uint8_t)1)
#define MWR     ((uint8_t)2)

extern const uint64_t instr_size[16];
extern const uint8_t instr_srcA[16];
extern const uint8_t instr_srcB[16];
extern const uint8_t instr_dstE[16];
extern const uint8_t instr_dstM[16];
extern const uint64_t instr_aluA[16];
extern const uint64_t instr_aluB[16];
extern const uint8_t instr_memio[16];
extern const uint64_t instr_memaddr[16];

#define LINE_MAX    ((size_t)512)
#define TOKLEN_MAX  ((size_t)32)
#define TOKNUM_MAX  ((size_t)16)
#define LABNUM_MAX  ((size_t)64)
#define MEM_MAX     ((size_t)1 << 20)

#define TOKNONE     ((uint8_t)0x00)
#define TOKANY      ((uint8_t)0x80)
#define TOKINS      ((uint8_t)0x81)
#define TOKREG      ((uint8_t)0x82)
#define TOKRA       ((uint8_t)0x83)
#define TOKRB       ((uint8_t)0x84)
#define TOKIMD      ((uint8_t)0x85)
#define TOKADR      ((uint8_t)0x86)
#define TOKDIR      ((uint8_t)0x87)
#define TOKLAB      ((uint8_t)0x88)
#define TOKERR      ((uint8_t)0xff)

#define LEMPTY      ((uint8_t)0x00)
#define LINSTR      ((uint8_t)0x01)
#define LDIREC      ((uint8_t)0x02)
#define LLABEL      ((uint8_t)0x03)
#define LERROR      ((uint8_t)0xff)

typedef struct {
  char      name[TOKLEN_MAX];
  uint64_t  addr;
} lab_t;

extern const uint8_t instr_syntax[16][TOKNUM_MAX];
extern const char *instr_name[16][16];
extern const char *reg_name[16];
extern const char *stat_name[16];
extern const uint8_t direc_syntax[16][TOKNUM_MAX];
extern const char *direc_name[16][16];

extern pstat_t pstat;
extern instr_t instr;
extern char *mem;
extern uint64_t mem_size;
extern opt_t opt;
extern lab_t lab[LABNUM_MAX];
extern lab_t ref[LABNUM_MAX];
extern size_t labsiz;
extern size_t labpnd;
extern size_t refsiz;
extern size_t align;

void pstat_init(void);
void mem_load(const char *pathname);
void instr_fetch(void);
void instr_decode(void);
void instr_execute(void);
void instr_memory(void);
void instr_writeback(void);
void instr_process(void);
void process_loop(void);
void print_loop(FILE *stream);
void run(const char *pathname);
void disas(const char *pathname, FILE *stream);
int setopt(int argc, char *argv[]);
void print_instr(FILE *stream);
void print_instr_addr(FILE *stream);
void print_instr_enc(FILE *stream);
int get_line(FILE *stream, char *line);
uint8_t scan_tok_any(const char **pcur, const char **pend);
uint8_t scan_tok(const char **pcur, const char **pend);
uint8_t scan_line(const char *line);
uint8_t scan_direc(const char *line, const char **cur, const char **end);
uint8_t scan_instr(const char *line, const char **cur, const char **end);
int pend_lab(const char *name);
void write_lab(uint64_t addr);
int insert_ref(const char *name, uint64_t addr);
void resolve_ref(const lab_t *pref);
const lab_t *retrieve_lab(const char *name);
void tokcpy(char *tok, const char *cur, const char *end);
void insert_imd(size_t size, uint64_t imd);
void goto_pos(uint64_t pos);
void set_align(size_t size);
void apply_align(size_t size, uint64_t *pos);
void expand_for(size_t size);
void scan_loop(FILE *stream);
void as(const char *pathname, FILE *stream);
