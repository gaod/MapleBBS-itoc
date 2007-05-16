/*-------------------------------------------------------*/
/* talk.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : talk/query routines		 		 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/


#define	_MODES_C_


#include "bbs.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


extern UCACHE *ushm;


typedef struct
{
  int curcol, curln;
  int sline, eline;
#ifdef HAVE_MULTI_BYTE
  int zhc;
#endif
}      talk_win;


/* ------------------------------------- */
/* �u��ʧ@				 */
/* ------------------------------------- */


char *
bmode(up, simple)
  UTMP *up;
  int simple;
{
  static char modestr[32];
  int mode;
  char *word, *mateid;

  word = ModeTypeTable[mode = up->mode];

  if (simple)
    return word;

#ifdef BMW_COUNT
  if (simple = up->bmw_count)	/* �ɥ� simple */
  {
    sprintf(modestr, "%d �Ӥ��y", simple);
    return modestr;
  }
#endif

#ifdef HAVE_BRDMATE
  /* itoc.020602: �����o���ϥΪ̦b�ݭ��ӪO */
  if (mode == M_READA && HAS_PERM(PERM_SYSOP))
  {
    sprintf(modestr, "�\\:%s", up->reading);
    return modestr;
  }
#endif

  if (mode < M_TALK || mode > M_IDLE)	/* M_TALK(�t) �P M_IDLE(�t) ���� mateid */
    return word;

  mateid = up->mateid;

  if (mode == M_TALK)
  {
    /* itoc.020829: up �b Talk �ɡA�Y up->mateid ���Ϋh�ݤ��� */
    if (!utmp_get(0, mateid))
      mateid = "�L�W��";
  }

  sprintf(modestr, "%s:%s", word, mateid);
  return modestr;
}


static void
showplans(userid)
  char *userid;
{
  int i;
  FILE *fp;
  char buf[ANSILINELEN];

  usr_fpath(buf, userid, fn_plans);
  if (fp = fopen(buf, "r"))
  {
    i = MAXQUERYLINES;
    while (i-- && fgets(buf, sizeof(buf), fp))
      outx(buf);
    fclose(fp);
  }
}


static void
do_query(acct)
  ACCT *acct;
{
  UTMP *up;
  int userno, rich;
  char *userid;
  char fortune[4][9] = {"���h�^��", "�@�����", "�a�Ҥp�d", "�]�֦a�D"};

  utmp_mode(M_QUERY);

  userno = acct->userno;
  userid = acct->userid;
  strcpy(cutmp->mateid, userid);

  up = utmp_find(userno);
  rich = acct->money >= 1000000 ? (acct->gold >= 100 ? 3 : 2) : (acct->money >= 50000 ? 1 : 0);

  prints("[�b��] %-12s [�ʺ�] %-16.16s [�W��] %5d �� [�峹] %5d �g\n",
    userid, acct->username, acct->numlogins, acct->numposts);

  prints("[�{��] %s�q�L�{�� [�ʺA] %-16.16s [�]��] %s [�H�c] %s\n",
    acct->userlevel & PERM_VALID ? "�w�g" : "�|��",
    (up && can_see(cutmp, up)) ? bmode(up, 1) : "���b���W",
    fortune[rich],
    (m_query(userid) & STATUS_BIFF) ? "���s�H��" : "���ݹL�F");

  prints("[�ӷ�] (%s) %s\n",
    Btime(&acct->lastlogin), acct->lasthost);

  showplans(userid);
  vmsg(NULL);
}


void
my_query(userid)
  char *userid;
{
  ACCT acct;

  if (acct_load(&acct, userid) >= 0)
    do_query(&acct);
  else
    vmsg(err_uid);
}


#ifdef HAVE_ALOHA
static int
chkfrienz(frienz)
  FRIENZ *frienz;
{
  int userno;

  userno = frienz->userno;
  return (userno > 0 && userno == acct_userno(frienz->userid));
}


static int
frienz_cmp(a, b)
  FRIENZ *a, *b;
{
  return a->userno - b->userno;
}


void
frienz_sync(fpath)
  char *fpath;
{
  rec_sync(fpath, sizeof(FRIENZ), frienz_cmp, chkfrienz);
}


void
aloha()
{
  UTMP *up;
  int fd;
  char fpath[64];
  BMW bmw;
  FRIENZ *frienz;
  int userno;

  usr_fpath(fpath, cuser.userid, FN_FRIENZ);

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    bmw.caller = cutmp;
    /* bmw.sender = cuser.userno; */	/* bmw.sender �̹��i�H call �ڡA�ݷ|�A�M�w */
    strcpy(bmw.userid, cuser.userid);
    strcpy(bmw.msg, "�� ����i"BBSNAME"���� [�W���q��] ��");

    mgets(-1);
    while (frienz = mread(fd, sizeof(FRIENZ)))
    {
      userno = frienz->userno;
      up = utmp_find(userno);

      if (up && (up->ufo & UFO_ALOHA) && !(up->status & STATUS_REJECT) && can_see(up, cutmp))	/* ���ݤ����ڤ��q�� */
      {
	/* �n�ͥB�ۤv�S���������ۤ~�i�H reply */
	bmw.sender = (is_mygood(userno) && !(cuser.ufo & UFO_QUIET)) ? cuser.userno : 0;
	bmw.recver = userno;
	bmw_send(up, &bmw);
      }
    }
    close(fd);
  }
}
#endif


#ifdef LOGIN_NOTIFY
extern LinkList *ll_head;


int
t_loginNotify()
{
  LinkList *wp;
  BENZ benz;
  char fpath[64];

  /* �]�w list ���W�� */

  vs_bar("�t�Ψ�M����");

  ll_new();

  if (pal_list(0))
  {
    wp = ll_head;
    benz.userno = cuser.userno;
    strcpy(benz.userid, cuser.userid);

    do
    {
      if (strcmp(cuser.userid, wp->data))	/* ���i��M�ۤv */
      {
	usr_fpath(fpath, wp->data, FN_BENZ);
	rec_add(fpath, &benz, sizeof(BENZ));
      }
    } while (wp = wp->next);

    vmsg("��M�]�w�����A���W���ɨt�η|�q���z");
  }
  return 0;
}


void
loginNotify()
{
  UTMP *up;
  int fd;
  char fpath[64];
  BMW bmw;
  BENZ *benz;
  int userno;
  int row, col;		/* �p��L��� */

  usr_fpath(fpath, cuser.userid, FN_BENZ);

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    vs_bar("�t�Ψ�M����");

    bmw.caller = cutmp;
    /* bmw.sender = cuser.userno; */	/* bmw.sender �̹��i�H call �ڡA�ݷ|�A�M�w */
    strcpy(bmw.userid, cuser.userid);
    strcpy(bmw.msg, "�� ����i"BBSNAME"���� [�t�Ψ�M] ��");

    row = 1;
    col = 0;
    mgets(-1);
    while (benz = mread(fd, sizeof(BENZ)))
    {
      /* �L�X���ǤH�b��� */
      if (row < b_lines)
      {
	move(row, col);
	outs(benz->userid);
	col += IDLEN + 1;
	if (col > b_cols + 1 - IDLEN - 1)	/* �`�@�i�H�� (b_cols + 1) / (IDLEN + 1) �� */
	{
	  row++;
	  col = 0;
	}
      }

      userno = benz->userno;
      up = utmp_find(userno);

      if (up && !(up->status & STATUS_REJECT) && can_see(up, cutmp))	/* ���ݤ����ڤ��q�� */
      {
	/* �n�ͥB�ۤv�S���������ۤ~�i�H reply */
	bmw.sender = (is_mygood(userno) && !(cuser.ufo & UFO_QUIET)) ? cuser.userno : 0;
	bmw.recver = userno;
	bmw_send(up, &bmw);
	outc('*');	/* Thor.980707: ���q���쪺���Ҥ��P */
      }
    }
    close(fd);
    unlink(fpath);
    vmsg("�o�ǨϥΪ̳]�z���W����M�A�� * ��ܥثe�b���W");
  }
}
#endif


#ifdef LOG_TALK
static void
talk_save()
{
  char fpath[64];
  
  /* lkchu.981201: ��i�p�H�H�c��/�M�� */
  usr_fpath(fpath, cuser.userid, FN_TALK_LOG);

  if (!(cuser.ufo & UFO_NTLOG) && vans("������Ѭ����B�z (M)�Ƨѿ� (C)�M���H[M] ") != 'c')
    mail_self(fpath, cuser.userid, "[�� �� ��] ��Ѭ���", MAIL_READ | MAIL_NOREPLY);

  unlink(fpath);
}
#endif


/* ----------------------------------------------------- */
/* talk sub-routines					 */
/* ----------------------------------------------------- */


static char page_requestor[40];
#ifdef HAVE_MULTI_BYTE
static int page_requestor_zhc;
#endif

/* �C�C�i�H��J���r�Ƭ� SCR_WIDTH */
#ifdef HAVE_MULTI_BYTE
static uschar talk_pic[T_LINES][SCR_WIDTH + 1];	/* �R���C���r�ɷ|�Ψ��@�X���ťաA�ҥH�n�h�@�X */
#else
static uschar talk_pic[T_LINES][SCR_WIDTH + 2];	/* �R���C������r�ɷ|�Ψ��G�X���ťաA�ҥH�n�h�G�X */
#endif
static int talk_len[T_LINES];			/* �C�C�ثe�w��J�h�֦r */


static void
talk_clearline(ln, col)
  int ln, col;
{
  int i, len;

  len = talk_len[ln];
  for (i = col; i < len; i++)
    talk_pic[ln][i] = ' ';

  talk_len[ln] = col;
}


static void
talk_outs(str, len)
  uschar *str;
  int len;
{
  int ch;
  uschar *end;

  /* �M�@�몺 outs() �O�ۦP�A�u�O����L len �Ӧr */
  end = str + len;
  while (ch = *str)
  {
    outc(ch);
    if (++str >= end)
      break;
  }
}


static void
talk_nextline(twin)
  talk_win *twin;
{
  int curln, max, len, i;

  curln = twin->curln;
  if (curln != twin->eline)
  {
    twin->curln = ++curln;
  }
  else	/* �w�g�O�̫�@�C�A�n�V�W���� */
  {
    max = twin->eline;
    for (curln = twin->sline; curln < max; curln++)
    {
      len = BMAX(talk_len[curln], talk_len[curln + 1]);
      for (i = 0; i < len; i++)
        talk_pic[curln][i] = talk_pic[curln + 1][i];
      talk_len[curln] = talk_len[curln + 1];
      move(curln, 0);
      talk_outs(talk_pic[curln], talk_len[curln]);
      clrtoeol();
    }
  }

  /* �s���@�C */
  talk_clearline(curln, 0);

  twin->curcol = 0;
  move(curln, 0);
  clrtoeol();
}


static void
talk_char(twin, ch)
  talk_win *twin;
  int ch;
{
  int col, ln, len, i;

  col = twin->curcol;
  ln = twin->curln;
  len = talk_len[ln];

  if (isprint2(ch))
  {
    if (col >= SCR_WIDTH)	/* �Y�w�g����C���A�����C */
    {
      talk_nextline(twin);
      col = twin->curcol;
      ln = twin->curln;
      len = talk_len[ln];
    }

    move(ln, col);
    if (col >= len)
    {
      talk_pic[ln][col] = ch;
      outc(ch);
      twin->curcol = ++col;
      talk_len[ln] = col;
    }
    else		/* �n insert */
    {
      for (i = SCR_WIDTH - 1; i > col; i--)
	talk_pic[ln][i] = talk_pic[ln][i - 1];
      talk_pic[ln][col] = ch;
      if (len < SCR_WIDTH)
	len++;
      talk_len[ln] = len;
      talk_outs(talk_pic[ln] + col, len - col);
      twin->curcol = ++col;
      move(ln, col);
    }
  }
  else
  {
    switch (ch)
    {
    case '\n':
      talk_nextline(twin);
      break;

    case KEY_BKSP:		/* backspace */
      if (col > 0)
      {
	if (col > len)
	{
	  /* ���M KEY_LEFT �@�˪��� */
	  twin->curcol = --col;
	  move(ln, col);
	}
	else
	{
	  col--;
#ifdef HAVE_MULTI_BYTE
	  /* hightman.060504: �P�_�{�b�R������m�O�_���~�r����b�q�A�Y�O�R�G�r�� */
	  if (twin->zhc && col && IS_ZHC_LO(talk_pic[ln], col))
	  {
	    col--;
	    ch = 2;
	  }
	  else
#endif
	    ch = 1;
	  for (i = col; i < SCR_WIDTH; i++)
	    talk_pic[ln][i] = talk_pic[ln][i + ch];
	  move(ln, col);
	  talk_outs(talk_pic[ln] + col, len - col);
	  twin->curcol = col;
	  talk_len[ln] = len - ch;
	  move(ln, col);
	}
      }
      break;

    case Ctrl('D'):		/* KEY_DEL */
      if (col < len)
      {
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: �P�_�{�b�R������m�O�_���~�r���e�b�q�A�Y�O�R�G�r�� */
	if (twin->zhc && col < len - 1 && IS_ZHC_HI(talk_pic[ln][col]))
	  ch = 2;
	else
#endif
	  ch = 1;
	for (i = col; i < SCR_WIDTH; i++)
	  talk_pic[ln][i] = talk_pic[ln][i + ch];
	move(ln, col);
	talk_outs(talk_pic[ln] + col, len - col);
	talk_len[ln] = len - ch;
	move(ln, col);
      }
      break;

    case Ctrl('B'):		/* KEY_LEFT */
      if (col > 0)
      {
	col--;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: �����ɸI��~�r������ */
	if (twin->zhc && col && IS_ZHC_LO(talk_pic[ln], col))
	  col--;
#endif
	twin->curcol = col;
	move(ln, col);
      }
      break;

    case Ctrl('F'):		/* KEY_RIGHT */
      if (col < SCR_WIDTH)
      {
	col++;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: �k���ɸI��~�r������ */
	if (twin->zhc && col < SCR_WIDTH && IS_ZHC_HI(talk_pic[ln][col - 1]))
	  col++;
#endif
	twin->curcol = col;
	move(ln, col);
      }
      break;

    case Ctrl('P'):		/* KEY_UP */ 
      if (ln > twin->sline)
      {
	twin->curln = --ln;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: �~�r��r�ո` */
	if (twin->zhc && col < SCR_WIDTH && IS_ZHC_LO(talk_pic[ln], col))
	  col++;
#endif
	move(ln, col);
      }
      break;

    case Ctrl('N'):		/* KEY_DOWN */
      if (ln < twin->eline)
      {
	twin->curln = ++ln;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: �~�r��r�ո` */
	if (twin->zhc && col < SCR_WIDTH && IS_ZHC_LO(talk_pic[ln], col))
	  col++;
#endif
	move(ln, col);
      }
      break;

    case Ctrl('A'):		/* KEY_HOME */
      twin->curcol = 0;
      move(ln, 0);
      break;

    case Ctrl('E'):		/* KEY_END */
      twin->curcol = len;
      move(ln, len);
      break;
    
    case Ctrl('Y'):		/* clear this line */
      talk_clearline(ln, 0);
      twin->curcol = 0;
      move(ln, 0);
      clrtoeol();
      break;

    case Ctrl('K'):		/* clear to end of line */
      talk_clearline(ln, col);
      move(ln, col);
      clrtoeol();
      break;

    case Ctrl('G'):		/* bell */
      bell();
      break;
    }
  }
}


static void
talk_string(twin, str)
  talk_win *twin;
  uschar *str;
{
  int ch;

  while (ch = *str)
  {
    talk_char(twin, ch);
    str++;
  }
}


static void
talk_speak(fd)
  int fd;
{
  talk_win mywin, itswin;
  uschar data[80];
  char buf[80];
  int i, ch;
#ifdef  LOG_TALK
  char mywords[80], itswords[80], itsuserid[40];
  FILE *fp;

#if 0	/* talk log �� algo */
  ��ܨ����O�� mywin & itswin, �u�n��@�� recv ���ܴN�|�e�W������ win, 
  �ҥH�ڭ̥����n����ۤv�����r�P��襴���r���}��.

  ��O�N���ب�� spool, ���O�N mywin/itswin recv �� char ���U�۪� spool 
  �̥�, �ثe�] spool ��n�O�@�C���j�p, �ҥH�u�n�O spool ���F, �άO�I�촫
  ��r��, �N�� spool �̪���Ƽg�^ log, �M��M�� spool, �p���~�� :)
#endif

  /* lkchu: make sure that's empty */
  mywords[0] = itswords[0] = '\0';
  
  strcpy(itsuserid, page_requestor);
  strtok(itsuserid, " (");
#endif

  utmp_mode(M_TALK);

  ch = 58 - strlen(page_requestor);

  sprintf(buf, "%s�i%s", cuser.userid, cuser.username);

  i = ch - strlen(buf);
  if (i >= 0)
  {
    i = (i >> 1) + 1;
  }
  else
  {
    buf[ch] = '\0';
    i = 1;
  }
  memset(data, ' ', i);
  data[i] = '\0';

  memset(&mywin, 0, sizeof(mywin));
  memset(&itswin, 0, sizeof(itswin));

  i = b_lines >> 1;
  mywin.eline = i - 1;
#ifdef HAVE_MULTI_BYTE
  mywin.zhc = cuser.ufo & UFO_ZHC;
#endif
  itswin.curln = itswin.sline = i + 1;
  itswin.eline = b_lines - 1;
#ifdef HAVE_MULTI_BYTE
  itswin.zhc = page_requestor_zhc;
#endif

  clear();
  move(i, 0);
  prints("\033[1;46;37m  �ͤѻ��a  \033[45m%s%s�j ��  %s%s\033[m",
    data, buf, page_requestor, data);
  outf(FOOTER_TALK);
  move(0, 0);

  /* talk_pic �O����ӵe������r�A��l�ȬO�ť� */
  memset(talk_pic, ' ', sizeof(talk_pic));
  /* talk_len �O����ӵe���U�C�w�g�ΤF�h�֦r */
  memset(talk_len, 0, sizeof(talk_len));

#ifdef LOG_TALK				/* lkchu.981201: ��ѰO�� */
  usr_fpath(buf, cuser.userid, FN_TALK_LOG);
  if (fp = fopen(buf, "a+"))
  {
    fprintf(fp, "�i %s �P %s ����ѰO�� �j\n", cuser.userid, page_requestor);
    fprintf(fp, "�}�l��Ѯɶ� [%s]\n", Now());	/* itoc.010108: �O���}�l��Ѯɶ� */
  }
#endif

  add_io(fd, 60);

  for (;;)
  {
    ch = vkey();

#ifdef EVERY_Z
    /* Thor.980725: talk��, ctrl-z */
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

    if (ch == Ctrl('D') || ch == Ctrl('C'))
      break;

    if (ch == I_OTHERDATA)
    {
      ch = recv(fd, data, 80, 0);
      if (ch <= 0)
	break;

#ifdef HAVE_GAME
      if (data[0] == Ctrl('O'))
      { /* Thor.990219: �I�s�~���ѽL */
	if (DL_func("bin/bwboard.so:vaBWboard", fd, 1) == -2)
	  break;
	continue;
      }
#endif
      for (i = 0; i < ch; i++)
      {
	talk_char(&itswin, data[i]);

#ifdef	LOG_TALK		/* ��軡���� */
	switch (data[i])
	{
	case '\n':
	  /* lkchu.981201: �����C�N�� itswords �L�X�M�� */
	  if (itswords[0] != '\0')
  	  {
  	    fprintf(fp, "\033[32m%s�G%s\033[m\n", itsuserid, itswords);
	    itswords[0] = '\0';
	  }
	  break;

	case KEY_BKSP:	/* lkchu.981201: backspace */
	  itswords[strlen(itswords) - 1] = '\0';
	  break;

	default:
	  if (isprint2(data[i]))
	  {
	    if (strlen(itswords) < sizeof(itswords))
  	    {
  	      strncat(itswords, (char *)&data[i], 1);
	    }
	    else	/* lkchu.981201: itswords �˺��F */
	    {
  	      fprintf(fp, "\033[32m%s�G%s%c\033[m\n", itsuserid, itswords, data[i]);
	      itswords[0] = '\0';
	    }
	  }
	  break;
	}
#endif

      }
    }

#ifdef HAVE_GAME
    else if (ch == Ctrl('O'))
    { /* Thor.990219: �I�s�~���ѽL */
      data[0] = ch;
      if (send(fd, data, 1, 0) != 1)
	break;
      if (DL_func("bin/bwboard.so:vaBWboard", fd, 0) == -2)
	break;
    }
#endif

    else if (ch == Ctrl('T'))
    {
      if (cuser.userlevel)	/* guest ���i��Q�����ܽ� Talk */
      {
	cuser.ufo ^= UFO_PAGER;
	cutmp->ufo = cuser.ufo;
	talk_string(&mywin, (cuser.ufo & UFO_PAGER) ? "�� �����I�s��\n" : "�� ���}�I�s��\n");
      }
    }

    else
    {
      switch (ch)
      {
      case KEY_DEL:
	ch = Ctrl('D');
	break;

      case KEY_LEFT:
	ch = Ctrl('B');
	break;

      case KEY_RIGHT:
	ch = Ctrl('F');
	break;

      case KEY_UP:
	ch = Ctrl('P');
	break;

      case KEY_DOWN:
	ch = Ctrl('N');
	break;

      case KEY_HOME:
	ch = Ctrl('A');
	break;

      case KEY_END:
	ch = Ctrl('E');
	break;
      }

      data[0] = ch;
      if (send(fd, data, 1, 0) != 1)
	break;

      talk_char(&mywin, ch);

#ifdef LOG_TALK			/* �ۤv������ */
      switch (ch)
      {
      case '\n':
	if (mywords[0] != '\0')
	{
	  fprintf(fp, "%s�G%s\n", cuser.userid, mywords);
	  mywords[0] = '\0';
	}
	break;
      
      case KEY_BKSP:
	mywords[strlen(mywords) - 1] = '\0';
	break;

      default:
	if (isprint2(ch))
	{
	  if (strlen(mywords) < sizeof(mywords))
	  {
	    strncat(mywords, (char *)&ch, 1);
	  }
	  else
	  {
	    fprintf(fp, "%s�G%s%c\n", cuser.userid, mywords, ch);
	    mywords[0] = '\0';
	  }
	}
	break;
      }
#endif

#ifdef EVERY_BIFF 
      /* Thor.980805: ���H�b�����enter�~�ݭncheck biff */ 
      if (ch == '\n')
      {
	static int old_biff; 
	int biff = HAS_STATUS(STATUS_BIFF);
	if (biff && !old_biff) 
	  talk_string(&mywin, "�� ���I�l�t�ӫ��a�F�I\n");
	old_biff = biff; 
      }
#endif
    }
  }

#ifdef LOG_TALK
  /* itoc.021205: �̫�@�y�ܭY�S���� ENTER �N���|�Q�O���i talk.log�A
     �b���S�O�B�z�A���ǩw���� myword �A itsword�A���i��|�ۤ� */
  if (mywords[0] != '\0')
    fprintf(fp, "%s�G%s\n", cuser.userid, mywords);
  if (itswords[0] != '\0')
    fprintf(fp, "\033[32m%s�G%s\033[m\n", itsuserid, itswords);

  fclose(fp);
#endif

  add_io(0, 60);
}


static void
talk_hangup(sock)
  int sock;
{
  cutmp->sockport = 0;
  add_io(0, 60);
  close(sock);
}


static char *talk_reason[] =
{
  "�藍�_�A�ڦ��Ʊ������z talk",
  "�ڲ{�b�ܦ��A�е��@�|��A call ��",
  "�{�b�����L�ӡA���@�U�ڷ|�D�� page �z",
  "�ڲ{�b���Q talk ��",
  "�ܷЫ��A�ڹ�b���Q talk",

#ifdef EVERY_Z
  "�ڪ��L�ڥ����۩M�O�H���ܩO�A�S���Ū��L�ڤF"
  /* Thor.980725: for chat&talk ��^z �@�ǳ� */
#endif
};


/* return 0: �S�� talk, 1: �� talk, -1: ��L */


int
talk_page(up)
  UTMP *up;
{
  int sock, msgsock;
  struct sockaddr_in sin;
  pid_t pid;
  int ans, length;
  char buf[60];
#if     defined(__OpenBSD__)
  struct hostent *h;
#endif

#ifdef EVERY_Z
  /* Thor.980725: �� talk & chat �i�� ^z �@�ǳ� */
  if (vio_holdon())
  {
    vmsg("�z�������@�b�٨S�����C");
    return 0;
  }
#endif

  pid = up->mode;
  if (pid >= M_SYSTEM && pid <= M_CHAT)
  {
    vmsg("���L�v���");
    return 0;
  }

  if (!(pid = up->pid) || kill(pid, 0))
  {
    vmsg(MSG_USR_LEFT);
    return 0;
  }

  /* showplans(up->userid); */

  if (vans("�T�w�n�M�L/�o�ͤѶ�(Y/N)�H[N] ") != 'y')
    return 0;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    return 0;

#if     defined(__OpenBSD__)		      /* lkchu */

  if (!(h = gethostbyname(str_host)))
    return -1;  
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = 0;
  memcpy(&sin.sin_addr, h->h_addr, h->h_length);  

#else

  sin.sin_family = AF_INET;
  sin.sin_port = 0;
  sin.sin_addr.s_addr = INADDR_ANY;
  memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

#endif

  length = sizeof(sin);
  if (bind(sock, (struct sockaddr *) &sin, length) < 0 || 
    getsockname(sock, (struct sockaddr *) &sin, &length) < 0)
  {
    close(sock);
    return 0;
  }

  cutmp->sockport = sin.sin_port;
  strcpy(cutmp->mateid, up->userid);
  up->talker = cutmp;
  utmp_mode(M_PAGE);
  kill(pid, SIGUSR1);

  clear();
  prints("���שI�s %s ...\n�i�� Ctrl-D ����", up->userid);

  listen(sock, 1);
  add_io(sock, 20);
  do
  {
    msgsock = igetch();

    if (msgsock == Ctrl('D'))
    {
      talk_hangup(sock);
      return -1;
    }

    if (msgsock == I_TIMEOUT)
    {
      move(0, 0);
      outs("�A");
      bell();

      if (kill(pid, SIGUSR1)) 
      /* Thor.990201.����: ���o�� kill�A�]�u�O�ݬݹ��O���O�٦b�u�W�Ӥw�A���osignal�����G talk_rqst ���|�A�Q�s */
      {
	talk_hangup(sock);
	vmsg(MSG_USR_LEFT);
	return -1;
      }
    }
  } while (msgsock != I_OTHERDATA);

  msgsock = accept(sock, NULL, NULL);
  talk_hangup(sock);
  if (msgsock == -1)
    return -1;

  length = read(msgsock, buf, sizeof(buf));
  ans = buf[0];
  if (ans == 'y')
  {
    sprintf(page_requestor, "%s (%s)", up->userid, up->username);
#ifdef HAVE_MULTI_BYTE
    page_requestor_zhc = up->ufo & UFO_ZHC;
#endif

    /* Thor.980814.�`�N: �b�����@�����P�n�����i�ౡ�p, �p�G A �� page B, ���b B �^���e�o���W���}, �� page C,
	C �|���^���e, �p�G B �^���F, B �N�|�Q accept, �Ӥ��O C.
	���ɦb�ù�����, �ݨ쪺 page_requestor�|�O C, �i�O�ƹ�W, talk����H�O B, �y�����P�n��!
	�Ȯɤ����ץ�, �H�@����ߪ̪��g�@ :P    */

    talk_speak(msgsock);
  }
  else
  {
    char *reply;

    if (ans == ' ')
    {
      reply = buf;
      reply[length] = '\0';
    }
    else
      reply = talk_reason[ans - '1'];

    move(4, 0);
    outs("�i�^���j");
    outs(reply);
  }

  close(msgsock);
  cutmp->talker = NULL;
#ifdef LOG_TALK
  if (ans == 'y')  	/* itoc.000512: ����Talk�Q�ڵ��ɡA���Ͳ�ѰO����record */
    talk_save();	/* lkchu.981201: talk �O���B�z */  
#endif
  vmsg("��ѵ���");
  return 1;
}


/* ----------------------------------------------------- */
/* talk main-routines					 */
/* ----------------------------------------------------- */


int
t_pager()
{
#if 0	/* itoc.010923: �קK�~���A���M���@�I */
  �z�w�w�w�w�s�w�w�w�w�w�s�w�w�w�w�w�s�w�w�w�w�w�{
  �x        �x UFO_PAGER�x UFO_RCVER�x UFO_QUIET�x
  �u�w�w�w�w�q�w�w�w�w�w�q�w�w�w�w�w�q�w�w�w�w�w�t
  �x�����}��x          �x          �x          �x
  �u�w�w�w�w�q�w�w�w�w�w�q�w�w�w�w�w�q�w�w�w�w�w�t   
  �x�n�ͱM�u�x    ��    �x          �x          �x
  �u�w�w�w�w�q�w�w�w�w�w�q�w�w�w�w�w�q�w�w�w�w�w�t
  �x�������ۢx    ��    �x    ��    �x    ��    �x
  �|�w�w�w�w�r�w�w�w�w�w�r�w�w�w�w�w�r�w�w�w�w�w�}
#endif

  switch (vans("���������� 1)�����}�� 2)�n�ͱM�u 3)�������� [Q] "))
  {
  case '1':
#ifdef HAVE_NOBROAD
    cuser.ufo &= ~(UFO_PAGER | UFO_RCVER | UFO_QUIET);
#else
    cuser.ufo &= ~(UFO_PAGER | UFO_QUIET);
#endif
    cutmp->ufo = cuser.ufo;
    return 0;	/* itoc.010923: ���M���ݭn��ø�ù��A���O�j����ø�~���s feeter ���� pager �Ҧ� */

  case '2':
    cuser.ufo |= UFO_PAGER;
#ifdef HAVE_NOBROAD
    cuser.ufo &= ~(UFO_RCVER | UFO_QUIET);
#else
    cuser.ufo &= ~UFO_QUIET;
#endif
    cutmp->ufo = cuser.ufo;
    return 0;

  case '3':
#ifdef HAVE_NOBROAD
    cuser.ufo |= (UFO_PAGER | UFO_RCVER | UFO_QUIET);
#else
    cuser.ufo |= (UFO_PAGER | UFO_QUIET);
#endif
    cutmp->ufo = cuser.ufo;
    return 0;
  }

  return XEASY;  
}


int
t_cloak()
{
#ifdef HAVE_SUPERCLOAK
  if (HAS_PERM(PERM_ALLADMIN))
  {
    switch (vans("�����Ҧ� 1)�@������ 2)�������� 3)������ [Q] "))
    {
    case '1':
      cuser.ufo |= UFO_CLOAK;
      cuser.ufo &= ~UFO_SUPERCLOAK;
      vmsg("�K�K�A���_���o�I");
      break;

    case '2':
      cuser.ufo |= (UFO_CLOAK | UFO_SUPERCLOAK);
      vmsg("�K�K�A�ð_���o�A��L�����]�ݤ���ڡI");
      break;

    case '3':
      cuser.ufo &= ~(UFO_CLOAK | UFO_SUPERCLOAK);
      vmsg("���{����F");
      break;
    }
  }
  else
#endif
  if (HAS_PERM(PERM_CLOAK))
  {
    cuser.ufo ^= UFO_CLOAK;
    vmsg(cuser.ufo & UFO_CLOAK ? "�K�K�A���_���o�I" : "���{����F");
  }

  cutmp->ufo = cuser.ufo;
  return XEASY;
}


int
t_query()
{
  ACCT acct;

  vs_bar("�d�ߺ���");
  if (acct_get(msg_uid, &acct) > 0)
  {
    move(1, 0);
    clrtobot();
    do_query(&acct);
  }

  return 0;
}


static int
talk_choose()
{
  UTMP *up, *ubase, *uceil;
  int self;
  char userid[IDLEN + 1];

  ll_new();

  self = cuser.userno;
  ubase = up = ushm->uslot;
  /* uceil = ushm->uceil; */
  uceil = ubase + ushm->count;
  do
  {
    if (up->pid && up->userno != self && up->userlevel && can_see(cutmp, up))
      ll_add(up->userid);
  } while (++up <= uceil);

  if (!vget(1, 0, msg_uid, userid, IDLEN + 1, GET_LIST))
    return 0;

  up = ubase;
  do
  {
    if (!str_cmp(userid, up->userid))
      return up->userno;
  } while (++up <= uceil);

  return 0;
}


int
t_talk()
{
  int tuid, unum, ucount;
  UTMP *up;
  char ans[4];

  if (total_user <= 1)
  {
    zmsg("�ثe�u�W�u���z�@�H�A���ܽФj�a�ӥ��{�i" BBSNAME "�j�a�I");
    return XEASY;
  }

  tuid = talk_choose();
  if (!tuid)
    return 0;

  /* ----------------- */
  /* multi-login check */
  /* ----------------- */

  move(3, 0);
  unum = 1;
  while ((ucount = utmp_count(tuid, 0)) > 1)
  {
    outs("(0) ���Q talk �F...\n");
    utmp_count(tuid, 1);
    vget(1, 33, "�п�ܤ@�Ӳ�ѹ�H [0]�G", ans, 3, DOECHO);
    unum = atoi(ans);
    if (unum == 0)
      return 0;
    move(3, 0);
    clrtobot();
    if (unum > 0 && unum <= ucount)
      break;
  }

  if (up = utmp_search(tuid, unum))
  {
    if (can_override(up))
    {
      if (tuid != up->userno)
	vmsg(MSG_USR_LEFT);
      else
	talk_page(up);
    }
    else
    {
      vmsg("��������I�s���F");
    }
  }

  return 0;
}


/* ------------------------------------- */
/* ���H�Ӧ���l�F�A�^���I�s��		 */
/* ------------------------------------- */


void
talk_rqst()
{
  UTMP *up;
  int mode, sock, ans, len, port;
  char buf[80];
  struct sockaddr_in sin;
  screenline sl[T_LINES];
#if     defined(__OpenBSD__)
  struct hostent *h;
#endif

  up = cutmp->talker;
  if (!up)
    return;

  port = up->sockport;
  if (!port)
    return;

  mode = bbsmode;
  utmp_mode(M_TRQST);

  vs_save(sl);

  clear();
  sprintf(page_requestor, "%s (%s)", up->userid, up->username);

#ifdef EVERY_Z
  /* Thor.980725: �� talk & chat �i�� ^z �@�ǳ� */

  if (vio_holdon())
  {
    sprintf(buf, "%s �Q�M�z��A���L�z�u���@�i�L", page_requestor);
    vmsg(buf);
    buf[0] = ans = '6';		/* Thor.980725: �u���@�i�L */
    len = 1;
    goto over_for;
  }
#endif

  bell();
  prints("�z�Q�� %s ��ѶܡH(�Ӧ� %s)", page_requestor, up->from);
  for (;;)	/* ���ϥΪ̥i�H���d�߭n�D��Ѫ���� */
  {
    ans = vget(1, 0, "==> Q)�d�� Y)��� N)�����H[Y] ", buf, 3, LCECHO);
    if (ans == 'q')
      my_query(up->userid);
    else
      break;
  }

  len = 1;

  if (ans == 'n')
  {
    move(2, 0);
    clrtobot();
    for (ans = 0; ans < 5; ans++)
      prints("\n (%d) %s", ans + 1, talk_reason[ans]);
    ans = vget(10, 0, "�п�J�ﶵ�Ψ�L���� [1]�G\n==> ", buf + 1, 60, DOECHO);

    if (ans == 0)
      ans = '1';

    if (ans >= '1' && ans <= '5')
    {
      buf[0] = ans;
    }
    else
    {
      buf[0] = ans = ' ';
      len = strlen(buf);
    }
  }
  else
  {
    buf[0] = ans = 'y';
  }

#ifdef EVERY_Z
over_for:
#endif

  sock = socket(AF_INET, SOCK_STREAM, 0);

#if     defined(__OpenBSD__)		      /* lkchu */

  if (!(h = gethostbyname(str_host)))
    return;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = port;
  memcpy(&sin.sin_addr, h->h_addr, h->h_length);
  
#else

  sin.sin_family = AF_INET;
  sin.sin_port = port;
  /* sin.sin_addr.s_addr = INADDR_LOOPBACK; */
  /* sin.sin_addr.s_addr = INADDR_ANY; */
  /* For FreeBSD 4.x */
  sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

#endif

  if (!connect(sock, (struct sockaddr *) & sin, sizeof(sin)))
  {
    send(sock, buf, len, 0);

    if (ans == 'y')
    {
      strcpy(cutmp->mateid, up->userid);
#ifdef HAVE_MULTI_BYTE
      page_requestor_zhc = up->ufo & UFO_ZHC;
#endif

      talk_speak(sock);
    }
  }

  close(sock);
#ifdef  LOG_TALK
  if (ans == 'y')	/* mat.991011: ����Talk�Q�ڵ��ɡA���Ͳ�ѰO����record */
    talk_save();	/* lkchu.981201: talk �O���B�z */
#endif
  vs_restore(sl);
  utmp_mode(mode);
}
