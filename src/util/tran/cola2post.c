/*-------------------------------------------------------*/
/* util/cola2post.c	( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : Cola �� Maple 3.02 �ݪO�峹�榡�ഫ		 */
/* create : 03/02/21					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"


#if 0

  �H�U�o�ӬO ColaBBS �峹�ɮת��d��

*[m*[47;34m �@�� *[44;37m userid (�ʺ�)                                 *[47;34m �H�� *[44;37m SYSOP               *[m\n\r
*[47;34m ���D *[44;37m Re: �հհհհհ�                                               *[m\n\r
*[47;34m �ɶ� *[44;37m Fri Mar 15 11:33:20 2002                                       *[m\n\r
*[36m�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w*[m\n\r
\n\r
�峹���e�Ĥ@��\n\r
�峹���e�ĤG��\n\r

  �n�令�o��

�@��: userid (�ʺ�) ����: SYSOP\n
���D: Re: �հհհհհ�\n
�ɶ�: Fri Mar 15 11:33:20 2002\n
\n
�峹���e�Ĥ@��\n
�峹���e�ĤG��\n

#endif


static void
reaper(fpath)
  char *fpath;
{
  FILE *fpr, *fpw;
  char src[256], dst[256];
  char fnew[64];
  char *ptr;
  int i;

  if (!(fpr = fopen(fpath, "r")))
    return;

  sprintf(fnew, "%s.new", fpath);
  fpw = fopen(fnew, "w");

  i = 0;
  while (fgets(src, 256, fpr))
  {
    if (ptr = strchr(src, '\r'))
      *ptr = '\0';
    if (ptr = strchr(src, '\n'))
      *ptr = '\0';

    if (i < 4)	/* �e�|�����Y */
    {
      i++;

      if (*src == '\033')	/* �������Y */
      {
        str_ansi(dst, src, sizeof(dst));	/* �h�� ANSI */

	if (i <= 3 && dst[0] == ' ' && dst[5] == ' ')	/* �@��: */
	{
	  dst[5] = ':';
	  fprintf(fpw, "%.78s\n", dst + 1);	/* �h���Ĥ@��ť� */
	  continue;
	}
	else if (i == 4 && !strncmp(dst, "�w�w�w�w", 8))
	{
	  /* ���j�u���n�F */
	  continue;
	}
      }
    }

    fprintf(fpw, "%s\n", src);	/* ���e�ӧ� */
  }

  fclose(fpr);
  fclose(fpw);

  unlink(fpath);
  rename(fnew, fpath);
}


static void
expireBrd(brdname)
  char *brdname;
{
  int fd;
  char folder[64], fpath[64];
  HDR hdr;

  printf("�ഫ %s �ݪO\n", brdname);

  brd_fpath(folder, brdname, FN_DIR);

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    while (read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      sprintf(fpath, "brd/%s/%c/%s", brdname, hdr.xname[7], hdr.xname);
      reaper(fpath);
    }
    close(fd);
  }
}


static void
expireGem(brdname)
  char *brdname;
{
  int i;
  char c;
  char fpath[64], *str;
  struct dirent *de;
  DIR *dirp;

  printf("�ഫ %s ��ذ�\n", brdname);

  for (i = 0; i < 32; i++)
  {
    c = radix32[i];
    sprintf(fpath, "gem/brd/%s/%c", brdname, c);

    if (!(dirp = opendir(fpath)))
      continue;

    while (de = readdir(dirp))
    {
      str = de->d_name;
      if (*str <= ' ' || *str == '.' || *str == 'F')
        continue;

      sprintf(fpath, "gem/brd/%s/%c/%s", brdname, c, str);
      reaper(fpath);
    }

    closedir(dirp);
  }
}


static void
expireUsr(userid)
  char *userid;
{
  int fd;
  char folder[64], fpath[64];
  HDR hdr;

  printf("�ഫ %s �H��\n", userid);

  usr_fpath(folder, userid, FN_DIR);

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    while (read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      sprintf(fpath, "usr/%c/%s/@/%s", *userid, userid, hdr.xname);
      reaper(fpath);
    }
    close(fd);
  }
}


int
main()
{
  int fd;
  char c, *str, buf[64];
  BRD brd;
  struct dirent *de;
  DIR *dirp;

  chdir(BBSHOME);

  /* �ഫ�ݪO/��ذϤ峹 */

  if ((fd = open(FN_BRD, O_RDONLY)) >= 0)
  {
    while (read(fd, &brd, sizeof(BRD)) == sizeof(BRD))
    {
      str = brd.brdname;
      expireBrd(str);
      expireGem(str);
    }
    close(fd);
  }

  /* �ഫ�ϥΪ̫H�� */

  for (c = 'a'; c <= 'z'; c++)
  {
    sprintf(buf, "usr/%c", c);
    if (!(dirp = opendir(buf)))
      continue;

    while (de = readdir(dirp))
    {
      str = de->d_name;
      if (*str <= ' ' || *str == '.')
	continue;

      expireUsr(str);
    }

    closedir(dirp);
  }

  return 0;
}
