/*-------------------------------------------------------*/
/* util/mailpost.c	( NTHU CS MapleBBS Ver 2.36 )	 */
/*-------------------------------------------------------*/
/* target : �f�֨����{�ҫH�礧�^�H			 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/
/* notice : brdshm (board shared memory) synchronize     */
/*-------------------------------------------------------*/


#include "bbs.h"


static void
mailog(msg)
  char *msg;
{
  FILE *fp;

  if (fp = fopen(BMTA_LOGFILE, "a"))
  {
    time_t now;
    struct tm *p;

    time(&now);
    p = localtime(&now);
    fprintf(fp, "%02d/%02d %02d:%02d:%02d <mailpost> %s\n",
      p->tm_mon + 1, p->tm_mday, 
      p->tm_hour, p->tm_min, p->tm_sec, 
      msg);
    fclose(fp);
  }
}


/* ----------------------------------------------------- */
/* �O�����Ҹ�ơGuser ���i�ॿ�b�u�W�A�G�g�J�ɮץH�O�P�� */
/* ----------------------------------------------------- */


static int
is_badid(userid)
  char *userid;
{
  int ch;
  char *str;

  ch = strlen(userid);
  if (ch < 2 || ch > IDLEN)
    return 1;

  if (!is_alpha(*userid))
    return 1;

  str = userid;
  while (ch = *(++str))
  {
    if (!is_alnum(ch))
      return 1;
  }
  return 0;
}


static void
justify_user(userid, email)
  char *userid, *email;
{
  char fpath[64];
  HDR hdr;
  FILE *fp;

  /* �H�{�ҳq�L�H���ϥΪ� */
  usr_fpath(fpath, userid, FN_DIR);
  if (!hdr_stamp(fpath, HDR_LINK, &hdr, FN_ETC_JUSTIFIED))
  {
    strcpy(hdr.title, "�z�w�g�q�L�����{�ҤF�I");
    strcpy(hdr.owner, STR_SYSOP);
    hdr.xmode = MAIL_NOREPLY;
    rec_add(fpath, &hdr, sizeof(HDR));
  }

  /* �O���b FN_JUSTIFY */
  usr_fpath(fpath, userid, FN_JUSTIFY);
  if (fp = fopen(fpath, "a"))
  {
    fprintf(fp, "RPY: %s\n", email);
    fclose(fp);
  }
}


static void
verify_user(str)
  char *str;
{
  int fd;
  char *ptr, *next, fpath[64];
  ACCT acct;

  /* itoc.����: "userid(regkey) [VALID]" */

  if ((ptr = strchr(str, '(')) && (next = strchr(ptr + 1, ')')))
  {
    *ptr = '\0';
    *next = '\0';

    if (!is_badid(str) && !str_ncmp(next + 1, " [VALID]", 8))
    {
      /* �즹�榡�����T */

      usr_fpath(fpath, str, FN_ACCT);
      if ((fd = open(fpath, O_RDWR, 0600)) >= 0)
      {
	if (read(fd, &acct, sizeof(ACCT)) == sizeof(ACCT))
	{
	  if (str_hash(acct.email, acct.tvalid) == chrono32(ptr))	/* regkey ���T */
	  {
	    /* �����v�� */
	    acct.userlevel |= PERM_VALID;
	    time(&acct.tvalid);
	    lseek(fd, (off_t) 0, SEEK_SET);
	    write(fd, &acct, sizeof(ACCT));

	    justify_user(str, acct.email);
	  }
	}
	close(fd);
      }
    }
  }
}


/* ----------------------------------------------------- */
/* �D�{��						 */
/* ----------------------------------------------------- */


static void
mailpost()
{
  int count;
  char *ptr, buf[512];

  /* �u�ݭn�� Subject: �����Y */

  count = 0;
  while ((++count < 20) && fgets(buf, sizeof(buf), stdin))	/* �̦h fgets 20 ���A�קK�S�� subject */
  {
    if (!str_ncmp(buf, "Subject: ", 9))
    {
      str_decode(buf);

      /* itoc.����: mail.c: TAG_VALID " userid(regkey) [VALID]" */
      if (ptr = strstr(buf, TAG_VALID " "))
      {
	/* gslin.990101: TAG_VALID ���פ��@�w */
	verify_user(ptr + sizeof(TAG_VALID));
      }
      break;
    }
  }
}


static void
sig_catch(sig)
  int sig;
{
  char buf[512];
  
  while (fgets(buf, sizeof(buf), stdin))
    ;
  sprintf(buf, "signal [%d]", sig);
  mailog(buf);
  exit(0);
}


int
main()
{
  char buf[512];

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  signal(SIGBUS, sig_catch);
  signal(SIGSEGV, sig_catch);
  signal(SIGPIPE, sig_catch);

  mailpost();

  /* eat mail queue */
  while (fgets(buf, sizeof(buf), stdin))
    ;

  exit(0);
}
