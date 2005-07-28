/*-------------------------------------------------------*/
/* util/windtop2usr.c					 */
/*-------------------------------------------------------*/
/* target : WindTop .ACCT �ഫ				 */
/* create : 03/06/30					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "windtop.h"

#define	MAK_DIRS	/* �إؿ� MF/ �� gem/ */


#define OLDUFO_PAGER       BFLAG(5)        /* �����I�s�� */
#define OLDUFO_QUIET       BFLAG(6)        /* ���f�b�H�ҡA�ӵL������ */
#define OLDUFO_MAXMSG      BFLAG(7)        /* �T���W���ڦ��T�� */
#define OLDUFO_FORWARD     BFLAG(8)        /* �۰���H */
#define OLDUFO_CLASSTABLE  BFLAG(9)        /* �\�Ҫ�q�� */
#define OLDUFO_BROADCAST   BFLAG(14)       /* �ڦ��s�� */
#define OLDUFO_HIDEDN      BFLAG(18)       /* ���èӷ� */
#define OLDUFO_CLOAK       BFLAG(19)       /* true if cloak was ON */
#define OLDUFO_WEB         BFLAG(22)       /* visor.020325: WEB */
#define OLDUFO_MPAGER      BFLAG(10)       /* lkchu.990428: �q�l�l��ǩI */
#define OLDUFO_MESSAGE     BFLAG(23)       /* visor.991030: �T������ */
#define OLDUFO_PAGER1      BFLAG(26)       /* visor.991030: �I�s������ */

#define OLDUFO2_COLOR      BFLAG(0)        /* true if the ANSI color mode open */
#define OLDUFO2_MOVIE      BFLAG(1)        /* true if show movie */
#define OLDUFO2_BRDNEW     BFLAG(2)        /* �s�峹�Ҧ� */
#define OLDUFO2_BNOTE      BFLAG(3)        /* ��ܶi�O�e�� */
#define OLDUFO2_VEDIT      BFLAG(4)        /* ²�ƽs�边 */
#define OLDUFO2_PAL        BFLAG(5)        /* true if show pals only */
#define OLDUFO2_MOTD       BFLAG(6)        /* ²�ƶi���e�� */
#define OLDUFO2_MIME       BFLAG(7)        /* MIME �ѽX */
#define OLDUFO2_SIGN       BFLAG(8)        /* ñ�W�� */
#define OLDUFO2_SHOWUSER   BFLAG(9)        /* ��� ID �M �ʺ� */
#define OLDUFO2_PRH        BFLAG(10)       /* ��ܱ��ˤ峹���� */
#define OLDUFO2_SHIP       BFLAG(11)       /* visor.991030: �n�ʹy�z */
#define OLDUFO2_NWLOG      BFLAG(12)       /* lkchu.990510: ���s��ܬ��� */
#define OLDUFO2_NTLOG      BFLAG(13)       /* lkchu.990510: ���s��Ѭ��� */
#define OLDUFO2_CIRCLE     BFLAG(14)       /* �`���\Ū */
#define OLDUFO2_ORIGUI     BFLAG(15)       /* ����������W������ */
#define OLDUFO2_DEF_ANONY  BFLAG(16)       /* �w�]���ΦW */
#define OLDUFO2_DEF_LEAVE  BFLAG(17)       /* �w�]������ */
#define OLDUFO2_ACL        BFLAG(24)       /* true if ACL was ON */
#define OLDUFO2_REALNAME   BFLAG(28)       /* visor.991030: �u��m�W */


static usint
trans_ufo(oldufo, oldufo2)
  usint oldufo, oldufo2;
{
  usint ufo;

  ufo = 0;

  if (oldufo2 & OLDUFO2_MOVIE)
    ufo |= UFO_MOVIE;

  if (oldufo2 & OLDUFO2_BRDNEW)
    ufo |= UFO_BRDPOST;

  if (oldufo2 & OLDUFO2_BNOTE)
    ufo |= UFO_BRDNOTE;

  if (oldufo2 & OLDUFO2_VEDIT)
    ufo |= UFO_VEDIT;

  if (oldufo2 & OLDUFO2_MOTD)
    ufo |= UFO_MOTD;

  if (oldufo & OLDUFO_PAGER)
    ufo |= UFO_PAGER;

  if (oldufo & OLDUFO_BROADCAST)
    ufo |= UFO_RCVER;

  if (oldufo & OLDUFO_QUIET)
    ufo |= UFO_QUIET;

  if (oldufo2 & OLDUFO2_PAL)
    ufo |= UFO_PAL;

  ufo |= UFO_ALOHA;		/* �w�] */

  /* ufo |= UFO_BMWDISPLAY; */	/* �w�]���n */

  if (oldufo2 & OLDUFO2_NWLOG)
    ufo |= UFO_NWLOG;

  if (oldufo2 & OLDUFO2_NTLOG)
    ufo |= UFO_NTLOG;

  ufo |= UFO_NOSIGN;		/* �w�] */

  /* ufo |= UFO_SHOWSIGN; */	/* �w�]���n */

  if (oldufo & OLDUFO_CLOAK)
    ufo |= UFO_CLOAK;

  if (oldufo2 & OLDUFO2_ACL)
    ufo |= UFO_ACL;

  return ufo;
}


int
main()
{
  ACCT new;
  char c;

  for (c = 'a'; c <= 'z'; c++)
  {
    char buf[64];
    struct dirent *de;
    DIR *dirp;

    sprintf(buf, BBSHOME "/usr/%c", c);
    chdir(buf);

    if (!(dirp = opendir(".")))
      continue;

    while (de = readdir(dirp))
    {
      userec old;
      int fd;
      char *str;

      str = de->d_name;
      if (*str <= ' ' || *str == '.')
	continue;

#ifdef MAK_DIRS
      sprintf(buf, "%s/MF", str);
      mkdir(buf, 0700);
      sprintf(buf, "%s/gem", str);
      mak_links(buf);
#endif

      sprintf(buf, "%s/" FN_ACCT, str);
      if ((fd = open(buf, O_RDONLY)) < 0)
	continue;

      read(fd, &old, sizeof(userec));
      close(fd);
      unlink(buf);			/* itoc.010831: �屼��Ӫ� FN_ACCT */

      memset(&new, 0, sizeof(ACCT));

      new.userno = old.userno;

      str_ncpy(new.userid, old.userid, sizeof(new.userid));
      str_ncpy(new.passwd, old.passwd, sizeof(new.passwd));
      str_ncpy(new.realname, old.realname, sizeof(new.realname));
      str_ncpy(new.username, old.username, sizeof(new.username));

      new.userlevel = old.userlevel;
      new.ufo = trans_ufo(old.ufo, old.ufo2);
      new.signature = old.signature;

      new.year = 0;
      new.month = 0;
      new.day = 0;
      new.sex = 1;		/* ����l�� */
      new.money = 100;
      new.gold = 1;

      new.numlogins = old.numlogins;
      new.numposts = old.numposts;
      new.numemails = old.numemail;

      new.firstlogin = old.firstlogin;
      new.lastlogin = old.lastlogin;
      new.tcheck = old.tcheck;
      new.tvalid = old.tvalid;

      str_ncpy(new.lasthost, old.lasthost, sizeof(new.lasthost));
      str_ncpy(new.email, old.email, sizeof(new.email));

      fd = open(buf, O_WRONLY | O_CREAT, 0600);	/* itoc.010831: ���طs�� FN_ACCT */
      write(fd, &new, sizeof(ACCT));
      close(fd);
    }

    closedir(dirp);    
  }
}
