#include "pmstruct.h"

char inicial[]="<!DOCTYPE html>\n<html>\n <head>\n  <meta charset=\"UTF-8\">\n </head>\n <body>\n  <h3>Eventos de troca de contexto</h3>\n";

char final[]="</body>\n</html>";

struct pmstruct* pmstructNew(int tipo,int numero)
{
  struct pmstruct* retorno;
  retorno = (struct pmstruct*) malloc(sizeof(pmstruct));
  retorno->tipo=tipo;
  retorno->numero=numero;
  retorno->next=NULL;
  return retorno;
}

void pmstructInclui(pmstruct** pm,int tipo,int numero)
{
  if(*pm)
  {
    if((*pm)->next){
      struct pmstruct* proximo;
      proximo = (*pm)->next;
      while(proximo->next)
        proximo=proximo->next;
      proximo->next=pmstructNew(tipo,numero);
    }
    else
      (*pm)->next=pmstructNew(tipo,numero);
  }
  else
    *pm=pmstructNew(tipo,numero);
}

int pmstructCount(pmstruct* pm)
{
  int retorno=0;
  pmstruct* aux=pm;
  while(aux){
    aux=aux->next;
    retorno++;
  }
  return retorno;
}

int pmstructPrint(pmstruct* pm,char** texto)
{
  int retorno=1;
  if(pm){
    int qnd=pmstructCount(pm);
    retorno=sizeof(char)*(sizeof(inicial)+9*qnd+7+sizeof(final));
    (*texto) = (char*)malloc(retorno);
    char* aux;
    aux = *texto;
    sprintf(aux,"%s",inicial);
    aux=aux+sizeof(inicial);
    sprintf(aux,"%d<br>\n",qnd);
    aux=aux+7;
    pmstruct* proximo;
    proximo=pm;
    for(int i=0;i<qnd;i++)
    {
      sprintf(aux,"%d,%d<br>\n",proximo->tipo,proximo->numero);
      aux=aux+9;
      proximo=proximo->next;
    }
    sprintf(aux,"%s",final);
    //sprintf(*texto,"%s%d,%d,%d%s",inicial,pm->tipo,pm->numero,qnd,final);
  }
  else{
    char ret[]="Dados Vasios";
    retorno=sizeof(char)*sizeof(ret);
    *texto = (char*)malloc(retorno);
    sprintf(*texto,"%s",ret);
  }
  return retorno;
}

void pmstructClear(pmstruct** pm)
{
}
