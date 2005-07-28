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

  if (chdir(BBSHOME "/brd") || !(dirp = opendir(".")))
    exit(-1);

  /* �إ߳ƥ����|�ؿ� */
  time(&now);
  ptime = localtime(&now);
  sprintf(bakpath, "%s/brd%02d%02d%02d", BAKPATH, ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
  mkdir(bakpath, 0755);

  /* �����v���� ftp ���ɤ��|�|�� */
  sprintf(cmd, "cp %s/%s %s/; chmod 644 %s/%s", BBSHOME, FN_BRD, bakpath, bakpath, FN_BRD);
  system(cmd);

  /* ��U�ݪO���O���Y���@�����Y�� */
  while (de = readdir(dirp))
  {
    ptr = de->d_name;

    if (ptr[0] > ' ' && ptr[0] != '.')
    {
      sprintf(cmd, "tar cfz %s/%s.tgz %s", bakpath, ptr, ptr);
      system(cmd);
    }
  }
  closedir(dirp);

  exit(0);
}
