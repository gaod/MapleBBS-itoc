/*-------------------------------------------------------*/
/* util/restoreacct.c	( NTHU MapleBBS Ver 3.10 )       */
/*-------------------------------------------------------*/
/* target : �٭�Ҧ��ϥΪ� .ACCT                         */
/* create : 02/03/26                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

   BAKPATH �U�|���ܦh���P����ƥ��� .ACCT ��ơA�Ҧp acct010101 acct010102
   ��n�_�쪺������W�� acct  (mv acct010101 acct)
   ���楻�{���Y�i�����_��

#endif


#include "bbs.h"


int
main()
{
  struct dirent *de;
  DIR *dirp;
  char *ptr, ch;
  char bakpath[128], cmd[256];

  strcpy(bakpath, BAKPATH "/acct");
  if (chdir(bakpath))
    exit(-1);

  sprintf(cmd, "cp %s %s/%s; chmod 600 %s/%s", FN_SCHEMA, BBSHOME, FN_SCHEMA, BBSHOME, FN_SCHEMA);
  system(cmd);

  for (ch = 'a'; ch <= 'z'; ch++)
  {
    sprintf(cmd, "%s/usr/%c", BBSHOME, ch);
    if (chdir(cmd) || !(dirp = opendir(".")))
      exit(-1);

    while (de = readdir(dirp))
    {
      ptr = de->d_name;

      if (ptr[0] > ' ' && ptr[0] != '.')
      {
	sprintf(cmd, "cp %s/%c/%s.ACCT %s/%s", bakpath, ch, ptr, ptr, FN_ACCT);
	system(cmd);
      }
    }
    closedir(dirp);
  }

  exit(0);
}
