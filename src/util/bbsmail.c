/*-------------------------------------------------------*/
/* util/bbsmail.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : �� Internet �H�H�� BBS �����ϥΪ�		 */
/* create : 95/03/29					 */
/* update : 97/03/29					 */
/*-------------------------------------------------------*/


#include "bbs.h"

#include <sysexits.h>

#define	ANTI_HTMLMAIL		/* itoc.021014: �� html_mail */
#define	ANTI_NOTMYCHARSETMAIL	/* itoc.030513: �� not-mycharset mail */


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
    fprintf(fp, "%02d/%02d %02d:%02d:%02d <bbsmail> %s\n",
      p->tm_mon + 1, p->tm_mday,
      p->tm_hour, p->tm_min, p->tm_sec,
      msg);
    fclose(fp);
  }
}


/* ----------------------------------------------------- */
/* user�Gshm �������P cache.c �ۮe			 */
/* ----------------------------------------------------- */


static UCACHE *ushm;


static inline void
init_ushm()
{
  ushm = shm_new(UTMPSHM_KEY, sizeof(UCACHE));
}


static inline void
bbs_biff(userid)
  char *userid;
{
  UTMP *utmp, *uceil;
  usint offset;

  offset = ushm->offset;
  if (offset > (MAXACTIVE - 1) * sizeof(UTMP))	/* Thor.980805: ���Mcall���� */
    offset = (MAXACTIVE - 1) * sizeof(UTMP);

  utmp = ushm->uslot;
  uceil = (void *) utmp + offset;

  do
  {
    if (!str_cmp(utmp->userid, userid))
      utmp->status |= STATUS_BIFF;
  } while (++utmp <= uceil);
}


/* ----------------------------------------------------- */
/* �D�{��						 */
/* ----------------------------------------------------- */


static int
mail2bbs(userid)
  char *userid;
{
  HDR hdr;
  char buf[512], title[256], sender[256], owner[256], nick[256], folder[64];
  char *str, *ptr, decode;
  int fd;
  FILE *fp;

  /* check if the userid is in our bbs now */

  usr_fpath(folder, userid, NULL);
  if (!dashd(folder))
  {
    sprintf(buf, "BBS user <%s> not existed", userid);
    mailog(buf);
    return EX_NOUSER;
  }
  strcat(folder, "/" FN_DIR);

  if (rec_num(folder, sizeof(HDR)) >= MAX_BBSMAIL)
  {
    sprintf(buf, "BBS user <%s> over-spammed", userid);
    mailog(buf);
    return EX_NOUSER;
  }

  /* parse header */

  title[0] = sender[0] = owner[0] = nick[0] = '\0';
  decode = 0;

  while (fgets(buf, sizeof(buf), stdin))
  {
start:
    if (!memcmp(buf, "From: ", 6))
    {
      str = buf + 6;

      if (*str == '\0')
	return EX_NOUSER;

      if (ptr = strchr(str, '\n'))
	*ptr = '\0';

      str_from(str, owner, nick);
      if (*nick)
	sprintf(sender, "%s (%s)", owner, nick);
      else
	strcpy(sender, owner);

      /* itoc.040804: �׫H�¥զW�� */
      str_lower(buf, owner);	/* �O���� email ���j�p�g */
      if (ptr = (char *) strchr(buf, '@'))
      {
	*ptr++ = '\0';

	if (!acl_has(MAIL_ACLFILE, buf, ptr) ||
	  acl_has(UNMAIL_ACLFILE, buf, ptr) > 0)
	{
	  sprintf(buf, "SPAM %s", sender);
	  mailog(buf);
	  return EX_NOUSER;
	}
      }
    }

    else if (!memcmp(buf, "Subject: ", 9))
    {
      str_ansi(title, buf + 9, sizeof(title));
      /* str_decode(title); */
      /* LHD.051106: �Y�i��g RFC 2047 QP encode �h���i��h�� subject */
      if (strstr(buf + 9, "=?"))
      {
	while (fgets(buf, sizeof(buf), stdin))
	{
	  if (buf[0] == ' ' || buf[0] == '\t')  /* �ĤG��H��|�H�ťթ� TAB �}�Y */
	    str_ansi(title + strlen(title), strstr(buf, "=?"), sizeof(title));
	  else
	  {
	    str_decode(title);
	    goto start;
	  }
	}
      }
    }

    else if (!memcmp(buf, "Content-Type: ", 14))
    {
      str = buf + 14;

#ifdef ANTI_HTMLMAIL
      /* �@�� BBS �ϥΪ̳q�`�u�H��r�l��άO�q��L BBS ���H�峹��ۤv���H�c
         �Ӽs�i�H��q�`�O html �榡�άO�̭������a��L�ɮ�
         �Q�ζl�����Y�� Content-Type: ���ݩʧⰣ�F text/plain (��r�l��) ���H�󳣾פU�� */
      if (*str != '\0' && str_ncmp(str, "text/plain", 10))
      {
	sprintf(buf, "ANTI-HTML [%d] %s => %s", getppid(), sender, userid);
	mailog(buf);
	return EX_NOUSER;
      }
#endif

#ifdef ANTI_NOTMYCHARSETMAIL
      {
	char charset[32];
	mm_getcharset(str, charset, sizeof(charset));
	if (str_cmp(charset, MYCHARSET) && str_cmp(charset, "us-ascii"))
	{
	  sprintf(buf, "ANTI-NONMYCHARSET [%d] %s => %s", getppid(), sender, userid);
	  mailog(buf);
	  return EX_NOUSER;
	}
      }
#endif
    }

    else if (!memcmp(buf, "Content-Transfer-Encoding: ", 27))
    {
      mm_getencode(buf + 27, &decode);
    }

    else if (buf[0] == '\n')
    {
      break;
    }
  }

  /* allocate a file for the new mail */

  fd = hdr_stamp(folder, 0, &hdr, buf);
  hdr.xmode = MAIL_INCOME;

  str_ncpy(hdr.owner, owner, sizeof(hdr.owner));
  str_ncpy(hdr.nick, nick, sizeof(hdr.nick));
  if (!title[0])
    sprintf(title, "�Ӧ� %.64s", sender);
  str_ncpy(hdr.title, title, sizeof(hdr.title));

  /* copy the stdin to the specified file */

  fp = fdopen(fd, "w");

  fprintf(fp, "�@��: %.72s\n���D: %.72s\n�ɶ�: %s\n\n",
    sender, title, Btime(&hdr.chrono));

  while (fgets(buf, sizeof(buf), stdin))
  {
    if (decode && ((fd = mmdecode(buf, decode, buf)) > 0))
      buf[fd] = '\0';

    fputs(buf, fp);
  }

  fclose(fp);

  /* append the record to the .DIR */

  rec_add(folder, &hdr, sizeof(HDR));

  bbs_biff(userid);	/* itoc.021113: �q�� user ���s�H�� */

  /* Thor.980827: �[�W parent process id�A�H�K��U���H */
  sprintf(buf, "[%d] %s => %s", getppid(), sender, userid); 
  mailog(buf);

  return 0;
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
main(argc, argv)
  int argc;
  char *argv[];
{
  char buf[512];

  /* argv[1] is userid in bbs */

  if (argc < 2)
  {
    printf("Usage:\t%s <bbs_userid>\n", argv[0]);
    exit(-1);
  }

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  signal(SIGBUS, sig_catch);
  signal(SIGSEGV, sig_catch);
  signal(SIGPIPE, sig_catch);

  init_ushm();
  str_lower(buf, argv[1]);	/* �� userid �����p�g */

  if (mail2bbs(buf))
  {
    /* eat mail queue */
    while (fgets(buf, sizeof(buf), stdin))
      ;
  }
  exit(0);
}
