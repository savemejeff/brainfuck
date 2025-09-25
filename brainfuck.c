#include <stdio.h>
#include <stdlib.h>

int stack[1024];
int *sp;
int l;

static void push(int n)
{
  *(sp++) = n;
}

static int pop()
{
  return *(--sp);
}

static int label()
{
  return l++;
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    printf("Usage: %s infile\n", argv[0]);
    exit(1);
  }

  const char *sour = argv[1];
  const char *dest = "a.s";
  FILE *s = fopen(sour, "rb+");
  FILE *d = fopen(dest, "wb+");
  int lab;
  sp = stack;
  l = 0;

  fputs("\t.bss\n", d);
  fputs("buffer:\t.skip 1024\n", d);
  fputs("\t.globl\tmain\n", d);
  fputs("\t.text\n", d);
  fputs("main:\n", d);
  fputs("\tpushq\t%rbp\n", d);
  fputs("\tmovq\t%rsp, %rbp\n", d);
  fputs("\tleaq\tbuffer(%rip), %r12\n", d);

  int c;
  while ((c = fgetc(s)) != EOF)
  {
    switch (c)
    {
    case '>':
      fputs("\tadd\t$1, %r12\n", d);
      break;
    case '<':
      fputs("\tsub\t$1, %r12\n", d);
      break;
    case '+':
      fputs("\tmovb\t(%r12), %al\n", d);
      fputs("\tadd\t$1, %al\n", d);
      fputs("\tmovb\t%al, (%r12)\n", d);
      break;
    case '-':
      fputs("\tmovb\t(%r12), %al\n", d);
      fputs("\tsub\t$1, %al\n", d);
      fputs("\tmovb\t%al, (%r12)\n", d);
      break;
    case ',':
      fputs("\tmovl	$0, %eax\n", d);
      fputs("\tmovl	$0, %edi\n", d);
      fputs("\tleaq	(%r12), %rsi\n", d);
      fputs("\tmovl	$1, %edx\n", d);
      fputs("\tsyscall\n", d);
      break;
    case '.':
      fputs("\tmovl	$1, %eax\n", d);
      fputs("\tmovl	$1, %edi\n", d);
      fputs("\tleaq	(%r12), %rsi\n", d);
      fputs("\tmovl	$1, %edx\n", d);
      fputs("\tsyscall\n", d);
      break;
    case '[':
      lab = label();
      fprintf(d, "LS%d:\n", lab);
      fprintf(d, "\tcmpb\t$0, (%%r12)\n");
      fprintf(d, "\tje\tLE%d\n", lab);
      push(lab);
      break;
    case ']':
      lab = pop();
      fprintf(d, "\tcmpb\t$0, (%%r12)\n");
      fprintf(d, "\tjne\tLS%d\n", lab);
      fprintf(d, "LE%d:\n", lab);
      break;
    default:
      // fatal();
      break;
    }
  }

  fputs("\tmovl\t$0, %eax\n", d);
  fputs("\tleave\n", d);
  fputs("\tret\n", d);

  return 0;
}