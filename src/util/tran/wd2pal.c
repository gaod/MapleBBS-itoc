/*-------------------------------------------------------*/
/* util/transpal.c                                       */
/*-------------------------------------------------------*/
/* target : WD �� Maple 3.02 (�ݪO)�n�ͦW���ഫ          */
/* create : 01/09/08                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.ee.nctu.edu.tw                  */
/*-------------------------------------------------------*/
/* syntax : transpal                                     */
/*-------------------------------------------------------*/


#if 0

   1. �ק� struct FRIEND �M transpal() transbrdpal()
   2. �ഫ(�ݪO)�n�ͦW�椧�e�A�z�������ഫ���ݪO�ΨϥΪ̡C

   ps. �ϥΫe�Х���ƥ��Ause on ur own risk. �{����H�Х]�[ :p
   ps. �P�� lkchu �� Maple 3.02 for FreeBSD

#endif


#include "wd.h"


/* ----------------------------------------------------- */
/* 3.02 functions                                        */
/* ----------------------------------------------------- */


static int
acct_uno(userid)
  char *userid;
{  
  int fd;
  int userno;
  char fpath[64];

  usr_fpath(fpath, userid, FN_ACCT);
  fd = open(fpath, O_RDONLY);
  if (fd >= 0)
  {
    read(fd, &userno, sizeof(userno));
    close(fd);
    return userno;
  }
  return -1;
}  


/* ----------------------------------------------------- */
/* �ഫ�D�{��                                            */
/* ----------------------------------------------------- */

        
static void
transpal(userid)
  char *userid;
{
  ACCT acct;
  int fd, friend_userno;
  char fpath[64], buf[64];
  PAL pal;
  FRIEND friend;

  usr_fpath(fpath, userid, FN_PAL);			/* �s���n�ͦW�� */

  /* sob �� usr �ؿ������j�p�g�A�ҥH�n�����o�j�p�g */
  usr_fpath(buf, userid, FN_ACCT);
  if ((fd = open(buf, O_RDONLY)) >= 0)
  {
    read(fd, &acct, sizeof(ACCT));
    close(fd);
  }
  else
  {
    return;
  }

  sprintf(buf, OLD_BBSHOME "/home/%s/pal", acct.userid);	/* �ª��n�ͦW�� */

  if (dashf(fpath))
    unlink(fpath);		/* �M������ */

  if ((fd = open(buf, O_RDONLY)) < 0)
    return;

  while (read(fd, &friend, sizeof(FRIEND)) == sizeof(FRIEND))
  {
    if ((friend_userno = acct_uno(friend.userid)) >= 0 &&
      strcmp(friend.userid, acct.userid))
    {
      memset(&pal, 0, sizeof(PAL));
      str_ncpy(pal.userid, friend.userid, sizeof(pal.userid));
      pal.ftype = (friend.savemode & 0x02) ? 0 : PAL_BAD;	/* �n�� vs �l�� */
      str_ncpy(pal.ship, friend.desc, sizeof(pal.ship));      
      pal.userno = friend_userno;
      rec_add(fpath, &pal, sizeof(PAL));
    }
  }
  close(fd);
}


static void
transbrdpal(brdname)
  char *brdname;
{
  int pos, fd, friend_userno;
  char fpath[64], buf[64];
  PAL pal;
  FRIEND friend;

  brd_fpath(fpath, brdname, FN_PAL);			/* �s���n�ͦW�� */
  sprintf(buf, OLD_BBSHOME "/boards/%s/.LIST", brdname);/* �ª��n�ͦW�� */

  if (dashf(fpath))
    unlink(fpath);		/* �M������ */

  pos = 0;
  if ((fd = open(buf, O_RDONLY)) < 0)
    return;

  while (fd)
  {
    lseek(fd, (off_t) (sizeof(FRIEND) * pos), SEEK_SET);
    if (read(fd, &friend, sizeof(FRIEND)) == sizeof(FRIEND))
    {
      if ((friend_userno = acct_uno(friend.userid)) >= 0)
      {
	memset(&pal, 0, sizeof(PAL));
	str_ncpy(pal.userid, friend.userid, sizeof(pal.userid));
	pal.ftype = 0;		/* �ݪO�n�ͤ@�w�O�n�� */
	str_ncpy(pal.ship, friend.desc, sizeof(pal.ship));
	pal.userno = friend_userno;
	rec_add(fpath, &pal, sizeof(PAL));
      }
      pos++;
    }
    else
    {
      close(fd);
      break;
    }
  }
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  char c, *str;
  char buf[64];
  struct dirent *de;
  DIR *dirp;

  /* argc == 1 ������ϥΪ̤άݪO */
  /* argc == 2 ��Y�S�w�ϥΪ� */

  if (argc > 2)
  {
    printf("Usage: %s [target_user]\n", argv[0]);
    exit(-1);
  }

  chdir(BBSHOME);

  if (argc == 2)
  {
    transpal(argv[1]);
    exit(1);
  }

  /* �ഫ�ϥΪ̦n�ͦW�� */
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

      transpal(str);
    }

    closedir(dirp);    
  }

  /* �ഫ�ݪO�n�ͦW�� */
  if (!(dirp = opendir("brd")))
    return 0;

  while (de = readdir(dirp))
  {
    str = de->d_name;
    if (*str <= ' ' || *str == '.')
      continue;

    transbrdpal(str);
  }

  closedir(dirp);

  return 0;
}
