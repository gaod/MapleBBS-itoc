/*-------------------------------------------------------*/
/* bbsd.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw		 	 */
/* target : BBS daemon/main/login/top-menu routines 	 */
/* create : 95/03/29				 	 */
/* update : 96/10/10				 	 */
/*-------------------------------------------------------*/


#define	_MAIN_C_


#include "bbs.h"
#include "dns.h"


#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/telnet.h>
#include <sys/resource.h>


#define	QLEN		3
#define	PID_FILE	"run/bbs.pid"
#define	LOG_FILE	"run/bbs.log"
#undef	SERVER_USAGE


static int myports[MAX_BBSDPORT] = BBSD_PORT;

static pid_t currpid;

extern BCACHE *bshm;
extern UCACHE *ushm;

/* static int mport; */ /* Thor.990325: ���ݭn�F:P */
static u_long tn_addr;


#ifdef CHAT_SECURE
char passbuf[PSWDLEN + 1];
#endif


#ifdef MODE_STAT
extern UMODELOG modelog;
extern time_t mode_lastchange;
#endif


/* ----------------------------------------------------- */
/* ���} BBS �{��					 */
/* ----------------------------------------------------- */


void
blog(mode, msg)
  char *mode, *msg;
{
  char buf[512];

  sprintf(buf, "%s %s %-13s%s\n", Now(), mode, cuser.userid, msg);
  f_cat(FN_RUN_USIES, buf);
}


#ifdef MODE_STAT
void
log_modes()
{
  time(&modelog.logtime);
  rec_add(FN_RUN_MODE_CUR, &modelog, sizeof(UMODELOG));
}
#endif


void
u_exit(mode)
  char *mode;
{
  int fd, diff;
  char fpath[80];
  ACCT tuser;

  if (currbno >= 0 && bshm->mantime[currbno] > 0)
    bshm->mantime[currbno]--;	/* �h�X�̫�ݪ����ӪO */

  utmp_free(cutmp);		/* ���� UTMP shm */

  diff = (time(&cuser.lastlogin) - ap_start) / 60;
  sprintf(fpath, "Stay: %d (%d)", diff, currpid);
  blog(mode, fpath);

  if (cuser.userlevel)
  {
    ve_backup();		/* �s�边�۰ʳƥ� */
    brh_save();			/* �x�s�\Ū�O���� */
  }

#ifndef LOG_BMW	/* �����R�����y */
  usr_fpath(fpath, cuser.userid, fn_amw);
  unlink(fpath);
  usr_fpath(fpath, cuser.userid, fn_bmw);
  unlink(fpath);
#endif

#ifdef MODE_STAT
  log_modes();
#endif


  /* �g�^ .ACCT */

  if (!HAS_STATUS(STATUS_DATALOCK))	/* itoc.010811: �S���Q������w�A�~�i�H�^�s .ACCT */
  {
    usr_fpath(fpath, cuser.userid, fn_acct);
    fd = open(fpath, O_RDWR);
    if (fd >= 0)
    {  
      if (read(fd, &tuser, sizeof(ACCT)) == sizeof(ACCT))
      {
	if (diff >= 1)
	{
	  cuser.numlogins++;	/* Thor.980727.����: �b���W���W�L�@���������p�⦸�� */
	  addmoney(diff);	/* itoc.010805: �W���@�����[�@�� */
	}

	if (HAS_STATUS(STATUS_COINLOCK))	/* itoc.010831: �Y�O multi-login ���ĤG���H��A���x�s���� */
	{
	  cuser.money = tuser.money;
	  cuser.gold = tuser.gold;
	}

	/* itoc.010811.����: �p�G�ϥΪ̦b�u�W�S���{�Ҫ��ܡA
	  ���� cuser �� tuser �� userlevel/tvalid �O�P�B���F
	  ���Y�ϥΪ̦b�u�W�^�{�ҫH/��{�ҽX/�Q�����f�ֵ��U��..���{�ҳq�L���ܡA
	  ���� tuser �� userlevel/tvalid �~�O����s�� */
	cuser.userlevel = tuser.userlevel;
	cuser.tvalid = tuser.tvalid;

	lseek(fd, (off_t) 0, SEEK_SET);
	write(fd, &cuser, sizeof(ACCT));
      }
      close(fd);
    }
  }
}


void
abort_bbs()
{
  if (bbstate)
    u_exit("AXXED");
  exit(0);
}


static void
login_abort(msg)
  char *msg;
{
  outs(msg);
  refresh();
  exit(0);
}


/* Thor.980903: lkchu patch: ���ϥΤW���ӽбb����, �h�U�C function������ */

#ifdef LOGINASNEW

/* ----------------------------------------------------- */
/* �ˬd user ���U���p					 */
/* ----------------------------------------------------- */


static int
belong(flist, key)
  char *flist;
  char *key;
{
  int fd, rc;

  rc = 0;
  if ((fd = open(flist, O_RDONLY)) >= 0)
  {
    mgets(-1);

    while (flist = mgets(fd))
    {
      str_lower(flist, flist);
      if (str_str(key, flist))
      {
	rc = 1;
	break;
      }
    }

    close(fd);
  }
  return rc;
}


static int
is_badid(userid)
  char *userid;
{
  int ch;
  char *str;

  if (strlen(userid) < 2)
    return 1;

  if (!is_alpha(*userid))
    return 1;

  if (!str_cmp(userid, STR_NEW))
    return 1;

  str = userid;
  while (ch = *(++str))
  {
    if (!is_alnum(ch))
      return 1;
  }
  return (belong(FN_ETC_BADID, userid));
}


static int
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


static void
acct_apply()
{
  SCHEMA slot;
  char buf[80];
  char *userid;
  int try, fd;

  film_out(FILM_APPLY, 0);

  memset(&cuser, 0, sizeof(ACCT));
  userid = cuser.userid;
  try = 0;
  for (;;)
  {
    if (!vget(18, 0, msg_uid, userid, IDLEN + 1, DOECHO))
      login_abort("\n�A�� ...");

    if (is_badid(userid))
    {
      vmsg("�L�k�����o�ӥN���A�Шϥέ^��r���A�åB���n�]�t�Ů�");
    }
    else
    {
      usr_fpath(buf, userid, NULL);
      if (dashd(buf))
	vmsg("���N���w�g���H�ϥ�");
      else
	break;
    }

    if (++try >= 10)
      login_abort("\n�z���տ��~����J�Ӧh�A�ФU���A�ӧa");
  }

  for (;;)
  {
    vget(19, 0, "�г]�w�K�X�G", buf, PSWDLEN + 1, NOECHO);
    if ((strlen(buf) < 4) || !strcmp(buf, userid))
    {
      vmsg("�K�X��²��A���D�J�I�A�ܤ֭n 4 �Ӧr�A�Э��s��J");
      continue;
    }

    vget(20, 0, "���ˬd�K�X�G", buf + PSWDLEN + 2, PSWDLEN + 1, NOECHO);
    if (!strcmp(buf, buf + PSWDLEN + 2))
      break;

    vmsg("�K�X��J���~, �Э��s��J�K�X");
  }

  str_ncpy(cuser.passwd, genpasswd(buf), sizeof(cuser.passwd));

  do
  {
    vget(20, 0, "��    �١G", cuser.username, UNLEN + 1, DOECHO);
  } while (strlen(cuser.username) < 2);

  /* itoc.010317: ���� user �H��N�����m�W */
  vmsg("�`�N�G�п�J�u��m�W�A���������ѭק�m�W���\\��");

  do
  {
    vget(21, 0, "�u��m�W�G", cuser.realname, RNLEN + 1, DOECHO);
  } while (strlen(cuser.realname) < 4);

  cuser.userlevel = PERM_DEFAULT;
  cuser.ufo = UFO_DEFAULT_NEW;
  cuser.numlogins = 1;
  cuser.tvalid = ap_start;		/* itoc.030724: ���W���ɶ���Ĥ@���{�ҽX�� seed */
  sprintf(cuser.email, "%s.bbs@%s", cuser.userid, str_host);	/* itoc.010902: �w�] email */

  /* Ragnarok.050528: �i��G�H�P�ɥӽЦP�@�� ID�A�b�������A�ˬd�@�� */
  usr_fpath(buf, userid, NULL);
  if (dashd(buf))
  {
    vmsg("���N����Q���U���A�Э��s�ӽ�");
    abort_bbs();
  }

  /* dispatch unique userno */

  cuser.firstlogin = cuser.lastlogin = cuser.tcheck = slot.uptime = ap_start;
  memcpy(slot.userid, userid, IDLEN);

  fd = open(FN_SCHEMA, O_RDWR | O_CREAT, 0600);
  {
    /* flock(fd, LOCK_EX); */
    /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
    f_exlock(fd);

    cuser.userno = try = uniq_userno(fd);
    write(fd, &slot, sizeof(slot));
    /* flock(fd, LOCK_UN); */
    /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
    f_unlock(fd);
  }
  close(fd);

  /* create directory */

  /* usr_fpath(buf, userid, NULL); */	/* �谵�L */
  mkdir(buf, 0700);
  strcat(buf, "/@");
  mkdir(buf, 0700);
  usr_fpath(buf, userid, "gem");	/* itoc.010727: �ӤH��ذ� */
  /* mak_dirs(buf); */
  mak_links(buf);			/* itoc.010924: ��֭ӤH��ذϥؿ� */
#ifdef MY_FAVORITE
  usr_fpath(buf, userid, "MF");
  mkdir(buf, 0700);
#endif

  usr_fpath(buf, userid, fn_acct);
  fd = open(buf, O_WRONLY | O_CREAT, 0600);
  write(fd, &cuser, sizeof(ACCT));
  close(fd);
  /* Thor.990416: �`�N: ���|�� .ACCT���׬O0��, �ӥB�u�� @�ؿ�, �����[� */

  sprintf(buf, "%d", try);
  blog("APPLY", buf);
}

#endif /* LOGINASNEW */


/* ----------------------------------------------------- */
/* bad login						 */
/* ----------------------------------------------------- */


#define	FN_BADLOGIN	"logins.bad"


static void
logattempt(type, content)
  int type;			/* '-' login failure   ' ' success */
  char *content;
{
  char buf[128], fpath[64];

  sprintf(buf, "%s %c %s\n", Btime(&ap_start), type, content);
    
  usr_fpath(fpath, cuser.userid, FN_LOG);
  f_cat(fpath, buf);

  if (type != ' ')
  {
    usr_fpath(fpath, cuser.userid, FN_BADLOGIN);
    sprintf(buf, "[%s] %s\n", Btime(&ap_start), fromhost);
    f_cat(fpath, buf);
  }
}


/* ----------------------------------------------------- */
/* �n�� BBS �{��					 */
/* ----------------------------------------------------- */


extern void talk_rqst();
extern void bmw_rqst();


static int		/* 1:�blist�� 0:���blist�� */
belong_list(filelist, key, desc)
  char *filelist, *key, *desc;
{
  FILE *fp;
  char buf[80], *str;
  int rc;

  rc = 0;
  if (fp = fopen(filelist, "r"))
  {
    while (fgets(buf, 80, fp))
    {
      if (buf[0] == '#')
	continue;

      if (str = (char *) strchr(buf, ' '))
      {
	*str = '\0';
	if (strstr(key, buf))
	{
	  /* ���L�ťդ��j */
	  for (str++; *str && isspace(*str); str++)
	    ;

	  strcpy(desc, str);
	  if (str = (char *) strchr(desc, '\n'))	/* �̫᪺ '\n' ���n */
	    *str = '\0';
	  rc = 1;
	  break;
	}
      }
    }
    fclose(fp);
  }
  return rc;
}


static void
utmp_setup(mode)
  int mode;
{
  UTMP utmp;
  uschar *addr;
  
  memset(&utmp, 0, sizeof(utmp));

  utmp.pid = currpid;
  utmp.userno = cuser.userno;
  utmp.mode = bbsmode = mode;
  /* utmp.in_addr = tn_addr; */ /* itoc.010112: ����umtp.in_addr�H��ulist_cmp_host���` */
  addr = (uschar *) &tn_addr;
  utmp.in_addr = (addr[0] << 24) + (addr[1] << 16) + (addr[2] << 8) + addr[3];
  utmp.userlevel = cuser.userlevel;	/* itoc.010309: �� userlevel �]��J cache */
  utmp.ufo = cuser.ufo;
  utmp.status = 0;
  
  strcpy(utmp.userid, cuser.userid);
#ifdef DETAIL_IDLETIME
  utmp.idle_time = ap_start;
#endif

#ifdef GUEST_NICK
  if (!cuser.userlevel)		/* guest */
  {
    char nick[9][5] = {"�C�l", "���w", "�X��", "�ɩ�", "���Y", "�v�l", "�f�r", "���~", "�۹�"};
    sprintf(cuser.username, "�Ӷ��U��%s", nick[ap_start % 9]);
  }
#endif	/* GUEST_NICK */

  strcpy(utmp.username, cuser.username);
  
#ifdef HAVE_WHERE

#  ifdef GUEST_WHERE
  if (!cuser.userlevel)		/* guest */
  {
    /* itoc.010910: GUEST_NICK �M GUEST_WHERE ���üƼҼ��קK�@�� */
    char from[16][9] = {"���F�E��", "�C�H�¶�", "�v�N�q��", "�n�x����", "�d�����f", "�ѯ�p��", "�t�z���", "��x���y",
			"�ɾ�V�a", "�񱻭���", "�Q�L�߻A", "�˴�᭷", "�˶�M�F", "���D����", "������", "���K��"};
    strcpy(utmp.from, from[ap_start % 16]);
  }
  else
#  endif	/* GUEST_WHERE */
  {

  /* �� hinet �o�� ip �ܦh�A DN �ܤ֪��A�N�g�J etc/fqdn    *
   * �� 140.112. �o�شN�g�b etc/host                       *
   * �Y�� DNS �걼�A�b etc/host �̭����٬O�i�H�Ӽ˧P�_���\ *
   * �p�G�� 140.112. �g�J etc/host ���A�N���Χ� ntu.edu.tw *
   * ���мg�J etc/fqdn �̤F                                */

    char name[40];

    /* ����� FQDN */
    str_lower(name, fromhost);	/* itoc.011011: �j�p�g���i�Aetc/fqdn �̭����n�g�p�g */
    if (!belong_list(FN_ETC_FQDN, name, utmp.from))
    {
      /* �A��� ip */
      sprintf(name, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
      if (!belong_list(FN_ETC_HOST, name, utmp.from))
	str_ncpy(utmp.from, fromhost, sizeof(utmp.from)); /* �p�G���S�������G�m�A�N�O�� fromhost */
    }
  }

#else
  str_ncpy(utmp.from, fromhost, sizeof(utmp.from));
#endif	/* HAVE_WHERE */      
  
  /* Thor: �i�DUser�w�g���F�񤣤U... */
  if (!utmp_new(&utmp))
    login_abort("\n�z���諸��l�w�g�Q�H�������n�F�A�ФU���A�ӧa");

  /* itoc.001223: utmp_new ���A pal_cache�A�p�G login_abort �N�����F */
  pal_cache();
}


/* ----------------------------------------------------- */
/* user login						 */
/* ----------------------------------------------------- */


static int		/* �^�� multi */
login_user(content)
  char *content;
{
  int attempts;		/* ���մX�����~ */
  int multi;
  char fpath[64], uid[IDLEN + 1];
#ifndef CHAT_SECURE
  char passbuf[PSWDLEN + 1];
#endif

  move(b_lines, 0);
  outs("   �� ���[�b���G\033[1;32m" STR_GUEST "\033[m  �ӽзs�b���G\033[1;31m" STR_NEW "\033[m");

  attempts = 0;
  multi = 0;
  for (;;)
  {
    if (++attempts > LOGINATTEMPTS)
    {
      film_out(FILM_TRYOUT, 0);
      login_abort("\n�A�� ...");
    }

    vget(b_lines - 2, 0, "   [�z���b��] ", uid, IDLEN + 1, DOECHO);

    if (!str_cmp(uid, STR_NEW))
    {
#ifdef LOGINASNEW
#  ifdef HAVE_GUARANTOR		/* itoc.000319: �O�ҤH��� */
      vget(b_lines - 2, 0, "   [�z���O�H] ", uid, IDLEN + 1, DOECHO);
      if (!*uid || (acct_load(&cuser, uid) < 0))
      {
	vmsg("��p�A�S�����ФH���o�[�J����");
      }
      else if (!HAS_PERM(PERM_GUARANTOR))
      {
	vmsg("��p�A�z����������O�H�����ФH");
      }
      else if (!vget(b_lines - 2, 40, "[�O�H�K�X] ", passbuf, PSWDLEN + 1, NOECHO))
      {       
	continue;
      }
      else
      {
	if (chkpasswd(cuser.passwd, passbuf))
	{
	  logattempt('-', content);
	  vmsg(ERR_PASSWD);
	}
	else
	{
	  FILE *fp;
	  char parentid[IDLEN + 1], buf[80];
	  time_t now;

	  /* itoc.010820: �O���O�H��O�ҤH�γQ�O�H */
	  strcpy(parentid, cuser.userid);
	  acct_apply();
	  time(&now);

	  /* itoc.010820.����: ���� log �b�歺�A�b reaper �ɥi�H��K�� tree */
	  sprintf(buf, "%s �� %s ���Ц��H(%s)�[�J����\n", parentid, Btime(&now), cuser.userid);
	  usr_fpath(fpath, cuser.userid, "guarantor");
	  if (fp = fopen(fpath, "a"))
	  {
	    fputs(buf, fp);
	    fclose(fp);
	  }
	  sprintf(buf, "%s �� %s �Q���H(%s)���Х[�J����\n", cuser.userid, Btime(&now), parentid);
	  usr_fpath(fpath, parentid, "guarantor");
	  if (fp = fopen(fpath, "a"))
	  {
	    fputs(buf, fp);
	    fclose(fp);
	  }

	  break;
	}
      }
#  else
      acct_apply(); /* Thor.980917: ����: setup cuser ok */
      break;
#  endif
#else
      outs("\n���t�Υثe�Ȱ��u�W���U, �Х� " STR_GUEST " �i�J");
      continue;
#endif
    }
    else if (!*uid)
    {
      /* �Y�S��J ID�A���� continue */
    }
    else if (str_cmp(uid, STR_GUEST))	/* �@��ϥΪ� */
    {
      if (!vget(b_lines - 2, 40, "[�z���K�X] ", passbuf, PSWDLEN + 1, NOECHO))
	continue;	/* �����K�X�h�����n�J */

      /* itoc.040110: �b��J�� ID �αK�X�A�~���J .ACCT */
      if (acct_load(&cuser, uid) < 0)
      {
	vmsg(err_uid);
	continue;
      }

      if (chkpasswd(cuser.passwd, passbuf))
      {
	logattempt('-', content);
	vmsg(ERR_PASSWD);
      }
      else
      {
	if (!str_cmp(cuser.userid, str_sysop))
	{
#ifdef SYSOP_SU
	  /* ²�檺 SU �\�� */
	  if (vans("�ܧ�ϥΪ̨���(Y/N)�H[N] ") == 'y')
	  {
	    for (;;)
	    {
	      if (vget(b_lines - 2, 0, "   [�ܧ�b��] ", uid, IDLEN + 1, DOECHO) && 
		acct_load(&cuser, uid) >= 0)
		break;
	      vmsg(err_uid);
	    }
	  }
	  else
#endif
	  {
	    /* SYSOP gets all permission bits */
	    /* itoc.010902: DENY perm �ƥ~ */
	    cuser.userlevel = ~0 ^ (PERM_DENYMAIL | PERM_DENYTALK | PERM_DENYCHAT | PERM_DENYPOST | PERM_DENYLOGIN | PERM_PURGE);
	  }   
	}

	if (cuser.ufo & UFO_ACL)
	{
	  usr_fpath(fpath, cuser.userid, FN_ACL);
	  str_lower(fromhost, fromhost);	/* lkchu.981201: ���p�g */
	  if (!acl_has(fpath, "", fromhost))
	  {	/* Thor.980728: �`�N acl �ɤ��n�����p�g */
	    logattempt('-', content);
	    login_abort("\n�z���W���a�I���ӹ�l�A�Юֹ� [�W���a�I�]�w��]");
	  }
	}

	logattempt(' ', content);

	/* check for multi-session */

	if (!HAS_PERM(PERM_ALLADMIN))
	{
	  UTMP *ui;
	  pid_t pid;

	  if (HAS_PERM(PERM_DENYLOGIN | PERM_PURGE))
	    login_abort("\n�o�ӱb���Ȱ��A�ȡA�Ա��ЦV�������ߡC");


	  if (!(ui = (UTMP *) utmp_find(cuser.userno)))
	    break;		/* user isn't logged in */

	  pid = ui->pid;
	  if (pid && vans("�z�Q�𱼨�L���ƪ� login (Y/N)�ܡH[Y] ") != 'n' && pid == ui->pid)
	  {
	    if ((kill(pid, SIGTERM) == -1) && (errno == ESRCH))
	      utmp_free(ui);
	    else
	      sleep(3);			/* �Q�𪺤H�o�ɭԥ��b�ۧڤF�_ */ 
	    blog("MULTI", cuser.userid);
	  }

	  if ((multi = utmp_count(cuser.userno, 0)) >= MULTI_MAX || 	/* �u�W�w�� MULTI_MAX ���ۤv�A�T��n�J */
	    (!multi && acct_load(&cuser, uid) < 0))			/* yiting.050101: �Y��w�𱼩Ҧ� multi-login�A���򭫷sŪ���H�M���ܧ� */
	    login_abort("\n�A�� ...");
	}
	break;
      }
    }
    else
    {				/* guest */
      if (acct_load(&cuser, uid) < 0)
      {
	vmsg(err_uid);
	continue;
      }
      logattempt(' ', content);
      cuser.userlevel = 0;	/* Thor.981207: �ȤH�ê�, �j��g�^cuser.userlevel */
      cuser.ufo = UFO_DEFAULT_GUEST;
      break;	/* Thor.980917: ����: cuser ok! */
    }
  }

  return multi;
}


static void
login_level()
{
  int fd;
  usint level;
  ACCT tuser;
  char fpath[64];

  /* itoc.010804.����: �� PERM_VALID �̦۰ʵo�� PERM_POST PERM_PAGE PERM_CHAT */
  level = cuser.userlevel | (PERM_ALLVALID ^ PERM_VALID);

  if (!(level & PERM_ALLADMIN))
  {
#ifdef JUSTIFY_PERIODICAL
    if ((level & PERM_VALID) && (cuser.tvalid + VALID_PERIOD < ap_start))
    {
      level ^= PERM_VALID;
      /* itoc.011116: �D�ʵo�H�q���ϥΪ̡A�@���e�H�����D�|���|�ӯӪŶ� !? */
      mail_self(FN_ETC_REREG, str_sysop, "�z���{�Ҥw�g�L���A�Э��s�{��", 0);
    }
#endif

#ifdef NEWUSER_LIMIT
    /* �Y�Ϥw�g�q�L�{�ҡA�٬O�n���ߤT�� */
    if (ap_start - cuser.firstlogin < 3 * 86400)
      level &= ~PERM_POST;
#endif

    /* itoc.000520: ���g�����{��, �T�� post/chat/talk/write */
    if (!(level & PERM_VALID))
      level &= ~(PERM_POST | PERM_CHAT | PERM_PAGE);

    if (level & PERM_DENYPOST)
      level &= ~PERM_POST;

    if (level & PERM_DENYTALK)
      level &= ~PERM_PAGE;

    if (level & PERM_DENYCHAT)
      level &= ~PERM_CHAT;

    if ((cuser.numemails >> 4) > (cuser.numlogins + cuser.numposts))
      level |= PERM_DENYMAIL;
  }

  cuser.userlevel = level;

  usr_fpath(fpath, cuser.userid, fn_acct);
  if ((fd = open(fpath, O_RDWR)) >= 0)
  {
    if (read(fd, &tuser, sizeof(ACCT)) == sizeof(ACCT))
    {
      /* itoc.010805.����: �o�����g�^ .ACCT �O���F���O�H Query �u�W�ϥΪ̮�
	 �X�{���W���ɶ�/�ӷ����T�A�H�Φ^�s���T�� userlvel */
      tuser.userlevel = level;
      tuser.lastlogin = ap_start;
      strcpy(tuser.lasthost, cuser.lasthost);

      lseek(fd, (off_t) 0, SEEK_SET);
      write(fd, &tuser, sizeof(ACCT));
    }
    close(fd);
  }
}


static void
login_status(multi)
  int multi;
{
  usint status;
  char fpath[64];
  struct tm *ptime;

  status = 0;

  /* itoc.010831: multi-login ���ĤG���[�W���i�ܰʿ������X�� */
  if (multi)
    status |= STATUS_COINLOCK;

  /* itoc.011022: �[�J�ͤ�X�� */
  ptime = localtime(&ap_start);
  if (cuser.day == ptime->tm_mday && cuser.month == ptime->tm_mon + 1)
    status |= STATUS_BIRTHDAY;

  /* �B�ͦW��P�B�B�M�z�L���H�� */
  if (ap_start > cuser.tcheck + CHECK_PERIOD)
  {
    outz(MSG_CHKDATA);
    refresh();

    cuser.tcheck = ap_start;
    usr_fpath(fpath, cuser.userid, fn_pal);
    pal_sync(fpath);
#ifdef HAVE_ALOHA
    usr_fpath(fpath, cuser.userid, FN_FRIENZ);
    frienz_sync(fpath);
#endif
#ifdef OVERDUE_MAILDEL
    status |= m_quota();		/* Thor.����: ��ƾ�z�]�֦��]�t BIFF check */
#endif
  }
#ifdef OVERDUE_MAILDEL
  else
#endif
    status |= m_query(cuser.userid);

  /* itoc.010924: �ˬd�ӤH��ذϬO�_�L�h */
#ifndef LINUX	/* �b Linux �U�o�ˬd�ǩǪ� */
  {
    struct stat st;
    usr_fpath(fpath, cuser.userid, "gem");
    if (!stat(fpath, &st) && (st.st_size >= 512 * 7))
      status |= STATUS_MGEMOVER;
  }
#endif

  cutmp->status |= status;
}


static void
login_other()
{
  usint status;
  char fpath[64];

  /* �R�����~�n�J�O�� */
  usr_fpath(fpath, cuser.userid, FN_BADLOGIN);
  if (more(fpath, (char *) -1) >= 0 && vans("�H�W����J�K�X���~�ɪ��W���a�I�O���A�n�R����(Y/N)�H[Y] ") != 'n')
    unlink(fpath);

  if (!HAS_PERM(PERM_VALID))
    film_out(FILM_NOTIFY, -1);		/* �|���{�ҳq�� */
#ifdef JUSTIFY_PERIODICAL
  else if (!HAS_PERM(PERM_ALLADMIN) && (cuser.tvalid + VALID_PERIOD - INVALID_NOTICE_PERIOD < ap_start))
    film_out(FILM_REREG, -1);		/* ���Įɶ��O�� 10 �ѫe���Xĵ�i */
#endif

#ifdef NEWUSER_LIMIT
  if (ap_start - cuser.firstlogin < 3 * 86400)
    film_out(FILM_NEWUSER, -1);		/* �Y�Ϥw�g�q�L�{�ҡA�٬O�n���ߤT�� */
#endif

  status = cutmp->status;

#ifdef OVERDUE_MAILDEL
  if (status & STATUS_MQUOTA)
    film_out(FILM_MQUOTA, -1);		/* �L���H��Y�N�M��ĵ�i */
#endif

  if (status & STATUS_MAILOVER)
    film_out(FILM_MAILOVER, -1);	/* �H��L�h�αH�H�L�h */

  if (status & STATUS_MGEMOVER)
    film_out(FILM_MGEMOVER, -1);	/* itoc.010924: �ӤH��ذϹL�hĵ�i */

  if (status & STATUS_BIRTHDAY)
    film_out(FILM_BIRTHDAY, -1);	/* itoc.010415: �ͤ��ѤW���� special �w��e�� */

  ve_recover();				/* �W���_�u�A�s�边�^�s */
}


static void
tn_login()
{
  int multi;
  char buf[128];

  bbsmode = M_LOGIN;	/* itoc.020828: �H�K�L�[����J�� igetch �|�X�{ movie */

  /* --------------------------------------------------- */
  /* �n���t��						 */
  /* --------------------------------------------------- */

  /* Thor.990415: �O��ip, �ȥ��d���� */
  sprintf(buf, "%s ip:%08x (%d)", fromhost, tn_addr, currpid);

  multi = login_user(buf);

  blog("ENTER", buf);

  /* --------------------------------------------------- */
  /* ��l�� utmp�Bflag�Bmode�B�H�c			 */
  /* --------------------------------------------------- */

  bbstate = STAT_STARTED;	/* �i�J�t�ΥH��~�i�H�^���y */
  utmp_setup(M_LOGIN);		/* Thor.980917: ����: cutmp, cutmp-> setup ok */
  total_user = ushm->count;	/* itoc.011027: ���i�ϥΪ̦W��e�A�ҩl�� total_user */

  mbox_main();

#ifdef MODE_STAT
  memset(&modelog, 0, sizeof(UMODELOG));
  mode_lastchange = ap_start;
#endif

  if (cuser.userlevel)		/* not guest */
  {
    /* ------------------------------------------------- */
    /* �ֹ� user level �ñN .ACCT �g�^			 */
    /* ------------------------------------------------- */

    /* itoc.030929: �b .ACCT �g�^�H�e�A���i�H������ vmsg(NULL) �� more(xxxx, NULL)
       �����F��A�o�˦p�G user �b vmsg(NULL) �ɦ^�{�ҫH�A�~���|�Q�g�^�� cuser �\�L */

    cuser.lastlogin = ap_start;
    str_ncpy(cuser.lasthost, fromhost, sizeof(cuser.lasthost));

    login_level();

    /* ------------------------------------------------- */
    /* �]�w status					 */
    /* ------------------------------------------------- */

    login_status(multi);

    /* ------------------------------------------------- */
    /* �q�Ǹ�T						 */
    /* ------------------------------------------------- */

    login_other();
  }

  srand(ap_start * cuser.userno * currpid);
}


static void
tn_motd()
{
  usint ufo;

  ufo = cuser.ufo;

  if (!(ufo & UFO_MOTD))
  {
    more("gem/@/@-day", NULL);	/* ����������D */
    pad_view();
  }

#ifdef HAVE_NOALOHA
  if (!(ufo & UFO_NOALOHA))
#endif
  {
#ifdef LOGIN_NOTIFY
    loginNotify();
#endif
#ifdef HAVE_ALOHA
    aloha();
#endif
  }

#ifdef HAVE_FORCE_BOARD
  brd_force();	/* itoc.000319: �j��\Ū���i�O */
#endif
}


/* ----------------------------------------------------- */
/* trap signals						 */
/* ----------------------------------------------------- */


static void
tn_signals()
{
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  act.sa_handler = (void *) abort_bbs;
  sigaction(SIGBUS, &act, NULL);
  sigaction(SIGSEGV, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGXCPU, &act, NULL);
#ifdef SIGSYS
  /* Thor.981221: easy for porting */
  sigaction(SIGSYS, &act, NULL);/* bad argument to system call */
#endif

  act.sa_handler = (void *) talk_rqst;
  sigaction(SIGUSR1, &act, NULL);

  act.sa_handler = (void *) bmw_rqst;
  sigaction(SIGUSR2, &act, NULL);

  /* �b���ɥ� sigset_t act.sa_mask */
  sigaddset(&act.sa_mask, SIGPIPE);
  sigprocmask(SIG_BLOCK, &act.sa_mask, NULL);

}


static inline void
tn_main()
{
  clear();

#ifdef HAVE_LOGIN_DENIED
  if (acl_has(BBS_ACLFILE, "", fromhost))
    login_abort("\n�Q�����󤣳Q�ͯ�����");
#endif

  time(&ap_start);

  prints("%s �� " SCHOOLNAME " �� " MYIPADDR "\n"
    "�w����{�i\033[1;33;46m %s \033[m�j�ثe�u�W�H�� [%d] �H",
    str_host, str_site, ushm->count);

  film_out((ap_start % 3) + FILM_OPENING0, 3);	/* �ü���ܶ}�Y�e�� */
  
  currpid = getpid();

  tn_signals();	/* Thor.980806: ��� tn_login�e, �H�K call in���|�Q�� */
  tn_login();

  board_main();
  gem_main();
#ifdef MY_FAVORITE
  mf_main();
#endif
  talk_main();

  tn_motd();

  menu();
  abort_bbs();	/* to make sure it will terminate */
}


/* ----------------------------------------------------- */
/* FSA (finite state automata) for telnet protocol	 */
/* ----------------------------------------------------- */


static void
telnet_init()
{
  static char svr[] = 
  {
    IAC, DO, TELOPT_TTYPE,
    IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE,
    IAC, WILL, TELOPT_ECHO,
    IAC, WILL, TELOPT_SGA
  };

  int n, len;
  char *cmd;
  int rset;
  struct timeval to;
  char buf[64];

  /* --------------------------------------------------- */
  /* init telnet protocol				 */
  /* --------------------------------------------------- */

  cmd = svr;

  for (n = 0; n < 4; n++)
  {
    len = (n == 1 ? 6 : 3);
    send(0, cmd, len, 0);
    cmd += len;

    rset = 1;
    /* Thor.981221: for future reservation bug */
    to.tv_sec = 1;
    to.tv_usec = 1;
    if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
      recv(0, buf, sizeof(buf), 0);
  }
}


/* ----------------------------------------------------- */
/* �䴩�W�L 24 �C���e��					 */
/* ----------------------------------------------------- */


static void
term_init()
{
#if 0   /* fuse.030518: ���� */
  server�ݡG�A�|���ܦ�C�ƶܡH(TN_NAWS, Negotiate About Window Size)
  client���GYes, I do. (TNCH_DO)

  ����b�s�u�ɡA��TERM�ܤƦ�C�ƮɴN�|�o�X�G
  TNCH_IAC + TNCH_SB + TN_NAWS + ��ƦC�� + TNCH_IAC + TNCH_SE;
#endif

  /* ask client to report it's term size */
  static char svr[] = 		/* server */
  {
    IAC, DO, TELOPT_NAWS
  };

  int rset;
  char buf[64], *rcv;
  struct timeval to;

  /* �ݹ�� (telnet client) ���S���䴩���P���ù��e�� */
  send(0, svr, 3, 0);

  rset = 1;
  to.tv_sec = 1;
  to.tv_usec = 1;
  if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
    recv(0, buf, sizeof(buf), 0);

  rcv = NULL;
  if ((uschar) buf[0] == IAC && buf[2] == TELOPT_NAWS)
  {
    /* gslin: Unix �� telnet �靈�L�[ port �Ѽƪ��欰���Ӥ@�� */
    if ((uschar) buf[1] == SB)
    {
      rcv = buf + 3;
    }
    else if ((uschar) buf[1] == WILL)
    {
      if ((uschar) buf[3] != IAC)
      {
	rset = 1;
	to.tv_sec = 1;
	to.tv_usec = 1;
	if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
	  recv(0, buf + 3, sizeof(buf) - 3, 0);
      }
      if ((uschar) buf[3] == IAC && (uschar) buf[4] == SB && buf[5] == TELOPT_NAWS)
	rcv = buf + 6;
    }
  }

  if (rcv)
  {
    b_lines = ntohs(* (short *) (rcv + 2)) - 1;
    b_cols = ntohs(* (short *) rcv) - 1;

    /* b_lines �ܤ֭n 23�A�̦h����W�L T_LINES - 1 */
    if (b_lines >= T_LINES)
      b_lines = T_LINES - 1;
    else if (b_lines < 23)
      b_lines = 23;
    /* b_cols �ܤ֭n 79�A�̦h����W�L T_COLS - 1 */
    if (b_cols >= T_COLS)
      b_cols = T_COLS - 1;
    else if (b_cols < 79)
      b_cols = 79;
  }
  else
  {
    b_lines = 23;
    b_cols = 79;
  }

  d_cols = b_cols - 79;
}


/* ----------------------------------------------------- */
/* stand-alone daemon					 */
/* ----------------------------------------------------- */


static void
start_daemon(port)
  int port; /* Thor.981206: �� 0 �N�� *�S���Ѽ�* , -1 �N�� -i (inetd) */
{
  int n;
  struct linger ld;
  struct sockaddr_in sin;
#ifdef HAVE_RLIMIT
  struct rlimit limit;
#endif
  char buf[80], data[80];
  time_t val;

  /*
   * More idiot speed-hacking --- the first time conversion makes the C
   * library open the files containing the locale definition and time zone.
   * If this hasn't happened in the parent process, it happens in the
   * children, once per connection --- and it does add up.
   */

  time(&val);
  strftime(buf, 80, "%d/%b/%Y %H:%M:%S", localtime(&val));

#ifdef HAVE_RLIMIT
  /* --------------------------------------------------- */
  /* adjust resource : 16 mega is enough		 */
  /* --------------------------------------------------- */

  limit.rlim_cur = limit.rlim_max = 16 * 1024 * 1024;
  /* setrlimit(RLIMIT_FSIZE, &limit); */
  setrlimit(RLIMIT_DATA, &limit);

#ifdef SOLARIS
#define RLIMIT_RSS RLIMIT_AS	/* Thor.981206: port for solaris 2.6 */
#endif

  setrlimit(RLIMIT_RSS, &limit);

  limit.rlim_cur = limit.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &limit);

  limit.rlim_cur = limit.rlim_max = 60 * 20;
  setrlimit(RLIMIT_CPU, &limit);
#endif

  /* --------------------------------------------------- */
  /* speed-hacking DNS resolve				 */
  /* --------------------------------------------------- */

  dns_init();

  /* --------------------------------------------------- */
  /* change directory to bbshome       			 */
  /* --------------------------------------------------- */

  chdir(BBSHOME);
  umask(077);

  /* --------------------------------------------------- */
  /* detach daemon process				 */
  /* --------------------------------------------------- */

  /* The integer file descriptors associated with the streams
     stdin, stdout, and stderr are 0,1, and 2, respectively. */

  close(1);
  close(2);

  if (port == -1) /* Thor.981206: inetd -i */
  {
    /* Give up root privileges: no way back from here	 */
    setgid(BBSGID);
    setuid(BBSUID);
#if 1
    n = sizeof(sin);
    if (getsockname(0, (struct sockaddr *) &sin, &n) >= 0)
      port = ntohs(sin.sin_port);
#endif
    /* mport = port; */ /* Thor.990325: ���ݭn�F:P */

    sprintf(data, "%d\t%s\t%d\tinetd -i\n", getpid(), buf, port);
    f_cat(PID_FILE, data);
    return;
  }

  close(0);

  if (fork())
    exit(0);

  setsid();

  if (fork())
    exit(0);

  /* --------------------------------------------------- */
  /* fork daemon process				 */
  /* --------------------------------------------------- */

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;

  if (port == 0) /* Thor.981206: port 0 �N��S���Ѽ� */
  {
    n = MAX_BBSDPORT - 1;
    while (n)
    {
      if (fork() == 0)
	break;

      sleep(1);
      n--;
    }
    port = myports[n];
  }

  n = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  val = 1;
  setsockopt(n, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(val));

  ld.l_onoff = ld.l_linger = 0;
  setsockopt(n, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

  /* mport = port; */ /* Thor.990325: ���ݭn�F:P */
  sin.sin_port = htons(port);
  if ((bind(n, (struct sockaddr *) &sin, sizeof(sin)) < 0) || (listen(n, QLEN) < 0))
    exit(1);

  /* --------------------------------------------------- */
  /* Give up root privileges: no way back from here	 */
  /* --------------------------------------------------- */

  setgid(BBSGID);
  setuid(BBSUID);

  /* standalone */
  sprintf(data, "%d\t%s\t%d\n", getpid(), buf, port);
  f_cat(PID_FILE, data);
}


/* ----------------------------------------------------- */
/* reaper - clean up zombie children			 */
/* ----------------------------------------------------- */


static inline void
reaper()
{
  while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}


#ifdef	SERVER_USAGE
static void
servo_usage()
{
  struct rusage ru;
  FILE *fp;

  fp = fopen("run/bbs.usage", "a");

  if (!getrusage(RUSAGE_CHILDREN, &ru))
  {
    fprintf(fp, "\n[Server Usage] %d: %d\n\n"
      "user time: %.6f\n"
      "system time: %.6f\n"
      "maximum resident set size: %lu P\n"
      "integral resident set size: %lu\n"
      "page faults not requiring physical I/O: %d\n"
      "page faults requiring physical I/O: %d\n"
      "swaps: %d\n"
      "block input operations: %d\n"
      "block output operations: %d\n"
      "messages sent: %d\n"
      "messages received: %d\n"
      "signals received: %d\n"
      "voluntary context switches: %d\n"
      "involuntary context switches: %d\n\n",

      getpid(), ap_start,
      (double) ru.ru_utime.tv_sec + (double) ru.ru_utime.tv_usec / 1000000.0,
      (double) ru.ru_stime.tv_sec + (double) ru.ru_stime.tv_usec / 1000000.0,
      ru.ru_maxrss,
      ru.ru_idrss,
      ru.ru_minflt,
      ru.ru_majflt,
      ru.ru_nswap,
      ru.ru_inblock,
      ru.ru_oublock,
      ru.ru_msgsnd,
      ru.ru_msgrcv,
      ru.ru_nsignals,
      ru.ru_nvcsw,
      ru.ru_nivcsw);
  }

  fclose(fp);
}
#endif


static void
main_term()
{
#ifdef	SERVER_USAGE
  servo_usage();
#endif
  exit(0);
}


static inline void
main_signals()
{
  struct sigaction act;

  /* act.sa_mask = 0; */ /* Thor.981105: �зǥΪk */
  sigemptyset(&act.sa_mask);      
  act.sa_flags = 0;

  act.sa_handler = reaper;
  sigaction(SIGCHLD, &act, NULL);

  act.sa_handler = main_term;
  sigaction(SIGTERM, &act, NULL);

#ifdef	SERVER_USAGE
  act.sa_handler = servo_usage;
  sigaction(SIGPROF, &act, NULL);
#endif

  /* sigblock(sigmask(SIGPIPE)); */
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int csock;			/* socket for Master and Child */
  int value;
  int *totaluser;
  struct sockaddr_in sin;

  /* --------------------------------------------------- */
  /* setup standalone daemon				 */
  /* --------------------------------------------------- */

  /* Thor.990325: usage, bbsd, or bbsd -i, or bbsd 1234 */
  /* Thor.981206: �� 0 �N�� *�S���Ѽ�*, -1 �N�� -i */
  start_daemon(argc > 1 ? strcmp("-i", argv[1]) ? atoi(argv[1]) : -1 : 0);

  main_signals();

  /* --------------------------------------------------- */
  /* attach shared memory & semaphore			 */
  /* --------------------------------------------------- */

#ifdef HAVE_SEM
  sem_init();
#endif
  ushm_init();
  bshm_init();
  fshm_init();

  /* --------------------------------------------------- */
  /* main loop						 */
  /* --------------------------------------------------- */

  totaluser = &ushm->count;
  /* avgload = &ushm->avgload; */

  for (;;)
  {
    value = 1;
    if (select(1, (fd_set *) & value, NULL, NULL, NULL) < 0)
      continue;

    value = sizeof(sin);
    csock = accept(0, (struct sockaddr *) &sin, &value);
    if (csock < 0)
    {
      reaper();
      continue;
    }

    ap_start++;
    argc = *totaluser;
    if (argc >= MAXACTIVE - 5 /* || *avgload > THRESHOLD */ )
    {
      /* �ɥ� currtitle */
      sprintf(currtitle, "�ثe�u�W�H�� [%d] �H�A�t�ι��M�A�еy��A��\n", argc);
      send(csock, currtitle, strlen(currtitle), 0);
      close(csock);
      continue;
    }

    if (fork())
    {
      close(csock);
      continue;
    }

    dup2(csock, 0);
    close(csock);

    /* ------------------------------------------------- */
    /* ident remote host / user name via RFC931		 */
    /* ------------------------------------------------- */

    tn_addr = sin.sin_addr.s_addr;
    dns_name((char *) &sin.sin_addr, fromhost);

    telnet_init();
    term_init();
    tn_main();
  }
}
