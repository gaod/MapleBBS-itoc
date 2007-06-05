/*-------------------------------------------------------*/
/* util/backupbrd.c	( NTHU MapleBBS Ver 3.10 )       */
/*-------------------------------------------------------*/
/* target : �ƥ��Ҧ��ݪO���                             */
/* create : 01/10/19                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"


int
main()
{
  struct dirent *de;
  DIR *dirp;
  char *ptr;
  char bakpath[128], cmd[256];
  time_t now;
  struct tm *ptime;

  chdir(BBSHOME);
  umask(077);

  /* �إ߳ƥ����|�ؿ� */
  time(&now);
  ptime = localtime(&now);
  sprintf(bakpath, "%s/brd%02d%02d%02d", BAKPATH, ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
  mkdir(bakpath, 0700);

  /* �ƥ� .BRD */
  sprintf(cmd, "%s/%s", bakpath, FN_BRD);
  f_cp(FN_BRD, cmd, O_EXCL);

  if (chdir(BBSHOME "/brd") || !(dirp = opendir(".")))
    exit(-1);

  /* ��U�ݪO���O���Y���@�����Y�� */
  while (de = readdir(dirp))
  {
    ptr = de->d_name;

    if (ptr[0] > ' ' && ptr[0] != '.')
    {
      sprintf(cmd, "tar cfz %s/%s.tgz ./%s", bakpath, ptr, ptr);
      system(cmd);
    }
  }
  closedir(dirp);

  exit(0);
}
