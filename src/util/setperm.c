/*-------------------------------------------------------*/
/* util/setperm.c	( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : �]�w�ϥΪ��v��                               */
/* author : gslin@abpe.org                               */
/* create : 00/07/28                                     */
/* update :                                              */
/*-------------------------------------------------------*/


#if 0
   �N itoc �� sysop �]���Ҧ��v������:
   setperm -1 itoc sysop
   setperm 11111111111111111111111111111111 itoc sysop

   �N itoc �]���u�����v��:
   setperm 1 itoc
   setperm 00000000000000000000000000000001 itoc

   �N guest �]���S���v��:
   setperm 0 guest
   setperm 00000000000000000000000000000000 guest
#endif


#include "bbs.h"


static void
usage(msg)
  char *msg;
{
  printf("Usage: %s Perm32 UserID1 [UserID2] ...\n", msg);
  exit(1);
}


int 
main(argc, argv)
  int argc;
  char *argv[];
{
  int i;
  usint userlevel;

  if (argc < 3)
    usage(argv[0]);

  if (strlen(argv[1]) != 32)
  {
    userlevel = (usint) atoi(argv[1]);
  }
  else
  {
    userlevel = 0;

    for (i = 0; i < 32; i++)
    {
      char c = argv[1][i];

      if (c != '0' && c != '1')
	usage(argv[0]);

      userlevel <<= 1;
      userlevel |= c - '0';
    }
  }

  chdir(BBSHOME);

  for (i = 2; i < argc; i++)
  {
    ACCT cuser;
    char fpath[64];

    usr_fpath(fpath, argv[i], FN_ACCT);
    if (rec_get(fpath, &cuser, sizeof(cuser), 0) < 0)
    {
      printf("%s: read error (maybe no such id?)\n", argv[i]);
      continue;
    }

    cuser.userlevel = userlevel;

    if (rec_put(fpath, &cuser, sizeof(cuser), 0, NULL) < 0)
      printf("%s: write error\n", argv[i]);
  }
}
