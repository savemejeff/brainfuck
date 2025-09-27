#include <stdio.h>
#include <stdlib.h>

#define STACK_SIZE 1024

int g_stack[STACK_SIZE];
int *g_sp;
int g_label;
int g_line;
int g_col;
int g_pucback;
FILE *g_sourstream;
FILE *g_deststream;

static void fatal(const char *s)
{
  fprintf(stderr, "Error: %s; on Line %d, col %d.\n", s, g_line, g_col);
  exit(1);
}

static void push(int n)
{
  if (g_sp - g_stack >= STACK_SIZE)
  {
    fatal("stack overflow");
  }
  *(g_sp++) = n;
}

static int pop()
{
  if (g_sp == g_stack)
  {
    fatal("stack overflow");
  }
  return *(--g_sp);
}

static int newlabel()
{
  return g_label++;
}

static void initglob()
{
  g_sp = g_stack;
  g_label = 0;
  g_line = 0;
  g_col = 0;
  g_sourstream = NULL;
  g_deststream = NULL;
}

static void putback(int c)
{
  g_pucback = c;
}

static int next()
{
  int c;
  if (g_pucback != 0)
  {
    c = g_pucback;
    g_pucback = 0;
    return c;
  }

  c = fgetc(g_sourstream);
  g_col++;
  if (c == '\n')
  {
    g_col = 0;
    g_line++;
  }
  return c;
}

static void usage(const char *exe)
{
  printf("Usage: %s [-o outfile] infile\n", exe);
  exit(-1);
}

static void genpreamble()
{
  fputs("\t.bss\n", g_deststream);
  fputs("buffer:\t.skip 30000\n", g_deststream);
  fputs("bufferend:", g_deststream);

  fputs("\t.text\n", g_deststream);
  fputs("\t.section\t.rodata\n", g_deststream);
  fputs(".LC0:\n", g_deststream);
  fputs("\t.string\t\"Runtime error: "
        "pointer out of boundary.\"\n",
        g_deststream);
  fputs("\t.text\n", g_deststream);
  fputs("error:\n", g_deststream);
  fputs("\tendbr64\n", g_deststream);
  fputs("\tpushq\t%rbp\n", g_deststream);
  fputs("\tmovq\t%rsp, %rbp\n", g_deststream);
  fputs("\tleaq\t.LC0(%rip), %rax\n", g_deststream);
  fputs("\tmovq\t%rax, %rdi\n", g_deststream);
  fputs("\tcall\tputs@PLT\n", g_deststream);
  fputs("\tmovl\t$1, %edi\n", g_deststream);
  fputs("\tcall\texit@PLT\n", g_deststream);

  fputs("\t.globl\tmain\n", g_deststream);
  fputs("\t.text\n", g_deststream);
  fputs("main:\n", g_deststream);
  fputs("\tpushq\t%rbp\n", g_deststream);
  fputs("\tmovq\t%rsp, %rbp\n", g_deststream);
  fputs("\tleaq\tbuffer(%rip), %r12\n", g_deststream);
  fputs("\tleaq\tbuffer(%rip), %r13\n", g_deststream);
  fputs("\tleaq\tbufferend(%rip), %r14\n", g_deststream);
}

static void genpostamble()
{
  fputs("\tmovl\t$0, %eax\n", g_deststream);
  fputs("\tleave\n", g_deststream);
  fputs("\tret\n", g_deststream);
}

static void genleft(int n)
{
  int l = newlabel();
  fprintf(g_deststream, "\tsub\t$%d, %%r12\n", n);
  fputs("\tcmpq\t%r13, %r12\n", g_deststream);
  fprintf(g_deststream, "\tjnb\tL%d\n", l);
  fputs("\tmovl\t$0, %eax\n", g_deststream);
  fputs("\tcall\terror\n", g_deststream);
  fprintf(g_deststream, "L%d:\n", l);
}

static void genright(int n)
{
  int l = newlabel();
  fprintf(g_deststream, "\tadd\t$%d, %%r12\n", n);
  fputs("\tcmpq\t%r14, %r12\n", g_deststream);
  fprintf(g_deststream, "\tjb\tL%d\n", l);
  fputs("\tmovl\t$0, %eax\n", g_deststream);
  fputs("\tcall\terror\n", g_deststream);
  fprintf(g_deststream, "L%d:\n", l);
}

static int more(int c)
{
  int n, cnt = 0;
  while ((n = next()) != EOF)
  {
    if (n == c)
      cnt++;
    else
    {
      putback(n);
      break;
    }
  }
  return cnt;
}

int main(int argc, char **argv)
{
  initglob();

  const char *sour = NULL;
  const char *dest = "a.s";
  for (int i = 1; i < argc; i++)
  {
    const char *p = argv[i];
    if (p[0] != '-')
    {
      sour = p;
      continue;
    }

    switch (p[1])
    {
    case 'o':
      i++;
      dest = argv[i];
      break;
    default:
      printf("Unknown flag: %c\n", p[1]);
      exit(1);
    }
  }

  if (sour == NULL)
  {
    usage(argv[0]);
  }

  g_sourstream = fopen(sour, "rb+");
  g_deststream = fopen(dest, "wb+");
  if (g_sourstream == NULL)
  {
    fprintf(stderr, "Cannon open '%s'.\n", sour);
    exit(-1);
  }
  if (g_deststream == NULL)
  {
    fprintf(stderr, "Cannot open '%s'\n", dest);
    exit(-1);
  }

  int c;
  int lab;
  int cnt;

  genpreamble();

  while ((c = next()) != EOF)
  {
    switch (c)
    {
    case '>':
      cnt = 1 + more(c);
      genright(cnt);
      break;
    case '<':
      cnt = 1 + more(c);
      genleft(cnt);
      break;
    case '+':
      cnt = 1 + more(c);
      fputs("\tmovb\t(%r12), %al\n", g_deststream);
      fprintf(g_deststream, "\tadd\t$%d, %%al\n", cnt);
      fputs("\tmovb\t%al, (%r12)\n", g_deststream);
      break;
    case '-':
      cnt = 1 + more(c);
      fputs("\tmovb\t(%r12), %al\n", g_deststream);
      fprintf(g_deststream, "\tsub\t$%d, %%al\n", cnt);
      fputs("\tmovb\t%al, (%r12)\n", g_deststream);
      break;
    case ',':
      fputs("\tmovl\t$0, %eax\n", g_deststream);
      fputs("\tmovl\t$0, %edi\n", g_deststream);
      fputs("\tleaq\t(%r12), %rsi\n", g_deststream);
      fputs("\tmovl\t$1, %edx\n", g_deststream);
      fputs("\tsyscall\n", g_deststream);
      break;
    case '.':
      fputs("\tmovl\t$1, %eax\n", g_deststream);
      fputs("\tmovl\t$1, %edi\n", g_deststream);
      fputs("\tleaq\t(%r12), %rsi\n", g_deststream);
      fputs("\tmovl\t$1, %edx\n", g_deststream);
      fputs("\tsyscall\n", g_deststream);
      break;
    case '[':
      lab = newlabel();
      fprintf(g_deststream, "LS%d:\n", lab);
      fprintf(g_deststream, "\tcmpb\t$0, (%%r12)\n");
      fprintf(g_deststream, "\tje\tLE%d\n", lab);
      push(lab);
      break;
    case ']':
      lab = pop();
      fprintf(g_deststream, "\tcmpb\t$0, (%%r12)\n");
      fprintf(g_deststream, "\tjne\tLS%d\n", lab);
      fprintf(g_deststream, "LE%d:\n", lab);
      break;
    default:
      // ignore any other characters
      break;
    }
  }

  genpostamble();

  if (g_sp != g_stack)
  {
    fatal("missing ']'");
  }

  return 0;
}