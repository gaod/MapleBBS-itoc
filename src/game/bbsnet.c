/*-------------------------------------------------------*/
/* bbsnet.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : BBSnet					 */
/* create : 01/12/13                                     */
/* update :   /  /                                       */
/* author : hightman.bbs@bbs.dot66.net			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_NETTOOL

/* ----------------------------------------------------- */
/* BBSNET                                                */
/* ----------------------------------------------------- */

static uschar netbuf[2048];
static struct timeval tv;
static fd_set readfds;
static int cfd;


static void 
break_check()
{
  int i, num;

  tv.tv_sec = 0;
  tv.tv_usec = 0;

  FD_ZERO(&readfds);
  FD_SET(0, &readfds);
  select(1, &readfds, NULL, NULL, &tv);

  if (FD_ISSET(0, &readfds))
  {
    num = read(0, netbuf, 80);
    for (i = 0; i < num; i++)
    {
      if (netbuf[i] == 3 || netbuf[i] == 4)
	close(cfd);
    }
  }
  alarm(1);
}


static void 
telnetopt(fd, max)
  int fd;
  int max;
{
  int i;
  uschar c2, c3;
  uschar tmp[30];

  for (i = 0; i < max; i++)
  {
    if (netbuf[i] != 255)
    {
      write(0, &netbuf[i], 1);
      continue;
    }
    c2 = netbuf[i + 1];
    c3 = netbuf[i + 2];
    i += 2;
    if (c2 == 253 && (c3 == 3 || c3 == 24))
    {
      sprintf(tmp, "%c%c%c", 255, 251, c3);
      write(fd, tmp, 3);
      continue;
    }
    if ((c2 == 251 || c2 == 252) && (c3 == 1 || c3 == 3 || c3 == 24))
    {
      sprintf(tmp, "%c%c%c", 255, 253, c3);
      write(fd, tmp, 3);
      continue;
    }
    if (c2 == 251 || c2 == 252)
    {
      sprintf(tmp, "%c%c%c", 255, 254, c3);
      write(fd, tmp, 3);
      continue;
    }
    if (c2 == 253 || c2 == 254)
    {
      sprintf(tmp, "%c%c%c", 255, 252, c3);
      write(fd, tmp, 3);
      continue;
    }
    if (c2 == 250)
    {
      while (c3 != 240 && i < max)
      {
	c3 = netbuf[i];
	i++;
      }
      sprintf(tmp, "%c%c%c%c%c%c%c%c%c%c", 255, 250, 24, 0, 65, 78, 83, 73, 255, 240);
      write(fd, tmp, 10);
    }
  }
  fflush(stdout);
}


static void 
telnet(server, port)
  char *server;
  int port;
{
  int num;
  struct sockaddr_in sin;
  struct hostent *host;

  signal(SIGALRM, break_check);
  alarm(1);

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);

  if (!(host = gethostbyname(server)))
    sin.sin_addr.s_addr = inet_addr(server);
  else
    memcpy(&sin.sin_addr.s_addr, host->h_addr, host->h_length);

  cfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (connect(cfd, (struct sockaddr *) & sin, sizeof(sin)) < 0)
  {
    close(cfd);
    return;
  }

  signal(SIGALRM, SIG_IGN);

  while (1)
  {
    tv.tv_sec = 2400;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(cfd, &readfds);
    FD_SET(0, &readfds);

    if (select(cfd + 1, &readfds, NULL, NULL, &tv) <= 0)
      break;
    if (FD_ISSET(0, &readfds))
    {
      num = read(0, netbuf, 2048);
      if (num <= 0)
	break;
      if (netbuf[0] == 29)
	return;
      if (netbuf[0] == 13 && num == 1)
      {
	netbuf[1] = 10;
	num++;
      }
      write(cfd, netbuf, num);
    }
    else
    {
      num = read(cfd, netbuf, 2048);
      if (num <= 0)
	break;
      if (strchr(netbuf, 255))
	telnetopt(cfd, num);
      else
	write(0, netbuf, num);
    }
  }

  close(cfd);
}


int
x_bbsnet()
{
  int fd, i, j;
  char buf[80];

#define MAX_CONNLIST	20	/* 假設目前只有 20 個站台，至多 51=3*(b_lines - 6) 個站台 */

  struct CONNLIST
  {
    char host[40];
    char name[21];
    int port;
  }        connlist[MAX_CONNLIST];

#if 0				/* etc/connlist 格式 */
210.70.137.5:23:與南共舞
ptt.twbbs.org:23:PTT
#endif

  cutmp->status |= STATUS_REJECT;	/* bbsnet 時不收熱訊 */

  while (1)		/* itoc.010824: 連去別的站回來以後還是留在 bbsnet 選單中，可以再連去別的站 */
  {
    if ((fd = open("etc/connlist", O_RDONLY)) >= 0)
    {
      char *str, *ptr1, *ptr2;

      j = 0;
      mgets(-1);
      while ((str = mgets(fd)) && (j < MAX_CONNLIST))
      {
	if (ptr1 = strchr(str, ':'))
	{
	  *ptr1 = '\0';
	  do
	  {
	    i = *++ptr1;
	  } while (i == ' ' || i == '\t');

	  if (ptr2 = strchr(ptr1, ':'))
	  {
	    *ptr2 = '\0';
	    do
	    {
	      i = *++ptr2;
	    } while (i == ' ' || i == '\t');
	  }

	  if (i)
	  {
	    strcpy(connlist[j].host, str);
	    strcpy(connlist[j].name, ptr2);
	    i = atoi(ptr1);
	    connlist[j].port = i > 0 ? i : 23;	/* 預設 port 23 */
	    j++;
	  }
	}
      }
      close(fd);
    }
    else
    {
      break;
    }

    vs_bar("環島旅行");
    outs("╭═════════════════════════════════════╮\n");

    for (i = 0; i < j; i++)
    {
      prints("║ \033[1;33m%2d.\033[m%-21s", i + 1, connlist[i].name);

      if (++i < j)
	prints("\033[1;33m%2d.\033[m%-21s", i + 1, connlist[i].name);
      else
	prints("%24s", "");

      if (++i < j)
        prints("\033[1;33m%2d.\033[m%-21s ║\n", i + 1, connlist[i].name);
      else
	prints("%24s ║\n", "");
    }

    for (i /= 3; i < b_lines - 7; i++)
    {
      outs("║                                                                          ║\n");
    }

    outs("║─────────────────────────────────────║\n"
      "║ \033[1;31m注意：\033[m當您選擇了站台後，螢幕像當機一樣不動了，表示現在無法登入對方主機， ║\n"
      "║       請稍候 telnet connect time out 後會自動退出。                      ║\n"
      "╰═════════════════════════════════════╯");

    vget(b_lines, 0, "請選擇一個連線站台：[Q] ", buf, 3, DOECHO);
    i = atoi(buf) - 1;
    if (i >= 0 && i < j)
    {
      sprintf(buf, "%s %s enter %s", cuser.userid, Now(), connlist[i].name);
      blog("BBSNET", buf);
      telnet(connlist[i].host, connlist[i].port);
      sprintf(buf, "%s %s leave %s", cuser.userid, Now(), connlist[i].name);
      blog("BBSNET", buf);
    }
    else
    {
      break;
    }
  }

  cutmp->status ^= STATUS_REJECT;
  return 0;
}
#endif	/* HAVE_NETTOOL */
