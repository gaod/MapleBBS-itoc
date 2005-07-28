/*-------------------------------------------------------*/
/* util/restorebrd.c	( NTHU MapleBBS Ver 3.10 )       */
/*-------------------------------------------------------*/
/* target : �٭�Ҧ��ݪO���                             */
/* create : 01/10/26                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

   BAKPATH �U�|���ܦh���P����ƥ����ݪO��ơA�Ҧp brd010101 brd010102
   ��n�_�쪺������W�� brd  (mv brd010101 brd)
   ���楻�{���Y�i�����_��

#endif


#include "bbs.h"


int
main()
{
  struct dirent *de;
  DIR *dirp;
  char *ptr;
  char brdpath[128], cmd[256];

  if (chdir(BAKPATH "/brd") || !(dirp = opendir(".")))
    exit(-1);

  sprintf(cmd, "cp %s %s/%s; chmod 600 %s/%s", FN_BRD, BBSHOME, FN_BRD, BBSHOME, FN_BRD);
  system(cmd);

  strcpy(brdpath, BBSHOME "/brd");
  mkdir(brdpath, 0700);

  while (de = readdir(dirp))
  {
    ptr = de->d_name;

    if (!strcmp(ptr, "BRD"))
      continue;

    if (ptr[0] > ' ' && ptr[0] != '.')
    {
      sprintf(cmd, "tar xfz %s -C %s/", ptr, brdpath);
      system(cmd);
    }
  }
  closedir(dirp);

  exit(0);
}
