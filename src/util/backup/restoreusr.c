/*-------------------------------------------------------*/
/* util/restoreusr.c	( NTHU MapleBBS Ver 3.10 )       */
/*-------------------------------------------------------*/
/* target : �٭�Ҧ��ϥΪ̸��                           */
/* create : 01/10/26                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

   BAKPATH �U�|���ܦh���P����ƥ����ϥΪ̸�ơA�Ҧp usr010101 usr010102
   ��n�_�쪺������W�� usr  (mv usr010101 usr)
   ���楻�{���Y�i�����_��

#endif


#include "bbs.h"


int
main()
{
  struct dirent *de;
  DIR *dirp;
  char *ptr, ch;
  char usrpath[128], cmd[256];

  if (chdir(BAKPATH "/usr"))
    exit(-1);

  sprintf(cmd, "cp %s %s/%s; chmod 600 %s/%s", FN_SCHEMA, BBSHOME, FN_SCHEMA, BBSHOME, FN_SCHEMA);
  system(cmd);

  strcpy(usrpath, BBSHOME "/usr");
  mkdir(usrpath, 0700);

  for (ch = 'a'; ch <= 'z'; ch++)
  {
    sprintf(cmd, "%s/usr/%c", BAKPATH, ch);
    if (chdir(cmd) || !(dirp = opendir(".")))
      exit(-1);

    sprintf(cmd, "%s/%c", usrpath, ch);
    mkdir(cmd, 0700);

    /* ��U�ϥΪ̤��O�����Y�^�� */
    while (de = readdir(dirp))
    {
      ptr = de->d_name;

      if (ptr[0] > ' ' && ptr[0] != '.')
      {
        sprintf(cmd, "tar xfz %s -C %s/%c/", ptr, usrpath, ch);
	system(cmd);
      }
    }
    closedir(dirp);
  }

  exit(0);
}
