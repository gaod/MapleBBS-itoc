/*-------------------------------------------------------*/
/* util/showUSR.c       ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : �q�X FN_SCHEMA                               */
/* create :   /  /                                       */
/* update : 02/11/03                                     */
/*-------------------------------------------------------*/
/* syntax : showUSR                                      */
/*-------------------------------------------------------*/


#include "bbs.h"


int 
main()
{
  int fd, n;
  SCHEMA *usr;
  struct stat st;

  chdir(BBSHOME);

  if ((fd = open(FN_SCHEMA, O_RDONLY)) < 0)
  {
    printf("ERROR at open file\n");
    exit(1);
  }

  fstat(fd, &st);
  usr = (SCHEMA *) malloc(st.st_size);
  read(fd, usr, st.st_size);
  close(fd);

  printf("\n%s  ==> %d bytes\n", FN_SCHEMA, st.st_size);

  fd = st.st_size / sizeof(SCHEMA);
  for (n = 0; n < fd; n++)
  {
    /* userno: �b .USR ���O�ĴX�� slot */
    /* uptime: ���U���ɶ� (�Y ID �O�ťիh�O�Q reaper �����ɶ�) */
    /* userid: ID (�Y�O�ťժ�ܦ��H�Q reaper �F) */

    printf("userno:%d  uptime:%s  userid:%-12.12s\n",
      n + 1, Btime(&usr[n].uptime), usr[n].userid);

    if (n % 23 == 22)	/* �C 23 �������N���~�� */
    {
      printf("-== Press ENTER to continue and 'q + ENTER' to quit ==-\n");
      if (getchar() == 'q')
	break;
    }
  }

  free(usr);
  return 0;
}
