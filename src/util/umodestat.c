/*-------------------------------------------------------*/
/* util/modestat.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : �έp�ϥΪ̰ʺA                               */
/* create : 95/11/21				 	 */
/* update : 97/11/21				 	 */
/*-------------------------------------------------------*/
/* syntax : modestat [log_filename]                      */
/*-------------------------------------------------------*/


#define _MODES_C_

#include "bbs.h"


int
main(argc, argv)
  int argc;
  char *argv[];
{
#ifdef MODE_STAT
  char *fname;
  FILE *fp;
  UMODELOG mlog;
  register int i, c;
  char buf[80];
  time_t sum;

  if (argc < 2)
  {
    chdir(BBSHOME);
    fname = FN_RUN_MODE_CUR;
  }
  else
    fname = argv[1];

  if (!(fp = fopen(fname, "rb")))
  {
    perror("Can't open file");
    exit(1);
  }

  while (fread(&mlog, sizeof(UMODELOG), 1, fp))
  {
    for (i = c = 0, sum = 0; i <= M_MAX; i++)
    {
      struct tm *tt;

      sum += mlog.used_time[i];
      tt = localtime(&mlog.used_time[i]);
      sprintf(buf + (c * 22), "%-12s%02d��%02d��  ", ModeTypeTable[i], tt->tm_min, tt->tm_sec);

      if (++c == 3)
      {
	printf(buf);
	printf("\n");
	c = 0;
      }
    }
    printf("�`�@���d�ɶ�: %s\n", Btime(&sum));
  }


  fclose(fp);
#endif	/* MODE_STAT */

  exit(0);
}
