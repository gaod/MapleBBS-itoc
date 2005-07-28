/*-------------------------------------------------------*/
/* util/transusr.c					 */
/*-------------------------------------------------------*/
/* target : WD �� Maple 3.02 �ϥΪ��ഫ			 */
/*          .PASSWDS => .USR .ACCT			 */
/* create : 98/06/14					 */
/* update : 02/01/05					 */
/* author : ernie@micro8.ee.nthu.edu.tw			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

   1. �ק� struct userec �� creat_dirs()
      (userec �⪩�w�q���r����פ��@�A�Цۦ洫���Ʀr)
   2. �� plans �ɦW�A�n�ͦW��B�Ȧs�ɵ������ഫ
   3. Sob ���E��ñ�W�ɡA�u��e�T��
   4. �H�c���� internet mail �p���ݭn�Х� chmod 644 `find PATH -perm 600`

   ps. �ϥΫe�Х���ƥ��Ause on ur own risk. �{����H�Х]�[ :p
   ps. �P�� lkchu �� Maple 3.02 for FreeBSD

#endif


#include "wd.h"


/* ----------------------------------------------------- */
/* �ഫ .ACCT						 */
/* ----------------------------------------------------- */


static inline int
is_bad_userid(userid)
  char *userid;
{
  register char ch;

  if (strlen(userid) < 2)
    return 1;

  if (!isalpha(*userid))
    return 1;

  if (!str_cmp(userid, "new"))
    return 1;

  while (ch = *(++userid))
  {
    if (!isalnum(ch))
      return 1;
  }
  return 0;
}


static inline int
uniq_userno(fd)
  int fd;
{
  char buf[4096];
  int userno, size;
  SCHEMA *sp;			/* record length 16 �i�㰣 4096 */

  userno = 1;

  while ((size = read(fd, buf, sizeof(buf))) > 0)
  {
    sp = (SCHEMA *) buf;
    do
    {
      if (sp->userid[0] == '\0')
      {
	lseek(fd, -size, SEEK_CUR);
	return userno;
      }
      userno++;
      size -= sizeof(SCHEMA);
      sp++;
    } while (size);
  }

  return userno;
}


#define LEVEL_BASIC	000000000001	/* ���v�O */
#define LEVEL_CHAT	000000000002	/* �i�J��ѫ� */
#define LEVEL_PAGE	000000000004	/* ��H��� */
#define LEVEL_POST	000000000010	/* �o��峹 */
#define LEVEL_LOGINOK	000000000020	/* ���U�{�ǻ{�� */
#define LEVEL_MAILLIMIT	000000000040	/* �H��L�W�� */
#define LEVEL_CLOAK	000000000100	/* �����N */
#define LEVEL_SEECLOAK	000000000200	/* �ݨ��Ԫ� */
#define LEVEL_XEMPT	000000000400	/* �ä[�O�d�b�� */
#define LEVEL_BM	000000002000	/* �O�D */
#define LEVEL_ACCOUNTS	000000004000	/* �b���`�� */
#define LEVEL_CHATROOM	000000010000	/* ��ѫ��`�� */
#define LEVEL_BOARD	000000020000	/* �ݪO�`�� */
#define LEVEL_SYSOP	000000040000	/* ���� */


static inline usint
trans_acct_level(perm)
  usint perm;
{
  usint userlevel;

  userlevel = 0;

  if (perm & LEVEL_BASIC)
    userlevel |= PERM_BASIC;

  if (perm & LEVEL_CHAT)
    userlevel |= PERM_CHAT;

  if (perm & LEVEL_PAGE)
    userlevel |= PERM_PAGE;

  if (perm & LEVEL_POST)
    userlevel |= PERM_POST;

  if (perm & LEVEL_LOGINOK)
    userlevel |= PERM_VALID;

  if (perm & LEVEL_MAILLIMIT)
    userlevel |= PERM_MBOX;

  if (perm & LEVEL_CLOAK)
    userlevel |= PERM_CLOAK;

  if (perm & LEVEL_SEECLOAK)
    userlevel |= PERM_SEECLOAK;

  if (perm & LEVEL_XEMPT)
    userlevel |= PERM_XEMPT;

  if (perm & LEVEL_BM)
    userlevel |= PERM_BM;

  if (perm & LEVEL_ACCOUNTS)
    userlevel |= PERM_ACCOUNTS;

  if (perm & LEVEL_CHATROOM)
    userlevel |= PERM_CHATROOM;

  if (perm & LEVEL_BOARD)
    userlevel |= PERM_BOARD;

  if (perm & LEVEL_SYSOP)
    userlevel |= PERM_SYSOP;

  return userlevel;
}


#define OLDUFO_MOVIE		000000000001	/* �}/���ʺA�ݪO */
#define OLDUFO_COLOR		000000000002	/* �m��/�¥դ��� */
#define OLDUFO_NOTE		000000000004	/* ��ܯd���O */
#define OLDUFO_ALARM		000000000010	/* �b�I���� */
#define OLDUFO_BELL		000000000020	/* �n�� */
#define OLDUFO_BOARDLIST	000000000040	/* �ݪO�C����ܤ峹�ƩάO�s�� */
#define OLDUFO_SEELOG		000000000100	/* �W�����h�ݬ����ƦW? */
#define OLDUFO_CYCLE		000000000200	/* �`�����\Ū */
#define OLDUFO_RPG		000000000400  
#define OLDUFO_FEELING		000000001000  
#define OLDUFO_FROM		000000002000  
#define OLDUFO_NOTEMONEY	000000004000  
#define OLDUFO_ALREADYSET 	000000010000	/* �C���W�����]�w? */
#define OLDUFO_BIG5GB		000000020000	/* use big5 code or gb */


static inline usint
trans_acct_ufo(oldufo)
  usint oldufo;
{
  usint ufo;

  ufo = UFO_DEFAULT_NEW;

  if (oldufo & OLDUFO_MOVIE)
    ufo |= UFO_MOVIE;
  else
    ufo &= ~UFO_MOVIE;

  if (oldufo & OLDUFO_NOTE)
    ufo &= ~UFO_MOTD;
  else
    ufo |= UFO_MOTD;

  return ufo;
}


static inline void
creat_dirs(old)
  userec *old;
{
  ACCT new;
  SCHEMA slot;
  int fd;
  char fpath[64];

  memset(&new, 0, sizeof(new));
  memset(&slot, 0, sizeof(slot));

  str_ncpy(new.userid, old->userid, sizeof(new.userid));
  str_ncpy(new.passwd, old->passwd, sizeof(new.passwd));
  str_ncpy(new.realname, old->realname, sizeof(new.realname));
  str_ncpy(new.username, old->username, sizeof(new.username));
  new.userlevel = trans_acct_level(old->userlevel);
  new.ufo = trans_acct_ufo(old->habit);
  new.signature = 0;
  new.year = old->year - 11;	/* �褸�������� */
  new.month = old->month;
  new.day = old->day;
  new.sex = 1 - (old->sex % 2);	/* (0)���� (1)�j�� (2)���} (3)���� (4)���� (5)���� (6)�Ӫ� (7)�q�� */
  new.money = old->silvermoney;
  new.gold = old->goldmoney;
  new.numlogins = old->numlogins;
  new.numposts = old->numposts;
  new.numemails = 0;
  new.firstlogin = old->firstlogin;
  new.lastlogin = old->lastlogin;
  new.tcheck = time(&new.tvalid);
  str_ncpy(new.lasthost, old->lasthost, sizeof(new.lasthost));
  str_ncpy(new.email, old->email, sizeof(new.email));

  slot.uptime = time(0);
  strcpy(slot.userid, new.userid);

  fd = open(FN_SCHEMA, O_RDWR | O_CREAT, 0600);
  new.userno = uniq_userno(fd);
  write(fd, &slot, sizeof(slot));
  close(fd);

  usr_fpath(fpath, new.userid, NULL);
  mkdir(fpath, 0700);
  strcat(fpath, "/@");
  mkdir(fpath, 0700);
  usr_fpath(fpath, new.userid, "MF");
  mkdir(fpath, 0700);
  usr_fpath(fpath, new.userid, "gem");		/* itoc.010727: �ӤH��ذ� */
  mak_links(fpath);

  usr_fpath(fpath, new.userid, ".ACCT");
  fd = open(fpath, O_WRONLY | O_CREAT, 0600);
  write(fd, &new, sizeof(ACCT));
  close(fd);
}


/* ----------------------------------------------------- */
/* �ഫ�{�Ҹ��						 */
/* ----------------------------------------------------- */


static inline void
trans_justify(old)
  userec *old;
{
  char fpath[64];
  FILE *fp;

  usr_fpath(fpath, old->userid, FN_JUSTIFY);
  if (fp = fopen(fpath, "a"))
  {
    fprintf(fp, "RPY: %s\n", old->justify);	/* �ഫ�w�]�H email �{�� */
    fclose(fp);
  }
}


/* ----------------------------------------------------- */
/* �ഫñ�W�ɡB�p�e��					 */
/* ----------------------------------------------------- */


static inline void
trans_sig(old)
  userec *old;
{
  int i;
  char buf[64], fpath[64], f_sig[20];

  for (i = 1; i <= 3; i++)	/* Maple 3.0 �u���T��ñ�W */
  {
    sprintf(buf, OLD_BBSHOME "/home/%s/sig.%d", old->userid, i);	/* �ª�ñ�W�� */
    if (dashf(buf))
    {
      sprintf(f_sig, "%s.%d", FN_SIGN, i);
      usr_fpath(fpath, old->userid, f_sig);
      f_cp(buf, fpath, O_TRUNC);
    }
  }
}


static inline void
trans_plans(old)
  userec *old;
{
  char buf[64], fpath[64];

  sprintf(buf, OLD_BBSHOME "/home/%s/plans", old->userid);
  if (dashf(buf))
  {
    usr_fpath(fpath, old->userid, FN_PLANS);
    f_cp(buf, fpath, O_TRUNC);
  }
}


/* ----------------------------------------------------- */
/* �ഫ�H��						 */
/* ----------------------------------------------------- */


static time_t
trans_hdr_chrono(filename)
  char *filename;
{
  char time_str[11];

  /* M.1087654321.A �� M.987654321.A */
  str_ncpy(time_str, filename + 2, filename[2] == '1' ? 11 : 10);

  return (time_t) atoi(time_str);
}


static inline void
trans_mail(old)
  userec *old;
{
  int fd;
  char index[64], folder[64], buf[64], fpath[64];
  fileheader fh;
  HDR hdr;
  time_t chrono;

  sprintf(index, OLD_BBSHOME "/home/%s/.DIR", old->userid);
  usr_fpath(folder, old->userid, FN_DIR);

  if ((fd = open(index, O_RDONLY)) >= 0)
  {
    while (read(fd, &fh, sizeof(fh)) == sizeof(fh))
    {
      sprintf(buf, OLD_BBSHOME "/home/%s/%s", old->userid, fh.filename);

      if (dashf(buf))	/* �峹�ɮצb�~���ഫ */
      {
	char new_name[10] = "@";      

	/* �ഫ�峹 .DIR */
	memset(&hdr, 0, sizeof(HDR));
	chrono = trans_hdr_chrono(fh.filename);
	new_name[1] = radix32[chrono & 31];
	archiv32(chrono, new_name + 1);

	hdr.chrono = chrono;
	str_ncpy(hdr.xname, new_name, sizeof(hdr.xname));
	str_ncpy(hdr.owner, strstr(fh.owner, "[��.") ? "[�Ƨѿ�]" : fh.owner, sizeof(hdr.owner));	/* [��.��.��] */
	str_ncpy(hdr.title, fh.title, sizeof(hdr.title));
	str_stamp(hdr.date, &hdr.chrono);
	hdr.xmode = (fh.filemode & 0x2) ? (MAIL_MARKED | MAIL_READ) : MAIL_READ;	/* �]���wŪ */

	rec_add(folder, &hdr, sizeof(HDR));

	/* �����ɮ� */
	usr_fpath(fpath, old->userid, "@/");
	strcat(fpath, new_name);
	f_cp(buf, fpath, O_TRUNC);
      }
    }
    close(fd);
  }
}


/* ----------------------------------------------------- */
/* �ഫ�ӤH��ذ�					 */
/* ----------------------------------------------------- */


#ifdef HAVE_PERSONAL_GEM
static void
trans_man_stamp(folder, token, hdr, fpath, time)
  char *folder;
  int token;
  HDR *hdr;
  char *fpath;
  time_t time;
{
  char *fname, *family;
  int rc;

  fname = fpath;
  while (rc = *folder++)
  {
    *fname++ = rc;
    if (rc == '/')
      family = fname;
  }
  if (*family != '.')
  {
    fname = family;
    family -= 2;
  }
  else
  {
    fname = family + 1;
    *fname++ = '/';
  }

  *fname++ = token;

  *family = radix32[time & 31];
  archiv32(time, fname);

  if (rc = open(fpath, O_WRONLY | O_CREAT | O_EXCL, 0600))
  {
    memset(hdr, 0, sizeof(HDR));
    hdr->chrono = time;
    str_stamp(hdr->date, &hdr->chrono);
    strcpy(hdr->xname, --fname);
    close(rc);
  }
  return;
}


static void
transman(index, folder)
  char *index, *folder;
{
  static int count = 100;

  int fd;
  char *ptr, buf[256], fpath[64];
  fileheader fh;
  HDR hdr;
  time_t chrono;

  if ((fd = open(index, O_RDONLY)) >= 0)
  {
    while (read(fd, &fh, sizeof(fh)) == sizeof(fh))
    {
      strcpy(buf, index);
      ptr = strrchr(buf, '/') + 1;
      strcpy(ptr, fh.filename);

      if (*fh.filename == 'M' && dashf(buf))	/* �u�� M.xxxx.A �� D.xxxx.a */
      {
	/* �ഫ�峹 .DIR */
	memset(&hdr, 0, sizeof(HDR));
	chrono = trans_hdr_chrono(fh.filename);
	trans_man_stamp(folder, 'A', &hdr, fpath, chrono);
	hdr.xmode = 0;
	str_ncpy(hdr.owner, fh.owner, sizeof(hdr.owner));
	str_ncpy(hdr.title, fh.title + 3, sizeof(hdr.title));
	rec_add(folder, &hdr, sizeof(HDR));

	/* �����ɮ� */
	f_cp(buf, fpath, O_TRUNC);
      }
      else if (*fh.filename == 'D' && dashd(buf))
      {
	char sub_index[256];

	/* �ഫ�峹 .DIR */
	memset(&hdr, 0, sizeof(HDR));
	 chrono = ++count;		/* WD ���ؿ��R�W����_�ǡA�u�n�ۤv���Ʀr */
	trans_man_stamp(folder, 'F', &hdr, fpath, chrono);
	hdr.xmode = GEM_FOLDER;
	str_ncpy(hdr.owner, fh.owner, sizeof(hdr.owner));
	str_ncpy(hdr.title, fh.title + 3, sizeof(hdr.title));
	rec_add(folder, &hdr, sizeof(HDR));

	/* recursive �i�h�ഫ�l�ؿ� */
	strcpy(sub_index, buf);
	ptr = strrchr(sub_index, '/') + 1;
	sprintf(ptr, "%s/.DIR", fh.filename);
	transman(sub_index, fpath);
      }
    }
   close(fd);
  }
}
#endif


/* ----------------------------------------------------- */
/* �ഫ�D�{��						 */
/* ----------------------------------------------------- */


static void
transusr(user)
  userec *user;
{
  char buf[64];

  printf("�ഫ %s �ϥΪ�\n", user->userid);

  if (is_bad_userid(user->userid))
  {
    printf("%s ���O�X�k ID\n", user->userid);
    return;
  }

  usr_fpath(buf, user->userid, NULL);
  if (dashd(buf))
  {
    printf("%s �w�g���� ID\n", user->userid);
    return;
  }

  sprintf(buf, OLD_BBSHOME "/home/%s", user->userid);
  if (!dashd(buf))
  {
    printf("%s ���ɮפ��s�b\n", user->userid);
    return;
  }

  creat_dirs(user);
  trans_justify(user);
  trans_sig(user);
  trans_plans(user);
  trans_mail(user);


#ifdef HAVE_PERSONAL_GEM
  sprintf(buf, OLD_BBSHOME "/home/%s/man", user->userid);
  if (dashd(buf))
  {
    char index[64], folder[64];

    sprintf(index, "%s/.DIR", buf);
    usr_fpath(folder, user->userid, "gem/" FN_DIR);
    transman(index, folder);
  }
#endif
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int fd;
  userec user;

  /* argc == 1 ������ϥΪ� */
  /* argc == 2 ��Y�S�w�ϥΪ� */

  if (argc > 2)
  {
    printf("Usage: %s [target_user]\n", argv[0]);
    exit(-1);
  }

  chdir(BBSHOME);

  if (!dashf(FN_PASSWD))
  {
    printf("ERROR! Can't open " FN_PASSWD "\n");
    exit(-1);
  }
  if (!dashd(OLD_BBSHOME "/home"))
  {
    printf("ERROR! Can't open " OLD_BBSHOME "/home\n");
    exit(-1);
  }

  if ((fd = open(FN_PASSWD, O_RDONLY)) >= 0)
  {
    while (read(fd, &user, sizeof(user)) == sizeof(user))
    {
      if (argc == 1)
      {
	transusr(&user);
      }
      else if (!strcmp(user.userid, argv[1]))
      {
	transusr(&user);
	exit(1);
      }
    }
    close(fd);
  }

  exit(0);
}
