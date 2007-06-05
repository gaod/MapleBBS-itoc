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
  char usrpath[128], bakpath[128], cmd[256];
  time_t now;
  struct tm *ptime;

  chdir(BBSHOME);
  umask(077);

  /* �إ߳ƥ����|�ؿ� */
  time(&now);
  ptime = localtime(&now);
  sprintf(bakpath, "%s/usr%02d%02d%02d", BAKPATH, ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
  mkdir(bakpath, 0700);

  /* �ƥ� .USR */
  sprintf(cmd, "%s/%s", bakpath, FN_SCHEMA);
  f_cp(FN_SCHEMA, cmd, O_EXCL);

  for (ch = 'a'; ch <= 'z'; ch++)
  {
    sprintf(cmd, "%s/usr/%c", BBSHOME, ch);
    if (chdir(cmd) || !(dirp = opendir(".")))
      exit(-1);

    sprintf(usrpath, "%s/%c", bakpath, ch);
    mkdir(usrpath, 0700);

    /* ��U�ϥΪ̤��O���Y���@�����Y�� */
    while (de = readdir(dirp))
    {
      ptr = de->d_name;

      if (ptr[0] > ' ' && ptr[0] != '.')
      {
	sprintf(cmd, "tar cfz %s/%s.tgz ./%s", usrpath, ptr, ptr);
	system(cmd);
      }
    }
    closedir(dirp);
  }

  exit(0);
}
