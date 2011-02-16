/* 

  cdocstring.

  Print the top of a file until the first closing comment *-and-/ are reached
  The newlines are converted to \n for printing. Double quotes are converted
  to \\\"

*/

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{

  char *file = argv[1];
  FILE *fptr;
  char c, clast;

  if (argc < 2)
    {
      printf("Usage: cdocstring file.c\n");
      return(1);
    }

  fptr = fopen(file, "r");
  
  c = 'q';
  do {
    clast = c;
    c = getc(fptr);
    if (c == '\n')
      printf("\\n");
    else if (c == '"')
      printf("\\\"");
    else
      printf("%c",c);
  } while ( ! ( (clast == '*') && (c == '/') ) && c != EOF);
  
  return(0);
}
