/*-------------------------------------------------------*/
/* util/restoregem.c    ( NTHU MapleBBS Ver 3.10 )       */
/*-------------------------------------------------------*/
/* target : �٭�Ҧ���ذϸ��                           */
/* create : 01/10/26                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#if 0

   BAKPATH �U�|���ܦh���P����ƥ�����ذϸ�ơA�Ҧp gem010101 gem010102
   ��n�_�쪺������W�� gem  (mv gem010101 gem)
   ���楻�{���Y�i�����_��

#endif


#include "bbs.h"


int
main()
{
  struct dirent *de;
  DIR *dirp;
  char *ptr;
  char gempath[128], cmd[256];

  if (chdir(BAKPATH "/gem") || !(dirp = opendir(".")))
    exit(-1);

  strcpy(gempath, BBSHOME "/gem");
  mkdir(gempath, 0700);

  sprintf(cmd, "cp %s %s/", FN_DIR, gempath);
  system(cmd);

  /* �� 0~9 @ A~V ���O�����Y�^�� */
  while (de = readdir(dirp))
  {
    ptr = de->d_name;

    /* �ݪO����ذϥt�~�����Y */
    if (!strcmp(ptr, "brd"))
      continue;

    if (ptr[0] > ' ' && ptr[0] != '.')
    {
      sprintf(cmd, "tar xfz %s -C %s/", ptr, gempath);
      system(cmd);
    }
  }
  closedir(dirp);


  /* �^�_�ݪO */

  if (chdir(BAKPATH "/gem/brd") || !(dirp = opendir(".")))
    exit(-1);

  strcat(gempath, "/brd");
  mkdir(gempath, 0700);

  /* ��U�ݪO���O�����Y�^�� */
  while (de = readdir(dirp))
  {
    ptr = de->d_name;

    if (ptr[0] > ' ' && ptr[0] != '.')
    {
      sprintf(cmd, "tar xfz %s -C %s/", ptr, gempath);
      system(cmd);
    }
  }
  closedir(dirp);

  exit(0);
}
