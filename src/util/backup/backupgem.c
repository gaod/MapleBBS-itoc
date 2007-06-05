/*-------------------------------------------------------*/
/* util/backupgem.c	( NTHU MapleBBS Ver 3.10 )       */
/*-------------------------------------------------------*/
/* target : �ƥ��Ҧ���ذϸ��                           */
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
  char gempath[128], bakpath[128], cmd[256];
  time_t now;
  struct tm *ptime;

  chdir(BBSHOME);
  umask(077);

  /* �إ߳ƥ����|�ؿ� */
  time(&now);
  ptime = localtime(&now);
  sprintf(bakpath, "%s/gem%02d%02d%02d", BAKPATH, ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
  mkdir(bakpath, 0700);

  if (chdir(BBSHOME "/gem") || !(dirp = opendir(".")))
    exit(-1);

  sprintf(cmd, "cp %s %s/", FN_DIR, bakpath);
  system(cmd);  

  /* �� 0~9 @ A~V ���O���Y���@�����Y�� */
  while (de = readdir(dirp))
  {
    ptr = de->d_name;

    /* �ݪO����ذϥt�~�ƥ� */
    if (!strcmp(ptr, "brd"))
      continue;

    if (ptr[0] > ' ' && ptr[0] != '.')
    {
      sprintf(cmd, "tar cfz %s/%s.tgz ./%s", bakpath, ptr, ptr);
      system(cmd);
    }
  }
  closedir(dirp);


  /* �ƥ��ݪO */

  if (chdir(BBSHOME "/gem/brd") || !(dirp = opendir(".")))
    exit(-1);

  /* �إ߳ƥ����|�ؿ� */
  sprintf(gempath, "%s/brd", bakpath);
  mkdir(gempath, 0700);

  /* ��U�ݪO���O���Y���@�����Y�� */
  while (de = readdir(dirp))
  {
    ptr = de->d_name;

    if (ptr[0] > ' ' && ptr[0] != '.')
    {
      sprintf(cmd, "tar cfz %s/%s.tgz ./%s", gempath, ptr, ptr);
      system(cmd);
    }
  }
  closedir(dirp);

  exit(0);
}
