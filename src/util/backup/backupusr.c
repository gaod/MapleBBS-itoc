/*-------------------------------------------------------*/
/* util/backupusr.c	( NTHU MapleBBS Ver 3.10 )       */
/*-------------------------------------------------------*/
/* target : �ƥ��Ҧ��ϥΪ̸��                           */
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
  char *ptr, ch;
  char fpath[128], bakpath[128], cmd[256];
  time_t now;
  struct tm *ptime;

  time(&now);
  ptime = localtime(&now);
  /* �إ߳ƥ����|�ؿ� */
  sprintf(fpath, "%s/usr%02d%02d%02d", BAKPATH, ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
  mkdir(fpath, 0755);

  /* �����v���� ftp ���ɤ��|�|�� */
  sprintf(cmd, "cp %s/%s %s/; chmod 644 %s/%s", BBSHOME, FN_SCHEMA, fpath, fpath, FN_SCHEMA);
  system(cmd);

  for (ch = 'a'; ch <= 'z'; ch++)
  {
    sprintf(cmd, "%s/usr/%c", BBSHOME, ch);
    if (chdir(cmd) || !(dirp = opendir(".")))
      exit(-1);

    sprintf(bakpath, "%s/%c", fpath, ch);
    mkdir(bakpath, 0755);

    /* ��U�ϥΪ̤��O���Y���@�����Y�� */
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
  }

  exit(0);
}
