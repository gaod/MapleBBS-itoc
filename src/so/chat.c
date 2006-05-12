/*-------------------------------------------------------*/
/* chat.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : chat client for xchatd			 */
/* create : 95/03/29					 */
/* update : 95/12/15					 */
/*-------------------------------------------------------*/


#include "bbs.h"


#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


static char chatroom[IDLEN];	/* Chat-Room Name */
static int chatline;		/* Where to display message now */
static char chatopic[48];
static FILE *frec;


#define	stop_line	(b_lines - 2)


extern char *bmode();


static void
chat_topic()
{
  move(0, 0);
  prints("[1;37;46m %s�ǡG%-14s[45m ���D�G%-48s[m",
    (frec ? "����" : "�ͤ�"), chatroom, chatopic);
}


static void
printchatline(msg)
  char *msg;
{
  int line;

  line = chatline;
  move(line, 0);
  outs(msg);
  outc('\n');

  if (frec)
    fprintf(frec, "%s\n", msg);

  if (++line == stop_line)
    line = 2;
  move(line, 0);
  outs("��");
  clrtoeol();
  chatline = line;
}


static void
chat_record()
{
  FILE *fp;
  time_t now;
  char buf[80];

  time(&now);

  if (fp = frec)
  {
    fprintf(fp, "%s\n�����G%s\n", msg_seperator, Btime(&now));
    fclose(fp);
    frec = NULL;
    printchatline("�� ���������I");
  }
  else
  {
#ifdef EVERY_Z
    /* Thor.980602: �ѩ� tbf_ask() �ݰ��ɦW�A���ɷ|�Ψ� igetch()�A
       ���F���� I_OTHERDATA �y�����A�b���� every_Z() ���覡�A
       ���O�s vio_fd�A�ݰݧ���A�٭� */

    vio_save();		/* Thor.980602: �Ȧs vio_fd */
#endif

    usr_fpath(buf, cuser.userid, tbf_ask());

#ifdef EVERY_Z
    vio_restore();	/* Thor.980602: �٭� vio_fd */
#endif

    move(b_lines, 0);
    clrtoeol();

    fp = fopen(buf, "a");
    if (fp)
    {
      fprintf(fp, "�D�D: %s\n�]�[: %s\n����: %s (%s)\n�}�l: %s\n%s\n",
	chatopic, chatroom, cuser.userid, cuser.username,
	Btime(&now), msg_seperator);
      printchatline("�� �}�l�����o�I");
      frec = fp;
    }
    else
    {
      printchatline("�� �������G�٤F�A�гq����������");
    }
  }
  bell();
  chat_topic();
}


static void
chat_clear()
{
  int line;

  for (line = 2; line < stop_line; line++)
  {
    move(line, 0);
    clrtoeol();
  }
  chatline = stop_line - 1;
  printchatline("");
}


static void
print_chatid(chatid)
  char *chatid;
{
  move(b_lines - 1, 0);
  outs(chatid);
  outc(':');
}


static inline int
chat_send(fd, buf)
  int fd;
  char *buf;
{
  int len;

  len = strlen(buf);
  return (send(fd, buf, len, 0) == len);
}


static inline int
chat_recv(fd, chatid)
  int fd;
  char *chatid;
{
  static char buf[512];
  static int bufstart = 0;
  int cc, len;
  char *bptr, *str;

  bptr = buf;
  cc = bufstart;
  len = sizeof(buf) - cc - 1;
  if ((len = recv(fd, bptr + cc, len, 0)) <= 0)
    return -1;
  cc += len;

  for (;;)
  {
    len = strlen(bptr);

    if (len >= cc)
    {				/* wait for trailing data */
      memcpy(buf, bptr, len);
      bufstart = len;
      break;
    }
    if (*bptr == '/')
    {
      str = bptr + 1;
      fd = *str++;

      if (fd == 'c')
      {
	chat_clear();
      }
      else if (fd == 'n')
      {
	str_ncpy(chatid, str, 9);
         
	/* Thor.980819: ���K���@�U mateid �n�F... */
	str_ncpy(cutmp->mateid, str, sizeof(cutmp->mateid));

	print_chatid(chatid);
	clrtoeol();
      }
      else if (fd == 'r')
      {
	str_ncpy(chatroom, str, sizeof(chatroom));
	chat_topic();
      }
      else if (fd == 't')
      {
	str_ncpy(chatopic, str, sizeof(chatopic));
	chat_topic();
      }
    }
    else
    {
      printchatline(bptr);
    }

    cc -= ++len;
    if (cc <= 0)
    {
      bufstart = 0;
      break;
    }
    bptr += len;
  }

  return 0;
}


static void
chat_pager(arg)
  char *arg;
{
  cuser.ufo ^= UFO_PAGER;
  cutmp->ufo ^= UFO_PAGER;
  /* Thor.980805: �ѨMufo �P�B���D */

  sprintf(arg, "�� �z���I�s���w�g%s�F!",
    cuser.ufo & UFO_PAGER ? "����" : "���}");
  printchatline(arg);
}


#if 0
/* Thor.980727: �M /flag ��key */
static void
chat_write(arg)
  char *arg;
{
  int uno;
  UTMP *up;
  char *str;
  CallMsg cmsg;

  strtok(arg, STR_SPACE);
  if ((str = strtok(NULL, STR_SPACE)) && (uno = acct_userno(str)) > 0)
  {
    cmsg.recver = uno;		/* ���O�U userno �@�� check */
    if (up = utmp_find(uno))
    {
      if (can_override(up))
      {
	if (str = strtok(NULL, "\n"))	/* Thor.980725:���y�� */
	{			/* Thor.980724: �q my_write ��L�� */
	  int len;
	  char buf[80];
	  extern char fpmsg[];
	  /* Thor.980722: msg file�[�W�ۤv������ */

	  sprintf(fpmsg + 4, "%s-", cuser.userid);
	  /* Thor.980722: �ɥ� len��@�Ufd :p */
	  len = open(fpmsg, O_WRONLY | O_CREAT | O_APPEND, 0600);
	  sprintf(buf, "��%s�G%s\n", up->userid, str);
	  write(len, buf, strlen(buf));
	  close(len);

	  sprintf(buf, "%s(%s", cuser.userid, cuser.username);
	  len = strlen(str);
	  buf[71 - len] = '\0';
	  sprintf(cmsg.msg, "\033[1;33;46m�� %s) \033[37;45m %s \033[m", buf, str);

	  cmsg.caller = cutmp;
	  cmsg.sender = cuser.userno;

	  if (do_write(up, &cmsg))
	    printchatline("�� ���w�g���h");
	}
	else
	{
	  printchatline("�� �O�u�w���A���Ǹܧa�I");
	}
      }
      else
      {
	printchatline("�� ����զ�ݳ���G�y�ڨSť��K�K�ڨSť��K�K�z");
      }
    }
    else
    {
      printchatline("�� ��褣�b���W");
    }
  }
  else
  {
    printchatline(err_uid);
  }
}


static int
printuserent(uentp)
  user_info *uentp;
{
  static char uline[80];
  static int cnt;
  char pline[30];
  int cloak;

  if (!uentp)
  {
    if (cnt)
      printchatline(uline);
    memset(uline, 0, 80);
    return cnt = 0;
  }
  cloak = uentp->ufo & UFO_CLOAK;
  if (cloak && !HAS_PERM(PERM_SEECLOAK))
    return 0;

  sprintf(pline, " %-13s%c%-10s", uentp->userid,
    cloak ? '#' : ' ', bmode(uentp, 1));
  if (cnt < 2)
    strcat(pline, "�x");
  strcat(uline, pline);
  if (++cnt == 3)
  {
    printchatline(uline);
    memset(uline, 0, 80);
    cnt = 0;
  }
  return 0;
}


static void
chat_users()
{				/* �]���H�ưʻ��W�ʡA�N�q���j */
  printchatline("");
  printchatline("�i " BBSNAME "�C�ȦC�� �j");
  printchatline(MSG_CHAT_ULIST);

  if (apply_ulist(printuserent) == -1)
    printchatline("�ŵL�@�H");

  printuserent(NULL);
}
#endif


struct chat_command
{
  char *cmdname;		/* Char-room command length */
  void (*cmdfunc) ();		/* Pointer to function */
};


struct chat_command chat_cmdtbl[] = 
{
  {"pager", chat_pager},
  {"tape", chat_record},

#if 0
  /* Thor.980727: �M /flag ��key */
  {"fire", chat_write},

  {"users", chat_users},
#endif

  {NULL, NULL}
};


static inline int
chat_cmd_match(buf, str)
  char *buf;
  char *str;
{
  int c1, c2;

  for (;;)
  {
    c1 = *str++;
    if (!c1)
      break;

    c2 = *buf++;
    if (!c2 || c2 == ' ' || c2 == '\n')
      break;

    if (c2 >= 'A' && c2 <= 'Z')
      c2 |= 0x20;

    if (c1 != c2)
      return 0;
  }

  return 1;
}


static inline int
chat_cmd(fd, buf)
  int fd;
  char *buf;
{
  struct chat_command *cmd;
  char *key;

  buf++;
  for (cmd = chat_cmdtbl; key = cmd->cmdname; cmd++)
  {
    if (chat_cmd_match(buf, key))
    {
      cmd->cmdfunc(buf);
      return '/';
    }
  }

  return 0;
}


extern char lastcmd[MAXLASTCMD][80];

#define CHAT_YPOS	10


int
t_chat()
{
  ACCT acct;
  int ch, cfd, cmdpos, cmdcol;
  char *ptr, buf[80], chatid[9];
  struct sockaddr_in sin;
#if     defined(__OpenBSD__)
  struct hostent *h;
#endif
  
#ifdef CHAT_SECURE
  extern char passbuf[];
#endif

#ifdef EVERY_Z
  /* Thor.980725: �� talk & chat �i�� ^z �@�ǳ� */
  if (vio_holdon())
  {
    vmsg("�z�������@�b�٨S�����C");
    return -1;
  }
#endif

#if     defined(__OpenBSD__)

  if (!(h = gethostbyname(str_host)))
    return -1;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(CHAT_PORT);
  memcpy(&sin.sin_addr, h->h_addr, h->h_length);
            
#else

  sin.sin_family = AF_INET;
  sin.sin_port = htons(CHAT_PORT);
  /* sin.sin_addr.s_addr = INADDR_LOOPBACK; */
  /* sin.sin_addr.s_addr = INADDR_ANY; */
  /* for FreeBSD 4.x */  
  sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

#endif

  cfd = socket(AF_INET, SOCK_STREAM, 0);
  if (cfd < 0)
    return -1;

  if (connect(cfd, (struct sockaddr *) & sin, sizeof sin))
  {
    close(cfd);
    blog("CHAT ", "connect");
    return -1;
  }

  for (;;)
  {
    ch = vget(b_lines, 0, "�п�J��ѥN���G", chatid, 9, DOECHO);
    if (ch == '/')
    {
      continue;
    }
    else if (!ch)
    {
      /* str_ncpy(chatid, cuser.userid, sizeof(chatid)); */
      close(cfd);	/* itoc.010322: �j�������O�~���� Talk->Chat �令�w�]�����} */
      return 0;
    }
    else
    {
      /* itoc.010528: ���i�H�ΧO�H�� id ������ѥN�� */
      if (acct_load(&acct, chatid) >= 0 && acct.userno != cuser.userno)
      {
	vmsg("��p�o�ӥN�����H���U�� id�A�ҥH�z�������ѥN��");
	continue;
      }
      /* Thor.980911: chatid�����i�H�ť�, ���� parse���~ */
      for(ch = 0; ch < 8; ch++)
      {
        if (chatid[ch] == ' ')
          break;
        else if (!chatid[ch]) /* Thor.980921: �p�G0���ܴN���� */
          ch = 8;
      }
      if (ch < 8) 
        continue;
    }

#ifdef CHAT_SECURE	/* Thor.980729: secured chat room */

#if 0
    �@��  opus (�H�ͦ����O�M�w)                                ���� sysopplan
    ���D  Re: ���� chatroom
    �ɶ�  Wed Jul 30 03:14:56 1997
    �w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w

    > passwd �O�s�򪺫D�ťզr����??

    �T�ӰѼ� userid + chatid + passwd ��, userid/chatid �Ҥ��t space, 
    �� passwd �i�]�t space, �ҥH�����N���\�b�ĤT�Ӧ�m�H�K token-parsing�C

    > �t�~, ACCT ���� passwd �OPASSLEN�۰ʸɪť��٬O�n��ʥ[?
    > �|�۰ʸɺ���?

    Unix �� crypt �̦h�u���e 14 �Ӧr, �ҥH���[��i�C
    �o�Ӧa�� �ѷ� user login ���a��g�N�n�F�C

    --
    �� Origin: �����毸(bbs.cs.nthu.edu.tw) From: thcs-8.cs.nthu.edu.tw
#endif

    /* Thor.980819: ����userno */
    /* Thor.980730: passwd �אּ�̫᪺�Ѽ� */
    /* Thor.980813: passwd �אּ�u���� password */
    /* Thor.980813: xchatd��, chatid �۰ʸ��L�ť�, �ҥH���ťշ| invalid login */
#if 0
    sprintf(buf, "/! %d %s %s %s\n",
      cuser.userno, cuser.userid, chatid, cuser.passwd);
      cuser.userno, cuser.userid, chatid, passbuf);
#endif
    sprintf(buf, "/! %s %s %s\n",
      cuser.userid, chatid, passbuf);

#else
    sprintf(buf, "/! %d %d %s %s\n",
      cuser.userno, cuser.userlevel, cuser.userid, chatid);
#endif

    chat_send(cfd, buf);
    if (recv(cfd, buf, 3, 0) != 3)
      return 0;

    if (!strcmp(buf, CHAT_LOGIN_OK))
      break;
    else if (!strcmp(buf, CHAT_LOGIN_EXISTS))
      ptr = "�o�ӥN���w�g���H�ΤF";
    else if (!strcmp(buf, CHAT_LOGIN_INVALID))
      ptr = "�o�ӥN���O���~��";
    else if (!strcmp(buf, CHAT_LOGIN_BOGUS))
    {				/* Thor: �T��ۦP�G�H�i�J */
      close(cfd);
      vmsg("�ФŬ����u�����v�i�J�ͤѫ�");
      return 0;
    }
    move(b_lines - 1, 0);
    outs(ptr);
    clrtoeol();
    bell();
  }

  clear();
  move(1, 0);
  outs(msg_seperator);
  move(stop_line, 0);
  outs(msg_seperator);
  print_chatid(chatid);
  memset(ptr = buf, 0, sizeof(buf));
  chatline = 2;
  cmdcol = 0;
  cmdpos = -1;

  add_io(cfd, 60);

  strcpy(cutmp->mateid, chatid);

  for (;;)
  {
    move(b_lines - 1, cmdcol + CHAT_YPOS);
    ch = vkey();

    if (ch == I_OTHERDATA)
    {				/* incoming */
      if (chat_recv(cfd, chatid) == -1)
	break;
      continue;
    }

    if (isprint2(ch))
    {
      if (cmdcol < 68)
      {
	if (ptr[cmdcol])
	{			/* insert */
	  int i;

	  for (i = cmdcol; ptr[i] && i < 68; i++);
	  ptr[i + 1] = '\0';
	  for (; i > cmdcol; i--)
	    ptr[i] = ptr[i - 1];
	}
	else
	{			/* append */
	  ptr[cmdcol + 1] = '\0';
	}
	ptr[cmdcol] = ch;
	move(b_lines - 1, cmdcol + CHAT_YPOS);
	outs(&ptr[cmdcol++]);
      }
      continue;
    }

    if (ch == '\n')
    {
#ifdef EVERY_BIFF
      /* Thor.980805: ���H�b�����enter�~�ݭncheck biff */
      static int old_biff;
      int biff = HAS_STATUS(STATUS_BIFF);
      if (biff && !old_biff)
        printchatline("�� ��! �l�t�ӫ��a�F!");
      old_biff = biff;
#endif
      if (ch = *ptr)
      {
	if (ch == '/')
	  ch = chat_cmd(cfd, ptr);

        /* Thor.980602: ���ӭn�`�N���p�a��, �쥻�p�G�O�y/�z, 
                        �|�q�X /help���e��,  
                        �{�b�� /, �|�ܦ� /p ���� pager */

        /* Thor.980925: �O�d ptr �̭�l��, ���[ /n */
	for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
	  strcpy(lastcmd[cmdpos], lastcmd[cmdpos - 1]);
	strcpy(lastcmd[0], ptr);

	if (ch != '/')
	{
	  strcat(ptr, "\n");
	  if (!chat_send(cfd, ptr))
	    break;
	}
	if (*ptr == '/' && ptr[1] == 'b')
	  break;

#if 0
	for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
	  strcpy(lastcmd[cmdpos], lastcmd[cmdpos - 1]);
	strcpy(lastcmd[0], ptr);
#endif

	*ptr = '\0';
	cmdcol = 0;
	cmdpos = -1;
	move(b_lines - 1, CHAT_YPOS);
	clrtoeol();
      }
      continue;
    }

    if (ch == KEY_BKSP)
    {
      if (cmdcol)
      {
	ch = cmdcol;
	cmdcol--;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: �P�_�{�b�R������m�O�_���~�r����b�q�A�Y�O�R�G�r�� */
	if ((cuser.ufo & UFO_ZHC) && cmdcol && IS_ZHC_LO(ptr, cmdcol))
	  cmdcol--;
#endif
	strcpy(ptr + cmdcol, ptr + ch);
	move(b_lines - 1, cmdcol + CHAT_YPOS);
	outs(ptr + cmdcol);
	clrtoeol();
      }
      continue;
    }

    if (ch == KEY_DEL)
    {
      if (ptr[cmdcol])
      {
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: �P�_�{�b�R������m�O�_���~�r���e�b�q�A�Y�O�R�G�r�� */
	if ((cuser.ufo & UFO_ZHC) && ptr[cmdcol + 1] && IS_ZHC_HI(ptr[cmdcol]))
	  ch = 2;
	else
#endif
	  ch = 1;
	strcpy(ptr + cmdcol, ptr + cmdcol + ch);
	move(b_lines - 1, cmdcol + CHAT_YPOS);
	outs(ptr + cmdcol);
	clrtoeol();
      }
      continue;
    }

    if (ch == Ctrl('D'))
    {
      chat_send(cfd, "/b\n");	/* /bye ���} */
      break;
    }

    if (ch == Ctrl('C'))	/* itoc.����: �M�� input ��� */
    {
      *ptr = '\0';
      cmdcol = 0;
      move(b_lines - 1, CHAT_YPOS);
      clrtoeol();
      continue;
    }

    if (ch == KEY_HOME || ch == Ctrl('A'))
    {
      cmdcol = 0;
      continue;
    }

    if (ch == KEY_END || ch == Ctrl('E'))
    {
      cmdcol = strlen(ptr);
      continue;
    }
    
    if (ch == KEY_LEFT)
    {
      if (cmdcol)
      {
	cmdcol--;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: �����ɸI��~�r������ */
	if ((cuser.ufo & UFO_ZHC) && cmdcol && IS_ZHC_LO(ptr, cmdcol))
	  cmdcol--;
#endif
      }
      continue;
    }

    if (ch == KEY_RIGHT)
    {
      if (ptr[cmdcol])
      {
	cmdcol++;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: �k���ɸI��~�r������ */
	if ((cuser.ufo & UFO_ZHC) && ptr[cmdcol] && IS_ZHC_HI(ptr[cmdcol - 1]))
	  cmdcol++;
#endif
      }
      continue;
    }

#ifdef EVERY_Z
    /* Thor: Chat ���� ctrl-z */
    if (ch == Ctrl('Z'))
    {
      char buf[IDLEN + 1];
      screenline slt[T_LINES];

      /* Thor.980731: �Ȧs mateid, �]���X�h�ɥi��|�α� mateid */
      strcpy(buf, cutmp->mateid);

      vio_save();	/* Thor.980727: �Ȧs vio_fd */
      vs_save(slt);
      every_Z(0);
      vs_restore(slt);
      vio_restore();	/* Thor.980727: �٭� vio_fd */

      /* Thor.980731: �٭� mateid, �]���X�h�ɥi��|�α� mateid */
      strcpy(cutmp->mateid, buf);
      continue;
    }
#endif

    if (ch == KEY_DOWN)
    {
      cmdpos += MAXLASTCMD - 2;
      ch = KEY_UP;
    }

    if (ch == KEY_UP)
    {
      cmdpos++;
      cmdpos %= MAXLASTCMD;
      strcpy(ptr, lastcmd[cmdpos]);
      move(b_lines - 1, CHAT_YPOS);
      outs(ptr);
      clrtoeol();
      cmdcol = strlen(ptr);
    }
  }

  if (frec)
    chat_record();

  close(cfd);
  add_io(0, 60);
  cutmp->mateid[0] = '\0';
  return 0;
}
