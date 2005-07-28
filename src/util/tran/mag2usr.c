/*-------------------------------------------------------*/
/* util/transusr.c					 */
/*-------------------------------------------------------*/
/* target : Magic �� Maple 3.02 �ϥΪ��ഫ		 */
/* create : 02/09/09					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

  0. �A�� Magic �� .ACCT �ΫH�c

  ps. User on ur own risk.

#endif


#include "mag.h"


/*-------------------------------------------------------*/
/* �ഫ.ACCT						 */
/*-------------------------------------------------------*/


static void
trans_acct()
{
  int fd, count;
  char *userid, buf[64];
  userec old;
  ACCT new;
  SCHEMA slot;

  count = 1;	/* userno �q 1 �_�� */

  if ((fd = open(FN_PASSWDS, O_RDONLY)) >= 0)
  {
    while (read(fd, &old, sizeof(old)) == sizeof(old))
    {
      if (!isalpha(old.userid[0]))
	continue;

      new.userno = count;
      count++;

      str_ncpy(new.userid, old.userid, sizeof(new.userid));
      str_ncpy(new.passwd, old.passwd, sizeof(new.passwd));
      str_ncpy(new.realname, old.realname, sizeof(new.realname));
      str_ncpy(new.username, old.username, sizeof(new.username));

      new.userlevel = PERM_BASIC | PERM_CHAT | PERM_PAGE | PERM_POST | PERM_VALID;
      new.ufo = UFO_DEFAULT_NEW;

      new.signature = 0;
      new.year = old.byear;
      new.month = old.bmonth;
      new.day = old.bday;
      new.sex = 0;

      new.money = old.money;
      new.gold = 0;

      new.numlogins = old.numlogins;
      new.numposts = old.numposts;
      new.numemails = 0;

      new.firstlogin = old.firstlogin;
      new.lastlogin = old.lastlogin;
      new.tcheck = time(&new.tvalid);

      str_ncpy(new.lasthost, old.lasthost, sizeof(new.lasthost));
      str_ncpy(new.email, old.email, sizeof(new.email));

      userid = new.userid;
      usr_fpath(buf, userid, NULL);
      mkdir(buf, 0700);
      strcat(buf, "/@");
      mkdir(buf, 0700);
      usr_fpath(buf, userid, "gem");	/* itoc.010727: �ӤH��ذ� */
      mak_links(buf);			/* itoc.010924: ��֭ӤH��ذϥؿ� */
#ifdef MY_FAVORITE
      usr_fpath(buf, userid, "MF");
      mkdir(buf, 0700);
#endif

      usr_fpath(buf, userid, FN_ACCT);
      rec_add(buf, &new, sizeof(ACCT));

      memset(&slot, 0, sizeof(SCHEMA));
      slot.uptime = count;
      memcpy(slot.userid, new.userid, IDLEN);
      rec_add(FN_SCHEMA, &slot, sizeof(SCHEMA));
    }
    close(fd);
  }
}


/*-------------------------------------------------------*/
/* �ഫ�H�c						 */
/*-------------------------------------------------------*/


static time_t
trans_hdr_chrono(filename)
  char *filename;
{
  char time_str[11];

  /* M.1087654321.A �� M.987654321.A */
  str_ncpy(time_str, filename + 2, filename[2] == '1' ? 11 : 10);

  return (time_t) atoi(time_str);
}


static void
trans_owner(hdr, old)
  HDR *hdr;
  char *old;
{
  char *left, *right;
  char owner[128];

  str_ncpy(owner, old, sizeof(owner));

  if (strchr(owner, '.'))	/* innbbsd ==> bbs */
  {
    /* �榡: itoc.bbs@bbs.tnfsh.tn.edu.tw (�ڪ��ʺ�) */

    left = strchr(owner, '(');
    right = strrchr(owner, ')');

    if (!left || !right)
    {
      str_ncpy(hdr->owner, "�H�W", sizeof(hdr->owner));
    }
    else
    {
      *(left - 1) = '\0';
      str_ncpy(hdr->owner, owner, sizeof(hdr->owner));
      *right = '\0';
      str_ncpy(hdr->nick, left + 1, sizeof(hdr->nick));
    }
  }
  else if (left = strchr(owner, '('))	/* local post */
  {
    /* �榡: itoc (�ڪ��ʺ�) */

    *(left - 1) = '\0';
    str_ncpy(hdr->owner, owner, sizeof(hdr->owner));
    if (right = strchr(owner, ')'))
    {
      *right = '\0';
      str_ncpy(hdr->nick, left + 1, sizeof(hdr->nick));
    }
  }
  else
  {
    /* �榡: itoc */

    str_ncpy(hdr->owner, owner, sizeof(hdr->owner));
  }
}


static void
trans_mail(userid)
  char *userid;
{
  int fd;
  char ch, index[64], folder[64], buf[64], fpath[64];
  fileheader fh;
  HDR hdr;
  time_t chrono;

  ch = userid[0];
  if (ch >= 'a' && ch <= 'z')	/* ���j�g */
    ch -= 32;

  sprintf(index, "%s/%c/%s/.DIR", OLD_MAILPATH, ch, userid);	/* �ª� header */
  usr_fpath(folder, userid, FN_DIR);				/* �s�� header */

  if ((fd = open(index, O_RDONLY)) >= 0)
  {
    while (read(fd, &fh, sizeof(fh)) == sizeof(fh))
    {
      sprintf(buf, "%s/%c/%s/%s", OLD_MAILPATH, ch, userid, fh.filename);

      if (dashf(buf))		/* �峹�ɮצb�~���ഫ */
      {
	char new_name[10] = "@";

	/* �ഫ�峹 .DIR */
	memset(&hdr, 0, sizeof(HDR));
	chrono = trans_hdr_chrono(fh.filename);
	new_name[1] = radix32[chrono & 31];
	archiv32(chrono, new_name + 1);

	hdr.chrono = chrono;
	str_ncpy(hdr.xname, new_name, sizeof(hdr.xname));
	trans_owner(&hdr, fh.owner);
	str_ncpy(hdr.title, fh.title, sizeof(hdr.title));
	str_stamp(hdr.date, &hdr.chrono);
	hdr.xmode = MAIL_READ;
	if (fh.accessed[0] & 0x8)	/* FILE_MARDKED */
	  hdr.xmode |= POST_MARKED;

	rec_add(folder, &hdr, sizeof(HDR));

	/* �����ɮ� */
	usr_fpath(fpath, userid, "@/");
	strcat(fpath, new_name);
	f_cp(buf, fpath, O_TRUNC);
      }
    }
    close(fd);
  }
}


int
main()
{
  char c, *userid;
  char buf[64];
  struct dirent *de;
  DIR *dirp;

  chdir(BBSHOME);

  /* ���� .ACCT�A�إؿ� */
  trans_acct();

  /* �Ѧ��Ī��ϥΪ� ID ���ഫ�H�c */
  for (c = 'a'; c <= 'z'; c++)
  {
    sprintf(buf, "usr/%c", c);

    if (!(dirp = opendir(buf)))
      continue;

    while (de = readdir(dirp))
    {
      userid = de->d_name;
      if (*userid <= ' ' || *userid == '.')
	continue;

      trans_mail(userid);
    }

    closedir(dirp);
  }

  return 0;
}
