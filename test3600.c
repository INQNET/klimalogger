/* ========================================================================== */
/*                                                                            */
/*   test3600.c                                                               */
/*   (C)2005 Grzegorz Wisniewski,Sander Eerkes. (Version alfa)                */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

#include "test3600.h"
#include "rw3600.h"

void wait ( int seconds )
{
  clock_t endwait;
  endwait = clock () + seconds * CLOCKS_PER_SEC ;
  while (clock() < endwait) {}
}



int main ()
{
  int n;
  printf ("Starting countdown...\n");
  
  printf("CLOCKS_PER_SEC=%il\n",CLOCKS_PER_SEC);
  for (n=10; n>0; n--)
  {
    printf ("%d\n",n);
    wait (1);
  }
  printf ("FIRE!!!\n");
  return 0;
}
