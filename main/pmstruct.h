#ifndef __PMSTRUCT__
#define __PMSTRUCT__
#include <stdio.h>
#include <stdlib.h>

typedef struct pmstruct{
  int tipo;
  int numero;
  struct pmstruct* next;
}pmstruct;

struct pmstruct* pm;

struct pmstruct* pmstructNew(int,int);
void pmstructInclui(struct pmstruct**,int,int);
int pmstructPrint(struct pmstruct*,char**);
void pmstructClear(struct pmstruct**);

#endif
