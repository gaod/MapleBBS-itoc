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


#include "fb.h"


static char uperid[80];         /* �j�g�� ID */

static void                             /* ���j�g */
str_uper(dst, src)
  char *dst, *src;
{
  int ch;
  do
  {
    ch = *src++;
    if (ch >= 'a' && ch <= 'z')
      ch = ch - 32;
    *dst++ = ch;
  } while (ch);
}


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
  int fd, friend_userno, i;
  char fpath[64], buf[64];
  PAL pal;
  FRIEND friend;

  /* FireBird �O�Τj�g id */
  str_uper(uperid, userid);

  /* FB �� usr �ؿ������j�p�g�A�ҥH�n�����o�j�p�g */
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

  usr_fpath(fpath, userid, FN_PAL);			/* �s���n�ͦW�� */

  if (dashf(fpath))
    unlink(fpath);		/* �M������ */

  for (i = 0; i <= 1; i++)	/* �n�ͦW��/�a�H�W�� �G�� */
  {
    sprintf(buf, OLD_BBSHOME "/home/%c/%s/%s", *uperid, acct.userid, i ? "friends" : "rejects");	/* �ª��n�ͦW�� */

    if ((fd = open(buf, O_RDONLY)) < 0)
      return;

    while (read(fd, &friend, sizeof(FRIEND)) == sizeof(FRIEND))
    {
      if ((friend_userno = acct_uno(friend.id)) >= 0 &&
	strcmp(friend.id, acct.userid))
      {
	memset(&pal, 0, sizeof(PAL));
	str_ncpy(pal.userid, friend.id, sizeof(pal.userid));
	pal.ftype = i ? 0 : PAL_BAD;     /* �n�� vs �l�� */
	str_ncpy(pal.ship, friend.exp, sizeof(pal.ship));      
	pal.userno = friend_userno;
	rec_add(fpath, &pal, sizeof(PAL));
      }
    }
    close(fd);
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

  return 0;
}
