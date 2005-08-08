/*-------------------------------------------------------*/
/* bhttpd.c		( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : BBS's HTTP daemon				 */
/* create : 03/07/09					 */
/* update : 04/02/21					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0   /* 連結一覽表 */

  http://my.domain/                            首頁
  http://my.domain/boardlist                   看板列表
  http://my.domain/favorlist                   我的最愛
  http://my.domain/userlist                    使用者名單
  http://my.domain/brd/brdname&##              文章列表，列出看板 [brdname] 編號 ## 開始的 50 篇文章
  http://my.domain/gem/brdname&folder          精華區列表，列出看板 [brdname] 精華區中 folder 這個卷宗下的所有東西
  http://my.domain/mbox/##                     信箱列表，列出信箱中編號 ## 開始的 50 篇文章
  http://my.domain/brdmore/brdname&##          閱讀看板文章，閱讀看板 [brdname] 的第 ## 篇文章
  http://my.domain/brdmost/brdname&##          閱讀看板文章，閱讀看板 [brdname] 中所有名第 ## 篇同標題的文章
  http://my.domain/gemmore/brdname&folder&##   閱讀精華區文章，閱讀看板 [brdname] 精華區中 folder 這個卷宗下的第 ## 篇文章
  http://my.domain/mboxmore/##                 閱讀信箱文章，閱讀信箱中第 ## 篇文章
  http://my.domain/dopost/brdname              發表文章於看板 [brdname]
  http://my.domain/domail/                     發送信件
  http://my.domain/predelpost/brdname&##&###   詢問確定刪除看板 [brdname] 中第 ## 篇文章 (其 chrono 是 ###)
  http://my.domain/delpost/brdname&##&###      刪除看板 [brdname] 中第 ## 篇文章 (其 chrono 是 ###)
  http://my.domain/markpost/brdname&##&###     標記看板 [brdname] 中第 ## 篇文章 (其 chrono 是 ###)
  http://my.domain/predelmail/##&###           詢問確定刪除信箱中第 ## 篇文章 (其 chrono 是 ###)
  http://my.domain/delmail/##&###              刪除信箱中第 ## 篇文章 (其 chrono 是 ###)
  http://my.domain/markmail/##&###             標記信箱中第 ## 篇文章 (其 chrono 是 ###)
  http://my.domain/query/userid                查詢 userid
  http://my.domain/image/filename              顯示圖檔

#endif


#define _MODES_C_

#include "bbs.h"


#include <sys/wait.h>
#include <netinet/tcp.h>
#include <sys/resource.h>


#define SERVER_USAGE


#define BHTTP_PIDFILE	"run/bhttp.pid"
#define BHTTP_LOGFILE	"run/bhttp.log"


#define BHTTP_PERIOD	(60 * 5)	/* 每 5 分鐘 check 一次 */
#define BHTTP_TIMEOUT	(60 * 10)	/* 超過 10 分鐘的連線就視為錯誤 */
#define BHTTP_FRESH	86400		/* 每 1 天整理一次 */


#define TCP_BACKLOG	3
#define TCP_RCVSIZ	2048


#define MIN_DATA_SIZE	(TCP_RCVSIZ + 3)
#define MAX_DATA_SIZE   262143		/* POST 的大小限制(byte) */


/* Thor.000425: POSIX 用 O_NONBLOCK */

#ifndef O_NONBLOCK
#define M_NONBLOCK  FNDELAY
#else
#define M_NONBLOCK  O_NONBLOCK
#endif

#define HTML_TALL	50	/* 列表一頁 50 篇 */


/*-------------------------------------------------------*/
/* 選單的顏色						 */
/*-------------------------------------------------------*/


#define HCOLOR_BG	"#000000"	/* 背景的顏色 */
#define HCOLOR_TEXT	"#ffffff"	/* 文字的顏色 */
#define HCOLOR_LINK	"#00ffff"	/* 未瀏覽過連結的顏色 */
#define HCOLOR_VLINK	"#c0c0c0"	/* 已瀏覽過連結的顏色 */
#define HCOLOR_ALINK	"#ff0000"	/* 連結被壓下時的顏色 */

#define HCOLOR_NECK	"#000070"	/* 脖子的顏色 */
#define HCOLOR_TIE	"#a000a0"	/* 領帶的顏色 */

#define HCOLOR_BAR	"#808080"	/* 光棒顏色 */


/* ----------------------------------------------------- */
/* SMTP commands					 */
/* ----------------------------------------------------- */


typedef struct
{
  int (*func) ();
  char *cmd;
  int len;		/* strlen(Command.cmd) */
}      Command;


/* ----------------------------------------------------- */
/* client connection structure				 */
/* ----------------------------------------------------- */


#define LEN_COOKIE	(IDLEN + PSWDLEN + 5 + 1)	/* u=userid&p=passwd */


typedef struct Agent
{
  struct Agent *anext;
  int sock;
  int sno;
  int state;

  unsigned int ip_addr;
  char fromhost[60];

  time_t tbegin;		/* 連線開始時間 */
  time_t uptime;		/* 上次下指令的時間 */

  char url[40];
  char *urlp;

  char cookie[LEN_COOKIE];
  int setcookie;

  /* 使用者資料要先 acct_fetch() 才能使用 */
  int userno;
  char userid[IDLEN + 1];
  char username[UNLEN + 1];
  usint userlevel;
  usint ufo;

  /* 所能看到的看板列表 */
  BRD *mybrd[MAXBOARD];
  int total_board;

  /* 所能看到的使用者名單 */
  UTMP *myusr[MAXACTIVE];
  int total_user;

  /* output 用 */
  FILE *fpw;
  char ftemp[64];

  char *data;
  int used;
  int size;			/* 目前 data 所 malloc 的空間大小 */
}     Agent;


static int servo_sno = 0;


/* ----------------------------------------------------- */
/* connection state					 */
/* ----------------------------------------------------- */


#define CS_FREE     0x00
#define CS_RECV     0x01
#define CS_SEND     0x02
#define CS_FLUSH    0x03	/* flush data and quit */


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
/* author : visor.bbs@bbs.yzu.edu.tw			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/* ----------------------------------------------------- */


#define LSTRLEN		4096


#define ATTR_HIGHLIGHT	1	/* 高亮度 */
#define ATTR_UNDER	2	/* 底線 */
#define ATTR_BLINK	4	/* 閃動 */
#define ATTR_ITALIC	8	/* 斜體 */


static void
strip_multicolorword(buf)	/* 將雙色字改成單色字 */
  uschar *buf;
{
  uschar *src;
  uschar *des;
  int in_chi = 0;
  int ch;

  src = des = buf;

  for (; ch = *src; src++)
  {
    if (!in_chi)
    {
      if (ch & 0x80)
	in_chi = 1;
    }
    else
    {
      /* 如果是中文字裡面的 ANSI code 就拋棄 */
      if (ch == KEY_ESC)
      {
        while (1)
	{
	  if (!(ch = *++src))
	    goto end_strip;
	  if (ch == 'm')
	    break;
	}
	continue;
      }

      in_chi = 0;
    }
    *des++ = ch;
  }

end_strip:
  *des = '\0';
}


static char *
ansi2color(src, attr, fg, bg)	/* 傳入 ANSI，回傳顏色 */
  char *src;
  int *attr, *fg, *bg;
{
  int color;
  char *str, *end;
  char *ptr, code[ANSILINELEN];	/* 弄這麼長，以免有人暴力 */

  str = src + 2;	/* 跳過 "\033[" */

  if (!(end = (char *) strchr(str, 'm')))
    return src;

  ptr = code;

  do
  {
    if (*str >= '0' && *str <= '7')
    {
      *ptr++ = *str;
      continue;
    }

    *ptr = '\0';
    ptr = code;
    color = atoi(ptr);

    /* 除非是還原碼，否則 ANSI code 有繼承的效果 */

    if (color == 0)
    {
      *fg = 37;
      *bg = 40;
      *attr = 0;
    }
    else if (color >= 30 && color <= 37)
      *fg = color;
    else if (color >= 40 && color <= 47)
      *bg = color;
    else if (color == 1)
      *attr |= ATTR_HIGHLIGHT;
    else if (color == 4)
      *attr |= ATTR_UNDER;
    else if (color == 5)
      *attr |= ATTR_BLINK;
    else if (color == 7)	/* 反白的效果用斜體來代替 */
      *attr |= ATTR_ITALIC;
  } while (++str <= end);

  return end;
}


#define	MOZILLA_SUPPORT		/* 為了支援 Mozilla browser */

#ifdef MOZILLA_SUPPORT
static int fontdirty = 0;		/* 1:在 <font> 裡面 */
#endif


static char *
ansi2tag(dstr, sstr)
  char *dstr, *sstr;
{
  int ch;
  char *src, *dst;

  static int oldattr = 0, attr = 0;
  static int oldfg = 37, fgcolor = 37;	/* 預設前景顏色 */
  static int oldbg = 40, bgcolor = 40;	/* 預設背景顏色 */


  strip_multicolorword(sstr);

  src = sstr;
  dst = dstr;

  for (; ch = *src; src++)
  {
    if (ch != '\033' || *(src + 1) != '[')	/* 不是 ANSI */
    {
      if (ch == '%')	/* itoc.030717: 裡面如果有 '%' 的話，得換成 "%%" */
	*dst++ = '%';

      *dst++ = ch;
      continue;
    }

    /* ------------------------------------------------- */
    /* 開始處理 ANSI -> HTML				 */
    /* ------------------------------------------------- */

    /* 分析顏色 */
    src = ansi2color(src, &attr, &fgcolor, &bgcolor);

#ifndef MOZILLA_SUPPORT
    /* 顏色不同才需要印出 */
    if (oldfg != fgcolor)
    {
      sprintf(dst, "<font class=col%d%d>", attr & ATTR_HIGHLIGHT, fgcolor);
      dst += 19;
      oldfg = fgcolor;
    }
    if (oldbg != bgcolor)
    {
      sprintf(dst, "<font class=col0%d>", bgcolor);
      dst += 19;
      oldbg = bgcolor;
    }
#else
    /* itoc.030720: 為了 Mozilla 要求一定要用 </font>，所以只得這樣做 */

    /* 顏色不同才需要印出 */
    if (oldfg != fgcolor || oldbg != bgcolor)
    {
      if (fontdirty)
      {
	sprintf(dst, "</font></font>");
	dst += 14;
      }
      sprintf(dst, "<font class=col%d%d><font class=col0%d>", attr & ATTR_HIGHLIGHT, fgcolor, bgcolor);
      dst += 38;
      oldfg = fgcolor;
      oldbg = bgcolor;
      fontdirty = 1;
    }
#endif

    /* 屬性不同才需要印出 */
    if (oldattr != attr)
    {
      if ((attr & ATTR_ITALIC) && !(oldattr & ATTR_ITALIC))
      {
	strcpy(dst, "<I>");
	dst += 3;
      }
      else if (!(attr & ATTR_ITALIC) && (oldattr & ATTR_ITALIC))
      {
	strcpy(dst, "</I>");
	dst += 4;
      }
      if ((attr & ATTR_UNDER) && !(oldattr & ATTR_UNDER))
      {
	strcpy(dst, "<U>");
	dst += 3;
      }
      else if (!(attr & ATTR_UNDER) && (oldattr & ATTR_UNDER))
      {
	strcpy(dst, "</U>");
	dst += 4;
      }
      if ((attr & ATTR_BLINK) && !(oldattr & ATTR_BLINK))
      {
	strcpy(dst, "<BLINK>");
	dst += 7;
      }
      else if (!(attr & ATTR_BLINK) && (oldattr & ATTR_BLINK))
      {
	strcpy(dst, "</BLINK>");
	dst += 8;
      }
      oldattr = attr;
    }
  }		/* end for() */

  *dst = '\0';

  return dstr;
}


static char *
quotebuf(buf)
  char *buf;
{
  int ch1, ch2;
  char tmp[ANSILINELEN];

  ch1 = buf[0];
  ch2 = buf[1];

  /* 如果是引言，就略過所有的 ANSI 碼 */
  if (ch2 == ' ' && (ch1 == QUOTE_CHAR1 || ch1 == QUOTE_CHAR2))	/* 引言 */
  {
    ch1 = buf[2];
    str_ansi(tmp, buf, sizeof(tmp) - 8);
    sprintf(buf, "\033[3%cm%s\033[m\n", (ch1 == QUOTE_CHAR1 || ch1 == QUOTE_CHAR2) ? '3' : '6', tmp);	/* 引用一層/二層不同顏色 */
  }
  else if (ch1 == '�' && ch2 == '�')	/* ※ 引言者 */
  {
    str_ansi(tmp, buf, sizeof(tmp) - 10);
    sprintf(buf, "\033[1;36m%s\033[m\n", tmp);
  }

  return buf;
}


#define LINE_HEADER	3	/* 檔頭有三行 */


static void
txt2htm(ap, fp)
  Agent *ap;
  FILE *fp;
{
  int i;
  char buf[ANSILINELEN], encodebuf[LSTRLEN];
  char header1[LINE_HEADER][LEN_AUTHOR1] = {"作者",   "標題",   "時間"};
  char header2[LINE_HEADER][LEN_AUTHOR2] = {"發信人", "標  題", "發信站"};
  char *headvalue, *pbrd, board[128];
  FILE *fpw;

  pbrd = NULL;
  fpw = ap->fpw;

  fprintf(fpw, "<table width=800 cellspacing=0 cellpadding=0 border=0>\r\n");

  /* 處理檔頭 */
  for (i = 0; i < LINE_HEADER; i++)
  {
    if (!fgets(buf, sizeof(buf), fp))	/* 雖然連檔頭都還沒印完，但是檔案已經結束，直接離開 */
    {
      fprintf(fpw, "</table>\r\n");
      return;
    }

    if (memcmp(buf, header1[i], LEN_AUTHOR1 - 1) && memcmp(buf, header2[i], LEN_AUTHOR2 - 1))	/* 不是檔頭 */
      break;

    /* 作者/看板 檔頭有二欄，特別處理 */
    if (i == 0 && (strstr(buf, "看板:") || strstr(buf, "站內:")))
    {
      if (pbrd = strrchr(buf, ':'))
      {
	*pbrd = '\0';
	strcpy(board, pbrd + 2);
	pbrd -= 5;
	*pbrd++ = '\0';
      }
    }

    if (!(headvalue = strchr(buf, ':')))
      break;

    *headvalue = '\0';
    headvalue = ansi2tag(encodebuf, headvalue + 2);

    if (i == 0 && pbrd)
    {
      fprintf(fpw, "<tr>\r\n"
	"  <td align=center width=10%% class=col047><font class=col034>%s</font></td>\r\n"
	"  <td width=60%% class=col044><font class=col037>&nbsp;%s</font></td>\r\n"
	"  <td align=center width=10%% class=col047><font class=col034>%s</font></td>\r\n"
	"  <td width=20%% class=col044><font class=col037>&nbsp;%s</font></td>\r\n"
	"</tr>\r\n",
	header1[i],
	headvalue,
	pbrd,
	board);
    }
    else
    {
      fprintf(fpw, "<tr>\r\n"
	"  <td align=center width=10%% class=col047><font class=col034>%s</font></td>\r\n"
	"  <td width=90%% COLSPAN=3 class=col044><font class=col037>&nbsp;%s</font></td>\r\n"
	"</tr>\r\n",
	header1[i],
	headvalue);
    }
  }

  fprintf(fpw, "<tr>\r\n"
    "<td colspan=4><pre>\r\n");

  if (i < LINE_HEADER)		/* 最後一行不是檔頭 */
  {
    ansi2tag(encodebuf, quotebuf(buf));
    fprintf(fpw, encodebuf);
  }

  /* 處理內文 */
  while (fgets(buf, sizeof(buf), fp))
  {
    ansi2tag(encodebuf, quotebuf(buf));
    fprintf(fpw, encodebuf);
  }

#ifdef MOZILLA_SUPPORT
  if (fontdirty)
    fprintf(fpw, "</font></font>");
#endif

  fprintf(fpw, "</pre></td>\r\n"
    "</table>\r\n");  
}


static void 
show_style(ap)
  Agent *ap;
{
  fprintf(ap->fpw, "<style type=text/css>\r\n"
    "  PRE         {font-size: 15pt; line-height: 15pt; font-weight: lighter; background-color: 000000; COLOR: c0c0c0;}\r\n"
    "  TD          {font-size: 15pt; line-height: 15pt; font-weight: lighter;}\r\n"
    "  FONT.col030 {COLOR: 000000;}\r\n"
    "  FONT.col031 {COLOR: a00000;}\r\n"
    "  FONT.col032 {COLOR: 00a000;}\r\n"
    "  FONT.col033 {COLOR: a0a000;}\r\n"
    "  FONT.col034 {COLOR: 0000a0;}\r\n"
    "  FONT.col035 {COLOR: a000a0;}\r\n"
    "  FONT.col036 {COLOR: 00a0a0;}\r\n"
    "  FONT.col037 {COLOR: c0c0c0;}\r\n"
    "  FONT.col040 {background-color: 000000;}\r\n"
    "  FONT.col041 {background-color: a00000;}\r\n"
    "  FONT.col042 {background-color: 00a000;}\r\n"
    "  FONT.col043 {background-color: a0a000;}\r\n"
    "  FONT.col044 {background-color: 0000a0;}\r\n"
    "  FONT.col045 {background-color: a000a0;}\r\n"
    "  FONT.col046 {background-color: 00a0a0;}\r\n"
    "  FONT.col047 {background-color: c0c0c0;}\r\n"
    "  FONT.col130 {COLOR: 606060;}\r\n"
    "  FONT.col131 {COLOR: ff0000;}\r\n"
    "  FONT.col132 {COLOR: 00ff00;}\r\n"
    "  FONT.col133 {COLOR: ffff00;}\r\n"
    "  FONT.col134 {COLOR: 0000ff;}\r\n"
    "  FONT.col135 {COLOR: ff00ff;}\r\n"
    "  FONT.col136 {COLOR: 00ffff;}\r\n"
    "  FONT.col137 {COLOR: e0e0e0;}\r\n"
    "  TD.col044   {background-color: 0000a0;}\r\n"
    "  TD.col047   {background-color: c0c0c0;}\r\n"
    "</style>\r\n");
}


/* ----------------------------------------------------- */
/* HTML output basic function				 */
/* ----------------------------------------------------- */


static void agent_write();


static char *
Gtime(now)
  time_t *now;
{
  static char datemsg[40];

  strftime(datemsg, sizeof(datemsg), "%a, %d %b %Y %T GMT", gmtime(now));
  return datemsg;
}


/* out_head() 中的 <HTML> <BODY> <CENTER> 三個大寫標籤貫穿整個 html 檔
   直到 out_tail() 才由 </HTML> </BODY> </CENTER> 還原 */


static void
out_head(ap, title)
  Agent *ap;
  char *title;
{
  time_t now;
  FILE *fpw;

  fpw = ap->fpw;

  /* HTTP 1.0 檔頭 */
  time(&now);
  fprintf(fpw, "HTTP/1.0 200 OK\r\n");
  fprintf(fpw, "Date: %s\r\n", Gtime(&now));
  fprintf(fpw, "Server: MapleBBS 3.10\r\n");
  if (ap->setcookie)	/* acct_login() 完以後才需要 Set-Cookie */
    fprintf(fpw, "Set-Cookie: user=%s; path=/\r\n", ap->cookie);
  fprintf(fpw, "Content-Type: text/html; charset="MYCHARSET"\r\n");
  fprintf(fpw, "\r\n");

  /* html 檔案開始 */
  fprintf(fpw, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
  fprintf(fpw, "<HTML>\r\n"
    "<style type=text/css>\r\n"
    "  pre {font-size: 15pt; line-height: 15pt; font-weight: lighter; background-color: 000000; color: c0c0c0;}\r\n"
    "  td  {font-size: 15pt; line-height: 15pt; font-weight: lighter;}\r\n"
    "</style>\r\n");

  fprintf(fpw, "<head>\r\n"
    "  <meta http-equiv=Content-Type content=\"text/html; charset="MYCHARSET"\">\r\n"
    "  <title>【"BBSNAME"】%s</title>\r\n"
    "</head>\r\n", title);
  
  fprintf(fpw, "<BODY bgcolor="HCOLOR_BG" text="HCOLOR_TEXT" link="HCOLOR_LINK" vlink="HCOLOR_VLINK" alink="HCOLOR_ALINK"><CENTER>\r\n");

  fprintf(fpw, "<a href=/><img src=/image/site.jpg border=0></a><br><br>\r\n"
    "<input type=image src=/image/up.jpg onclick=history.go(-1);return;> / "
    "<a href=/boardlist><img src=/image/boardlist.jpg border=0></a> / "
    "<a href=/favorlist><img src=/image/favorlist.jpg border=0></a> / "
    "<a href=/mbox/0><img src=/image/mboxlist.jpg border=0></a> / "
    "<a href=/userlist><img src=/image/userlist.jpg border=0></a> / "
    "<a href=telnet://"MYHOSTNAME"><img src=/image/telnet.jpg border=0></a> / "
    "<a href=mailto:"STR_SYSOP".bbs@"MYHOSTNAME"><img src=/image/mailsysop.jpg border=0></a>"
    "<br>\r\n");
}


static void
out_head2(ap, title)		/* 和 out_head() 類似，錯誤時使用 */
  Agent *ap;
  char *title;
{
  time_t now;
  FILE *fpw;

  fpw = ap->fpw;

  /* HTTP 1.0 檔頭 */
  time(&now);
  fprintf(fpw, "HTTP/1.0 400 Bad Request\r\n");
  fprintf(fpw, "Date: %s\r\n", Gtime(&now));
  fprintf(fpw, "Server: MapleBBS 3.10\r\n");
  fprintf(fpw, "Connection: close\r\n");
  fprintf(fpw, "Content-Type: text/html; charset="MYCHARSET"\r\n");
  fprintf(fpw, "\r\n");

  /* html 檔案開始 */
  fprintf(fpw, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\r\n");
  fprintf(fpw, "<HTML>\r\n");

  fprintf(fpw, "<head>\r\n"
    "  <meta http-equiv=Content-Type content=\"text/html; charset="MYCHARSET"\">\r\n"
    "  <title>【"BBSNAME"】%s</title>\r\n"
    "</head>\r\n", title);
  
  fprintf(fpw, "<BODY><CENTER>\r\n");
}


static void
out_mesg(ap, msg)
  Agent *ap;
  char *msg;
{
  fprintf(ap->fpw, "<br>%s<br><br>\r\n", msg);
}


static void
out_reload(ap, msg)		/* God.050327: 將主視窗 reload 並關掉新開視窗 */
  Agent *ap;
  char *msg;
{
  fprintf(ap->fpw, "<BODY onLoad='opener.location.reload(); parent.close();'>\n");
  fprintf(ap->fpw, "<script language=VBScript>MsgBox \"%s\",vbInformation+vbOKonly,\"確認\"</script>\n", msg);
}


static void
out_article(ap, fpath)
  Agent *ap;
  char *fpath;
{
  FILE *fp;

  if (fp = fopen(fpath, "r"))
  {
    show_style(ap);
    txt2htm(ap, fp);

    fclose(fp);
  }
}


static void
out_tail(ap)
  Agent *ap;
{
  fprintf(ap->fpw, "</CENTER></BODY></HTML>\r\n");

  agent_write(ap);
}


/* ----------------------------------------------------- */
/* 解碼							 */
/* ----------------------------------------------------- */


#define is_hex(x)	((x >= '0' && x <= '9') || (x >= 'A' && x <= 'F'))
#define hex2int(x)	((x >= 'A') ? (x - 'A' + 10) : (x - '0'))


static void
str_dehtml(src, dst, size)
  char *src, *dst;
  int size;
{
  int ch, len;
  char *s, *d;

  len = 1;	/* 保留 1 給 '\0' */
  s = src;
  d = dst;
  while (ch = *s++)
  {
    if (ch == '+')
    {
      ch = ' ';
    }
    else if (ch == '%')
    {
      if (is_hex(s[0]) && is_hex(s[1]))
      {
	ch = (hex2int(s[0]) << 4) + hex2int(s[1]);
	s += 2;
	if (ch == '\r')		/* '\r' 就不要了 */
	  continue;
      }
    }

    *d++ = ch;
    if (++len >= size)
      break;
  }
  *d = '\0';
}


/*-------------------------------------------------------*/
/* 分析參數						 */
/*-------------------------------------------------------*/


static int			/* 1:成功 */
arg_analyze(argc, str, arg1, arg2, arg3, arg4)
  int argc;		/* 有幾個參數 */
  char *str;		/* 引數 */
  char **arg1;		/* 參數一 */
  char **arg2;		/* 參數二 */
  char **arg3;		/* 參數三 */
  char **arg4;		/* 參數四 */
{
  int i;
  char *ptr;

  ptr = str;
  for (i = 1; i <= argc; i++)
  {
    if (!*ptr)
      break;

    if (i == 1)
      *arg1 = ptr;
    else if (i == 2)
      *arg2 = ptr;
    else if (i == 3)
      *arg3 = ptr;
    else /* if (i == 4) */	/* 最多四個參數 */
    {
      *arg4 = ptr;
      continue;		/* 跳過 do-while */
    }

    do
    {
      if (*ptr == '&')
      {
	*ptr++ = '\0';
	break;;
      }
      ptr++;
    } while (*ptr);
  }

  return i > argc;
}   


/*-------------------------------------------------------*/
/* 由 Cookie 看使用者是否登入				 */
/*-------------------------------------------------------*/


static int		/* 1:登入成功 0:登入失敗 */
acct_fetch(ap)
  Agent *ap;
{
  char *userid, *passwd;
  char fpath[64], dst[LEN_COOKIE];
  ACCT acct;

  if (ap->cookie[0])
  {
    str_dehtml(ap->cookie, dst, sizeof(dst));

    /* u=userid&p=passwd */

    if (!arg_analyze(2, dst, &userid, &passwd, NULL, NULL))
      return;

    userid += 2;	/* skip "u=" */
    passwd += 2;	/* skip "p=" */
    
    if (*userid && *passwd && strlen(userid) <= IDLEN && strlen(passwd) <= PSWDLEN)
    {
      usr_fpath(fpath, userid, FN_ACCT);
      if (!rec_get(fpath, &acct, sizeof(ACCT), 0) &&
	!(acct.userlevel & (PERM_DENYLOGIN | PERM_PURGE)) &&
	!chkpasswd(acct.passwd, passwd))			/* 登入成功 */
      {
	ap->userno = acct.userno;
	strcpy(ap->userid, acct.userid);
	strcpy(ap->username, acct.username);
	ap->userlevel = acct.userlevel;
	ap->ufo = acct.ufo;
	return 1;
      }
    }
  }

  /* 沒有登入、登入失敗 */
  ap->userno = 0;
  strcpy(ap->userid, STR_GUEST);
  strcpy(ap->username, STR_GUEST);
  ap->userlevel = 0;
  ap->ufo = UFO_DEFAULT_GUEST;
  return 0;
}


/*-------------------------------------------------------*/
/* UTMP shm 部分須與 cache.c 相容			 */
/*-------------------------------------------------------*/


static UCACHE *ushm;


static void
init_ushm()
{
  ushm = shm_new(UTMPSHM_KEY, sizeof(UCACHE));
}


static int			/* 1: userno 在 pool 名單上 */
pertain_pal(pool, max, userno)	/* 參考 pal.c:belong_pal() */
  int *pool;
  int max;
  int userno;
{
  int *up, datum, mid;

  up = pool;
  while (max > 0)
  {
    datum = up[mid = max >> 1];
    if (userno == datum)
    {
      return 1;
    }
    if (userno > datum)
    {
      up += (++mid);
      max -= mid;
    }
    else
    {
      max = mid;
    }
  }
  return 0;
}


static int			/* 1: 對方設我為壞人 */
is_hisbad(ap, up)		/* 參考 pal.c:is_obad() */
  Agent *ap;
  UTMP *up;
{
#ifdef HAVE_BADPAL
  return pertain_pal(up->pal_spool, up->pal_max, -ap->userno);
#else
  return 0;
#endif
}


static int			/* 1:可看見 0:不可看見 */
can_seen(ap, up)		/* 參考 bmw.c:can_see() */
  Agent *ap;
  UTMP *up;
{
  usint mylevel, myufo, urufo;

  mylevel = ap->userlevel;
  myufo = ap->ufo;
  urufo = up->ufo;

  if (!(mylevel & PERM_SEECLOAK) && ((urufo & UFO_CLOAK) || is_hisbad(ap, up)))
    return 0;

#ifdef HAVE_SUPERCLOAK
  if (!(myufo & UFO_SUPERCLOAK) && (urufo & UFO_SUPERCLOAK))
    return 0;
#endif

  return 1;
}


/* itoc.030711: 加上檢查使用者帳號的部分，以免有人亂踹 */

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
/* board：shm 部份須與 cache.c 相容			 */
/* ----------------------------------------------------- */


static BCACHE *bshm;


static void
init_bshm()
{
  /* itoc.030727: 在開啟 bbsd 之前，應該就要執行過 account，
     所以 bshm 應該已設定好 */

  if (bshm)
    return;

  bshm = shm_new(BRDSHM_KEY, sizeof(BCACHE));

  if (bshm->uptime <= 0)	/* bshm 未設定完成 */
    exit(0);
}


#ifdef HAVE_MODERATED_BOARD
static int		/* !=0:是板好  0:不在名單中 */
is_brdgood(ap, bpal)	/* 參考 pal.c:is_bgood() */
  Agent *ap;
  BPAL *bpal;
{
  return pertain_pal(bpal->pal_spool, bpal->pal_max, ap->userno);
}


static int		/* !=0:是板壞  0:不在名單中 */
is_brdbad(ap, bpal)	/* 參考 pal.c:is_bbad() */
  Agent *ap;
  BPAL *bpal;
{
#ifdef HAVE_BADPAL
  return pertain_pal(bpal->pal_spool, bpal->pal_max, -ap->userno);
#else
  return 0;
#endif
}
#endif


static int
Ben_Perm(ap, bno, ulevel)	/* 參考 board.c:Ben_Perm() */
  Agent *ap;
  int bno;
  usint ulevel;
{
  BRD *brd;
  usint readlevel, postlevel, bits;
  char *blist, *bname;
#ifdef HAVE_MODERATED_BOARD
  BPAL *bpal;
  int ftype;	/* 0:一般ID 1:板好 2:板壞 */

  /* itoc.040103: 看板閱讀等級說明表

  ┌────┬────┬────┬────┐
  │        │一般用戶│看板好友│看板壞人│
  ├────┼────┼────┼────┤
  │一般看板│權限決定│  水桶  │ 看不見 │    看不見：在看板列表中無法看到這個板，也進不去
  ├────┼────┼────┼────┤    進不去：在看板列表中可以看到這個板，但是進不去
  │好友看板│ 進不去 │  完整  │  水桶  │    水  桶：在看板列表中可以看到這個板，也進得去，但是不能發文
  ├────┼────┼────┼────┤    完  整：在看板列表中可以看到這個板，也進得去及發文
  │秘密看板│ 看不見 │  完整  │  水桶  │
  └────┴────┴────┴────┘
  */

  static int bit_data[9] =
  {                /* 一般用戶   看板好友                           看板壞人 */
    /* 公開看板 */    0,         BRD_L_BIT | BRD_R_BIT,             0,
    /* 好友看板 */    BRD_L_BIT, BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT,
    /* 秘密看板 */    0,         BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT,
  };
#endif

  brd = bshm->bcache + bno;
  bname = brd->brdname;
  if (!*bname)
    return 0;

  readlevel = brd->readlevel;

#ifdef HAVE_MODERATED_BOARD
  bpal = bshm->pcache + bno;
  ftype = is_brdgood(ap, bpal) ? 1 : is_brdbad(ap, bpal) ? 2 : 0;

  if (readlevel == PERM_SYSOP)		/* 秘密看板 */
    bits = bit_data[6 + ftype];
  else if (readlevel == PERM_BOARD)	/* 好友看板 */
    bits = bit_data[3 + ftype];
  else if (ftype)			/* 公開看板，若在板好/板壞名單中 */
    bits = bit_data[ftype];
  else					/* 公開看板，其他依權限判定 */
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

  /* Thor.980813.註解: 特別為 BM 考量，板主有該板的所有權限 */
  blist = brd->BM;
  if ((ulevel & PERM_BM) && blist[0] > ' ' && str_has(blist, ap->userid, strlen(ap->userid)))
    bits = BRD_L_BIT | BRD_R_BIT | BRD_W_BIT | BRD_X_BIT | BRD_M_BIT;

  /* itoc.030515: 看板總管重新判斷 */
  else if (ulevel & PERM_ALLBOARD)
    bits = BRD_L_BIT | BRD_R_BIT | BRD_W_BIT | BRD_X_BIT | BRD_M_BIT;

  return bits;
}


static BRD *
allow_brdname(ap, brdname, bits, warn)
  Agent *ap;
  char *brdname;
  usint bits;		/* 若 bits != BRD_R_BIT，會順便做 acct_fetch() */
  int warn;		/* 1: 若 disallow 要給 warning */
{
  BRD *bcache, *bhdr, *tail;

  bcache = bshm->bcache;
  bhdr = bcache;
  tail = bcache + bshm->number;

  do
  {
    if (!strcmp(bhdr->brdname, brdname))
    {
      /* 若 readlevel == 0，表示 guest 可讀，無需 acct_fetch() */
      if (bits == BRD_R_BIT && !bhdr->readlevel)
	return bhdr;

      acct_fetch(ap);
      if (Ben_Perm(ap, bhdr - bcache, ap->userlevel) & bits)
	return bhdr;

      break;
    }
  } while (++bhdr < tail);

  if (warn)
    out_mesg(ap, "操作錯誤，可能原因有：無此看板、您尚未登入、您的權限不足");

  return NULL;
}


/* ----------------------------------------------------- */
/* command dispatch (GET)				 */
/* ----------------------------------------------------- */


  /*-----------------------------------------------------*/
  /* 使用者名單						 */
  /*-----------------------------------------------------*/


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
  int num;
  UTMP *uentp, *uceil;

  acct_fetch(ap);

  uentp = ushm->uslot;
  uceil = (void *) uentp + ushm->offset;
  num = 0;

  do
  {
    if (!uentp->pid || !uentp->userno || !can_seen(ap, uentp))
      continue;

    ap->myusr[num] = uentp;
    num++;
  } while (++uentp <= uceil);

  if (num > 1)
    qsort(ap->myusr, num, sizeof(UTMP *), userid_cmp);

  ap->total_user = num;
}


static void
userlist_neck(ap)
  Agent *ap;
{
  fprintf(ap->fpw, "<br>\r\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=100%% align=center bgcolor="HCOLOR_NECK">目前站上有 %d 個人</td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    ap->total_user);
}


static int
cmd_userlist(ap)
  Agent *ap;
{
  int i;
  UTMP *up;
  FILE *fpw;

  init_myusr(ap);

  out_head(ap, "使用者名單");

  userlist_neck(ap);

  fpw = ap->fpw;

  fprintf(fpw, "<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE">編號</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE">網友代號</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE">網友暱稱</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE">客途故鄉</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE">網友動態</td>\r\n"
    "</tr>\r\n");

  for (i = 0; i < ap->total_user; i++)
  {
    up = ap->myusr[i];
    fprintf(fpw, "<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
      "  <td>%d</td>\r\n"
      "  <td><a href=/query/%s>%s</a></td>\r\n"
      "  <td>%s</td>\r\n"
      "  <td>%s</td>\r\n"
      "  <td>%s</td>\r\n"
      "</tr>\r\n",
      i + 1,
      up->userid, up->userid,
      up->username,
      up->from,
      ModeTypeTable[up->mode]);
  }

  fprintf(fpw, "</table>\r\n");

  userlist_neck(ap);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 看板列表						 */
  /*-----------------------------------------------------*/


static int
brdtitle_cmp(a, b)
  BRD **a, **b;
{
  /* itoc.010413: 分類/板名交叉比對 */
  int k = strcmp((*a)->class, (*b)->class);
  return k ? k : str_cmp((*a)->brdname, (*b)->brdname);
}


static void
init_mybrd(ap)
  Agent *ap;
{
  int num, bno, max;
  usint ulevel;
  BRD *bhdr;

  acct_fetch(ap);

  ulevel = ap->userlevel;
  bhdr = bshm->bcache;
  bno = 0;
  max = bshm->number;
  num = 0;

  do
  {
    if (Ben_Perm(ap, bno, ulevel) & BRD_R_BIT)
    {
      ap->mybrd[num] = bhdr;
      num++;
    }
    bhdr++;
  } while (++bno < max);

  if (num > 1)
    qsort(ap->mybrd, num, sizeof(BRD *), brdtitle_cmp);

  ap->total_board = num;
}


static void
boardlist_neck(ap)
  Agent *ap;
{
  fprintf(ap->fpw, "<br>\r\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=100%% align=center bgcolor="HCOLOR_NECK">目前站上有 %d 個板</td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    ap->total_board);
}


static int
cmd_boardlist(ap)
  Agent *ap;
{
  int i;
  BRD *brd;
  FILE *fpw;

  init_mybrd(ap);

  out_head(ap, "看板列表");

  boardlist_neck(ap);

  fpw = ap->fpw;

  fprintf(fpw, "<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=40>編號</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=80>看板</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=40>類別</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=25>轉</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=350>中文敘述</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=75>板主</td>\r\n"
    "</tr>\r\n");

  for (i = 0; i < ap->total_board; i++)
  {
    brd = ap->mybrd[i];
    fprintf(fpw, "<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
      "  <td>%d</td>\r\n"
      "  <td><a href=/brd/%s&0>%s</a></td>\r\n"
      "  <td>%s</td>\r\n"
      "  <td>%s</td>\r\n"
      "  <td>%.33s</td>\r\n"
      "  <td>%.13s</td>\r\n"
      "</tr>\r\n",
      i + 1,
      brd->brdname, brd->brdname,
      brd->class,
      (brd->battr & BRD_NOTRAN) ? ICON_NOTRAN_BRD : ICON_TRAN_BRD,
      brd->title,
      brd->BM);
  }

  fprintf(fpw, "</table>\r\n");

  boardlist_neck(ap);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 我的最愛						 */
  /*-----------------------------------------------------*/


static void
init_myfavor(ap)
  Agent *ap;
{
  int num, bno, max;
  usint ulevel;
  char fpath[64];
  BRD *bhdr;
  FILE *fp;
  MF mf;

  acct_fetch(ap);

  ulevel = ap->userlevel;
  max = bshm->number;
  num = 0;
  usr_fpath(fpath, ap->userid, "MF/"FN_MF);

  if (fp = fopen(fpath, "r"))
  {
    while (fread(&mf, sizeof(MF), 1, fp) == 1)
    {
      /* 只支援第一層的看板 */
      if (mf.mftype & MF_BOARD)
      {
	bhdr = bshm->bcache;
	bno = 0;

	do
	{
	  if (!strcmp(bhdr->brdname, mf.xname) && (Ben_Perm(ap, bno, ulevel) & BRD_R_BIT))
	  {
	    ap->mybrd[num] = bhdr;
	    num++;
	  }
	  bhdr++;
	} while (++bno < max);
      }
    }
    fclose(fp);
  }

#if 0	/* 我的最愛依使用者自己的排序 */
  if (num > 1)
    qsort(mybrd, num, sizeof(BRD *), brdtitle_cmp);
#endif

  ap->total_board = num;
}


static void
favorlist_neck(ap)
  Agent *ap;
{
  fprintf(ap->fpw, "<br>\r\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=100%% align=center bgcolor="HCOLOR_NECK">我的最愛</td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n");
}


static int
cmd_favorlist(ap)
  Agent *ap;
{
  int i;
  BRD *brd;
  FILE *fpw;

  init_myfavor(ap);

  out_head(ap, "我的最愛");

  favorlist_neck(ap);

  fpw = ap->fpw;

  fprintf(fpw, "<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=40>編號</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=80>看板</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=40>類別</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=25>轉</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=350>中文敘述</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=75>板主</td>\r\n"
    "</tr>\r\n");

  for (i = 0; i < ap->total_board; i++)
  {
    brd = ap->mybrd[i];
    fprintf(fpw, "<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
      "  <td>%d</td>\r\n"
      "  <td><a href=/brd/%s&0>%s</a></td>\r\n"
      "  <td>%s</td>\r\n"
      "  <td>%s</td>\r\n"
      "  <td>%.33s</td>\r\n"
      "  <td>%.13s</td>\r\n"
      "</tr>\r\n",
      i + 1,
      brd->brdname, brd->brdname,
      brd->class,
      (brd->battr & BRD_NOTRAN) ? ICON_NOTRAN_BRD : ICON_TRAN_BRD,
      brd->title,
      brd->BM);
  }

  fprintf(fpw, "</table>\r\n");

  favorlist_neck(ap);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 文章列表						 */
  /*-----------------------------------------------------*/


static void
postlist_neck(ap, brdname, start, total)
  Agent *ap;
  char *brdname;
  int start, total;
{
  FILE *fpw = ap->fpw;

  fprintf(fpw, "<br>\r\n"
    "<table cellspacing=0 cellpadding=4 border=0 width=80%%>\r\n"
    "<tr>\r\n");

  if (start > HTML_TALL)
  {
    fprintf(fpw, "  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&%d>上%d篇</a></td>\r\n",
      brdname, start - HTML_TALL, HTML_TALL);
  }
  else
  {
    fprintf(fpw, "  <td width=20%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  start += HTML_TALL;
  if (start <= total)
  {
    fprintf(fpw, "  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&%d>下%d篇</a></td>\r\n",
      brdname, start, HTML_TALL);
  }
  else
  {
    fprintf(fpw, "  <td width=20%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  fprintf(fpw, "  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/dopost/%s target=_blank>發表文章</a></td>\r\n"
    "  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/gem/%s&"FN_DIR">精華區</a></td>\r\n"
    "  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/boardlist>看板列表</a></td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    brdname, 
    brdname, 
    brdname);
}


static int
cmd_postlist(ap)
  Agent *ap;
{
  int fd, start, total;
  char folder[64], owner[80], *brdname, *number, *ptr1, *ptr2;
  HDR hdr;
  FILE *fpw;

  out_head(ap, "文章列表");

  if (!arg_analyze(2, ap->urlp, &brdname, &number, NULL, NULL))
    return -1;

  if (!allow_brdname(ap, brdname, BRD_R_BIT, 1))
    return 0;

  brd_fpath(folder, brdname, FN_DIR);

  start = atoi(number);
  total = rec_num(folder, sizeof(HDR));
  if (start <= 0 || start > total)	/* 超過範圍的話，直接到最後一頁 */
    start = (total - 1) / HTML_TALL * HTML_TALL + 1;

  postlist_neck(ap, brdname, start, total);

  fpw = ap->fpw;

  fprintf(fpw, "<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=15>刪</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=15>標</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=50>編號</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=10>m</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=10>%%</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=50>日期</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=100>作者</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=400>標題</td>\r\n"
    "</tr>\r\n");

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int i, end;
#ifdef HAVE_SCORE
    char score;
#endif

    /* 秀出看板的第 start 篇開始的 HTML_TALL 篇 */
    i = start;
    end = i + HTML_TALL;

    lseek(fd, (off_t) (sizeof(HDR) * (i - 1)), SEEK_SET);

    while (i < end && read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      strcpy(owner, hdr.owner);
      if (ptr1 = strchr(owner, '.'))	/* 站外作者 */
	*(ptr1 + 1) = '\0';
      if (ptr2 = strchr(owner, '@'))	/* 站外作者 */
	*ptr2 = '\0';

#ifdef HAVE_SCORE
      if (hdr.xmode & POST_SCORE)
      {
	score = abs(hdr.score);
	score = score < 10 ? (score + '0') : (score - 10 + 'A');
      }
      else
	score = ' ';
#endif

      fprintf(fpw, "<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	"  <td><a href=/predelpost/%s&%d&%d><img src=/image/delpost.gif border=0></a></td>\r\n"
	"  <td><a href=/markpost/%s&%d&%d><img src=/image/markpost.gif border=0></a></td>\r\n"
	"  <td>%d</td>\r\n"
	"  <td>%c</td>\r\n"
#ifdef HAVE_SCORE
	"  <td><font color='%s'>%c</font></td>\r\n"
#endif
	"  <td>%s</td>\r\n"
	"  <td><a href=%s%s>%s</a></td>\r\n"
	"  <td><a href=/brdmore/%s&%d>%.50s</td>\r\n"
	"</tr>\r\n",
	brdname, i, hdr.chrono,
	brdname, i, hdr.chrono,
	i,
	hdr.xmode & POST_MARKED ? 'm' : ' ',
#ifdef HAVE_SCORE
	hdr.score >= 0 ? "red" : "green", score,
#endif
	hdr.date + 3,
	(ptr1 || ptr2) ? "mailto:" : "/query/", hdr.owner, owner,
	brdname, i, hdr.title);

      i++;
    }
    close(fd);
  }

  fprintf(fpw, "</table>\r\n");

  postlist_neck(ap, brdname, start, total);
  return 0; 
}


  /*-----------------------------------------------------*/
  /* 精華區列表						 */
  /*-----------------------------------------------------*/


static void
gemlist_neck(ap, brdname)
  Agent *ap;
  char *brdname;
{
  fprintf(ap->fpw, "<br>\r\n"
    "<table cellspacing=0 cellpadding=2 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=50%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&0>回到看板</a></td>\r\n"
    "  <td width=50%% align=center bgcolor="HCOLOR_NECK"><a href=/boardlist>看板列表</a></td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    brdname);
}


static int
cmd_gemlist(ap)
  Agent *ap;
{
  int fd, i;
  char folder[64], *brdname, *xname;
  HDR hdr;
  FILE *fpw;

  out_head(ap, "精華區");

  if (!arg_analyze(2, ap->urlp, &brdname, &xname, NULL, NULL))
    return -1;

  if (*xname != 'F' && strlen(xname) != 8 && strcmp(xname, FN_DIR))
    return -1;

  if (!allow_brdname(ap, brdname, BRD_R_BIT, 1))
    return 0;

  gemlist_neck(ap, brdname);

  fpw = ap->fpw;

  fprintf(fpw, "<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=50>編號</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=400>標題</td>\r\n"
    "</tr>\r\n");

  if (*xname == '.')
    sprintf(folder, "gem/brd/%s/%s", brdname, FN_DIR);
  else /* if (*xname == 'F') */
    sprintf(folder, "gem/brd/%s/%c/%s", brdname, xname[7], xname);

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    i = 1;
    while (read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      if (hdr.xmode & GEM_RESTRICT)
      {
	fprintf(fpw, "<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	  "  <td>%d</td>\r\n"
	  "  <td>[唯讀] 資料保密</td>\r\n"
	  "</tr>\r\n", 
	  i);
      }
      else if (hdr.xname[0] == 'A')	/* 文章 */
      {
	fprintf(fpw, "<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	  "  <td>%d</td>\r\n"
	  "  <td><a href=/gemmore/%s&%s&%d>[文章] %s</a></td>\r\n"
	  "</tr>\r\n",
	  i, 
	  brdname, xname, i, hdr.title);
      }
      else if (hdr.xname[0] == 'F')	/* 卷宗 */
      {
	fprintf(fpw, "<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	  "  <td>%d</td>\r\n"
	  "  <td><a href=/gem/%s&%s>[卷宗] %s</a></td>\r\n"
	  "</tr>\r\n",
	  i, 
	  brdname, hdr.xname, hdr.title);
      }
      else				/* 其他類別就不秀了 */
      {
	fprintf(fpw, "<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	  "  <td>%d</td>\r\n"
	  "  <td>[唯讀] 其他資料</td>\r\n"
	  "</tr>\r\n", 
	  i);
      }

      i++;
    }
    close(fd);
  }

  fprintf(fpw, "</table>\r\n");

  gemlist_neck(ap, brdname);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 信箱列表						 */
  /*-----------------------------------------------------*/


static void
mboxlist_neck(ap, start, total)
  Agent *ap;
  int start, total;
{
  FILE *fpw = ap->fpw;

  fprintf(fpw, "<br>\r\n"
    "<table cellspacing=0 cellpadding=4 border=0 width=80%%>\r\n"
    "<tr>\r\n");

  if (start > HTML_TALL)
  {
    fprintf(fpw, "  <td width=33%% align=center bgcolor="HCOLOR_NECK"><a href=/mbox/%d>上%d篇</a></td>\r\n",
      start - HTML_TALL, HTML_TALL);
  }
  else
  {
    fprintf(fpw, "  <td width=33%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  start += HTML_TALL;
  if (start <= total)
  {
    fprintf(fpw, "  <td width=33%% align=center bgcolor="HCOLOR_NECK"><a href=/mbox/%d>下%d篇</a></td>\r\n",
      start, HTML_TALL);
  }
  else
  {
    fprintf(fpw, "  <td width=33%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  fprintf(fpw, "  <td width=34%% align=center bgcolor="HCOLOR_NECK"><a href=/domail/ target=_blank>發送信件</a></td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n");
}


static int
cmd_mboxlist(ap)
  Agent *ap;
{
  int fd, start, total;
  char folder[64], owner[80], *number, *ptr1, *ptr2;
  HDR hdr;
  FILE *fpw;

  out_head(ap, "信箱列表");

  if (!acct_fetch(ap))
  {
    out_mesg(ap, "您尚未登入");
    return 0;
  }

  if (!arg_analyze(1, ap->urlp, &number, NULL, NULL, NULL))
    return -1;

  usr_fpath(folder, ap->userid, FN_DIR);

  start = atoi(number);
  total = rec_num(folder, sizeof(HDR));
  if (start <= 0 || start > total)	/* 超過範圍的話，直接到最後一頁 */
    start = (total - 1) / HTML_TALL * HTML_TALL + 1;

  mboxlist_neck(ap, start, total);

  fpw = ap->fpw;

  fprintf(fpw, "<table cellspacing=0 cellpadding=4 border=0>\r\n"
    "<tr>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=15>刪</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=15>標</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=50>編號</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=10>m</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=50>日期</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=100>作者</td>\r\n"
    "  <td bgcolor="HCOLOR_TIE" width=400>標題</td>\r\n"
    "</tr>\r\n");

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int i, end;

    /* 秀出信箱的第 start 篇開始的 HTML_TALL 篇 */
    i = start;
    end = i + HTML_TALL;

    lseek(fd, (off_t) (sizeof(HDR) * (i - 1)), SEEK_SET);

    while (i < end && read(fd, &hdr, sizeof(HDR)) == sizeof(HDR))
    {
      strcpy(owner, hdr.owner);
      if (ptr1 = strchr(owner, '.'))	/* 站外作者 */
	*(ptr1 + 1) = '\0';
      if (ptr2 = strchr(owner, '@'))	/* 站外作者 */
	*ptr2 = '\0';

      fprintf(fpw, "<tr onMouseOver=\"this.bgColor='"HCOLOR_BAR"'\" onMouseOut=\"this.bgColor='"HCOLOR_BG"'\">\r\n"
	"  <td><a href=/predelmail/%d&%d><img src=/image/delpost.gif border=0></a></td>\r\n"
	"  <td><a href=/markmail/%d&%d><img src=/image/markpost.gif border=0></a></td>\r\n"
	"  <td>%d</td>\r\n"
	"  <td>%c</td>\r\n"
	"  <td>%s</td>\r\n"
	"  <td><a href=%s%s>%s</a></td>\r\n"
	"  <td><a href=/mboxmore/%d>%.50s</a></td>\r\n"
	"</tr>\r\n",
	i, hdr.chrono,
	i, hdr.chrono,
	i,
	hdr.xmode & POST_MARKED ? 'm' : ' ',
	hdr.date + 3,
	(ptr1 || ptr2) ? "mailto:" : "/query/", hdr.owner, owner,
	i, hdr.title);

      i++;
    }
    close(fd);
  }

  fprintf(fpw, "</table>\r\n");

  mboxlist_neck(ap, start, total);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 閱讀看板文章					 */
  /*-----------------------------------------------------*/


static void
brdmore_neck(ap, brdname, pos, total)
  Agent *ap;
  char *brdname;
  int pos, total;
{
  FILE *fpw = ap->fpw;

  fprintf(fpw, "<br>\r\n"
    "<table cellspacing=0 cellpadding=5 border=0 width=80%%>\r\n"
    "<tr>\r\n");

  if (pos > 1)
  {
    fprintf(fpw, "  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brdmore/%s&%d>上一篇</a></td>\r\n",
      brdname, pos - 1);
  }
  else
  {
    fprintf(fpw, "  <td width=20%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  if (pos < total)
  {
    fprintf(fpw, "  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brdmore/%s&%d>下一篇</a></td>\r\n",
      brdname, pos + 1);
  }
  else
  {
    fprintf(fpw, "  <td width=20%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  fprintf(fpw, "  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brdmost/%s&%d target=_blank>同標題</a></td>\r\n",
    brdname, pos);

  fprintf(fpw, "  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/dopost/%s target=_blank>發表文章</a></td>\r\n",
    brdname);

  fprintf(fpw, "  <td width=20%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&0>文章列表</a></td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    brdname);
}


static int
cmd_brdmore(ap)
  Agent *ap;
{
  int fd, pos, total;
  char folder[64], *brdname, *number;
  HDR hdr;

  out_head(ap, "閱\讀看板文章");

  if (!arg_analyze(2, ap->urlp, &brdname, &number, NULL, NULL))
    return -1;

  if (!allow_brdname(ap, brdname, BRD_R_BIT, 1))
    return 0;

  brd_fpath(folder, brdname, FN_DIR);

  if ((pos = atoi(number)) <= 0)
    pos = 1;
  total = rec_num(folder, sizeof(HDR));

  brdmore_neck(ap, brdname, pos, total);

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int find;

    lseek(fd, (off_t) (sizeof(HDR) * (pos - 1)), SEEK_SET);
    find = read(fd, &hdr, sizeof(HDR)) == sizeof(HDR);
    close(fd);

    if (find)
    {
#ifdef HAVE_REFUSEMARK
      if (!(hdr.xmode & POST_RESTRICT))
#endif
      {
	sprintf(folder, "brd/%s/%c/%s", brdname, hdr.xname[7], hdr.xname);
	out_article(ap, folder);
      }
#ifdef HAVE_REFUSEMARK
      else
      {
	out_mesg(ap, "此為加密文章，您無法閱\讀");
      }
#endif
    }
    else
    {
      out_mesg(ap, "您所選取的文章編號已超過本看板所有文章");
    }
  }

  brdmore_neck(ap, brdname, pos, total);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 閱讀看板同標題文章					 */
  /*-----------------------------------------------------*/


static void
do_brdmost(ap, folder, title)
  Agent *ap;
  char *folder, *title;
{
  int fd;
  char fpath[64];
  FILE *fp;
  HDR hdr;

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    show_style(ap);

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
	  txt2htm(ap, fp);
	  fclose(fp);
	}
      }
    }
    close(fd);
  }
}


static void
brdmost_neck(ap, brdname)
  Agent *ap;
  char *brdname;
{
  FILE *fpw = ap->fpw;

  fprintf(fpw, "<br>\r\n"
    "<table cellspacing=0 cellpadding=4 border=0 width=80%%>\r\n"
    "<tr>\r\n");

  fprintf(fpw, "  <td width=100%% align=center bgcolor="HCOLOR_NECK"><a href=/brd/%s&0>文章列表</a></td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    brdname);
}


static int
cmd_brdmost(ap)
  Agent *ap;
{
  int fd, pos;
  char folder[64], *brdname, *number;
  HDR hdr;

  out_head(ap, "閱\讀看板同標題文章");

  if (!arg_analyze(2, ap->urlp, &brdname, &number, NULL, NULL))
    return -1;

  if (!allow_brdname(ap, brdname, BRD_R_BIT, 1))
    return 0;

  brd_fpath(folder, brdname, FN_DIR);

  if ((pos = atoi(number)) <= 0)
    pos = 1;

  brdmost_neck(ap, brdname);

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int find;

    lseek(fd, (off_t) (sizeof(HDR) * (pos - 1)), SEEK_SET);
    find = read(fd, &hdr, sizeof(HDR)) == sizeof(HDR);
    close(fd);

    if (find)
      do_brdmost(ap, folder, str_ttl(hdr.title));
    else
      out_mesg(ap, "您所選取的文章編號已超過本看板所有文章");
  }

  brdmost_neck(ap, brdname);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 閱讀精華區文章					 */
  /*-----------------------------------------------------*/


static void
gemmore_neck(ap, brdname, xname, pos, total)
  Agent *ap;
  char *brdname, *xname;
  int pos, total;
{
  FILE *fpw = ap->fpw;

  fprintf(fpw, "<br>\r\n"
    "<table cellspacing=0 cellpadding=4 border=0 width=80%%>\r\n"
    "<tr>\r\n");

  if (pos > 1)
  {
    fprintf(fpw, "  <td width=30%% align=center bgcolor="HCOLOR_NECK"><a href=/gemmore/%s&%s&%d>上一篇</a></td>\r\n",
      brdname, xname, pos - 1);
  }
  else
  {
    fprintf(fpw, "  <td width=30%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  if (pos < total)
  {
    fprintf(fpw, "  <td width=30%% align=center bgcolor="HCOLOR_NECK"><a href=/gemmore/%s&%s&%d>下一篇</a></td>\r\n",
      brdname, xname, pos + 1);
  }
  else
  {
    fprintf(fpw, "  <td width=30%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  fprintf(fpw, "  <td width=40%% align=center bgcolor="HCOLOR_NECK"><a href=/gem/%s&%s>回到卷宗</a></td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    brdname, xname);
}


static int
cmd_gemmore(ap)
  Agent *ap;
{
  int fd, pos, total;
  char folder[64], *brdname, *xname, *number;
  HDR hdr;

  out_head(ap, "閱\讀精華區文章");

  if (!arg_analyze(3, ap->urlp, &brdname, &xname, &number, NULL))
    return -1;

  if (*xname != 'F' && strlen(xname) != 8 && strcmp(xname, FN_DIR))
    return -1;

  if (!allow_brdname(ap, brdname, BRD_R_BIT, 1))
    return 0;

  if (*xname == '.')
    gem_fpath(folder, brdname, FN_DIR);
  else /* if (*xname == 'F') */
    sprintf(folder, "gem/brd/%s/%c/%s", brdname, xname[7], xname);

  if ((pos = atoi(number)) <= 0)
    pos = 1;
  total = rec_num(folder, sizeof(HDR));

  gemmore_neck(ap, brdname, xname, pos, total);

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int find;

    lseek(fd, (off_t) (sizeof(HDR) * (pos - 1)), SEEK_SET);
    find = read(fd, &hdr, sizeof(HDR)) == sizeof(HDR);
    close(fd);

    if (find)
    {
      if (hdr.xname[0] == 'A')
      {
	if (!(hdr.xmode & GEM_RESTRICT))
	{
	  sprintf(folder, "gem/brd/%s/%c/%s", brdname, hdr.xname[7], hdr.xname);
	  out_article(ap, folder);
	}
	else
	{
	  out_mesg(ap, "此為保密精華區，您無法閱\讀");
	}
      }
      else
      {
	out_mesg(ap, "這是卷宗或唯讀資料，您必須由精華區列表來讀取");
      }
    }
    else
    {
      out_mesg(ap, "您所選取的文章編號已超過本卷宗所有文章");
    }
  }

  gemmore_neck(ap, brdname, xname, pos, total);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 閱讀信箱文章					 */
  /*-----------------------------------------------------*/


static void
mboxmore_neck(ap, pos, total)
  Agent *ap;
  int pos, total;
{
  FILE *fpw = ap->fpw;

  fprintf(fpw, "<br>\r\n"
    "<table cellspacing=0 cellpadding=4 border=0 width=80%%>\r\n"
    "<tr>\r\n");

  if (pos > 1)
  {
    fprintf(fpw, "  <td width=30%% align=center bgcolor="HCOLOR_NECK"><a href=/mboxmore/%d>上一篇</a></td>\r\n",
      pos - 1);
  }
  else
  {
    fprintf(fpw, "  <td width=30%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  if (pos < total)
  {
    fprintf(fpw, "  <td width=30%% align=center bgcolor="HCOLOR_NECK"><a href=/mboxmore/%d>下一篇</a></td>\r\n",
      pos + 1);
  }
  else
  {
    fprintf(fpw, "  <td width=30%% bgcolor="HCOLOR_NECK"></td>\r\n");
  }

  fprintf(fpw, "  <td width=40%% align=center bgcolor="HCOLOR_NECK"><a href=/mbox/0>信箱列表</a></td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n");
}


static int
cmd_mboxmore(ap)
  Agent *ap;
{
  int fd, pos, total;
  char folder[64], fpath[64], *number;
  HDR hdr;

  out_head(ap, "閱\讀信箱文章");

  if (!arg_analyze(1, ap->urlp, &number, NULL, NULL, NULL))
    return -1;

  if (!acct_fetch(ap))
    return -1;

  usr_fpath(folder, ap->userid, FN_DIR);

  if ((pos = atoi(number)) <= 0)
    pos = 1;
  total = rec_num(folder, sizeof(HDR));
  if (pos > total)
    pos = total;

  mboxmore_neck(ap, pos, total);

  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    int find;

    lseek(fd, (off_t) (sizeof(HDR) * (pos - 1)), SEEK_SET);
    find = read(fd, &hdr, sizeof(HDR)) == sizeof(HDR);
    close(fd);

    if (find)
    {
      hdr_fpath(fpath, folder, &hdr);
      out_article(ap, fpath);
    }
  }

  mboxmore_neck(ap, pos, total);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 發表文章						 */
  /*-----------------------------------------------------*/


static int
cmd_dopost(ap)
  Agent *ap;
{
  char *brdname;

  out_head(ap, "發表文章");

  if (!arg_analyze(1, ap->urlp, &brdname, NULL, NULL, NULL))
    return -1;

  if (!allow_brdname(ap, brdname, BRD_W_BIT, 1))
    return 0;

  fprintf(ap->fpw, "<form method=post>\r\n"
    "  <input type=hidden name=dopost>\r\n"
    "  <input type=hidden name=b value=%s>\r\n"
    "  請輸入標題：<br>\r\n"
    "  <input type=text name=t size=%d maxlength=%d><br><br>\r\n"
    "  請輸入內容：<br>\r\n"
    "  <textarea name=c rows=10 cols=%d></textarea><br><br>\r\n"
    "  <input type=hidden name=end>\r\n"
    "  <input type=submit value=送出文章> "
    "  <input type=reset value=重新填寫>"
    "</form>\r\n",
    brdname,
    TTLEN, TTLEN,
    SCR_WIDTH);

  return 0;
}


  /*-----------------------------------------------------*/
  /* 發送信件						 */
  /*-----------------------------------------------------*/


static int
cmd_domail(ap)
  Agent *ap;
{
  out_head(ap, "發送信件");

  fprintf(ap->fpw, "<form method=post>\r\n"
    "  <input type=hidden name=domail>\r\n"
    "  請輸入收信人ＩＤ：<br>\r\n"
    "  <input type=text name=u size=%d maxlength=%d><br><br>\r\n"
    "  請輸入標題：<br>\r\n"
    "  <input type=text name=t size=%d maxlength=%d><br><br>\r\n"
    "  請輸入內容：<br>\r\n"
    "  <textarea name=c rows=10 cols=%d></textarea><br><br>\r\n"
    "  <input type=hidden name=end>\r\n"
    "  <input type=submit value=送出信件> "
    "  <input type=reset value=重新填寫>"
    "</form>\r\n",
    IDLEN, IDLEN,
    TTLEN, TTLEN,
    SCR_WIDTH);

  return 0;
}


  /*-----------------------------------------------------*/
  /* 刪除文章/信件					 */
  /*-----------------------------------------------------*/


static int
cmd_delpost(ap)
  Agent *ap;
{
  char folder[64], fpath[64], *brdname, *number, *stamp;
  int pos;
  time_t chrono;
  HDR hdr;
  BRD *is_bm;

  out_head(ap, "刪除文章");

  if (!arg_analyze(3, ap->urlp, &brdname, &number, &stamp, NULL))
    return -1;

  if ((pos = atoi(number) - 1) < 0)
    return -1;
  if ((chrono = atoi(stamp)) < 0)
    return -1;

  /* 順便做 acct_fetch() */
  is_bm = allow_brdname(ap, brdname, BRD_X_BIT, 0);

  if (ap->userlevel)	/* guest 不能砍自己文章 */
  {
    brd_fpath(folder, brdname, FN_DIR);
    if (!rec_get(folder, &hdr, sizeof(HDR), pos) &&
      (hdr.chrono == chrono) && !(hdr.xmode & POST_MARKED) &&
      (!strcmp(ap->userid, hdr.owner) || is_bm))
    {
      rec_del(folder, sizeof(HDR), pos, NULL);

      hdr_fpath(fpath, folder, &hdr);
      unlink(fpath);

      pos--;	/* 連結要去的頁數為被刪除文章的前一篇 */
    }
  }

  out_mesg(ap, "已執行刪除指令，若未刪除表示此文章被標記了，或您不是板主、作者");
  fprintf(ap->fpw, "<a href=/brd/%s&%d>回文章列表</a>\r\n", brdname, pos / HTML_TALL * HTML_TALL + 1);

  return 0;
}


static int
cmd_predelpost(ap)
  Agent *ap;
{
  char *brdname, *number, *stamp;

  out_head(ap, "確認刪除文章");

  if (!arg_analyze(3, ap->urlp, &brdname, &number, &stamp, NULL))
    return -1;

  out_mesg(ap, "若確定要刪除此篇文章，請再次點選以下連結；若要取消刪除，請按 [上一頁]");
  fprintf(ap->fpw, "<a href=/delpost/%s&%s&%s>刪除 [%s] 板第 %s 篇文章</a><br>\r\n", 
    brdname, number, stamp, brdname, number);

  return 0;
}


static int
cmd_delmail(ap)
  Agent *ap;
{
  char folder[64], fpath[64], *number, *stamp;
  int pos;
  time_t chrono;
  HDR hdr;

  out_head(ap, "刪除信件");

  if (!arg_analyze(2, ap->urlp, &number, &stamp, NULL, NULL))
    return -1;

  if ((pos = atoi(number) - 1) < 0)
    return -1;
  if ((chrono = atoi(stamp)) < 0)
    return -1;

  if (!acct_fetch(ap))
    return -1;

  usr_fpath(folder, ap->userid, FN_DIR);
  if (!rec_get(folder, &hdr, sizeof(HDR), pos) &&
    (hdr.chrono == chrono) && !(hdr.xmode & POST_MARKED))
  {
    rec_del(folder, sizeof(HDR), pos, NULL);

    hdr_fpath(fpath, folder, &hdr);
    unlink(fpath);
  }
  out_mesg(ap, "已執行刪除指令，若未刪除表示此信件被標記了");
  fprintf(ap->fpw, "<a href=/mbox/0>回信箱列表</a>\r\n");

  return 0;
}


static int
cmd_predelmail(ap)
  Agent *ap;
{
  char *number, *stamp;

  out_head(ap, "確認刪除信件");

  if (!arg_analyze(2, ap->urlp, &number, &stamp, NULL, NULL))
    return -1;

  out_mesg(ap, "若確定要刪除此篇信件，請再次點選以下連結；若要取消刪除，請按 [上一頁]");
  fprintf(ap->fpw, "<a href=/delmail/%s&%s>刪除信箱第 %s 篇信件</a><br>\r\n", 
    number, stamp, number);

  return 0;
}


  /*-----------------------------------------------------*/
  /* 標記文章/信件					 */
  /*-----------------------------------------------------*/


static int
cmd_markpost(ap)
  Agent *ap;
{
  char folder[64], *brdname, *number, *stamp;
  int pos;
  time_t chrono;
  HDR hdr;

  out_head(ap, "標記文章");

  if (!arg_analyze(3, ap->urlp, &brdname, &number, &stamp, NULL))
    return -1;

  if ((pos = atoi(number) - 1) < 0)
    return -1; 
  if ((chrono = atoi(stamp)) < 0)
    return -1;

  if (!allow_brdname(ap, brdname, BRD_X_BIT, 1))
    return 0;

  brd_fpath(folder, brdname, FN_DIR);
  if (!rec_get(folder, &hdr, sizeof(HDR), pos) &&
    (hdr.chrono == chrono))
  {
    hdr.xmode ^= POST_MARKED;
    rec_put(folder, &hdr, sizeof(HDR), pos, NULL);
  }

  out_mesg(ap, "已執行(取消)標記指令");
  fprintf(ap->fpw, "<a href=/brd/%s&%d>回文章列表</a>\r\n", brdname, (pos - 1) / HTML_TALL * HTML_TALL + 1);

  return 0;
}


static int
cmd_markmail(ap)
  Agent *ap;
{
  char folder[64], *number, *stamp;
  int pos;
  time_t chrono;
  HDR hdr;

  out_head(ap, "標記信件");

  if (!arg_analyze(2, ap->urlp, &number, &stamp, NULL, NULL))
    return -1;

  if ((pos = atoi(number) - 1) < 0)
    return -1;
  if ((chrono = atoi(stamp)) < 0)
    return -1;

  if (!acct_fetch(ap))
    return -1;

  usr_fpath(folder, ap->userid, FN_DIR);
  if (!rec_get(folder, &hdr, sizeof(HDR), pos) &&
    (hdr.chrono == chrono))
  {
    hdr.xmode ^= POST_MARKED;
    rec_put(folder, &hdr, sizeof(HDR), pos, NULL);
  }

  out_mesg(ap, "已執行(取消)標記指令");
  fprintf(ap->fpw, "<a href=/mbox/0>回信箱列表</a>\r\n");

  return 0;
}


  /*-----------------------------------------------------*/
  /* 查詢使用者						 */
  /*-----------------------------------------------------*/


static int
cmd_query(ap)
  Agent *ap;
{
  int fd;
  ACCT acct;
  char fpath[64], *userid;

  out_head(ap, "查詢使用者");

  if (!arg_analyze(1, ap->urlp, &userid, NULL, NULL, NULL))
    return -1;

  if (!allow_userid(ap, userid))
    return -1;

  usr_fpath(fpath, userid, FN_ACCT);
  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    read(fd, &acct, sizeof(ACCT));
    close(fd);

    fprintf(ap->fpw, "<pre>\r\n"
      "<a href=mailto:%s.bbs@%s>%s (%s)</a><br>\r\n"
      "%s通過認證，共上站 %d 次，發表過 %d 篇文章<br>\r\n"
      "最近(%s)從[%s]上站<br>\r\n"
      "</pre>\r\n",
      acct.userid, MYHOSTNAME, acct.userid, acct.username,
      (acct.userlevel & PERM_VALID) ? "已" : "未", acct.numlogins, acct.numposts,
      Btime(&(acct.lastlogin)), acct.lasthost);

    usr_fpath(fpath, acct.userid, FN_PLANS);
    out_article(ap, fpath);
  }
  else
  {
    out_mesg(ap, "沒有這個帳號");
  }

  return 0;
}


  /* --------------------------------------------------- */
  /* 顯示圖片						 */
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


static void agent_fire();


static int
cmd_image(ap)
  Agent *ap;
{
  int size;
  char fpath[64], *fname, *ptr;
  time_t now;
  struct stat st;
  FILE *fpw;

  if (!arg_analyze(1, ap->urlp, &fname, NULL, NULL, NULL))
  {
    agent_fire(ap);
    return 1;
  }

  if (!valid_path(fname) || !(ptr = strrchr(fname, '.')))
  {
    agent_fire(ap);
    return 1;
  }

  /* 只支援 gif 及 jpg 格式 */
  if (!str_cmp(ptr, ".gif"))
    ptr = "gif";
  else if (!str_cmp(ptr, ".jpg") || !str_cmp(ptr, ".jpeg"))
    ptr = "jpeg";
  else
  {
    agent_fire(ap);
    return 1;
  }

  /* 只可以讀取 run/html/ 底下的檔案 */
  sprintf(fpath, "run/html/%.30s", fname);
  if (stat(fpath, &st))
  {
    agent_fire(ap);
    return 1;
  }

  size = st.st_size;

  fpw = ap->fpw;

  /* itoc.040413: 還是乾脆就都不要秀檔頭了？ */
  if (size > 1024)     /* size 小於 1KB 則不需要檔頭 */
  {
    /* HTTP 1.0 檔頭 */
    time(&now);
    fprintf(fpw, "HTTP/1.0 200 OK\r\n");
    fprintf(fpw, "Date: %s\r\n", Gtime(&now));
    fprintf(fpw, "Server: MapleBBS 3.10\r\n");
    fprintf(fpw, "Last-Modified: %s\r\n", Gtime(&st.st_mtime));
    fprintf(fpw, "Content-Length: %d\r\n", size);
    fprintf(fpw, "Content-Type: image/%s\r\n", ptr);
    fprintf(fpw, "\r\n");
  }

  f_suck(fpw, fpath);

  agent_write(ap);
  return 1;
}


  /*-----------------------------------------------------*/
  /* 首頁						 */
  /*-----------------------------------------------------*/


static void
mainpage_neck(ap, logined)
  Agent *ap;
  int logined;
{
  fprintf(ap->fpw, "<br>\r\n"
    "<table cellspacing=0 cellpadding=1 border=0 width=80%%>\r\n"
    "<tr>\r\n"
    "  <td width=100%% align=center bgcolor="HCOLOR_NECK">%s%s歡迎光臨</td>\r\n"
    "</tr>\r\n"
    "</table>\r\n"
    "<br>\r\n",
    logined ? ap->userid : "",
    logined ? "，" : "");
}


static int
cmd_mainpage(ap)
  Agent *ap;
{
  int logined;
  char fpath[64];

  out_head(ap, "開頭畫面");

  logined = acct_fetch(ap);

  mainpage_neck(ap, logined);

  /* 開頭畫面 */
  sprintf(fpath, "gem/@/@opening.%d", ap->tbegin % 3);
  out_article(ap, fpath);

  /* 登入 */
  if (!logined)
  {
    fprintf(ap->fpw, "<form method=post>\r\n"
      "  <input type=hidden name=login>\r\n"
      "  帳號 <input type=text name=u size=12 maxlength=12> "
      "  密碼 <input type=password name=p size=12 maxlength=8> "
      "  <input type=submit value=登入> "
      "  <input type=reset value=清除>"
      "</form>\r\n");
  }

  mainpage_neck(ap, logined);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 錯誤連結						 */
  /*-----------------------------------------------------*/


static int
cmd_what(ap)
  Agent *ap;
{
  out_head2(ap, "400 Bad Request");

  fprintf(ap->fpw, "<h1>Bad Request</h1>\r\n"
    "Your browser sent a request that this server could not understand.\r\n");

  return 0;
}


  /* --------------------------------------------------- */
  /* 指令集						 */
  /* --------------------------------------------------- */


static Command cmd_table_get[] =
{
  cmd_userlist,   "/userlist",    9,
  cmd_boardlist,  "/boardlist",   10,
  cmd_favorlist,  "/favorlist",   10,

  cmd_postlist,   "/brd/",        5,
  cmd_gemlist,    "/gem/",        5,
  cmd_mboxlist,   "/mbox/",       6,

  cmd_brdmore,    "/brdmore/",    9,
  cmd_brdmost,    "/brdmost/",    9,
  cmd_gemmore,    "/gemmore/",    9,
  cmd_mboxmore,   "/mboxmore/",   10,

  cmd_dopost,     "/dopost/",     8,
  cmd_domail,     "/domail/",     8,

  cmd_delpost,    "/delpost/",    9,
  cmd_predelpost, "/predelpost/", 12,
  cmd_delmail,    "/delmail/",    9,
  cmd_predelmail, "/predelmail/", 12,
  cmd_markpost,   "/markpost/",   10,
  cmd_markmail,   "/markmail/",   10,

  cmd_query,      "/query/",      7,

  cmd_image,      "/image/",      7,

  cmd_mainpage,   "/",		  1,

  cmd_what,       NULL,           0
};


static void
cmd_GET(ap)
  Agent *ap;
{
  int ret;
  char *url, *ptr;
  Command *cmd;

  for (cmd = cmd_table_get, url = ap->url; ptr = cmd->cmd; cmd++)
  {
    if (!str_ncmp(url, ptr, cmd->len))
    {
      ap->urlp = url + cmd->len;
      break;
    }
  }

  ret = (*cmd->func) (ap);	/* 1:cmd_image專用 0:正常 -1:發生錯誤 */
  if (ret <= 0)
  {
    if (ret < 0)
      out_mesg(ap, "語法錯誤");
    out_tail(ap);
  }
}


/* ----------------------------------------------------- */
/* command dispatch (POST)				 */
/* ----------------------------------------------------- */


  /*-----------------------------------------------------*/
  /* 使用者登入						 */
  /*-----------------------------------------------------*/


static int
cmd_login(ap)
  Agent *ap;
{
  char *userid, *passwd;
  char fpath[64], dst[LEN_COOKIE];
  ACCT acct;

  str_dehtml(ap->urlp, dst, sizeof(dst));

  /* u=userid&p=passwd */

  if (!arg_analyze(2, dst, &userid, &passwd, NULL, NULL))
    return 0;

  userid += 2;	/* skip "u=" */
  passwd += 2;	/* skip "p=" */

  if (*userid && *passwd && strlen(userid) <= IDLEN && strlen(passwd) <= PSWDLEN)
  {
    usr_fpath(fpath, userid, FN_ACCT);
    if (!rec_get(fpath, &acct, sizeof(ACCT), 0) &&
      !(acct.userlevel & (PERM_DENYLOGIN | PERM_PURGE)) &&
      !chkpasswd(acct.passwd, passwd))				/* 登入成功 */
    {
      /* itoc.040308: 產生 Cookie */
      ap->setcookie = 1;
      str_ncpy(ap->cookie, ap->urlp, sizeof(ap->cookie));
    }
  }

  cmd_mainpage(ap);

  return 0;
}


  /*-----------------------------------------------------*/
  /* 發表新文章						 */
  /*-----------------------------------------------------*/


static inline void
outgo_post(hdr, board)
  HDR *hdr;
  char *board;
{
  bntp_t bntp;

  memset(&bntp, 0, sizeof(bntp_t));
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
  int used;
  char *dst;
  char *brdname, *title, *content, *end;
  char folder[64], fpath[64];
  HDR hdr;
  BRD *brd;
  FILE *fp;

  out_head(ap, "文章發表");

  used = ap->used + 1;		/* 保留空間給 '\0' */
  dst = (char *) malloc(used);
  str_dehtml(ap->urlp, dst, used);

  /* b=brdname&t=title&c=content&end= */

  if (arg_analyze(4, dst, &brdname, &title, &content, &end))
  {
    brdname += 2;	/* skip "b=" */
    title += 2;		/* skip "t=" */
    content += 2;	/* skip "c=" */

    if (*brdname && *title && *content)
    {
      if (brd = allow_brdname(ap, brdname, BRD_W_BIT, 1))
      {
	brd_fpath(folder, brdname, FN_DIR);

	fp = fdopen(hdr_stamp(folder, 'A', &hdr, fpath), "w");
	fprintf(fp, "%s %s (%s) %s %s\n",
	  STR_AUTHOR1, ap->userid, ap->username,
	  STR_POST2, brdname);
	fprintf(fp, "標題: %s\n時間: %s\n\n", title, Now());
	fprintf(fp, "%s\n", content);
	fprintf(fp, EDIT_BANNER, ap->userid, ap->fromhost);
	fclose(fp);

	hdr.xmode = (brd->battr & BRD_NOTRAN) ? 0 : POST_OUTGO;
	strcpy(hdr.owner, ap->userid);
	strcpy(hdr.nick, ap->username);
	str_ncpy(hdr.title, title, sizeof(hdr.title));	/* 怕被惡搞 */
	rec_bot(folder, &hdr, sizeof(HDR));

	brd->btime = -1;
	if (hdr.xmode & POST_OUTGO)
	  outgo_post(&hdr, brdname);

	free(dst);
	out_reload(ap, "您的文章發表成功\");
	return 0;
      }
    }
  }

  free(dst);
  out_reload(ap, "您的文章發表失敗，也許\是因為您尚未登入");
  return 0;
}


  /*-----------------------------------------------------*/
  /* 發送新信件						 */
  /*-----------------------------------------------------*/


static int
cmd_addmail(ap)
  Agent *ap;
{
  int used;
  char *dst;
  char *userid, *title, *content, *end;
  char folder[64], fpath[64];
  HDR hdr;
  FILE *fp;

  out_head(ap, "信件發送");

  used = ap->used + 1;		/* 保留空間給 '\0' */
  dst = (char *) malloc(used);
  str_dehtml(ap->urlp, dst, used);

  /* u=userid&t=title&c=content&end= */

  if (arg_analyze(4, dst, &userid, &title, &content, &end))
  {
    userid += 2;	/* skip "u=" */
    title += 2;		/* skip "t=" */
    content += 2;	/* skip "c=" */

    if (*userid && *title && *content && allow_userid(ap, userid))
    {
      usr_fpath(fpath, userid, FN_ACCT);
      if (dashf(fpath) && acct_fetch(ap))
      {
	usr_fpath(folder, userid, FN_DIR);

	fp = fdopen(hdr_stamp(folder, 0, &hdr, fpath), "w");
	fprintf(fp, "%s %s (%s)\n",
	  STR_AUTHOR1, ap->userid, ap->username);
	fprintf(fp, "標題: %s\n時間: %s\n\n", title, Now());
	fprintf(fp, "%s\n", content);
	fprintf(fp, EDIT_BANNER, ap->userid, ap->fromhost);
	fclose(fp);

	strcpy(hdr.owner, ap->userid);
	strcpy(hdr.nick, ap->username);
	str_ncpy(hdr.title, title, sizeof(hdr.title));	/* 怕被惡搞 */
	rec_add(folder, &hdr, sizeof(HDR));

	free(dst);
	out_reload(ap, "您的信件發送成功\");
	return 0;
      }
    }
  }

  free(dst);
  out_reload(ap, "您的信件發送失敗，也許\是因為您尚未登入或是查無此使用者");
  return 0;
}


  /*-----------------------------------------------------*/
  /* 指令集						 */
  /*-----------------------------------------------------*/


static Command cmd_table_post[] =
{
  cmd_login,      "login=&",      7,
  cmd_addpost,    "dopost=&",     8,
  cmd_addmail,    "domail=&",     8,

  cmd_what,       NULL,           0
};


static void
cmd_POST(ap, data)
  Agent *ap;
  char *data;
{
  char *ptr;
  Command *cmd;

  if (ptr = strchr(data, '\r'))
    *ptr = '\0';
  if (ptr = strchr(data, '\n'))
    *ptr = '\0';

  for (cmd = cmd_table_post; ptr = cmd->cmd; cmd++)
  {
    if (!strncmp(data, ptr, cmd->len))
    {
      ap->urlp = data + cmd->len;
      break;
    }
  }
	
  (*cmd->func) (ap);
  out_tail(ap);
}


/* ----------------------------------------------------- */
/* send output to client				 */
/* ----------------------------------------------------- */
/* return value :					 */
/* > 0 : bytes sent					 */
/* = 0 : close this agent				 */
/* < 0 : there are some error, but keep trying		 */
/* ----------------------------------------------------- */


static int
agent_send(ap)
  Agent *ap;
{
  int csock, len, cc;
  char *data;

  csock = ap->sock;
  data = ap->data;
  len = ap->used;
  cc = send(csock, data, len, 0);

  if (cc < 0)
  {
    cc = errno;
    if (cc != EWOULDBLOCK)
    {
      fprintf(flog, "SEND\t[%d] %s\n", ap->sno, strerror(cc));
      return 0;
    }

    /* would block, so leave it to do later */
    return -1;
  }

  if (cc == 0)
    return -1;

  len -= cc;
  ap->used = len;
  if (len)
  {
    memcpy(data, data + cc, len);
    return cc;
  }

  if (ap->state == CS_FLUSH)
  {
    shutdown(csock, 2);
    close(csock);
    ap->sock = -1;
    return 0;
  }

  ap->state = CS_RECV;
  return cc;
}


static void
agent_write(ap)			/* 將 ftemp 裡面的內容送出去 */
  Agent *ap;
{
  int fd, fsize;
  char *data;
  struct stat st;

  /* 在 main() 裡面打開，在這裡關閉 */
  fclose(ap->fpw);

  if ((fd = open(ap->ftemp, O_RDONLY)) >= 0)
  {
    fstat(fd, &st);
    if ((fsize = st.st_size) > 0)
    {
      data = ap->data;
      if (fsize > ap->size)
      {
	ap->data = data = realloc(data, fsize);
	ap->size = fsize;
      }
      read(fd, data, fsize);
    }

    close(fd);
    unlink(ap->ftemp);

    if (fsize > 0)
    {
      ap->used = fsize;
      ap->state = CS_FLUSH;
      return;
    }
  }

  agent_fire(ap);
}


/* ----------------------------------------------------- */
/* receive request from client				 */
/* ----------------------------------------------------- */


static int
agent_recv(ap)
  Agent *ap;
{
  int cc, mode, size, used;
  char *data, *head, *ptr;

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

	if (data = (char *) realloc(data, size))
	{
	  ap->data = data;
	  ap->size = size;
	}
	else
	{
	  fprintf(flog, "ERROR\t[%d] malloc: %d\n", ap->sno, size);
	  return 0;
	}
      }
      else
      {
	fprintf(flog, "ERROR\t[%d] data too long\n", ap->sno);
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
      fprintf(flog, "RECV\t[%d] %s\n", ap->sno, strerror(cc));
      return 0;
    }

    /* would block, so leave it to do later */

    return -1;
  }

  head[cc] = '\0';
  ap->used = (used += cc);

  /* itoc.050807: recv() 一次還讀不完的，一定是 dopost 或 domail，這二者的結束都有 &end= */
  if (used >= TCP_RCVSIZ)
  {
    /* 多 -2 是因為有些瀏覽器會自動補上 \r\n */
    if (!strstr(head + cc - strlen("&end=") - 2, "&end="))	/* 還沒讀完，繼續讀 */
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
	  if (ptr = strchr(data, ' '))
	  {
	    *ptr = '\0';
	    str_ncpy(ap->url, data, sizeof(ap->url));
	  }
	}
	else if (!str_ncmp(data, "POST ", 5))
	{
	  mode ^= AM_POST;
	}
      }
      else
      {
	if (*data)		/* 不是空行：檔頭 */
	{
	  /* 分析檔頭：只需要 Cookie 一種檔頭 */
	  if (!str_ncmp(data, "Cookie: user=", 13))
	    str_ncpy(ap->cookie, data + 13, sizeof(ap->cookie));
	}
	else			/* 空行 */
	{
	  if (mode & AM_GET)
	  {
	    cmd_GET(ap);
	  }
	  else /* if (mode & AM_POST) */
	  {
	    /* 找下一行的開始 */
	    for (ptr = head + 1; cc = *ptr; ptr++)
	    {
	      if (cc != '\r' && cc != '\n')
	      {
		data = ptr;
		break;
	      }
	    }
		
	    cmd_POST(ap, data);
	  }

	  return 1;
	}
      }

      data = head + 1;
    }

    head++;
  }

  ap->used = 0;
  return 1;
}


/* ----------------------------------------------------- */
/* close a connection & release its resource		 */
/* ----------------------------------------------------- */


static void
agent_fire(ap)
  Agent *ap;
{
  int num;

  num = ap->sock;
  if (num > 0)
  {
    fcntl(num, F_SETFL, M_NONBLOCK);
    shutdown(num, 2);
    close(num);
  }

  free(ap->data);
}


/* ----------------------------------------------------- */
/* accept a new connection				 */
/* ----------------------------------------------------- */


static int
agent_accept(ipaddr, from, len)
  unsigned int *ipaddr;
  char *from;
  int len;
{
  int csock;
  int value;
  struct sockaddr_in csin;
  struct hostent *hp;

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
      logit("ACCEPT", strerror(csock));
      return -1;
    }

    while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
  }

  value = 1;

  /* Thor.000511: 註解: don't delay send to coalesce(聯合) packets */
  setsockopt(csock, IPPROTO_TCP, TCP_NODELAY, (char *) &value, sizeof(value));

  /* --------------------------------------------------- */
  /* check remote host / user name			 */
  /* --------------------------------------------------- */

  *ipaddr = csin.sin_addr.s_addr;
  if (hp = gethostbyaddr((char *) &csin.sin_addr, sizeof(struct in_addr), csin.sin_family))
    str_ncpy(from, (char *) hp->h_name, len);
  else
    str_ncpy(from, inet_ntoa(csin.sin_addr), len);

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

  fflush(flog);
}
#endif


#define SS_CONFIG   1
#define SS_SHUTDOWN 2


static int servo_state;


static void
sig_hup()
{
  servo_state |= SS_CONFIG;
}


static void
sig_term()			/* graceful termination */
{
  servo_state |= SS_SHUTDOWN;
}


static void
sig_abort(sig)
  int sig;
{
  char buf[80];

  sprintf(buf, "abort: %d, errno: %d", sig, errno);
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

  /* sigblock(sigmask(SIGPIPE)); *//* Thor.981206: 統一 POSIX 標準用法 */

  /* act.sa_mask = 0; *//* Thor.981105: 標準用法 */
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  act.sa_handler = sig_term;		/* forced termination */
  sigaction(SIGTERM, &act, NULL);

  act.sa_handler = sig_abort;		/* forced termination */
  sigaction(SIGSEGV, &act, NULL);	/* if rlimit violate */
  sigaction(SIGBUS, &act, NULL);

#if 1	/* Thor.990203: 抓 signal */
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
  /* itoc.010317: 我的 linux 沒有這個說 :p */
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

  act.sa_handler = sig_hup;		/* restart config */
  sigaction(SIGHUP, &act, NULL);

  act.sa_handler = reaper;
  sigaction(SIGCHLD, &act, NULL);

#ifdef  SERVER_USAGE
  act.sa_handler = servo_usage;
  sigaction(SIGPROF, &act, NULL);
#endif

  /* Thor.981206: lkchu patch: 統一 POSIX 標準用法 */
  /* 在此借用 sigset_t act.sa_mask */
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
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &value, sizeof(value));

  ld.l_onoff = ld.l_linger = 0;
  setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

  sin.sin_family = AF_INET;
  sin.sin_port = htons(BHTTP_PORT);
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  memset((char *) &sin.sin_zero, 0, sizeof(sin.sin_zero));

  if (bind(fd, (struct sockaddr *) & sin, sizeof(sin)) ||
    listen(fd, TCP_BACKLOG))
    exit(1);
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int n, sock, state;
  time_t uptime, tcheck, tfresh;
  Agent **FBI, *Scully, *Mulder, *agent;
  fd_set rset, wset, xset;
  static struct timeval tv = {BHTTP_PERIOD, 0};

  state = 0;

  while ((n = getopt(argc, argv, "i")) != -1)
  {
    switch (n)
    {
    case 'i':
      state = 1;
      break;

    default:
      fprintf(stderr, "Usage: %s [options]\n"
	"\t-i  start from inetd with wait option\n",
	argv[0]);
      exit(0);
    }
  }

  servo_daemon(state);

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  init_bshm();
  init_ushm();

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
      /* agent_audit (uptime - BHTTP_TIMEOUT)		 */
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
      /* maintain SPAM & server log			 */
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
    /* check servo operation state			 */
    /* ------------------------------------------------- */

    n = 0;

    if (state = servo_state)
    {
      if (state & SS_CONFIG)
      {
	state ^= SS_CONFIG;
      }

      if (state & SS_SHUTDOWN)	/* graceful shutdown */
      {
	n = -1;
	close(0);
      }

      servo_state = state;
    }

    /* ------------------------------------------------- */
    /* Set up the fdsets				 */
    /* ------------------------------------------------- */

    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_ZERO(&xset);

    if (n == 0)
      FD_SET(0, &rset);

    for (agent = Scully; agent; agent = agent->anext)
    {
      sock = agent->sock;
      state = agent->state;

      if (n < sock)
	n = sock;

      if (state == CS_RECV)
      {
	FD_SET(sock, &rset);
      }
      else
      {
	FD_SET(sock, &wset);
      }

      FD_SET(sock, &xset);
    }

    /* no active agent and ready to die */

    if (n < 0)
    {
      break;
    }

    {
      struct timeval tv_tmp = tv;
      /* Thor.981221: for future reservation bug */
      n = select(n + 1, &rset, &wset, &xset, &tv_tmp);
    }

    if (n == 0)
    {
      continue;
    }

    if (n < 0)
    {
      n = errno;
      if (n != EINTR)
      {
	logit("SELECT", strerror(n));
      }
      continue;
    }

    /* ------------------------------------------------- */
    /* serve active agents				 */
    /* ------------------------------------------------- */

    uptime = time(0);

    for (FBI = &Scully; agent = *FBI;)
    {
      sock = agent->sock;

      if (FD_ISSET(sock, &wset))
      {
	state = agent_send(agent);
      }
      else if (FD_ISSET(sock, &rset))
      {
	state = agent_recv(agent);
      }
      else if (FD_ISSET(sock, &xset))
      {
	state = 0;
      }
      else
      {
	state = -1;
      }

      if (state == 0)		/* fire this agent */
      {
	agent_fire(agent);

	*FBI = agent->anext;

	agent->anext = Mulder;
	Mulder = agent;

	continue;
      }

      if (state > 0)
      {
	agent->uptime = uptime;
      }

      FBI = &(agent->anext);
    }

    /* ------------------------------------------------- */
    /* serve new connection				 */
    /* ------------------------------------------------- */

    /* Thor.000209: 考慮移前此部分, 免得卡在 accept() */
    if (FD_ISSET(0, &rset))
    {
      unsigned int ip_addr;
      char from[60];

      sock = agent_accept(&ip_addr, from, sizeof(from));

      if (sock > 0)
      {
	if (agent = Mulder)
	{
	  Mulder = agent->anext;
	}
	else
	{
	  agent = (Agent *) malloc(sizeof(Agent));
	  if (!agent)		/* Thor.990205: 記錄空間不夠 */
	    logit("ERROR", "Not enough space in main()");
	}

	*FBI = agent;

	/* variable initialization */

	memset(agent, 0, sizeof(Agent));

	agent->sock = sock;
	agent->sno = ++servo_sno;
	agent->state = CS_RECV;
	agent->tbegin = agent->uptime = uptime;

	agent->ip_addr = ip_addr;
	strcpy(agent->fromhost, from);

	sprintf(agent->ftemp, "tmp/bhttpd.%d", servo_sno);
	agent->fpw = fopen(agent->ftemp, "w");

	agent->data = (char *) malloc(MIN_DATA_SIZE);
	if (!agent->data)	/* Thor.990205: 記錄空間不夠 */
	  logit("ERROR", "Not enough space in agent->data");
	agent->size = MIN_DATA_SIZE;
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
