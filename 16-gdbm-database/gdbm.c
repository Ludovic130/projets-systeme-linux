#include "lib.h"

void create_gdbm(void)
{
  GDBM_FILE base;
  datum cle;
  datum donnee;

  if((base = gdbm_open("test.gdbm", 512, GDBM_NEWDB, 0644, NULL)) == NULL)
  {
    fprintf(stderr, "error : %s\n",gdbm_strerror(gdbm_errno));
    exit(EXIT_FAILURE);
  }

  cle.dptr = "Name-grandsoeur";
  cle.dsize = 14;
  donnee.dptr = "Diana";
  donnee.dsize = 5;
  gdbm_store(base, cle, donnee, GDBM_REPLACE);

  cle.dptr = "Age-grandsoeur";
  cle.dsize = 14;
  donnee.dptr = "23";
  donnee.dsize = 2;
  gdbm_store(base, cle, donnee, GDBM_REPLACE);

  cle.dptr = "Name-frère";
  cle.dsize = 9;
  donnee.dptr = "David";
  donnee.dsize = 5;
  gdbm_store(base, cle, donnee, GDBM_REPLACE);

  cle.dptr = "Age-frère";
  cle.dsize = 9;
  donnee.dptr = "19";
  donnee.dsize = 2;
  gdbm_store(base, cle, donnee, GDBM_REPLACE); 

  gdbm_close(base);
}

int main(int argc, char *argv[])
{
  GDBM_FILE base;
  datum cle;
  datum donnee;
  
  create_gdbm();

  if(argc != 2)
  {
    fprintf(stderr, "Syntax : %s name_base \n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if((base = gdbm_open(argv[1], 512, GDBM_READER, 0644, NULL)) == NULL)
  {
    fprintf(stderr, "%s : %s\n",argv[1], gdbm_strerror(gdbm_errno));
    exit(EXIT_FAILURE);
  }

  for(cle = gdbm_firstkey(base); cle.dptr != NULL; cle = gdbm_nextkey(base, cle))
  {
    donnee = gdbm_fetch(base, cle);
    if(donnee.dptr != NULL)
    {
      fprintf(stdout, "%s : %s\n", cle.dptr, donnee.dptr);
    }
    free(donnee.dptr);
  }
  gdbm_close(base);

  return EXIT_SUCCESS;
}

