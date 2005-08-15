/*-------------------------------------------------------*/
/* bhttpd.c		( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : BBS's HTTP daemon				 */
/* create : 05/07/11					 */
/* update : 05/08/04					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/* author : yiting.bbs@bbscs.tku.edu.tw			 */
/*-------------------------------------------------------*/


#if 0	/* �s���@���� */

  http://my.domain/                            ����
  http://my.domain/brdlist                     �ݪO�C��
  http://my.domain/fvrlist                     �ڪ��̷R
  http://my.domain/usrlist                     �ϥΪ̦W��
  http://my.domain/brd?brdname&##              �峹�C��A�C�X�ݪO [brdname] �s�� ## �}�l�� 50 �g�峹
  http://my.domain/gem?brdname&folder          ��ذϦC��A�C�X�ݪO [brdname] ��ذϤ� folder �o�Ө��v�U���Ҧ��F��
  http://my.domain/mbox?##                     �H�c�C��A�C�X�H�c���s�� ## �}�l�� 50 �g�峹
  http://my.domain/bmore?brdname&##            �\Ū�ݪO�峹�A�\Ū�ݪO [brdname] ���� ## �g�峹
  http://my.domain/bmost?brdname&##            �\Ū�ݪO�峹�A�\Ū�ݪO [brdname] ���Ҧ��W�� ## �g�P���D���峹
  http://my.domain/gmore?brdname&folder&##     �\Ū��ذϤ峹�A�\Ū�ݪO [brdname] ��ذϤ� folder �o�Ө��v�U���� ## �g�峹
  http://my.domain/mmore?##                    �\Ū�H�c�峹�A�\Ū�H�c���� ## �g�峹
  http://my.domain/dopost?brdname              �o��峹��ݪO [brdname]
  http://my.domain/domail?userid               �o�e�H�� [userid]
  http://my.domain/dpost?brdname&##&###        �߰ݽT�w�R���ݪO [brdname] ���� ## �g�峹 (�� chrono �O ###)
  http://my.domain/delpost?brdname&##&###      �R���ݪO [brdname] ���� ## �g�峹 (�� chrono �O ###)
  http://my.domain/mpost?brdname&##&###        �аO�ݪO [brdname] ���� ## �g�峹 (�� chrono �O ###)
  http://my.domain/dmail?##&###                �߰ݽT�w�R���H�c���� ## �g�峹 (�� chrono �O ###)
  http://my.domain/delmail?##&###              �R���H�c���� ## �g�峹 (�� chrono �O ###)
  http://my.domain/mmail?##&###                �аO�H�c���� ## �g�峹 (�� chrono �O ###)
  http://my.domain/query?userid                �d�� userid
  http://my.domain/img/filename                ��ܹ���
  http://my.domain/rss?brdname                 �U�ݪO��RSS Feed
  http://my.domain/class?folder                �C�X������ [folder] �o�Ө��v�U���Ҧ��ݪO

#endif


#define _MODES_C_

#include "bbs.h"


#include <sys/wait.h>
#include <netinet/tcp.h>
#include <sys/resource.h>


#define SERVER_USAGE
#define	LOG_VERBOSE		/* �ԲӬ��� */


#define BHTTP_PIDFILE	"run/bhttp.pid"
#define BHTTP_LOGFILE	"run/bhttp.log"


#define BHTTP_PERIOD	(60 * 5)	/* �C 5 ���� check �@�� */
#define BHTTP_TIMEOUT	(60 * 3)	/* �W�L 3 �������s�u�N�������~ */
#define BHTTP_FRESH	86400		/* �C 1 �Ѿ�z�@�� log �� */


#define TCP_BACKLOG	3
#define TCP_RCVSIZ	2048


#define MIN_DATA_SIZE	(TCP_RCVSIZ + 3)
#define MAX_DATA_SIZE   262143		/* POST ���j�p����(byte) */


/* Thor.000425: POSIX �� O_NONBLOCK */

#ifndef O_NONBLOCK
#define M_NONBLOCK  FNDELAY
#else
#define M_NONBLOCK  O_NONBLOCK
#endif

#define HTML_TALL	50	/* �C��@�� 50 �g */


/* ----------------------------------------------------- */
/* ��檺�C��						 */
/* ----------------------------------------------------- */

#define HCOLOR_BG	"#000000"	/* �I�����C�� */
#define HCOLOR_TEXT	"#ffffff"	/* ��r���C�� */
#define HCOLOR_LINK	"#00ffff"	/* ���s���L�s�����C�� */
#define HCOLOR_VLINK	"#c0c0c0"	/* �w�s���L�s�����C�� */
#define HCOLOR_ALINK	"#ff0000"	/* �s���Q���U�ɪ��C�� */

#define HCOLOR_NECK	"#000070"	/* ��l���C�� */
#define HCOLOR_TIE	"#a000a0"	/* ��a���C�� */

#define HCOLOR_BAR	"#808080"	/* �����C�� */


/* ----------------------------------------------------- */
/* HTTP commands					 */
/* ----------------------------------------------------- */

typedef struct
{
  int (*func) ();
  char *cmd;
  int len;			/* strlen(Command.cmd) */
}      Command;


/* ----------------------------------------------------- */
/* client connection structure				 */
/* ----------------------------------------------------- */

#define LEN_COOKIE	(IDLEN + PASSLEN + 3 + 1)	/* userid&p=passwd */

typedef struct Agent
{
  struct Agent *anext;
  int sock;

  unsigned int ip_addr;

  time_t tbegin;		/* �s�u�}�l�ɶ� */
  time_t uptime;		/* �W���U���O���ɶ� */

  char url[48];
  char *urlp;

  char cookie[32];
  int setcookie;
  char *modified;

  /* �ϥΪ̸�ƭn�� acct_fetch() �~��ϥ� */
  int userno;
  char userid[IDLEN + 1];
  char username[UNLEN + 1];
  usint userlevel;

  /* �ү�ݨ쪺�ݪO�C��ΨϥΪ̦W�� */

#if MAXBOARD > MAXACTIVE
  void *myitem[MAXBOARD];
#else
  void *myitem[MAXACTIVE];
#endif
  int total_item;

  /* input �� */

  char *data;
  int size;			/* �ثe data �� malloc ���Ŷ��j�p */
  int used;

  /* output �� */

  FILE *fpw;
}     Agent;


/* ----------------------------------------------------- */
/* http state code					 */
/* ----------------------------------------------------- */

enum
{
  HS_END,

  HS_ERROR,			/* �y�k���~ */
  HS_ERR_LOGIN,			/* �|���n�J */
  HS_ERR_USER,			/* �b��Ū�����~ */
  HS_ERR_MORE,			/* �峹Ū�����~ */
  HS_ERR_BOARD,			/* �ݪOŪ�����~ */
  HS_ERR_MAIL,			/* �H��Ū�����~ */
  HS_ERR_CLASS,			/* ����Ū�����~ */
  HS_ERR_PERM,			/* �v������ */

  HS_OK,

  HS_REDIRECT,			/* ���s�ɦV */
  HS_NOTMOIDIFY,		/* �ɮרS���ܧ� */
  HS_BADREQUEST,		/* ���~���n�D */
  HS_FORBIDDEN,			/* �����v������ */
  HS_NOTFOUND,			/* �䤣���ɮ� */

  LAST_HS
};


static char *http_msg[LAST_HS] =
{
  NULL,

  "�y�k���~",
  "�z�|���n�J",
  "�S���o�ӱb��",
  "�ާ@���~�G�z�ҿ�����峹���s�b�Τw�R��",
  "�ާ@���~�G�L���ݪO�αz���v������",
  "�ާ@���~�G�L���H��αz�|���n�J",
  "�ާ@���~�G�z�ҿ�����������s�b�Τw�R��",
  "�ާ@���~�G�z�|���n�J���v�������A�L�k�i��o���ާ@",

  "200 OK",

  "302 Found",
  "304 Not Modified",
  "400 Bad Request",
  "403 Forbidden",
  "404 Not Found",
};


#define HS_REFRESH	0x0100	/* �۰ʸ���(�w�]�O3��) */


/* ----------------------------------------------------- */
/* AM : Agent Mode					 */
/* ----------------------------------------------------- */

#define AM_GET		0x010
#define AM_POST		0x020


/* ----------------------------------------------------- */
/* operation log and debug information			 */
/* ----------------------------------------------------- */
/* @START | ... | time					 */
/* ----------------------------------------------------- */

static FILE *flog;

extern int errno;
extern char *crypt();

static void
log_fresh()
{
  int count;
  char fsrc[64], fdst[64];
  char *fpath = BHTTP_LOGFILE;

  if (flog)
    fclose(flog);

  count = 9;
  do
  {
    sprintf(fdst, "%s.%d", fpath, count);
    sprintf(fsrc, "%s.%d", fpath, --count);
    rename(fsrc, fdst);
  } while (count);

  rename(fpath, fsrc);
  flog = fopen(fpath, "a");
}


static void
logit(key, msg)
  char *key;
  char *msg;
{
  time_t now;
  struct tm *p;

  time(&now);
  p = localtime(&now);
  /* Thor.990329: y2k */
  fprintf(flog, "%s\t%s\t%02d/%02d/%02d %02d:%02d:%02d\n",
    key, msg, p->tm_year % 100, p->tm_mon + 1, p->tm_mday,
    p->tm_hour, p->tm_min, p->tm_sec);
}


static void
log_open()
{
  FILE *fp;

  umask(077);

  if (fp = fopen(BHTTP_PIDFILE, "w"))
  {
    fprintf(fp, "%d\n", getpid());
    fclose(fp);
  }

  flog = fopen(BHTTP_LOGFILE, "a");
  logit("START", "MTA daemon");
}


/* ----------------------------------------------------- */
/* target : ANSI text to HTML tag			 */
/* author : yiting.bbs@bbs.cs.tku.edu.tw		 */
/* ----------------------------------------------------- */

#define ANSI_TAG	27
#define is_ansi(ch)	((ch >= '0' && ch <= '9') || ch == ';' || ch == '[')

#define	HAVE_HYPERLINK		/* �B�z�W�s�� */
#undef	HAVE_ANSIATTR		/* �ܤ֥Ψ�ӥBIE���䴩�{�{�A���ܤ��B�z :( */
#define HAVE_SAKURA		/* �����۰���Unicode */

#ifdef HAVE_ANSIATTR
#define ATTR_UNDER	0x1	/* ���u */
#define ATTR_BLINK	0x2	/* �{�� */
#define ATTR_ITALIC	0x4	/* ���� */

static int old_attr, now_attr;
#endif

static int old_color, now_color;
static char ansi_buf[1024];	/* ANSILINELEN * 4 */


#ifdef HAVE_HYPERLINK
static uschar *linkEnd = NULL;

static void
ansi_hyperlink(fpw, src)
  FILE *fpw;
  uschar *src;
{
  int ch;

  linkEnd = src;
  fputs("<a class=PRE target=_blank href=", fpw);
  while (ch = *linkEnd)
  {
    if (ch < '#' || ch == '<' || ch == '>' || ch > '~')
      break;
    fputc(ch, fpw);
    linkEnd++;
  }
  fputc('>', fpw);
}
#endif


#ifdef HAVE_SAKURA
static int
sakura2unicode(code)
  int code;
{
  if (code > 0xC6DD && code < 0xC7F3)
  {
    if (code > 0xC7A0)
      code -= 38665;
    else if (code > 0xC700)
      code -= 38631;
    else if (code > 0xC6E6)
      code -= 38566;
    else if (code == 0xC6E3)
      return 0x30FC;
    else
      code -= 38619;
    if (code > 0x3093)
      code += 13;
    return code;
  }
  return 0;
}
#endif


static int
ansi_remove(psrc)
  uschar **psrc;
{
  uschar *src = *psrc;
  int ch = *src;

  while (is_ansi(ch))
    ch = *(++src);

  if (ch && ch != '\n')
    ch = *(++src);

  *psrc = src;
  return ch;
}


static int
ansi_color(psrc)
  uschar **psrc;
{
  uschar *src, *ptr;
  int ch, value;
  int color = old_color;

#ifdef HAVE_ANSIATTR
  int attr = old_attr;
#endif
  uschar *cptr = (uschar *) & color;

  src = ptr = (*psrc) + 1;

  ch = *src;
  while (ch)
  {
    if (ch == ';' || ch == 'm')
    {
      *src = '\0';
      value = atoi(ptr);
      ptr = src + 1;
      if (value == 0)
      {
	color = 0x00003740;

#ifdef HAVE_ANSIATTR
	attr = 0;
#endif
      }
      else if (value >= 30 && value <= 37)
	cptr[1] = value + 18;
      else if (value >= 40 && value <= 47)
	cptr[0] = value + 24;
      else if (value == 1)
	cptr[2] = 1;

#ifdef HAVE_ANSIATTR
      else if (value == 4)
	attr |= ATTR_UNDER;
      else if (value == 5)
	attr |= ATTR_BLINK;
      else if (value == 7)	/* �ϥժ��ĪG�α���ӥN�� */
	attr |= ATTR_ITALIC;
#endif

      if (ch == 'm')
      {
	now_color = color;

#ifdef HAVE_ANSIATTR
	now_attr = attr;
#endif

	ch = *(++src);
	break;
      }
    }
    else if (ch < '0' || ch > '9')
    {
      ch = *(++src);
      break;
    }
    ch = *(++src);
  }

  *psrc = src;
  return ch;
}


static void
ansi_tag(fpw)
  FILE *fpw;
{
#ifdef HAVE_ANSIATTR
  /* �ݩʤ��P�~�ݭn�L�X */
  if (!(now_attr & ATTR_ITALIC) && (old_attr & ATTR_ITALIC))
  {
    fputs("</I>", fpw);
  }
  if (!(now_attr & ATTR_UNDER) && (old_attr & ATTR_UNDER))
  {
    fputs("</U>", fpw);
  }
  if (!(now_attr & ATTR_BLINK) && (old_attr & ATTR_BLINK))
  {
    fputs("</BLINK>", fpw);
  }
#endif

  /* �C�⤣�P�~�ݭn�L�X */
  if (old_color != now_color)
  {
    fprintf(fpw, "</font><font class=A%05X>", now_color);
    old_color = now_color;
  }

#ifdef HAVE_ANSIATTR
  /* �ݩʤ��P�~�ݭn�L�X */
  if (oldattr != attr)
  {
    if ((now_attr & ATTR_ITALIC) && !(old_attr & ATTR_ITALIC))
    {
      fputs("<I>", fpw);
    }
    if ((now_attr & ATTR_UNDER) && !(old_attr & ATTR_UNDER))
    {
      fputs("<U>", fpw);
    }
    if ((now_attr & ATTR_BLINK) && !(old_attr & ATTR_BLINK))
    {
      fputs("<BLINK>", fpw);
    }
    old_attr = now_attr;
  }
#endif
}


static void
ansi_html(fpw, src)
  FILE *fpw;
  uschar *src;
{
  int ch1, ch2;
  int has_ansi = 0;

#ifdef HAVE_SAKURA
  int scode;
#endif

  ch2 = *src;
  while (ch2)
  {
    ch1 = ch2;
    ch2 = *(++src);
    if (ch1 & 0x80)
    {
      while (ch2 == ANSI_TAG)
      {
	if (*(++src) == '[')	/* �C�� */
	{
	  ch2 = ansi_color(&src);
	  has_ansi = 1;
	}
	else			/* ��L�����R�� */
	  ch2 = ansi_remove(&src);
      }
      if (ch2)
      {
	if (ch2 < ' ')		/* �ȥX�{\n */
	  fputc(ch2, fpw);

#ifdef HAVE_SAKURA
	else if (scode = sakura2unicode((ch1 << 8) | ch2))
	  fprintf(fpw, "&#%d;", scode);
#endif

	else
	{
	  fputc(ch1, fpw);
	  fputc(ch2, fpw);
	}
	ch2 = *(++src);
      }
      if (has_ansi)
      {
	has_ansi = 0;
	if (ch2 != ANSI_TAG)
	  ansi_tag(fpw);
      }
      continue;
    }
    else if (ch1 == ANSI_TAG)
    {
      do
      {
	if (ch2 == '[')		/* �C�� */
	{
	  ch2 = ansi_color(&src);
	}
	else if (ch2 == '*')	/* ����X */
	{
	  fputc('*', fpw);
	}
	else			/* ��L�����R�� */
	  ch2 = ansi_remove(&src);
      } while (ch2 == ANSI_TAG && (ch2 = *(++src)));
      ansi_tag(fpw);
      continue;
    }
    /* �ѤU���r����html�ഫ */
    if (ch1 == '<')
    {
      fputs("&lt;", fpw);
    }
    else if (ch1 == '>')
    {
      fputs("&gt;", fpw);
    }
    else if (ch1 == '&')
    {
      fputc(ch1, fpw);
      if (ch2 == '#')		/* Unicode�r�����ഫ */
      {
	fputc(ch2, fpw);
	ch2 = *(++src);
      }
      else if (ch2 >= 'A' && ch2 <= 'z')
      {
	fputs("amp;", fpw);
	fputc(ch2, fpw);
	ch2 = *(++src);
      }
    }

#ifdef HAVE_HYPERLINK
    /* �B�z�W�s�� */
    else if (linkEnd)
    {
      fputc(ch1, fpw);
      if (linkEnd <= src)
      {
	fputs("</a>", fpw);
	linkEnd = NULL;
      }
    }
    else
    {
      /* ��L���ۤv�[�a :) */
      if (!str_ncmp(src - 1, "http://", 7))
	ansi_hyperlink(fpw, src - 1);
      else if (!str_ncmp(src - 1, "telnet://", 9))
	ansi_hyperlink(fpw, src - 1);

      fputc(ch1, fpw);
    }
#else
    else
      fputc(ch1, fpw);
#endif
  }
}


static char *
str_html(src, len)
  uschar *src;
  int len;
{
  int in_chi, ch;
  uschar *dst = ansi_buf, *end = src + len;

  ch = *src;
  while (ch && src < end)
  {
    if (ch & 0x80)
    {
      in_chi = *(++src);
      while (in_chi == ANSI_TAG)
      {
	++src;
	in_chi = ansi_remove(&src);
      }

      if (in_chi)
      {
	if (in_chi < ' ')	/* �i��u���b�Ӧr�A�e�b���N���n�F */
	  *dst++ = in_chi;

#ifdef HAVE_SAKURA
	else if (len = sakura2unicode((ch << 8) + in_chi))
	{
	  sprintf(dst, "&#%d;", len);	/* 12291~12540 */
	  dst += 8;
	}
#endif

	else
	{
	  *dst++ = ch;
	  *dst++ = in_chi;
	}
      }
      else
	break;
    }
    else if (ch == ANSI_TAG)
    {
      ++src;
      ch = ansi_remove(&src);
      continue;
    }
    else if (ch == '<')
    {
      strcpy(dst, "&lt;");
      dst += 4;
    }
    else if (ch == '>')
    {
      strcpy(dst, "&gt;");
      dst += 4;
    }
    else if (ch == '&')
    {
      ch = *(++src);
      if (ch == '#')
      {
	if ((uschar *) strchr(src + 1, ';') >= end)	/* �i��|���O�Ϊ��רS�W�L */
	  break;
	*dst++ = '&';
	*dst++ = '#';
      }
      else
      {
	strcpy(dst, "&amp;");
	dst += 5;
	continue;
      }
    }
    else
      *dst++ = ch;
    ch = *(++src);
  }

  *dst = '\0';
  return ansi_buf;
}


static int
ansi_quote(fpw, src)		/* �p�G�O�ި��A�N���L�Ҧ��� ANSI �X */
  FILE *fpw;
  uschar *src;
{
  int ch1, ch2;

  ch1 = src[0];
  ch2 = src[1];
  if (ch2 == ' ' && (ch1 == QUOTE_CHAR1 || ch1 == QUOTE_CHAR2))	/* �ި� */
  {
    ch2 = src[2];
    if (ch2 == QUOTE_CHAR1 || ch2 == QUOTE_CHAR2)	/* �ޥΤ@�h/�G�h���P�C�� */
      now_color = 0x00003340;
    else
      now_color = 0x00003640;
  }
  else if (ch1 == 0xA1 && ch2 == 0xB0)	/* �� �ި��� */
  {
    now_color = 0x00013640;
  }
  else
  {
    ansi_tag(fpw);
    return 0;			/* ���O�ި� */
  }

  ansi_tag(fpw);
  fputs(str_html(src, ANSILINELEN), fpw);
  now_color = 0x00003740;
  return 1;
}


#define LINE_HEADER	3	/* ���Y���T�� */

static void
txt2htm(fpw, fp)
  FILE *fpw;
  FILE *fp;
{
  static const char header1[LINE_HEADER][LEN_AUTHOR1] = {"�@��", "���D", "�ɶ�"};
  static const char header2[LINE_HEADER][LEN_AUTHOR2] = {"�o�H�H", "��  �D", "�o�H��"};
  int i;
  char *headvalue, *pbrd, *board;
  char buf[ANSILINELEN];

  fputs("<table width=760 cellspacing=0 cellpadding=0 border=0>\n", fpw);
  /* �B�z���Y */
  for (i = 0; i < LINE_HEADER; i++)
  {
    if (!fgets(buf, ANSILINELEN, fp))	/* ���M�s���Y���٨S�L���A���O�ɮפw�g�����A
					 * �������} */
    {
      fputs("</table>\n", fpw);
      return;
    }

    if (memcmp(buf, header1[i], LEN_AUTHOR1 - 1) && memcmp(buf, header2[i], LEN_AUTHOR2 - 1))	/* ���O���Y */
      break;

    /* �@��/�ݪO ���Y���G��A�S�O�B�z */
    if (i == 0 && (
	(pbrd = strstr(buf, "�ݪO:")) ||
	(pbrd = strstr(buf, "����:"))))
    {
      if (board = strchr(pbrd, '\n'))
	*board = '\0';
      board = pbrd + 6;
      pbrd[-1] = '\0';
      pbrd[4] = '\0';
    }

    if (!(headvalue = strchr(buf, ':')))
      break;

    fprintf(fpw, "<tr>\n"
      "  <td align=center width=10%% class=A03447>%s</td>\n", header1[i]);

    str_html(headvalue + 2, TTLEN);
    if (i == 0 && pbrd)
    {
      fprintf(fpw, "  <td width=60%% class=A03744>&nbsp;%s</td>\n"
	"  <td align=center width=10%% class=A03447>%s</td>\n"
	"  <td width=20%% class=A03744>&nbsp;%s</td>\n</tr>\n",
	ansi_buf, pbrd, board);
    }
    else
    {
      fputs("  <td width=90% colspan=3 class=A03744>&nbsp;", fpw);
      fputs(ansi_buf, fpw);
      fputs("</td>\n</tr>\n", fpw);
    }
  }

  fputs("<tr>\n"
    "<td colspan=4><pre><font class=A03740>", fpw);

  old_color = now_color = 0x00003740;

#ifdef HAVE_ANSIATTR
  old_attr = now_attr = 0;
#endif

  if (i >= LINE_HEADER)		/* �̫�@��O���Y */
    fgets(buf, ANSILINELEN, fp);

  /* �B�z���� */
  do
  {
    if (!ansi_quote(fpw, buf))
      ansi_html(fpw, buf);
  } while (fgets(buf, ANSILINELEN, fp));

  fputs("</font></pre></td>\n</table>\n", fpw);
}


/* ----------------------------------------------------- */
/* HTML output basic function				 */
/* ----------------------------------------------------- */

static char *
Gtime(now)
  time_t *now;
{
  static char datemsg[32];

  strftime(datemsg, sizeof(datemsg), "%a, %d %b %Y %T GMT", gmtime(now));
  return datemsg;
}


static FILE *
out_http(ap, code, type)
  Agent *ap;
  int code;
  char *type;
{
  time_t now;
  FILE *fpw;
  int state;

  fpw = ap->fpw;
  state = code & 0xFF;

  /* HTTP 1.0 ���Y */
  time(&now);

  fprintf(fpw, "HTTP/1.0 %s\r\n"
    "Date: %s\r\n"
    "Server: MapleBBS 3.10\r\n"
    "Connection: close\r\n", http_msg[state], Gtime(&now));

  if (state == HS_NOTMOIDIFY)
  {
    fputs("\r\n", fpw);
  }
  else if (state == HS_REDIRECT)/* Location���ᤣ�ݭn���e */
  {
    if (!type)
      type = "";

#if BHTTP_PORT == 80
    fprintf(fpw, "Location: http://" MYHOSTNAME "/%s\r\n\r\n", type);
#else
    fprintf(fpw, "Location: http://" MYHOSTNAME ":%d/%s\r\n\r\n", BHTTP_PORT, type);
#endif
  }
  else
  {
    if (code & HS_REFRESH)
    {
      if (!type)
	type = "/";
      fprintf(fpw, "Refresh: 3; url=%s\r\n", type);
    }
    if ((code & HS_REFRESH) || !type)
    {
      fputs("Pragma: no-cache\r\n"	/* �����@�ߤ���proxy��cache */
	"Content-Type: text/html; charset=" MYCHARSET "\r\n", fpw);
    }
    else
      fprintf(fpw, "Content-Type: %s\r\n", type);

    if (ap->setcookie)		/* acct_login() ���H��~�ݭn Set-Cookie */
      fprintf(fpw, "Set-Cookie: user=%s; path=/\r\n", ap->cookie);
  }

  return fpw;
}


static void
out_error(ap, code)		/* code���i�H�OHS_OK */
  Agent *ap;
  int code;
{
  char *msg;

  if (code < HS_OK)
  {
    fprintf(ap->fpw, "<BR>%s<BR><BR>\n", http_msg[code]);
    return;
  }

  out_http(ap, code, NULL);
  switch (code)
  {
  case HS_BADREQUEST:
    msg = "Your browser sent a request that this server could not understand.";
    break;
  case HS_FORBIDDEN:
    msg = "You don't have permission to access the URL on this server.";
    break;
  case HS_NOTFOUND:
    msg = "The requested URL was not found on this server.";
    break;
  default:			/* HS_REDIRECT, HS_NOTMOIDIFY */
    return;
  }
  /* html �ɮ׶}�l */
  fprintf(ap->fpw, "\r\n<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n"
    "<HTML><HEAD>\n"
    "<TITLE>%s</TITLE>\n"
    "</HEAD><BODY>\n"
    "<H1>%s</H1>\n%s\n<HR>\n"
    "<ADDRESS>MapleBBS/3.10 Server at " MYHOSTNAME "</ADDRESS>\n"
    "</BODY></HTML>\n", http_msg[code], http_msg[code] + 4, msg);
}


/* out_head() ���� <HTML> <BODY> <CENTER> �T�Ӥj�g���ҳe���� html ��
   ���� out_tail() �~�� </HTML> </BODY> </CENTER> �٭� */


static void
out_title(fpw, title)
  FILE *fpw;
  char *title;
{
  /* html �ɮ׶}�l */
  fprintf(fpw, "\r\n<HTML><HEAD>\n"
    "<meta http-equiv=Content-Type content=\"text/html; charset=" MYCHARSET "\">\n"
    "<title>-=" BBSNAME "=- %s</title>\n", title);

  fputs("<script language=javascript>\n"
    "  function mOver(obj) {obj.bgColor='" HCOLOR_BAR "';}\n"
    "  function mOut(obj) {obj.bgColor='" HCOLOR_BG "';}\n"
    "</script>\n<style type=text/css>\n"
    "  PRE {font-size: 15pt; line-height: 15pt; font-weight: lighter; background-color: #000000; color: #C0C0C0;}\n"
    "  TD  {font-size: 15pt; line-height: 15pt; font-weight: lighter;}\n"
    "</style>\n"
    "<link rel=stylesheet href=/img/ansi.css type=text/css>\n"
    "</head>\n"
    "<BODY bgcolor=" HCOLOR_BG " text=" HCOLOR_TEXT " link=" HCOLOR_LINK " vlink=" HCOLOR_VLINK " alink=" HCOLOR_ALINK "><CENTER>\n"
    "<a href=/><img src=/img/site.gif border=0></a><br>\n"
    "<input type=image src=/img/back.gif onclick=\"javascript:history.go(-1);\"> / "
    "<a href=/class><img src=/img/class.gif border=0></a> / "
    "<a href=/brdlist><img src=/img/board.gif border=0></a> / "
    "<a href=/fvrlist><img src=/img/favor.gif border=0></a> / "
    "<a href=/mbox><img src=/img/mbox.gif border=0></a> / "
    "<a href=/usrlist><img src=/img/user.gif border=0></a> / "
    "<a href=telnet://" MYHOSTNAME "><img src=/img/telnet.gif border=0></a><br>\n", fpw);
}


static FILE *
out_head(ap, title)
  Agent *ap;
  char *title;
{
  FILE *fpw = out_http(ap, HS_OK, NULL);
  out_title(fpw, title);
  return fpw;
}


static void
out_mesg(fpw, msg)
  FILE *fpw;
  char *msg;
{
  fprintf(fpw, "<BR>%s<BR><BR>\n", msg);
}


static void
out_style(fpw)
  FILE *fpw;
{

#ifdef HAVE_HYPERLINK
  fputs("<style type=text/css>\n"
    "  a:link.PRE    {COLOR: #FFFFFF}\n"
    "  a:visited.PRE {COLOR: #FFFFFF}\n"
    "  a:active.PRE {COLOR: " HCOLOR_ALINK "; TEXT-DECORATION: none}\n"
    "  a:hover.PRE  {COLOR: " HCOLOR_ALINK "; TEXT-DECORATION: none}\n"
    "</style>\n", fpw);
#endif
}


static void
out_article(fpw, fpath)
  FILE *fpw;
  char *fpath;
{
  FILE *fp;

  if (fp = fopen(fpath, "r"))
  {
    out_style(fpw);
    txt2htm(fpw, fp);
    fclose(fp);
  }
}


static void
out_tail(fpw)
  FILE *fpw;
{
  fputs("</CENTER></BODY></HTML>\n", fpw);
}


static void
out_reload(fpw, msg)		/* God.050327: �N�D���� reload �������s�}���� */
  FILE *fpw;
  char *msg;
{
  fprintf(fpw, "</CENTER></BODY></HTML>\n"
    "<script>alert(\'%s\');\n"
    "opener.location.reload();\n"
    "parent.close();</script>", msg);
}


/* ----------------------------------------------------- */
/* �ѽX���R�Ѽ�						 */
/* ----------------------------------------------------- */

#define hex2int(x)	((x >= 'A') ? (x - 'A' + 10) : (x - '0'))

static int			/* 1:���\ */
arg_analyze(argc, str, arg1, arg2, arg3, arg4)
  int argc;			/* ���X�ӰѼ� */
  char *str;			/* �޼� */
  char **arg1;			/* �ѼƤ@ */
  char **arg2;			/* �ѼƤG */
  char **arg3;			/* �ѼƤT */
  char **arg4;			/* �Ѽƥ| */
{
  int i, ch;
  char *dst;

  if (!(ch = *str))
  {
    *arg1 = NULL;
    return 0;
  }

  *arg1 = dst = str;
  i = 2;

  while (ch)
  {
    if (ch == '&' || ch == '\r')
    {
      if (i > argc)
	break;

      *dst++ = '\0';
      if (i == 2)
	*arg2 = dst;
      else if (i == 3)
	*arg3 = dst;
      else /* if (i == 4) */
	*arg4 = dst;
      i++;
    }
    else if (ch == '+')
    {
      *dst++ = ' ';
    }
    else if (ch == '%')
    {
      ch = *(++str);
      if (isxdigit(ch) && isxdigit(str[1]))
      {
	ch = (hex2int(ch) << 4) + hex2int(str[1]);
	++str;
	if (ch != '\r')		/* '\r' �N���n�F */
	  *dst++ = ch;
      }
      else
      {
	*dst++ = '%';
	continue;
      }
    }
    else
      *dst++ = ch;

    ch = *(++str);
  }
  *dst = '\0';

  return i > argc;
}


/* ----------------------------------------------------- */
/* �� Cookie �ݨϥΪ̬O�_�n�J				 */
/* ----------------------------------------------------- */

static int			/* 1:�n�J���\ 0:�n�J���� */
acct_fetch(ap)
  Agent *ap;
{
  char *userid, *passwd;
  char fpath[64];
  ACCT acct;

  if (ap->cookie[0])
  {
    /* u=userid&p=passwd */
    if (!arg_analyze(2, ap->cookie, &userid, &passwd, NULL, NULL))
      return 0;

    passwd += 2;		/* skip "p=" */

    if (*userid && *passwd && strlen(userid) <= IDLEN && strlen(passwd) == PASSLEN)
    {
      usr_fpath(fpath, userid, FN_ACCT);
      if (!rec_get(fpath, &acct, sizeof(ACCT), 0) &&
	!(acct.userlevel & (PERM_DENYLOGIN | PERM_PURGE)) &&
	!strncmp(acct.passwd, passwd, PASSLEN))	/* �n�J���\ */
      {
	ap->userno = acct.userno;
	strcpy(ap->userid, acct.userid);
	strcpy(ap->username, acct.username);
	ap->userlevel = acct.userlevel;
	return 1;
      }
    }
  }

  /* �S���n�J�B�n�J���� */
  ap->userno = 3;		/* ��J���guest��userno�A�H�K��pal�ˬd */
  ap->userlevel = 0;
  strcpy(ap->userid, STR_GUEST);
  strcpy(ap->username, STR_GUEST);
  return 0;
}


/* ----------------------------------------------------- */
/* UTMP shm �������P cache.c �ۮe			 */
/* ----------------------------------------------------- */

static UCACHE *ushm;

static void
init_ushm()
{
  ushm = shm_new(UTMPSHM_KEY, sizeof(UCACHE));
}


static int			/* 1: userno �b pool �W��W */
pertain_pal(pool, max, userno)	/* �Ѧ� pal.c:belong_pal() */
  int *pool;
  int max;
  int userno;
{
  int datum, mid;

  while (max > 0)
  {
    datum = pool[mid = max >> 1];
    if (userno == datum)
    {
      return 1;
    }
    if (userno > datum)
    {
      pool += (++mid);
      max -= mid;
    }
    else
    {
      max = mid;
    }
  }
  return 0;
}


static int			/* 1: ���]�ڬ��a�H */
is_hisbad(up, userno)		/* �Ѧ� pal.c:is_obad() */
  UTMP *up;
  int userno;
{

#ifdef HAVE_BADPAL
  return pertain_pal(up->pal_spool, up->pal_max, -userno);
#else
  return 0;
#endif
}


static int			/* 1:�i�ݨ� 0:���i�ݨ� */
can_seen(up, userno, ulevel)	/* �Ѧ� bmw.c:can_see() */
  UTMP *up;
  int userno;
  usint ulevel;
{
  usint urufo;

  urufo = up->ufo;

  if (!(ulevel & PERM_SEECLOAK) && ((urufo & UFO_CLOAK) || is_hisbad(up, userno)))
    return 0;

#ifdef HAVE_SUPERCLOAK
  if (urufo & UFO_SUPERCLOAK)
    return 0;
#endif

  return 1;
}


/* itoc.030711: �[�W�ˬd�ϥΪ̱b���������A�H�K���H�ÿ� */
static int
allow_userid(ap, userid)
  Agent *ap;
  char *userid;
{
  int ch, rc;
  char *str;

  rc = 0;
  ch = strlen(userid);
  if (ch >= 2 && ch <= IDLEN && is_alpha(*userid))
  {
    rc = 1;
    str = userid;
    while (ch = *(++str))
    {
      if (!is_alnum(ch))
      {
	rc = 0;
	break;
      }
    }
  }

  return rc;
}


/* ----------------------------------------------------- */
/* board�Gshm �������P cache.c �ۮe			 */
/* ----------------------------------------------------- */

static BCACHE *bshm;

static void
init_bshm()
{
  /* itoc.030727: �b�}�� bbsd ���e�A���ӴN�n����L account�A
     �ҥH bshm ���Ӥw�]�w�n */

  if (bshm)
    return;

  bshm = shm_new(BRDSHM_KEY, sizeof(BCACHE));

  if (bshm->uptime <= 0)	/* bshm ���]�w���� */
    exit(0);
}


#ifdef HAVE_MODERATED_BOARD
static int			/* !=0:�O�O�n  0:���b�W�椤 */
is_brdgood(userno, bpal)	/* �Ѧ� pal.c:is_bgood() */
  int userno;
  BPAL *bpal;
{
  return pertain_pal(bpal->pal_spool, bpal->pal_max, userno);
}


static int			/* !=0:�O�O�a  0:���b�W�椤 */
is_brdbad(userno, bpal)		/* �Ѧ� pal.c:is_bbad() */
  int userno;
  BPAL *bpal;
{

#ifdef HAVE_BADPAL
  return pertain_pal(bpal->pal_spool, bpal->pal_max, -userno);
#else
  return 0;
#endif
}
#endif


static int
Ben_Perm(brd, uno, uid, ulevel)	/* �Ѧ� board.c:Ben_Perm() */
  BRD *brd;
  int uno;
  char *uid;
  usint ulevel;
{
  usint readlevel, postlevel, bits;
  char *blist, *bname;

#ifdef HAVE_MODERATED_BOARD
  BPAL *bpal;
  int ftype;			/* 0:�@��ID 1:�O�n 2:�O�a */

  /* itoc.040103: �ݪO�\Ū���Ż�����

  �z�w�w�w�w�s�w�w�w�w�s�w�w�w�w�s�w�w�w�w�{
  �x        �x�@��Τ�x�ݪO�n�͢x�ݪO�a�H�x
  �u�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�t
  �x�@��ݪO�x�v���M�w�x  ����  �x �ݤ��� �x    �ݤ����G�b�ݪO�C���L�k�ݨ�o�ӪO�A�]�i���h
  �u�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�t    �i���h�G�b�ݪO�C���i�H�ݨ�o�ӪO�A���O�i���h
  �x�n�ͬݪO�x �i���h �x  ����  �x  ����  �x    ��  ��G�b�ݪO�C���i�H�ݨ�o�ӪO�A�]�i�o�h�A���O����o��
  �u�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�t    ��  ��G�b�ݪO�C���i�H�ݨ�o�ӪO�A�]�i�o�h�εo��
  �x���K�ݪO�x �ݤ��� �x  ����  �x  ����  �x
  �|�w�w�w�w�r�w�w�w�w�r�w�w�w�w�r�w�w�w�w�}
  */

  static int bit_data[9] =
  {                /* �@��Τ�   �ݪO�n��                           �ݪO�a�H */
    /* ���}�ݪO */    0,         BRD_L_BIT | BRD_R_BIT,             0,
    /* �n�ͬݪO */    BRD_L_BIT, BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT,
    /* ���K�ݪO */    0,         BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT,
  };
#endif

  if (!brd)
    return 0;
  bname = brd->brdname;
  if (!*bname)
    return 0;

  readlevel = brd->readlevel;

#ifdef HAVE_MODERATED_BOARD
  bpal = bshm->pcache + (brd - bshm->bcache);
  ftype = is_brdgood(uno, bpal) ? 1 : is_brdbad(uno, bpal) ? 2 : 0;

  if (readlevel == PERM_SYSOP)		/* ���K�ݪO */
    bits = bit_data[6 + ftype];
  else if (readlevel == PERM_BOARD)	/* �n�ͬݪO */
    bits = bit_data[3 + ftype];
  else if (ftype)			/* ���}�ݪO�A�Y�b�O�n/�O�a�W�椤 */
    bits = bit_data[ftype];
  else					/* ���}�ݪO�A��L���v���P�w */
#endif

  if (!readlevel || (readlevel & ulevel))
  {
    bits = BRD_L_BIT | BRD_R_BIT;

    postlevel = brd->postlevel;
    if (!postlevel || (postlevel & ulevel))
      bits |= BRD_W_BIT;
  }
  else
  {
    bits = 0;
  }

  /* Thor.980813.����: �S�O�� BM �Ҷq�A�O�D���ӪO���Ҧ��v�� */
  blist = brd->BM;
  if ((ulevel & PERM_BM) && blist[0] > ' ' && str_has(blist, uid, strlen(uid)))
    bits = BRD_L_BIT | BRD_R_BIT | BRD_W_BIT | BRD_X_BIT | BRD_M_BIT;

  /* itoc.030515: �ݪO�`�ޭ��s�P�_ */
  else if (ulevel & PERM_ALLBOARD)
    bits = BRD_L_BIT | BRD_R_BIT | BRD_W_BIT | BRD_X_BIT | BRD_M_BIT;

  return bits;
}


static BRD *
brd_get(bname)
  char *bname;
{
  BRD *bhdr, *tail;

  bhdr = bshm->bcache;
  tail = bhdr + bshm->number;
  do
  {
    if (!str_cmp(bname, bhdr->brdname))
      return bhdr;
  } while (++bhdr < tail);
  return NULL;
}


static int		/* �^��PermBits��K���h���P�_ */
ben_perm(ap, brdname)
  Agent *ap;
  char *brdname;
{
  if (acct_fetch(ap))
    return Ben_Perm(brd_get(brdname), ap->userno, ap->userid, ap->userlevel);
  return 0;
}


static BRD *
allow_brdname(ap, brdname)
  Agent *ap;
  char *brdname;
{
  BRD *bhdr;

  if (bhdr = brd_get(brdname))
  {
    /* �Y readlevel == 0�A��� guest �iŪ�A�L�� acct_fetch() */
    if (!bhdr->readlevel)
      return bhdr;

    if (acct_fetch(ap) && (Ben_Perm(bhdr, ap->userno, ap->userid, ap->userlevel) & BRD_R_BIT))
      return bhdr;
  }
  return NULL;
}


/* ----------------------------------------------------- */
/* movie�Gshm �������P cache.c �ۮe			 */
/* ----------------------------------------------------- */

static FCACHE *fshm;


static void
init_fshm()
{
  fshm = shm_new(FILMSHM_KEY, sizeof(FCACHE));
}


static void
out_film(fpw, tag)
  FILE *fpw;
  int tag;
{
  int i, *shot, len;
  char *film, buf[FILM_SIZ];

  shot = fshm->shot;
  for (i = 0; !(*shot) && i < 5; ++i)
    sleep(1);

  if (!(*shot))
    return;		/* �Y 5 ��H���٨S���n���A�i��O�S�] camera�A�������} */

  film = fshm->film;
  if (tag)
  {
    len = shot[tag];
    film += len;
    len = shot[tag + 1] - len;
  }
  else
    len = shot[1];

  memcpy(buf, film, len);
  buf[len] = '\0';

  out_style(fpw);
  fputs("<table width=760 cellspacing=0 cellpadding=0 border=0>\n"
    "<tr>\n"
    "<td colspan=4><pre><font class=A03740>", fpw);
  ansi_html(fpw, buf);
  fputs("</font></pre></td>\n</table>\n", fpw);
}


/* ----------------------------------------------------- */
/* command dispatch (GET)				 */
/* ----------------------------------------------------- */

  /* --------------------------------------------------- */
  /* �q�βM��						 */
  /* --------------------------------------------------- */

static void
list_neck(fpw, start, total, title)
  FILE *fpw;
  int start, total;
  char *title;
{
  fputs("<br>\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=760>\n"
    "<tr bgcolor=" HCOLOR_NECK ">\n  <td width=15%", fpw);
  if (start != 1)
  {
    fprintf(fpw, " align=center><a href=?%d>�W%d��</a",
      (start > HTML_TALL ? start - HTML_TALL : 1), HTML_TALL);
  }
  fputs("></td>\n  <td width=15%", fpw);

  start += HTML_TALL;
  if (start <= total)
  {
    fprintf(fpw, " align=center><a href=?%d>�U%d��</a",
      start, HTML_TALL);
  }
  fputs("></td>\n  <td width=40% align=center>", fpw);
  fprintf(fpw, title, total);
  fprintf(fpw, "</td>\n"
    "  <td width=15%% align=center><a href=?1>�e%d��</a></td>\n"
    "  <td width=15%% align=center><a href=?0>��%d��</a></td>\n"
    "</tr></table><br>\n", HTML_TALL, HTML_TALL);
}


static void
cmdlist_list(ap, title, list_tie, list_item)
  Agent *ap;
  char *title;
  void (*list_tie) (FILE *);
  void (*list_item) (FILE *, void *, int);
{
  int i, start, end, total;
  char *number;
  FILE *fpw;

  if (!arg_analyze(1, ap->urlp, &number, NULL, NULL, NULL))
    start = 1;
  else
    start = atoi(number);
  total = ap->total_item;
  if (start <= 0 || start > total)	/* �W�L�d�򪺸ܡA������̫�@�� */
    start = total > HTML_TALL ? total - HTML_TALL + 1 : 1;

  fpw = ap->fpw;
  list_neck(fpw, start, total, title);
  fputs("<table cellspacing=0 cellpadding=4 border=0>\n<tr bgcolor=" HCOLOR_TIE ">\n", fpw);
  list_tie(fpw);

  end = start + HTML_TALL - 1;
  if (end > total)
    end = total;
  for (i = start - 1; i < end; i++)
  {
    fputs("<tr onmouseover=mOver(this); onmouseout=mOut(this);>\n", fpw);
    list_item(fpw, ap->myitem[i], i + 1);
  }

  fputs("</table>\n", fpw);
  list_neck(fpw, start, total, title);
}


  /* --------------------------------------------------- */
  /* �ϥΪ̦W��						 */
  /* --------------------------------------------------- */

static int
userid_cmp(a, b)
  UTMP **a, **b;
{
  return str_cmp((*a)->userid, (*b)->userid);
}


static void
init_myusr(ap)
  Agent *ap;
{
  int num, userno;
  UTMP *uentp, *uceil;
  usint ulevel;

  acct_fetch(ap);
  uentp = ushm->uslot;
  uceil = (void *)uentp + ushm->offset;
  userno = ap->userno;
  ulevel = ap->userlevel;
  num = 0;

  do
  {
    if (!uentp->pid || !uentp->userno || !can_seen(uentp, userno, ulevel))
      continue;

    ap->myitem[num] = uentp;
    num++;
  } while (++uentp <= uceil);

  if (num > 1)
    qsort(ap->myitem, num, sizeof(UTMP *), userid_cmp);

  ap->total_item = num;
}


static void
userlist_tie(fpw)
  FILE *fpw;
{
  fputs("  <td width=40>�s��</td>\n"
    "  <td width=100>���ͥN��</td>\n"
    "  <td width=210>���ͼʺ�</td>\n"
    "  <td width=230>�ȳ~�G�m</td>\n"
    "  <td width=100>���ͰʺA</td>\n"
    "</tr>\n", fpw);
}


static void
userlist_item(fpw, up, n)
  FILE *fpw;
  UTMP *up;
  int n;
{
  fprintf(fpw, "  <td>%d</td>\n"
    "  <td><a href=/query?%s>%s</a></td>\n"
    "  <td>%s</td>\n"
    "  <td>%s</td>\n"
    "  <td>%s</td>\n"
    "</tr>\n",
    n,
    up->userid, up->userid,
    str_html(up->username, UNLEN),
    up->from,
    ModeTypeTable[up->mode]);
}


static int
cmd_userlist(ap)
  Agent *ap;
{
  init_myusr(ap);
  out_head(ap, "�ϥΪ̦W��");

  cmdlist_list(ap, "�ثe���W�� %d �ӤH", userlist_tie, userlist_item);

  return HS_END;
}


  /* --------------------------------------------------- */
  /* �ݪO�C��						 */
  /* --------------------------------------------------- */

static int
brdtitle_cmp(a, b)
  BRD **a, **b;
{
  /* itoc.010413: ����/�O�W��e��� */
  int k = strcmp((*a)->class, (*b)->class);
  return k ? k : str_cmp((*a)->brdname, (*b)->brdname);
}


static void
init_mybrd(ap)
  Agent *ap;
{
  int num, uno;
  char *uid;
  usint ulevel;
  BRD *bhdr, *tail;

  acct_fetch(ap);
  uno = ap->userno;
  uid = ap->userid;
  ulevel = ap->userlevel;
  bhdr = bshm->bcache;
  tail = bhdr + bshm->number;
  num = 0;

  do
  {
    if (Ben_Perm(bhdr, uno, uid, ulevel) & BRD_R_BIT)
    {
      ap->myitem[num] = bhdr;
      num++;
    }
  } while (++bhdr < tail);

  if (num > 1)
    qsort(ap->myitem, num, sizeof(BRD *), brdtitle_cmp);

  ap->total_item = num;
}


static void
boardlist_tie(fpw)
  FILE *fpw;
{
  fputs("  <td width=40>�s��</td>\n"
    "  <td width=80>�ݪO</td>\n"
    "  <td width=40>���O</td>\n"
    "  <td width=25>��</td>\n"
    "  <td width=350>����ԭz</td>\n"
    "  <td width=75>�O�D</td>\n"
    "</tr>\n", fpw);
}


static void
boardlist_item(fpw, brd, n)
  FILE *fpw;
  BRD *brd;
  int n;
{
  fprintf(fpw, "  <td>%d</td>\n"
    "  <td><a href=/brd?%s>%s</a></td>\n"
    "  <td>%s</td>\n"
    "  <td>%s</td>\n"
    "  <td>%s</td>\n"
    "  <td>%.13s</td>\n"
    "</tr>\n",
    n,
    brd->brdname, brd->brdname,
    brd->class,
    (brd->battr & BRD_NOTRAN) ? ICON_NOTRAN_BRD : ICON_TRAN_BRD,
    str_html(brd->title, 33),
    brd->BM);
}


static int
cmd_boardlist(ap)
  Agent *ap;
{
  init_mybrd(ap);
  out_head(ap, "�ݪO�C��");

  cmdlist_list(ap, "�ثe���W�� %d �ӪO", boardlist_tie, boardlist_item);

  return HS_END;
}


  /* --------------------------------------------------- */
  /* �ڪ��̷R						 */
  /* --------------------------------------------------- */

static void
init_myfavor(ap)
  Agent *ap;
{
  int num, uno;
  usint ulevel;
  char *uid, fpath[64];
  BRD *bhdr;
  FILE *fp;
  MF mf;

  uno = ap->userno;
  uid = ap->userid;
  ulevel = ap->userlevel;
  num = 0;
  usr_fpath(fpath, ap->userid, "MF/" FN_MF);

  if (fp = fopen(fpath, "r"))
  {
    while (fread(&mf, sizeof(MF), 1, fp) == 1)
    {
      /* �u�䴩�Ĥ@�h���ݪO */
      if ((mf.mftype & MF_BOARD) &&
	(bhdr = brd_get(mf.xname)) &&
	(Ben_Perm(bhdr, uno, uid, ulevel) & BRD_R_BIT))
      {
	ap->myitem[num] = bhdr;
	num++;
      }
    }
    fclose(fp);
  }
  ap->total_item = num;
}


static int
cmd_favorlist(ap)
  Agent *ap;
{
  out_head(ap, "�ڪ��̷R");
  if (!acct_fetch(ap))
    return HS_ERR_LOGIN;

  init_myfavor(ap);

  cmdlist_list(ap, "�ڪ��̷R", boardlist_tie, boardlist_item);

  return HS_END;
}


  /* --------------------------------------------------- */
  /* �����ݪO�C��					 */
  /* --------------------------------------------------- */

static void
class_neck(fpw)
  FILE *fpw;
{
  fputs("<br>\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=760>\n"
    "<tr bgcolor=" HCOLOR_NECK ">\n"
    "  <td width=50%></td>\n"
    "  <td width=50% align=center><a href=/class>�^�̤W�h</a></td>\n"
    "</tr></table><br>\n", fpw);
}


static int
cmd_class(ap)
  Agent *ap;
{
  int fd, i, userno;
  usint ulevel;
  char folder[64], *xname, *userid;
  BRD *brd;
  HDR hdr;
  FILE *fpw = out_head(ap, "�����ݪO");

  if (!arg_analyze(1, ap->urlp, &xname, NULL, NULL, NULL))
    xname = CLASS_INIFILE;

  if (strlen(xname) > 12)
    return HS_ERROR;

  sprintf(folder, "gem/@/@%s", xname);
  if ((fd = open(folder, O_RDONLY)) < 0)
    return HS_ERR_CLASS;

  acct_fetch(ap);
  userno = ap->userno;
  userid = ap->userid;
  ulevel = ap->userlevel;

  class_neck(fpw);
  fputs("<table cellspacing=0 cellpadding=4 border=0>\n<tr bgcolor=" HCOLOR_TIE ">\n", fpw);
  boardlist_tie(fpw);
  i = 1;
  while (read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
  {
    fputs("<tr onmouseover=mOver(this); onmouseout=mOut(this);>\n", fpw);
    if (hdr.xmode & GEM_BOARD)	/* �ݪO */
    {
      if ((brd = brd_get(hdr.xname)) &&
	Ben_Perm(brd, userno, userid, ulevel) & BRD_R_BIT)
      {
	boardlist_item(fpw, brd, i);
      }
      else
	continue;
    }
    else if ((hdr.xmode & GEM_FOLDER) && *hdr.xname == '@')	/* ���� */
    {
      fprintf(fpw, "  <td>%d</td>\n"
	"  <td><a href=/class?%s>%s/</a></td>\n"
	"  <td>����</td>\n"
	"  <td>��</td>\n"
	"  <td colspan=2>%s</td>\n</tr>\n",
	i,
	hdr.xname + 1, hdr.xname + 1,
	str_html(hdr.title + 21, 52));
    }
    else			/* ��L���O�N���q�F */
    {
      continue;
    }

    i++;
  }
  close(fd);
  fputs("</table>\n", fpw);
  class_neck(fpw);
  return HS_END;
}


  /* --------------------------------------------------- */
  /* �峹�C��						 */
  /* --------------------------------------------------- */

static void
postlist_list(fpw, folder, brdname, start, total)
  FILE *fpw;
  char *folder, *brdname;
  int start, total;
{
  HDR hdr;
  char owner[80], *ptr1, *ptr2;
  int fd;

  fputs("<table cellspacing=0 cellpadding=4 border=0>\n<tr bgcolor=" HCOLOR_TIE ">\n"
    "  <td width=15>��</td>\n"
    "  <td width=15>�R</td>\n"
    "  <td width=50>�s��</td>\n"
    "  <td width=10>m</td>\n"

#ifdef HAVE_SCORE
    "  <td width=10>&nbsp;</td>\n"
#endif

    "  <td width=50>���</td>\n"
    "  <td width=100>�@��</td>\n"
    "  <td width=400>���D</td>\n"
    "</tr>\n", fpw);

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int i, end;

    /* �q�X�ݪO���� start �g�}�l�� HTML_TALL �g */
    i = start;
    end = i + HTML_TALL;

    lseek(fd, (off_t) (sizeof(HDR) * (i - 1)), SEEK_SET);

    while (i < end && read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      strcpy(owner, hdr.owner);
      if (ptr1 = strchr(owner, '.'))	/* ���~�@�� */
	*(ptr1 + 1) = '\0';
      if (ptr2 = strchr(owner, '@'))	/* ���~�@�� */
	*ptr2 = '\0';

      fputs("<tr onmouseover=mOver(this); onmouseout=mOut(this);>\n", fpw);
      if (brdname)
	fprintf(fpw, "  <td><a href=/mpost?%s&%d&%d><img src=/img/mark.gif border=0></a></td>\n"
	  "  <td><a href=/dpost?%s&%d&%d><img src=/img/del.gif border=0></a></td>\n",
	  brdname, i, hdr.chrono, brdname, i, hdr.chrono);
      else
	fprintf(fpw, "  <td><a href=/mmail?%d&%d><img src=/img/mark.gif border=0></a></td>\n"
	  "  <td><a href=/dmail?%d&%d><img src=/img/del.gif border=0></a></td>\n",
	  i, hdr.chrono, i, hdr.chrono);

      if (hdr.xmode & POST_BOTTOM)
	fputs("  <td>���n</td>\n", fpw);
      else
	fprintf(fpw, "  <td>%d</td>\n", i);
      fprintf(fpw, "  <td>%s</td>\n  <td>", hdr.xmode & POST_MARKED ? "m" : "");

#ifdef HAVE_SCORE
      if (hdr.xmode & POST_SCORE)
	fprintf(fpw, "<font color='%s'>%d</font>", hdr.score >= 0 ? "red" : "green", abs(hdr.score));
#endif

      fprintf(fpw, "</td>\n  <td>%s</td>\n  <td><a href=%s%s>%s</a></td>\n",
	hdr.date + 3, (ptr1 || ptr2) ? "mailto:" : "/query?", hdr.owner, owner);

      if (brdname)
	fprintf(fpw, "  <td><a href=/bmore?%s&%d>", brdname, i);
      else
	fprintf(fpw, "  <td><a href=/mmore?%d&>", i);

      fprintf(fpw, "%s</td>\n</tr>\n", str_html(hdr.title, 50));

      i++;
    }
    close(fd);
  }

  fputs("</table>\n", fpw);
}


static void
postlist_neck(fpw, start, total, brdname)
  FILE *fpw;
  int start, total;
  char *brdname;
{
  fputs("<br>\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=760>\n"
    "<tr bgcolor=" HCOLOR_NECK ">\n  <td width=20%", fpw);

  if (start > HTML_TALL)
  {
    fprintf(fpw, " align=center><a href=?%s&%d>�W%d�g</a",
      brdname, start - HTML_TALL, HTML_TALL);
  }
  fputs("></td>\n  <td width=20%", fpw);

  start += HTML_TALL;
  if (start <= total)
  {
    fprintf(fpw, " align=center><a href=?%s&%d>�U%d�g</a",
      brdname, start, HTML_TALL);
  }

  fprintf(fpw, "></td>\n  <td width=20%% align=center><a href=/dopost?%s target=_blank>�o��峹</a></td>\n"
    "  <td width=20%% align=center><a href=/gem?%s>��ذ�</a></td>\n"
    "  <td width=20%% align=center><a href=/brdlist>�ݪO�C��</a>&nbsp;"
    "<a href=/rss?%s><img border=0 src=/img/xml.gif alt=\"RSS �q�\\�o�ӬݪO\"></a></td>\n"
    "</tr></table><br>\n",
    brdname, brdname, brdname, brdname);
}


static int
cmd_postlist(ap)
  Agent *ap;
{
  int start, total;
  char folder[64], *brdname, *number;
  FILE *fpw = out_head(ap, "�峹�C��");

  if (!arg_analyze(2, ap->urlp, &brdname, &number, NULL, NULL))
  {
    if (brdname)
      number = "0";
    else
      return HS_ERROR;
  }

  if (!allow_brdname(ap, brdname))
    return HS_ERR_BOARD;

  brd_fpath(folder, brdname, FN_DIR);

  start = atoi(number);
  total = rec_num(folder, sizeof(HDR));
  if (start <= 0 || start > total)	/* �W�L�d�򪺸ܡA������̫�@�� */
    start = (total - 1) / HTML_TALL * HTML_TALL + 1;

  postlist_neck(fpw, start, total, brdname);

  postlist_list(fpw, folder, brdname, start, total);

  postlist_neck(fpw, start, total, brdname);
  return HS_END;
}


  /* --------------------------------------------------- */
  /* �H�c�C��						 */
  /* --------------------------------------------------- */

static void
mboxlist_neck(fpw, start, total)
  FILE *fpw;
  int start, total;
{
  fputs("<br>\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=760>\n"
    "<tr bgcolor=" HCOLOR_NECK ">\n  <td width=33% ", fpw);

  if (start > HTML_TALL)
  {
    fprintf(fpw, "align=center><a href=?%d>�W%d�g</a",
      start - HTML_TALL, HTML_TALL);
  }
  fputs("></td>\n  <td width=33%", fpw);

  start += HTML_TALL;
  if (start <= total)
  {
    fprintf(fpw, " align=center><a href=?%d>�U%d�g</a",
      start, HTML_TALL);
  }

  fputs("></td>\n  <td width=34% align=center><a href=/domail target=_blank>�o�e�H��</a></td>\n"
    "</tr></table><br>\n", fpw);
}


static int
cmd_mboxlist(ap)
  Agent *ap;
{
  int start, total;
  char folder[64], *number;
  FILE *fpw = out_head(ap, "�H�c�C��");

  if (!acct_fetch(ap))
    return HS_ERR_LOGIN;

  if (!arg_analyze(1, ap->urlp, &number, NULL, NULL, NULL))
    number = "0";

  usr_fpath(folder, ap->userid, FN_DIR);

  start = atoi(number);
  total = rec_num(folder, sizeof(HDR));
  if (start <= 0 || start > total)	/* �W�L�d�򪺸ܡA������̫�@�� */
    start = (total - 1) / HTML_TALL * HTML_TALL + 1;

  mboxlist_neck(fpw, start, total);

  postlist_list(fpw, folder, NULL, start, total);

  mboxlist_neck(fpw, start, total);
  return HS_END;
}


  /* --------------------------------------------------- */
  /* ��ذϦC��						 */
  /* --------------------------------------------------- */

static void
gemlist_neck(fpw, brdname)
  FILE *fpw;
  char *brdname;
{
  fprintf(fpw, "<br>\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=760>\n"
    "<tr bgcolor=" HCOLOR_NECK ">\n"
    "  <td width=50%% align=center><a href=/brd?%s>�^��ݪO</a></td>\n"
    "  <td width=50%% align=center><a href=/brdlist>�ݪO�C��</a></td>\n"
    "</tr></table><br>\n",
    brdname);
}


static int
cmd_gemlist(ap)
  Agent *ap;
{
  int fd, i;
  char folder[64], *brdname, *xname;
  HDR hdr;
  FILE *fpw = out_head(ap, "��ذ�");

  if (!arg_analyze(2, ap->urlp, &brdname, &xname, NULL, NULL))
  {
    if (brdname)
      xname = FN_DIR;
    else
      return HS_ERROR;
  }

  if ((*xname != 'F' || strlen(xname) != 8) && strcmp(xname, FN_DIR))
    return HS_ERROR;

  if (!allow_brdname(ap, brdname))
    return HS_ERR_BOARD;

  gemlist_neck(fpw, brdname);

  fputs("<table cellspacing=0 cellpadding=4 border=0>\n"
    "<tr bgcolor=" HCOLOR_TIE ">\n"
    "  <td width=50>�s��</td>\n"
    "  <td width=400>���D</td>\n"
    "</tr>\n", fpw);

  if (*xname == '.')
    sprintf(folder, "gem/brd/%s/%s", brdname, FN_DIR);
  else				/* if (*xname == 'F') */
    sprintf(folder, "gem/brd/%s/%c/%s", brdname, xname[7], xname);

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    i = 1;
    while (read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      fprintf(fpw, "<tr onmouseover=mOver(this); onmouseout=mOut(this);>\n"
	"  <td>%d</td>\n", i);
      if (hdr.xmode & GEM_RESTRICT)
      {
	fputs("  <td>[��Ū] ��ƫO�K</td>\n</tr>\n", fpw);
      }
      else if (hdr.xname[0] == 'A')	/* �峹 */
      {
	fprintf(fpw, "  <td><a href=/gmore?%s&%s&%d>[�峹] %s</a></td>\n</tr>\n",
	  brdname, xname, i, str_html(hdr.title, TTLEN));
      }
      else if (hdr.xname[0] == 'F')	/* ���v */
      {
	fprintf(fpw, "  <td><a href=/gem?%s&%s>[���v] %s</a></td>\n</tr>\n",
	  brdname, hdr.xname, str_html(hdr.title, TTLEN));
      }
      else				/* ��L���O�N���q�F */
      {
	fputs("  <td>[��Ū] ��L���</td>\n</tr>\n", fpw);
      }

      i++;
    }
    close(fd);
  }

  fputs("</table>\n", fpw);

  gemlist_neck(fpw, brdname);
  return HS_END;
}


  /* --------------------------------------------------- */
  /* �\Ū�ݪO�峹					 */
  /* --------------------------------------------------- */

static void
more_neck(fpw, pos, total, brdname, xname)
  FILE *fpw;
  int pos, total;
  char *brdname, *xname;
{
  fputs("<br>\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=760>\n"
    "<tr bgcolor=" HCOLOR_NECK ">\n  <td width=20%", fpw);

  if (pos > 1)
  {
    fputs(" align=center><a href=?", fpw);
    if (brdname)
      fprintf(fpw, "%s&", brdname);
    if (xname)
      fprintf(fpw, "%s&", xname);
    fprintf(fpw, "%d>�W�@�g</a", pos - 1);
  }
  fputs("></td>\n  <td width=20%", fpw);

  if (pos < total)
  {
    fputs(" align=center><a href=?", fpw);
    if (brdname)
      fprintf(fpw, "%s&", brdname);
    if (xname)
      fprintf(fpw, "%s&", xname);
    fprintf(fpw, "%d>�U�@�g</a", pos + 1);
  }

  if (xname)
    fprintf(fpw, "></td>\n  <td width=60%% align=center><a href=/gem?%s&%s>�^����v</a", brdname, xname);
  else if (brdname)
  {
    fprintf(fpw, "></td>\n  <td width=20%% align=center><a href=/bmost?%s&%d target=_blank>�P���D</a></td>\n"
      "  <td width=20%% align=center><a href=/dopost?%s target=_blank>�o��峹</a></td>\n"
      "  <td width=20%% align=center><a href=/brd?%s>�峹�C��</a",
      brdname, pos,
      brdname, brdname);
  }
  else
    fputs("></td>\n  <td width=60% align=center><a href=/mbox>�H�c�C��</a", fpw);

  fputs("></td>\n</tr></table><br>\n", fpw);
}


static int
more_item(fpw, folder, pos, brdname)
  FILE *fpw;
  char *folder;
  int pos;
  char *brdname;
{
  int fd, total;
  HDR hdr;
  char fpath[64];

  total = rec_num(folder, sizeof(HDR));
  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int find;

    lseek(fd, (off_t) (sizeof(HDR) * (pos - 1)), SEEK_SET);
    find = read(fd, &hdr, sizeof(HDR)) == sizeof(HDR);
    close(fd);

    if (find)
    {
      more_neck(fpw, pos, total, brdname, NULL);

#ifdef HAVE_REFUSEMARK
      if (!(hdr.xmode & POST_RESTRICT))
      {
#endif

	hdr_fpath(fpath, folder, &hdr);
	out_article(fpw, fpath);

#ifdef HAVE_REFUSEMARK
      }
      else
	out_mesg(fpw, "�o�O�[�K���峹�A�z�L�k�\\Ū");
#endif

      more_neck(fpw, pos, total, brdname, NULL);
      return HS_END;
    }
  }

  return HS_ERR_MORE;
}


static int
cmd_brdmore(ap)
  Agent *ap;
{
  int pos;
  char folder[64], *brdname, *number;
  FILE *fpw = out_head(ap, "�\\Ū�ݪO�峹");

  if (!arg_analyze(2, ap->urlp, &brdname, &number, NULL, NULL))
    return HS_ERROR;

  if (!allow_brdname(ap, brdname))
    return HS_ERR_BOARD;

  brd_fpath(folder, brdname, FN_DIR);

  if ((pos = atoi(number)) <= 0)
    pos = 1;

  return more_item(fpw, folder, pos, brdname);
}


  /* --------------------------------------------------- */
  /* �\Ū�H�c�峹					 */
  /* --------------------------------------------------- */

static int
cmd_mboxmore(ap)
  Agent *ap;
{
  int pos;
  char folder[64], *number;
  FILE *fpw = out_head(ap, "�\\Ū�H�c�峹");

  if (!arg_analyze(1, ap->urlp, &number, NULL, NULL, NULL))
    return HS_ERROR;

  if (!acct_fetch(ap))
    return HS_ERR_LOGIN;

  usr_fpath(folder, ap->userid, FN_DIR);

  if ((pos = atoi(number)) <= 0)
    pos = 1;

  return more_item(fpw, folder, pos, NULL);
}


  /* --------------------------------------------------- */
  /* �\Ū�ݪO�P���D�峹					 */
  /* --------------------------------------------------- */

static void
do_brdmost(fpw, folder, title)
  FILE *fpw;
  char *folder, *title;
{
  int fd;
  char fpath[64];
  FILE *fp;
  HDR hdr;

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    while (read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {

#ifdef HAVE_REFUSEMARK
      if (hdr.xmode & POST_RESTRICT)
	continue;
#endif

      if (!strcmp(str_ttl(hdr.title), title))
      {
	hdr_fpath(fpath, folder, &hdr);
	if (fp = fopen(fpath, "r"))
	{
	  txt2htm(fpw, fp);
	  fclose(fp);
	}
      }
    }
    close(fd);
  }
}


static void
brdmost_neck(fpw)
  FILE *fpw;
{
  fputs("<br>\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=760>\n"
    "<tr bgcolor=" HCOLOR_NECK ">\n"
    "  <td align=center>�P���D�\\Ū</td>\n"
    "</tr></table><br>\n", fpw);
}


static int
cmd_brdmost(ap)
  Agent *ap;
{
  int fd, pos;
  char folder[64], *brdname, *number;
  HDR hdr;
  FILE *fpw = out_head(ap, "�\\Ū�ݪO�P���D�峹");

  if (!arg_analyze(2, ap->urlp, &brdname, &number, NULL, NULL))
    return HS_ERROR;

  if (!allow_brdname(ap, brdname))
    return HS_ERR_BOARD;

  brd_fpath(folder, brdname, FN_DIR);

  if ((pos = atoi(number)) <= 0)
    pos = 1;

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int find;

    lseek(fd, (off_t) (sizeof(HDR) * (pos - 1)), SEEK_SET);
    find = read(fd, &hdr, sizeof(HDR)) == sizeof(HDR);
    close(fd);

    if (find)
    {
      brdmost_neck(fpw);
      out_style(fpw);
      do_brdmost(fpw, folder, str_ttl(hdr.title));
      brdmost_neck(fpw);
      return HS_END;
    }
  }
  return HS_ERR_MORE;
}


  /* --------------------------------------------------- */
  /* �\Ū��ذϤ峹					 */
  /* --------------------------------------------------- */

static int
cmd_gemmore(ap)
  Agent *ap;
{
  int fd, pos, total;
  char *brdname, *xname, *number, folder[64];
  HDR hdr;
  FILE *fpw = out_head(ap, "�\\Ū��ذϤ峹");

  if (!arg_analyze(3, ap->urlp, &brdname, &xname, &number, NULL))
    return HS_ERROR;

  if (*xname != 'F' && strlen(xname) != 8 && strcmp(xname, FN_DIR))
    return HS_ERROR;

  if (!allow_brdname(ap, brdname))
    return HS_ERR_BOARD;

  if (*xname == '.')
    gem_fpath(folder, brdname, FN_DIR);
  else				/* if (*xname == 'F') */
    sprintf(folder, "gem/brd/%s/%c/%s", brdname, xname[7], xname);

  if ((pos = atoi(number)) <= 0)
    pos = 1;
  total = rec_num(folder, sizeof(HDR));

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int find;

    lseek(fd, (off_t) (sizeof(HDR) * (pos - 1)), SEEK_SET);
    find = read(fd, &hdr, sizeof(HDR)) == sizeof(HDR);
    close(fd);

    if (find)
    {
      more_neck(fpw, pos, total, brdname, xname);
      if (hdr.xname[0] == 'A')
      {
	if (!(hdr.xmode & GEM_RESTRICT))
	{
	  sprintf(folder, "gem/brd/%s/%c/%s", brdname, hdr.xname[7], hdr.xname);
	  out_article(fpw, folder);
	}
	else
	  out_mesg(fpw, "�����O�K��ذϡA�z�L�k�\\Ū");
      }
      else
	out_mesg(fpw, "�o�O���v�ΰ�Ū��ơA�z�����Ѻ�ذϦC���Ū��");
      more_neck(fpw, pos, total, brdname, xname);
      return HS_END;
    }
  }

  return HS_ERR_MORE;
}


  /* --------------------------------------------------- */
  /* �o��峹						 */
  /* --------------------------------------------------- */

static int
cmd_dopost(ap)
  Agent *ap;
{
  char *brdname;
  FILE *fpw = out_head(ap, "�o��峹");

  if (!arg_analyze(1, ap->urlp, &brdname, NULL, NULL, NULL))
    return HS_ERROR;

  if (!(ben_perm(ap, brdname) & BRD_W_BIT))
    return HS_ERR_BOARD;

  fputs("<form method=post onsubmit=\"if(t.value.length==0 || c.value.length==0) {alert('���D�B���e�����i���ť�'); return false;} return true;\">\n"
    "  <input type=hidden name=dopost>\n"
    "  �п�J���D�G<br>\n", fpw);
  fprintf(fpw,
    "  <input type=hidden name=b value=%s>\n"
    "  <input type=text name=t size=%d maxlength=%d><br><br>\n"
    "  �п�J���e�G<br>\n"
    "  <textarea name=c rows=10 cols=%d></textarea><br><br>\n"
    "  <input type=hidden name=end>\r\n"
    "  <input type=submit value=�e�X�峹> "
    "  <input type=reset value=���s��g>"
    "</form>\n",
    brdname,
    TTLEN, TTLEN,
    SCR_WIDTH);

  return HS_END;
}


  /* --------------------------------------------------- */
  /* �o�e�H��						 */
  /* --------------------------------------------------- */

static int
cmd_domail(ap)
  Agent *ap;
{
  char *userid;
  FILE *fpw = out_head(ap, "�o�e�H��");

  if (!acct_fetch(ap))
    return HS_ERR_LOGIN;

  if (!arg_analyze(1, ap->urlp, &userid, NULL, NULL, NULL))
    userid = "";

  fputs("<form method=post onsubmit=\"if(u.value.length==0 || t.value.length==0 || c.value.length==0) {alert('���H�H�B���D�B���e�����i���ť�'); return false;} return true;\">\n"
    "  <input type=hidden name=domail>\n"
    "  �п�J���H�H�עҡG<br>\n", fpw);
  fprintf(fpw,
    "  <input type=text name=u size=%d maxlength=%d value=%s><br><br>\n"
    "  �п�J���D�G<br>\n"
    "  <input type=text name=t size=%d maxlength=%d><br><br>\n"
    "  �п�J���e�G<br>\n"
    "  <textarea name=c rows=10 cols=%d></textarea><br><br>\n"
    "  <input type=hidden name=end>\r\n"
    "  <input type=submit value=�e�X�H��> "
    "  <input type=reset value=���s��g>"
    "</form>\n",
    IDLEN, IDLEN, userid,
    TTLEN, TTLEN,
    SCR_WIDTH);

  return HS_END;
}


  /* --------------------------------------------------- */
  /* �аO/�R�� �峹					 */
  /* --------------------------------------------------- */

static void outgo_post();

static void
move_post(userid, hdr, folder, by_bm)	/* �N hdr �q folder �h��O���O */
  char *userid;
  HDR *hdr;
  char *folder;
  int by_bm;
{
  HDR post;
  int xmode;
  char fpath[64], fnew[64], *board;

  xmode = hdr->xmode;
  hdr_fpath(fpath, folder, hdr);

  if (!(xmode & POST_BOTTOM))	/* �m����Q�夣�� move_post */
  {
#ifdef HAVE_REFUSEMARK
    board = by_bm && !(xmode & POST_RESTRICT) ? BN_DELETED : BN_JUNK;	/* �[�K�峹��h junk */
#else
    board = by_bm ? BN_DELETED : BN_JUNK;
#endif

    brd_fpath(fnew, board, FN_DIR);
    hdr_stamp(fnew, HDR_LINK | 'A', &post, fpath);

    /* �����ƻs trailing data */
    memcpy(post.owner, hdr->owner, TTLEN + 140);

    if (by_bm)
      sprintf(post.title, "%-13s%.59s", userid, hdr->title);

    rec_bot(fnew, &post, sizeof(HDR));
    brd_get(board)->btime = -1;
  }
  unlink(fpath);
}


static int
op_shell(op_func, ap, title, msg)
  int (*op_func) (Agent *, char *, char *);
  Agent *ap;
  char *title, *msg;
{
  int ret = op_func(ap, title, msg);
  if (ret != HS_END)
    out_head(ap, title + 1);
  return ret;
}


static int
post_op(ap, title, msg)
  Agent *ap;
  char *title, *msg;
{
  int pos;
  time_t chrono;
  HDR hdr;
  usint bits;
  char *brdname, *number, *stamp, folder[64];

  if (!arg_analyze(3, ap->urlp, &brdname, &number, &stamp, NULL) ||
    (pos = atoi(number) - 1) < 0 ||
    (chrono = atoi(stamp)) < 0)
    return HS_ERROR;

  if (bits = ben_perm(ap, brdname))
  {
    brd_fpath(folder, brdname, FN_DIR);
    if (rec_get(folder, &hdr, sizeof(HDR), pos) ||
      (hdr.chrono != chrono))
      return HS_ERR_MORE;

    if (*title == 'm')
    {
      if (!(bits & BRD_X_BIT))
	return HS_ERR_PERM;

      hdr.xmode ^= POST_MARKED;
      rec_put(folder, &hdr, sizeof(HDR), pos, NULL);
    }
    else if (!(hdr.xmode & POST_MARKED))
    {
      if (!(bits & BRD_X_BIT) &&
	(!(bits & BRD_W_BIT) || strcmp(ap->userid, hdr.owner)))
	return HS_ERR_PERM;

      rec_del(folder, sizeof(HDR), pos, NULL);
      move_post(ap->userid, &hdr, folder, bits & BRD_X_BIT);
      brd_get(brdname)->btime = -1;
      /* �s�u��H */
      if ((hdr.xmode & POST_OUTGO) &&	/* �~��H�� */
	hdr.chrono > (time(0) - 7 * 86400))	/* 7 �Ѥ������� */
      {
	hdr.chrono = -1;
	outgo_post(&hdr, brdname);
      }
    }

    sprintf(folder, "/brd?%s&%d", brdname, (pos - 1) / HTML_TALL * HTML_TALL + 1);
    out_title(out_http(ap, HS_OK | HS_REFRESH, folder), title + 1);
    out_mesg(ap->fpw, msg);
    fprintf(ap->fpw, "<a href=%s>�^�峹�C��</a>\n", folder);

    return HS_END;
  }
  return HS_ERR_BOARD;
}


static int
cmd_markpost(ap)
  Agent *ap;
{
  return op_shell(post_op, ap, "m�аO�峹", "�w����(����)�аO���O");
}


static int
cmd_delpost(ap)
  Agent *ap;
{
  return op_shell(post_op, ap, "d�R���峹", "�w����R�����O�A�Y���R����ܦ��峹�Q�аO�F");
}


static int
cmd_predelpost(ap)
  Agent *ap;
{
  char *brdname, *number, *stamp;
  FILE *fpw = out_head(ap, "�T�{�R���峹");

  if (!arg_analyze(3, ap->urlp, &brdname, &number, &stamp, NULL))
    return HS_ERROR;

  out_mesg(fpw, "�Y�T�w�n�R�����g�峹�A�ЦA���I��H�U�s���F�Y�n�����R���A�Ы� [�W�@��]");
  fprintf(fpw, "<a href=/delpost?%s&%s&%s>�R�� [%s] �O�� %s �g�峹</a><br>\n",
    brdname, number, stamp, brdname, number);

  return HS_END;
}


  /* --------------------------------------------------- */
  /* �аO/�R�� �H��					 */
  /* --------------------------------------------------- */

static int
mail_op(ap, title, msg)
  Agent *ap;
  char *title, *msg;
{
  int pos;
  time_t chrono;
  HDR hdr;
  char *number, *stamp, folder[64], fpath[64];

  if (!arg_analyze(2, ap->urlp, &number, &stamp, NULL, NULL) ||
    (pos = atoi(number) - 1) < 0 ||
    (chrono = atoi(stamp)) < 0)
    return HS_ERROR;

  if (acct_fetch(ap))
  {
    usr_fpath(folder, ap->userid, FN_DIR);
    if (rec_get(folder, &hdr, sizeof(HDR), pos) ||
      (hdr.chrono != chrono))
      return HS_ERR_MAIL;

    if (*title == 'm')
    {
      hdr.xmode ^= POST_MARKED;
      rec_put(folder, &hdr, sizeof(HDR), pos, NULL);
    }
    else if (!(hdr.xmode & POST_MARKED))
    {
      rec_del(folder, sizeof(HDR), pos, NULL);
      hdr_fpath(fpath, folder, &hdr);
      unlink(fpath);
    }

    sprintf(folder, "/mbox?%d", (pos - 1) / HTML_TALL * HTML_TALL + 1);
    out_title(out_http(ap, HS_OK | HS_REFRESH, folder), title + 1);
    out_mesg(ap->fpw, msg);
    fprintf(ap->fpw, "<a href=%s>�^�H�c�C��</a>\n", folder);
    return HS_END;
  }
  return HS_ERR_LOGIN;
}


static int
cmd_markmail(ap)
  Agent *ap;
{
  return op_shell(mail_op, ap, "m�аO�H��", "�w����(����)�аO���O");
}


static int
cmd_delmail(ap)
  Agent *ap;
{
  return op_shell(mail_op, ap, "d�R���H��", "�w����R�����O�A�Y���R����ܦ��H��Q�аO�F");
}


static int
cmd_predelmail(ap)
  Agent *ap;
{
  char *number, *stamp;
  FILE *fpw = out_head(ap, "�T�{�R���H��");

  if (!arg_analyze(2, ap->urlp, &number, &stamp, NULL, NULL))
    return HS_ERROR;

  out_mesg(fpw, "�Y�T�w�n�R�����g�H��A�ЦA���I��H�U�s���F�Y�n�����R���A�Ы� [�W�@��]");
  fprintf(fpw, "<a href=/delmail?%s&%s>�R���H�c�� %s �g�H��</a><br>\n",
    number, stamp, number);

  return HS_END;
}


  /* --------------------------------------------------- */
  /* �d�ߨϥΪ�						 */
  /* --------------------------------------------------- */

static int
cmd_query(ap)
  Agent *ap;
{
  int fd;
  ACCT acct;
  char fpath[64], *userid;
  FILE *fpw = out_head(ap, "�d�ߨϥΪ�");

  if (!arg_analyze(1, ap->urlp, &userid, NULL, NULL, NULL))
    return HS_ERROR;

  if (!allow_userid(ap, userid))
    return HS_ERR_USER;

  usr_fpath(fpath, userid, FN_ACCT);
  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    read(fd, &acct, sizeof(ACCT));
    close(fd);

    fprintf(fpw, "<pre>\n"
      "<a target=_blank href=domail?%s>%s (%s)</a><br>\n"
      "%s�q�L�{�ҡA�@�W�� %d ���A�o��L %d �g�峹<br>\n"
      "�̪�(%s)�q[%s]�W��<br>\n"
      "</pre>\n",
      acct.userid, acct.userid,
      str_html(acct.username, UNLEN),
      (acct.userlevel & PERM_VALID) ? "�w" : "��", acct.numlogins, acct.numposts,
      Btime(&(acct.lastlogin)), acct.lasthost);

    usr_fpath(fpath, acct.userid, FN_PLANS);
    out_article(fpw, fpath);
  }
  else
    return HS_ERR_USER;

  return HS_END;
}


  /* --------------------------------------------------- */
  /* ��ܹϤ�						 */
  /* --------------------------------------------------- */

static int
valid_path(str)
  char *str;
{
  int ch;

  if (!*str)
    return;

  while (ch = *str++)
  {
    if (!is_alnum(ch) && ch != '.' && ch != '-' && ch != '_')
      return 0;
  }
  return 1;
}


static int
cmd_image(ap)
  Agent *ap;
{
  FILE *fpw;
  struct stat st;
  char *fname, *ptr, fpath[80];

  if (!arg_analyze(1, ap->urlp, &fname, NULL, NULL, NULL))
    return HS_NOTFOUND;

  if (!valid_path(fname) || !(ptr = strrchr(fname, '.')))
    return HS_NOTFOUND;

  /* �䴩�榡 */
  if (!str_cmp(ptr, ".html"))
    ptr = "text/html";
  else if (!str_cmp(ptr, ".gif"))
    ptr = "image/gif";
  else if (!str_cmp(ptr, ".jpg"))
    ptr = "image/jpeg";
  else if (!str_cmp(ptr, ".css"))
    ptr = "text/css";
  else if (!str_cmp(ptr, ".png"))
    ptr = "image/png";
  else
    return HS_NOTFOUND;

  sprintf(fpath, "run/html/%.40s", fname);
  if (stat(fpath, &st))
    return HS_NOTFOUND;

  if (ap->modified && !strcmp(Gtime(&st.st_mtime), ap->modified))	/* �S���ܧ󤣻ݭn�ǿ� */
    return HS_NOTMOIDIFY;

  fpw = out_http(ap, HS_OK, ptr);
  fprintf(fpw, "Content-Length: %d\r\n", st.st_size);
  fprintf(fpw, "Last-Modified: %s\r\n\r\n", Gtime(&st.st_mtime));
  f_suck(fpw, fpath);

  return HS_OK;
}


  /* --------------------------------------------------- */
  /* RSS						 */
  /* --------------------------------------------------- */

static int
cmd_rss(ap)
  Agent *ap;
{
  time_t blast;
  char folder[64], *brdname, *ptr;
  BRD *brd;
  HDR hdr;
  FILE *fpw;
  int fd;

  if (!arg_analyze(1, ap->urlp, &brdname, NULL, NULL, NULL))
    return HS_BADREQUEST;

  if (!(brd = brd_get(brdname)))
    return HS_NOTFOUND;

  if (brd->readlevel)		/* �u�����}�O�~���� rss */
    return HS_FORBIDDEN;

  blast = brd->blast;

  if (ap->modified && !strcmp(Gtime(&blast), ap->modified))	/* �S���ܧ󤣻ݭn�ǿ� */
    return HS_NOTMOIDIFY;

  fpw = out_http(ap, HS_OK, "application/xml");
  fprintf(fpw, "Last-Modified: %s\r\n\r\n", Gtime(&blast));

  /* xml header */
  fputs("<?xml version=\"1.0\" encoding=\"" MYCHARSET "\" ?>\n"
    "<rss version=\"2.0\">\n"
    "<channel>\n", fpw);
  ptr = Gtime(&blast);
  ptr[4] = '\0';
  fprintf(fpw, "<title>" BBSNAME "-%s�O</title>\n"
    "<link>http://" MYHOSTNAME "/brd?%s</link>\n"
    "<description>%s</description>\n"
    "<language>zh-tw</language>\n"
    "<lastBuildDate>%s ",
    brdname, brdname,
    str_html(brd->title, TTLEN), ptr);
  ptr += 5;
  if (*ptr == '0')
    ++ptr;
  fprintf(fpw, "%s</lastBuildDate>\n<image>\n"
    "<title>" BBSNAME "</title>"
    "<link>http://" MYHOSTNAME "</link>\n"
    "<url>http://" MYHOSTNAME "/img/rss.gif</url>\n"
    "</image>\n", ptr);

  /* rss item */
  brd_fpath(folder, brdname, FN_DIR);
  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int i, end;

    i = rec_num(folder, sizeof(HDR));
    if (i > 20)
      end = i - 20;
    else
      end = 0;
    lseek(fd, (off_t) (-256), SEEK_END);
    for (; i > end && read(fd, &hdr, sizeof(HDR)) == sizeof(HDR); --i)
    {
      ptr = Gtime(&hdr.chrono);
      ptr[4] = '\0';
      fprintf(fpw, "<!-- %d --><item><title>%s</title>"
	"<link>http://" MYHOSTNAME "/bmore?%s&amp;%d</link>"
	"<author>%s</author>"
	"<pubDate>%s ",
	hdr.chrono, str_html(hdr.title, TTLEN),
	brdname, i,
	hdr.owner,
	ptr);
      ptr += 5;
      if (*ptr == '0')
	++ptr;
      fprintf(fpw, "%s</pubDate></item>\n", ptr);
      lseek(fd, (off_t) (-512), SEEK_CUR);
    }
    close(fd);
  }
  fputs("</channel>\n</rss>\n", fpw);

  return HS_OK;
}


  /* --------------------------------------------------- */
  /* ����						 */
  /* --------------------------------------------------- */

static void
mainpage_neck(fpw, userid, logined)
  FILE *fpw;
  char *userid;
  int logined;
{
  fprintf(fpw, "<br>\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=760>\n"
    "<tr bgcolor=" HCOLOR_NECK ">\n"
    "  <td width=100%% align=center>%s%s�w����{</td>\n"
    "</tr></table><br>\n",
    logined ? userid : "",
    logined ? "�A" : "");
}

static int
cmd_mainpage(ap)
  Agent *ap;
{
  int logined;
  FILE *fpw = out_head(ap, "");

  logined = acct_fetch(ap);
  mainpage_neck(fpw, ap->userid, logined);

  out_film(fpw, (ap->uptime % 3) + FILM_OPENING0);
  /* �n�J */
  if (!logined)
  {
    /* �}�Y�e�� */
    fputs("<form method=post>\n"
      "  <input type=hidden name=login>\n"
      "  �b�� <input type=text name=u size=12 maxlength=12> "
      "  �K�X <input type=password name=p size=12 maxlength=8> "
      "  <input type=submit value=�n�J> "
      "  <input type=reset value=�M��>"
      "</form>\n", fpw);
  }

  mainpage_neck(fpw, ap->userid, logined);
  return HS_END;
}


  /* --------------------------------------------------- */
  /* ���O��						 */
  /* --------------------------------------------------- */

static Command cmd_table_get[] =
{
  cmd_userlist,   "usrlist", 7,
  cmd_boardlist,  "brdlist", 7,
  cmd_favorlist,  "fvrlist", 7,

  cmd_postlist,   "brd",     3,
  cmd_gemlist,    "gem",     3,
  cmd_mboxlist,   "mbox",    4,

  cmd_brdmore,    "bmore",   5,
  cmd_brdmost,    "bmost",   5,
  cmd_gemmore,    "gmore",   5,
  cmd_mboxmore,   "mmore",   5,

  cmd_dopost,     "dopost",  6,
  cmd_domail,     "domail",  6,

  cmd_delpost,    "delpost", 7,
  cmd_predelpost, "dpost",   5,
  cmd_delmail,    "delmail", 7,
  cmd_predelmail, "dmail",   5,
  cmd_markpost,   "mpost",   5,
  cmd_markmail,   "mmail",   5,

  cmd_query,      "query",   5,

  cmd_image,      "img/",    4,
  cmd_class,      "class",   5,
  cmd_rss,        "rss",     3,

  cmd_mainpage,   NULL,      0
};


/* ----------------------------------------------------- */
/* command dispatch (POST)				 */
/* ----------------------------------------------------- */

static char *
getfromhost(pip)
  void *pip;
{
  struct hostent *hp;

  if (hp = gethostbyaddr((char *)pip, 4, AF_INET))
    return hp->h_name;
  else
    return inet_ntoa(*(struct in_addr *) pip);
}


  /* --------------------------------------------------- */
  /* �ϥΪ̵n�J						 */
  /* --------------------------------------------------- */

static int
cmd_login(ap)
  Agent *ap;
{
  char *userid, *passwd;
  char fpath[64];
  ACCT acct;

  /* u=userid&p=passwd */
  if (!arg_analyze(2, ap->urlp, &userid, &passwd, NULL, NULL))
    return HS_ERROR;

  userid += 2;			/* skip "u=" */
  passwd += 2;			/* skip "p=" */

  if (*userid && *passwd && strlen(userid) <= IDLEN && strlen(passwd) <= PSWDLEN)
  {
    usr_fpath(fpath, userid, FN_ACCT);
    if (!rec_get(fpath, &acct, sizeof(ACCT), 0) &&
      !(acct.userlevel & (PERM_DENYLOGIN | PERM_PURGE)) &&
      !chkpasswd(acct.passwd, passwd))	/* �n�J���\ */
    {
      /* itoc.040308: ���� Cookie */
      sprintf(ap->cookie, "%s&p=%s", userid, acct.passwd);
      ap->setcookie = 1;
    }
  }

  return cmd_mainpage(ap);
}


  /* --------------------------------------------------- */
  /* �o��s�峹						 */
  /* --------------------------------------------------- */

static void
outgo_post(hdr, board)
  HDR *hdr;
  char *board;
{
  bntp_t bntp;

  memset(&bntp, 0, sizeof(bntp_t));
  bntp.chrono = hdr->chrono;
  strcpy(bntp.board, board);
  strcpy(bntp.xname, hdr->xname);
  strcpy(bntp.owner, hdr->owner);
  strcpy(bntp.nick, hdr->nick);
  strcpy(bntp.title, hdr->title);
  rec_add("innd/out.bntp", &bntp, sizeof(bntp_t));
}


static int
cmd_addpost(ap)
  Agent *ap;
{
  char *brdname, *title, *content, *end;
  char folder[64], fpath[64];
  HDR hdr;
  BRD *brd;
  FILE *fp;

  out_head(ap, "�峹�o��");

  if (!acct_fetch(ap))
    return HS_ERR_LOGIN;

  /* b=brdname&t=title&c=content&end= */
  if (arg_analyze(4, ap->urlp, &brdname, &title, &content, &end))
  {
    brdname += 2;		/* skip "b=" */
    title += 2;			/* skip "t=" */
    content += 2;		/* skip "c=" */

    if (*brdname && *title && *content)
    {
      if ((brd = brd_get(brdname)) &&
	(Ben_Perm(brd, ap->userno, ap->userid, ap->userlevel) & BRD_W_BIT))
      {
	brd_fpath(folder, brdname, FN_DIR);

	fp = fdopen(hdr_stamp(folder, 'A', &hdr, fpath), "w");
	fprintf(fp, "%s %s (%s) %s %s\n",
	  STR_AUTHOR1, ap->userid, ap->username,
	  STR_POST2, brdname);
	str_ncpy(hdr.title, title, sizeof(hdr.title));
	fprintf(fp, "���D: %s\n�ɶ�: %s\n\n", hdr.title, Now());
	fprintf(fp, "%s\n", content);
	fprintf(fp, EDIT_BANNER, ap->userid, getfromhost(&(ap->ip_addr)));
	fclose(fp);

	hdr.xmode = (brd->battr & BRD_NOTRAN) ? 0 : POST_OUTGO;
	strcpy(hdr.owner, ap->userid);
	strcpy(hdr.nick, ap->username);
	rec_bot(folder, &hdr, sizeof(HDR));

	brd->btime = -1;
	if (hdr.xmode & POST_OUTGO)
	  outgo_post(&hdr, brdname);

	out_reload(ap->fpw, "�z���峹�o���\\");
	return HS_OK;
      }
      return HS_ERR_BOARD;
    }
  }
  return HS_ERROR;
}


  /* --------------------------------------------------- */
  /* �o�e�s�H��						 */
  /* --------------------------------------------------- */

static int
cmd_addmail(ap)
  Agent *ap;
{
  char *userid, *title, *content, *end;
  char folder[64], fpath[64];
  HDR hdr;
  FILE *fp;

  out_head(ap, "�H��o�e");

  /* u=userid&t=title&c=content&end= */
  if (arg_analyze(4, ap->urlp, &userid, &title, &content, &end))
  {
    userid += 2;		/* skip "u=" */
    title += 2;			/* skip "t=" */
    content += 2;		/* skip "c=" */

    if (*userid && *title && *content && allow_userid(ap, userid))
    {
      usr_fpath(fpath, userid, FN_ACCT);
      if (dashf(fpath) && acct_fetch(ap))
      {
	usr_fpath(folder, userid, FN_DIR);

	fp = fdopen(hdr_stamp(folder, 0, &hdr, fpath), "w");
	fprintf(fp, "%s %s (%s)\n",
	  STR_AUTHOR1, ap->userid, ap->username);
	str_ncpy(hdr.title, title, sizeof(hdr.title));
	fprintf(fp, "���D: %s\n�ɶ�: %s\n\n", hdr.title, Now());
	fprintf(fp, "%s\n", content);
	fprintf(fp, EDIT_BANNER, ap->userid, getfromhost(&(ap->ip_addr)));
	fclose(fp);

	strcpy(hdr.owner, ap->userid);
	strcpy(hdr.nick, ap->username);
	rec_add(folder, &hdr, sizeof(HDR));

	out_reload(ap->fpw, "�z���H��o�e���\\");
	return HS_OK;
      }
    }
  }

  out_reload(ap->fpw, "�z���H��o�e���ѡA�]�\\�O�]���z�|���n�J�άO�d�L���ϥΪ�");
  return HS_OK;
}


  /* --------------------------------------------------- */
  /* ���O��						 */
  /* --------------------------------------------------- */

static Command cmd_table_post[] =
{
  cmd_login,    "login=&",  7,
  cmd_addpost,  "dopost=&", 8,
  cmd_addmail,  "domail=&", 8,

  cmd_mainpage,  NULL,      0
};


/* ----------------------------------------------------- */
/* close a connection & release its resource		 */
/* ----------------------------------------------------- */

static void
agent_fire(ap)
  Agent *ap;
{
  int csock;

  csock = ap->sock;
  if (csock > 0)
  {
    fcntl(csock, F_SETFL, M_NONBLOCK);
    shutdown(csock, 2);

    /* fclose(ap->fpw); */
    close(csock);
  }

  free(ap->data);
}


/* ----------------------------------------------------- */
/* receive request from client				 */
/* ----------------------------------------------------- */

static int
agent_recv(ap)
  Agent *ap;
{
  int cc, mode, size, used;
  uschar *data, *head, *ptr;

  used = ap->used;
  data = ap->data;

  if (used > 0)
  {
    /* check the available space */

    size = ap->size;
    cc = size - used;

    if (cc < TCP_RCVSIZ + 3)
    {
      if (size < MAX_DATA_SIZE)
      {
	size += TCP_RCVSIZ + (size >> 2);

	if (data = (uschar *) realloc(data, size))
	{
	  ap->data = data;
	  ap->size = size;
	}
	else
	{
	  fprintf(flog, "ERROR\tmalloc: %d\n", size);
	  return 0;
	}
      }
      else
      {
	fprintf(flog, "ERROR\tdata too long\n");
	return 0;
      }
    }
  }

  head = data + used;
  cc = recv(ap->sock, head, TCP_RCVSIZ, 0);

  if (cc <= 0)
  {
    cc = errno;
    if (cc != EWOULDBLOCK)
    {
#ifdef LOG_VERBOSE
      fprintf(flog, "RECV\t%s\n", strerror(cc));
#endif
      return 0;
    }

    /* would block, so leave it to do later */

    return -1;
  }

  head[cc] = '\0';
  ap->used = (used += cc);

  /* itoc.050807: recv() �@����Ū�������A�@�w�O cmd_dopost �� cmd_domail�A�o�G�̪��������� &end= */
  if (used >= TCP_RCVSIZ)
  {
    /* �h -2 �O�]�������s�����|�۰ʸɤW \r\n */
    if (!strstr(head + cc - strlen("&end=") - 2, "&end="))	/* �٨SŪ���A�~��Ū */
      return 1;
  }

  mode = 0;
  head = data;

  while (cc = *head)
  {
    if (cc == '\n')
    {
      data++;
    }
    else if (cc == '\r')
    {
      *head = '\0';

      if (!(mode & (AM_GET | AM_POST)))
      {
	if (!str_ncmp(data, "GET ", 4))
	{
	  mode ^= AM_GET;
	  data += 4;
	  if (*data != '/')
	  {
	    out_error(ap, HS_BADREQUEST);
	    fflush(ap->fpw);
	    return 0;
	  }
	  if (ptr = strchr(data, ' '))
	  {
	    *ptr = '\0';
	    str_ncpy(ap->url, data + 1, sizeof(ap->url));
	  }
	  else
	  {
	    *ap->url = '\0';
	  }
	}
	else if (!str_ncmp(data, "POST ", 5))
	{
	  mode ^= AM_POST;
	}
      }
      else
      {
	if (*data)		/* ���O�Ŧ�G���Y */
	{
	  /* ���RCookie */
	  if (!str_ncmp(data, "Cookie: user=", 13))
	  {
	    str_ncpy(ap->cookie, data + 13, LEN_COOKIE);
	  }
	  /* ���R�ɮ׮ɶ� */
	  if ((mode & AM_GET) && !str_ncmp(data, "If-Modified-Since: ", 19))
	  {
	    ap->modified = data + 19;
	    data[48] = '\0';
	  }
	}
	else			/* �Ŧ�G�U�@�欰���e */
	{
	  Command *cmd;
	  char *url;

	  if (mode & AM_GET)
	  {
	    cmd = cmd_table_get;
	    url = ap->url;
	  }
	  else
	  {
	    cmd = cmd_table_post;
	    for (url = head + 1; cc = *url; ++url)	/* ��}�Y(²�ƥu�P�_>'9') */
	    {
	      if (cc != '\0' && cc > '9')
		break;
	    }
	  }
	  for (; ptr = cmd->cmd; cmd++)
	  {
	    if (!str_ncmp(url, ptr, cmd->len))
	      break;
	  }
	  cc = cmd->len;

	  /* �B�z�۰ʭ��s�ɦV */
	  if (mode & AM_GET)
	  {
	    if (ptr)
	    {
	      if (ptr[cc - 1] == '/')
		goto noredirect;
	      if (ptr[cc] == '/')
	      {
		if (url[cc] == '/')
		{
		  cc++;
		  goto noredirect;
		}
	      }
	      else if (!url[cc])
	      {
		goto noredirect;
	      }
	      else if (url[cc] == '?')
	      {
		cc++;
		goto noredirect;
	      }
	      else if (url[cc] != '/')
	      {
		ptr = NULL;
	      }
	    }
	    else if (!*url)
	    {
	      goto noredirect;
	    }

	    out_http(ap, HS_REDIRECT, ptr);
	    fflush(ap->fpw);
	    return 0;
	  }
      noredirect:
	  ap->urlp = url + cc;
	  cc = (*cmd->func) (ap);
	  if (cc != HS_OK)
	  {
	    if (cc != HS_END)
	      out_error(ap, cc);
	    if (cc < HS_OK)
	      out_tail(ap->fpw);
	  }
	  fflush(ap->fpw);
	  return 0;
	}
      }
      data = head + 1;
    }
    head++;
  }

  return 0;
}


/* ----------------------------------------------------- */
/* accept a new connection				 */
/* ----------------------------------------------------- */

static int
agent_accept(ipaddr)
  unsigned int *ipaddr;
{
  int csock;
  int value;
  struct sockaddr_in csin;

  for (;;)
  {
    value = sizeof(csin);
    csock = accept(0, (struct sockaddr *) & csin, &value);
    /* if (csock > 0) */
    if (csock >= 0)		/* Thor.000126: more proper */
      break;

    csock = errno;
    if (csock != EINTR)
    {
#ifdef LOG_VERBOSE
      logit("ACCEPT", strerror(csock));
#endif
      return -1;
    }

    while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
  }

  value = 1;
  /* Thor.000511: ����: don't delay send to coalesce(�p�X) packets */
  setsockopt(csock, IPPROTO_TCP, TCP_NODELAY, (char *)&value, sizeof(value));

  *ipaddr = csin.sin_addr.s_addr;

  return csock;
}


/* ----------------------------------------------------- */
/* signal routines					 */
/* ----------------------------------------------------- */

#ifdef  SERVER_USAGE
static void
servo_usage()
{
  struct rusage ru;

  if (getrusage(RUSAGE_SELF, &ru))
    return;

  fprintf(flog, "\n[Server Usage]\n\n"
    " user time: %.6f\n"
    " system time: %.6f\n"
    " maximum resident set size: %lu P\n"
    " integral resident set size: %lu\n"
    " page faults not requiring physical I/O: %d\n"
    " page faults requiring physical I/O: %d\n"
    " swaps: %d\n"
    " block input operations: %d\n"
    " block output operations: %d\n"
    " messages sent: %d\n"
    " messages received: %d\n"
    " signals received: %d\n"
    " voluntary context switches: %d\n"
    " involuntary context switches: %d\n",

    (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000.0,
    (double)ru.ru_stime.tv_sec + (double)ru.ru_stime.tv_usec / 1000000.0,
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

  fflush(flog);
}
#endif


static void
sig_term(sig)
  int sig;
{
  char buf[80];

  sprintf(buf, "sig: %d, errno: %d => %s", sig, errno, strerror(errno));
  logit("EXIT", buf);
  fclose(flog);
  exit(0);
}


static void
reaper()
{
  while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}


static void
servo_signal()
{
  struct sigaction act;

  /* sigblock(sigmask(SIGPIPE)); *//* Thor.981206: �Τ@ POSIX �зǥΪk */

  /* act.sa_mask = 0; *//* Thor.981105: �зǥΪk */
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  act.sa_handler = sig_term;		/* forced termination */
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGSEGV, &act, NULL);	/* if rlimit violate */
  sigaction(SIGBUS, &act, NULL);

#if 1	/* Thor.990203: �� signal */
  sigaction(SIGURG, &act, NULL);
  sigaction(SIGXCPU, &act, NULL);
  sigaction(SIGXFSZ, &act, NULL);

#ifdef SOLARIS
  sigaction(SIGLOST, &act, NULL);
  sigaction(SIGPOLL, &act, NULL);
  sigaction(SIGPWR, &act, NULL);
#endif

#ifdef LINUX
  sigaction(SIGSYS, &act, NULL);
  /* sigaction(SIGEMT, &act, NULL); */
  /* itoc.010317: �ڪ� linux �S���o�ӻ� :p */
#endif

  sigaction(SIGFPE, &act, NULL);
  sigaction(SIGWINCH, &act, NULL);
  sigaction(SIGINT, &act, NULL);
  sigaction(SIGQUIT, &act, NULL);
  sigaction(SIGILL, &act, NULL);
  sigaction(SIGTRAP, &act, NULL);
  sigaction(SIGABRT, &act, NULL);
  sigaction(SIGTSTP, &act, NULL);
  sigaction(SIGTTIN, &act, NULL);
  sigaction(SIGTTOU, &act, NULL);
  sigaction(SIGVTALRM, &act, NULL);
#endif

  sigaction(SIGHUP, &act, NULL);

  act.sa_handler = reaper;
  sigaction(SIGCHLD, &act, NULL);

#ifdef  SERVER_USAGE
  act.sa_handler = servo_usage;
  sigaction(SIGPROF, &act, NULL);
#endif

  /* Thor.981206: lkchu patch: �Τ@ POSIX �зǥΪk */
  /* �b���ɥ� sigset_t act.sa_mask */
  sigaddset(&act.sa_mask, SIGPIPE);
  sigprocmask(SIG_BLOCK, &act.sa_mask, NULL);
}


/* ----------------------------------------------------- */
/* server core routines					 */
/* ----------------------------------------------------- */

static void
servo_daemon(inetd)
  int inetd;
{
  int fd, value;
  char buf[80];
  struct linger ld;
  struct sockaddr_in sin;

#ifdef HAVE_RLIMIT
  struct rlimit limit;
#endif

  /* More idiot speed-hacking --- the first time conversion makes the C     *
   * library open the files containing the locale definition and time zone. *
   * If this hasn't happened in the parent process, it happens in the       *
   * children, once per connection --- and it does add up.                  */

  time((time_t *) & value);
  gmtime((time_t *) & value);
  strftime(buf, 80, "%d/%b/%Y:%H:%M:%S", localtime((time_t *) & value));

#ifdef HAVE_RLIMIT
  /* --------------------------------------------------- */
  /* adjust the resource limit				 */
  /* --------------------------------------------------- */

  getrlimit(RLIMIT_NOFILE, &limit);
  limit.rlim_cur = limit.rlim_max;
  setrlimit(RLIMIT_NOFILE, &limit);

  limit.rlim_cur = limit.rlim_max = 16 * 1024 * 1024;
  setrlimit(RLIMIT_FSIZE, &limit);

  limit.rlim_cur = limit.rlim_max = 16 * 1024 * 1024;
  setrlimit(RLIMIT_DATA, &limit);

#ifdef SOLARIS
#define RLIMIT_RSS RLIMIT_AS	/* Thor.981206: port for solaris 2.6 */
#endif

  setrlimit(RLIMIT_RSS, &limit);

  limit.rlim_cur = limit.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &limit);
#endif

  /* --------------------------------------------------- */
  /* detach daemon process				 */
  /* --------------------------------------------------- */

  close(1);
  close(2);

  if (inetd)
    return;

  close(0);

  if (fork())
    exit(0);

  setsid();

  if (fork())
    exit(0);

  /* --------------------------------------------------- */
  /* setup socket					 */
  /* --------------------------------------------------- */

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  value = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&value, sizeof(value));

  ld.l_onoff = ld.l_linger = 0;
  setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&ld, sizeof(ld));

  sin.sin_family = AF_INET;
  sin.sin_port = htons(BHTTP_PORT);
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  memset((char *)&sin.sin_zero, 0, sizeof(sin.sin_zero));

  if (bind(fd, (struct sockaddr *) & sin, sizeof(sin)) ||
    listen(fd, TCP_BACKLOG))
    exit(1);
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int n, sock, cc;
  time_t uptime, tcheck, tfresh;
  Agent **FBI, *Scully, *Mulder, *agent;
  fd_set rset;
  struct timeval tv;

  cc = 0;

  while ((n = getopt(argc, argv, "i")) != -1)
  {
    switch (n)
    {
    case 'i':
      cc = 1;
      break;

    default:
      fprintf(stderr, "Usage: %s [options]\n"
	"\t-i  start from inetd with wait option\n",
	argv[0]);
      exit(0);
    }
  }

  servo_daemon(cc);

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  init_bshm();
  init_ushm();
  init_fshm();

  servo_signal();

  log_open();
  dns_init();

  uptime = time(0);
  tcheck = uptime + BHTTP_PERIOD;
  tfresh = uptime + BHTTP_FRESH;

  Scully = Mulder = NULL;

  for (;;)
  {
    /* maintain : resource and garbage collection */

    uptime = time(0);
    if (tcheck < uptime)
    {
      /* ----------------------------------------------- */
      /* �N�L�[�S���ʧ@�� agent ��			 */
      /* ----------------------------------------------- */

      tcheck = uptime - BHTTP_TIMEOUT;

      for (FBI = &Scully; agent = *FBI;)
      {
	if (agent->uptime < tcheck)
	{
	  agent_fire(agent);

	  *FBI = agent->anext;

	  agent->anext = Mulder;
	  Mulder = agent;
	}
	else
	{
	  FBI = &(agent->anext);
	}
      }

      /* ----------------------------------------------- */
      /* maintain server log				 */
      /* ----------------------------------------------- */

      if (tfresh < uptime)
      {
	tfresh = uptime + BHTTP_FRESH;
#ifdef SERVER_USAGE
	servo_usage();
#endif
	log_fresh();
      }
      else
      {
	fflush(flog);
      }

      tcheck = uptime + BHTTP_PERIOD;
    }

    /* ------------------------------------------------- */
    /* Set up the fdsets				 */
    /* ------------------------------------------------- */

    FD_ZERO(&rset);
    FD_SET(0, &rset);

    n = 0;
    for (agent = Scully; agent; agent = agent->anext)
    {
      sock = agent->sock;

      if (n < sock)
	n = sock;

      FD_SET(sock, &rset);
    }
    if (n < 0)		/* no active agent and ready to die */
      break;

    /* in order to maintain history, timeout every BHTTP_PERIOD seconds in case no connections */
    tv.tv_sec = BHTTP_PERIOD;
    tv.tv_usec = 0;
    if (select(n + 1, &rset, NULL, NULL, &tv) <= 0)
      continue;

    /* ------------------------------------------------- */
    /* serve active agents				 */
    /* ------------------------------------------------- */

    uptime = time(0);

    for (FBI = &Scully; agent = *FBI;)
    {
      sock = agent->sock;

      if (FD_ISSET(sock, &rset))
	cc = agent_recv(agent);
      else
	cc = -1;

      if (cc == 0)
      {
	agent_fire(agent);

	*FBI = agent->anext;

	agent->anext = Mulder;
	Mulder = agent;

	continue;
      }

      if (cc > 0)		/* �٦���ƭn recv */
	agent->uptime = uptime;

      FBI = &(agent->anext);
    }

    /* ------------------------------------------------- */
    /* serve new connection				 */
    /* ------------------------------------------------- */

    /* Thor.000209: �Ҽ{���e������, �K�o�d�b accept() */
    if (FD_ISSET(0, &rset))
    {
      unsigned int ip_addr;

      sock = agent_accept(&ip_addr);
      if (sock > 0)
      {
	if (agent = Mulder)
	  Mulder = agent->anext;
	else
	  agent = (Agent *) malloc(sizeof(Agent));

	*FBI = agent;

	/* variable initialization */

	memset(agent, 0, sizeof(Agent));

	agent->sock = sock;
	agent->tbegin = agent->uptime = uptime;

	agent->ip_addr = ip_addr;

	agent->data = (char *) malloc(MIN_DATA_SIZE);
	agent->size = MIN_DATA_SIZE;
	agent->used = 0;

	agent->fpw = fdopen(sock, "w");
      }
    }

    /* ------------------------------------------------- */
    /* tail of main loop				 */
    /* ------------------------------------------------- */
  }

  logit("EXIT", "shutdown");
  fclose(flog);

  exit(0);
}
