/*-------------------------------------------------------*/
/* visio.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : VIrtual Screen Input Output routines 	 */
/* create : 95/03/29				 	 */
/* update : 96/10/10				 	 */
/*-------------------------------------------------------*/


#include <stdarg.h>
#include <arpa/telnet.h>


#include "bbs.h"


#define VO_MAX		3072	/* output buffer �j�p */
#define VI_MAX		256	/* input buffer �j�p */


#define INPUT_ACTIVE	0
#define INPUT_IDLE	1


static int cur_row, cur_col;
static int cur_pos;			/* current position with ANSI codes */


/* ----------------------------------------------------- */
/* �~�r (zh-char) �P�_					 */
/* ----------------------------------------------------- */


int			/* 1:�O 0:���O */
is_zhc_low(str, n)	/* hightman.060504: �P�_�r�ꤤ���� n �Ӧr�ŬO�_���~�r����b�r */
  char *str;
  int n;
{
  char *end;

  end = str + n;
  while (str < end)
  {
    if (!*str)
      return 0;
    if (IS_ZHC_HI(*str))
      str++;
    str++;
  }

  return (str - end);
}


/* ----------------------------------------------------- */
/* output routines					 */
/* ----------------------------------------------------- */


static uschar vo_pool[VO_MAX];
static int vo_size;


#ifdef	VERBOSE
static void
telnet_flush(data, size)
  char *data;
  int size;
{
  int oset;

  oset = 1;

  if (select(1, NULL, &oset, NULL, NULL) <= 0)
  {
    abort_bbs();
  }

  xwrite(0, data, size);
}

#else

# define telnet_flush(data, size)	send(0, data, size, 0)
#endif


static void
oflush()
{
  int size;

  if (size = vo_size)
  {
    telnet_flush(vo_pool, size);
    vo_size = 0;
  }
}


static void
output(str, len)
  uschar *str;
  int len;
{
  int size, ch;
  uschar *data;

  size = vo_size;
  data = vo_pool;
  if (size + len > VO_MAX - 8)
  {
    telnet_flush(data, size);
    size = len;
  }
  else
  {
    data += size;
    size += len;
  }

  while (--len >= 0)
  {
    ch = *str++;
    *data++ = ch;
    if (ch == IAC)
    {
      *data++ = ch;
      size++;
    }
  }
  vo_size = size;
}


static void
ochar(ch)
  int ch;
{
  uschar *data;
  int size;

  data = vo_pool;
  size = vo_size;
  if (size > VO_MAX - 2)
  {
    telnet_flush(data, size);
    size = 0;
  }
  data[size++] = ch;
  vo_size = size;
}


void
bell()
{
  static char sound[1] = {Ctrl('G')};

  telnet_flush(sound, sizeof(sound));
}


/* ----------------------------------------------------- */
/* virtual screen					 */
/* ----------------------------------------------------- */


#define	o_ansi(x)	output(x, sizeof(x)-1)

#define o_clear()	o_ansi("\033[;H\033[2J")
#define o_cleol()	o_ansi("\033[K")
#define o_standup()	o_ansi("\033[7m")
#define o_standdown()	o_ansi("\033[m")


static int docls;
static int roll;
static int scrollcnt, tc_col, tc_row;


static screenline vbuf[T_LINES];
static screenline *cur_slp;	/* current screen line pointer */


/* itoc.020611.����: �y�����ӬO (x, y) �Ӥ��O (y, x)
   �]�� row �O �V�U�����Acolumn �O�V�k�����A
   �� +x cross +y = +z �O�X�ù��� */

void
move(x, y)
  int x;	/* row */
  int y;	/* column */
{
  screenline *cslp;

  /* itoc.070517.����: �e���j�p�O (b_lines+1)*(b_cols+1)�A���\���y�� (x,y) �d��O x=0~b_lines�By=0~b_cols */

  if (x > b_lines)
    return;

  if (y > b_cols)
    y = 0;

  cur_row = x;
  if ((x += roll) > b_lines)
    x -= b_lines + 1;
  cur_slp = cslp = &vbuf[x];
  cur_col = y;

#if 0

  /* ------------------------------------- */
  /* �L�o ANSI codes�A�p���Яu���Ҧb��m */
  /* ------------------------------------- */

  if (y)
  {
    int ch, ansi;
    int len;
    uschar *str;

    ansi = 0;
    x = y;
    len = cslp->len;
    str = cslp->data;
    str[len] = '\0';
    while (len && (ch = *str))
    {
      str++;
      len--;

      if (ansi)
      {
	x++;
	if (ch == 'm')
	  ansi = 0;
	continue;
      }
      if (ch == KEY_ESC)
      {
	x++;
	ansi = 1;
	continue;
      }
      y--;
      if (y <= 0)
	break;
    }
    y = x;
  }
#endif

  cur_pos = y;
}


#if 0
static void
getxy(x, y)
  int *x, *y;
{
  *x = cur_row;
  *y = cur_col;
}
#endif


/*-------------------------------------------------------*/
/* �p�� slp �� len ���B����� column �Ҧb		 */
/*-------------------------------------------------------*/


#if 0
static int
ansicol(slp, len)
  screenline *slp;
  int len;
{
  uschar *str;
  int ch, ansi, col;

  if (!len || !(slp->mode & SL_ANSICODE))
    return len;

  ansi = col = 0;
  str = slp->data;

  while (len-- && (ch = *str++))
  {
    if (ch == KEY_ESC && *str == '[')
    {
      ansi = 1;
      continue;
    }
    if (ansi)
    {
      if (ch == 'm')
	ansi = 0;
      continue;
    }
    col++;
  }
  return col;
}
#endif


static void
rel_move(new_col, new_row)
  int new_col, new_row;
{
  int was_col, was_row;
  char buf[16];

  if (new_row > b_lines || new_col > b_cols)
    return;

  was_col = tc_col;
  was_row = tc_row;

  tc_col = new_col;
  tc_row = new_row;

  if (new_col == 0)
  {
    if (new_row == was_row)
    {
      if (was_col)
	ochar('\r');
      return;
    }
    else if (new_row == was_row + 1)
    {
      ochar('\n');
      if (was_col)
	ochar('\r');
      return;
    }
  }

  if (new_row == was_row)
  {
    if (was_col == new_col)
      return;

    if (new_col == was_col - 1)
    {
      ochar(KEY_BKSP);
      return;
    }
  }

  sprintf(buf, "\033[%d;%dH", new_row + 1, new_col + 1);
  output(buf, strlen(buf));
}


static void
standoutput(slp, ds, de)
  screenline *slp;
  int ds, de;
{
  uschar *data;
  int sso, eso;

  data = slp->data;
  sso = slp->sso;
  eso = slp->eso;

  if (eso <= ds || sso >= de)
  {
    output(data + ds, de - ds);
    return;
  }

  if (sso > ds)
    output(data + ds, sso - ds);
  else
    sso = ds;

  o_standup();
  output(data + sso, BMIN(eso, de) - sso);
  o_standdown();

  if (de > eso)
    output(data + eso, de - eso);
}


#define	STANDOUT	cur_slp->sso = cur_pos; cur_slp->mode |= SL_STANDOUT;
#define	STANDEND	cur_slp->eso = cur_pos;


#if 0
static int standing;


static void
standout()
{
  if (!standing)
  {
    standing = 1;
    cur_slp->sso = cur_slp->eso = cur_pos;
    cur_slp->mode |= SL_STANDOUT;
  }
}


static void
standend()
{
  if (standing)
  {
    standing = 0;
    if (cur_slp->eso < cur_pos)
      cur_slp->eso = cur_pos;
  }
}
#endif


static void
vs_redraw()
{
  screenline *slp;
  int i, j, len, mode, width;

  tc_col = tc_row = docls = scrollcnt = vo_size = i = 0;
  o_clear();
  for (slp = &vbuf[j = roll]; i <= b_lines; i++, j++, slp++)
  {
    if (j > b_lines)
    {
      j = 0;
      slp = vbuf;
    }

    len = slp->len;
    width = slp->width;
    slp->oldlen = width;
    mode = slp->mode &=
      (len <= slp->sso) ? ~(SL_MODIFIED | SL_STANDOUT) : ~(SL_MODIFIED);
    if (len)
    {
      rel_move(0, i);

      if (mode & SL_STANDOUT)
	standoutput(slp, 0, len);
      else
	output(slp->data, len);

      tc_col = width;
    }
  }
  rel_move(cur_col, cur_row);
  oflush();
}


void
refresh()
{
  screenline *slp;
  int i, j, len, mode, width, smod, emod;

  i = scrollcnt;

  if (docls || abs(i) >= b_lines)
  {
    vs_redraw();
    return;
  }

  if (i)
  {
    char buf[T_LINES];

    scrollcnt = j = 0;
    if (i < 0)
    {
      sprintf(buf, "\033[%dL", -i);
      i = strlen(buf);
    }
    else
    {
      do
      {
	buf[j] = '\n';
      } while (++j < i);
      j = b_lines;
    }
    rel_move(0, j);
    output(buf, i);
  }

  for (i = 0, slp = &vbuf[j = roll]; i <= b_lines; i++, j++, slp++)
  {
    if (j > b_lines)
    {
      j = 0;
      slp = vbuf;
    }

    len = slp->len;
    width = slp->width;
    mode = slp->mode;

    if (mode & SL_MODIFIED)
    {
      slp->mode = mode &=
	(len <= slp->sso) ? ~(SL_MODIFIED | SL_STANDOUT) : ~(SL_MODIFIED);

      if ((smod = slp->smod) < len)
      {
	emod = slp->emod + 1;
	if (emod >= len)
	  emod = len;

	rel_move(smod, i);

	/* rel_move(ansicol(slp, smod), i); */

	if (mode & SL_STANDOUT)
	  standoutput(slp, smod, emod);
	else
	  output(&slp->data[smod], emod - smod);

	/* tc_col = ansicol(slp, emod); */

#if 0
	if (mode & SL_ANSICODE)
	{
	  uschar *data;

	  data = slp->data;
	  mode = 0;
	  len = emod;

	  while (len--)
	  {
	    smod = *data++;
	    if (smod == KEY_ESC)
	    {
	      mode = 1;
	      emod--;
	      continue;
	    }

	    if (mode)
	    {
	      if (smod == 'm')
		mode = 0;
	      emod--;
	    }
	  }
	}

	tc_col = emod;
#endif

	tc_col = (width != len) ? width : emod;
      }
    }

    if (slp->oldlen > width)
    {
      rel_move(width, i);
      o_cleol();
    }
    slp->oldlen = width;
  }
  rel_move(cur_col, cur_row);
  oflush();
}


void
clear()
{
  int i;
  screenline *slp;

  docls = 1;
  cur_pos = cur_col = cur_row = roll = i = 0;
  cur_slp = slp = vbuf;
  while (i++ <= b_lines)
  {
    /* memset(slp, 0, sizeof(screenline)); */
    /* �u�� slp->data[0] = '\0' �Y�i�A���ݲM��� ANSILINELEN */
    memset(slp, 0, sizeof(screenline) - ANSILINELEN + 1);
    slp++;
  }
}


void
clrtoeol()	/* clear screen to end of line (�C��) */
{
  screenline *slp = cur_slp;
  int len;

  if (len = cur_pos)
  {
    slp->len = len;
    slp->width = cur_col;
  }
  else
  {
    /* �M�� oldlen �H�᪺�����Fdata �u�ݲM�� byte �Y�i */
    memset((char *) slp + sizeof(slp->oldlen), 0, sizeof(screenline) - ANSILINELEN + 1 - sizeof(slp->oldlen));
  }
}


void
clrtobot()	/* clear screen to bottom (�ù�����) */
{
  screenline *slp;
  int i, j;

  i = cur_row;
  j = i + roll;
  slp = cur_slp;
  while (i <= b_lines)
  {
    if (j > b_lines)
    {
      j = 0;
      slp = vbuf;
    }
    /* �M�� oldlen �H�᪺�����Fdata �u�ݲM�� byte �Y�i */
    memset((char *) slp + sizeof(slp->oldlen), 0, sizeof(screenline) - ANSILINELEN + 1 - sizeof(slp->oldlen));

    i++;
    j++;
    slp++;
  }
}


void
outc(ch)
  int ch;
{
  screenline *slp;
  uschar *data;
  int i, cy, pos;

  static char ansibuf[16] = "\033";
  static int ansipos = 0;

  slp = cur_slp;
  pos = cur_pos;

  if (ch == '\n')
  {
    cy = cur_col;

new_line:

    ansipos = 0;
    if (pos)
    {
      slp->len = pos;
      slp->width = cy;

#if 0
      if (standing)
      {
	standing = 0;
	if (pos <= slp->sso)
	  slp->mode &= ~SL_STANDOUT;
	else if (slp->eso < pos)
	  slp->eso = pos;
      }
#endif
    }
    else
    {
      /* �M�� oldlen �H�᪺�����Fdata �u�ݲM�� byte �Y�i */
      memset((char *) slp + sizeof(slp->oldlen), 0, sizeof(screenline) - ANSILINELEN + 1 - sizeof(slp->oldlen));
    }

    move(cur_row + 1, 0);
    return;
  }

  if (ch < 0x20)
  {
    if (ch == KEY_ESC)
      ansipos = 1;

    return;
  }

  data = &(slp->data[pos]);	/* ���V�ثe��X��m */

  /* -------------------- */
  /* �ɨ��һݭn���ťզr�� */
  /* -------------------- */

  cy = slp->len - pos;
  if (cy > 0)
  {
    cy = *data;
  }
  else
  {
    while (cy < 0)
    {
      data[cy++] = ' ';
    }

    slp->len = /* slp->width = */ pos + 1;
  }

  /* ---------------------------- */
  /* ANSI control code ���S�O�B�z */
  /* ---------------------------- */

  if (i = ansipos)
  {
    if ((i < 15) &&
      ((ch >= '0' && ch <= '9') || ch == '[' || ch == 'm' || ch == ';'))
    {
      ansibuf[i++] = ch;

      if (ch != 'm')
      {
	ansipos = i;
	return;
      }

      ch = i + pos;
      if (ch < ANSILINELEN - 1)
      {
	memcpy(data, ansibuf, i);
	slp->len = slp->emod = cur_pos = ch;
	slp->mode |= SL_MODIFIED;
	if (slp->smod > pos)
	  slp->smod = pos;
      }
    }
    ansipos = 0;
    return;
  }

  /* ---------------------------- */
  /* �P�w���Ǥ�r�ݭn���s�e�X�ù� */
  /* ---------------------------- */

  if ( /* !(slp->mode & SL_ANSICODE) && */ (ch != cy))
  {
    *data = ch;
    cy = slp->mode;
    if (cy & SL_MODIFIED)
    {
      if (slp->smod > pos)
	slp->smod = pos;
      if (slp->emod < pos)
	slp->emod = pos;
    }
    else
    {
      slp->mode = cy | SL_MODIFIED;
      slp->smod = slp->emod = pos;
    }
  }

  cur_pos = ++pos;
  cy = ++cur_col;

  if ((pos >= ANSILINELEN) /* || (cy > b_cols) */ )
    goto new_line;

  if (slp->width < cy)
    slp->width = cy;
}


void
outs(str)
  uschar *str;
{
  int ch;

  while (ch = *str)
  {
    outc(ch);
    str++;
  }
}


/* ----------------------------------------------------- */
/* eXtended output: �q�X user �� name �M nick		 */
/* ----------------------------------------------------- */


#ifdef SHOW_USER_IN_TEXT
void
outx(str)
  uschar *str;
{
  int ch;

  while (ch = *str)
  {
    /* itoc.020301: ESC + * + s ������X */
    if (ch == KEY_ESC && str[1] == '*')
    {
      switch (str[2])
      {
      case 's':		/* **s ��� ID */
	outs(cuser.userid);
	str += 3;
	continue;
      case 'n':		/* **n ��ܼʺ� */
	outs(cuser.username);
	str += 3;
	continue;
      }
    }
    outc(ch);
    str++;
  }
}
#endif


/* ----------------------------------------------------- */
/* clear the bottom line and show the message		 */
/* ----------------------------------------------------- */


void
outz(str)
  uschar *str;
{
  move(b_lines, 0);
  clrtoeol();
  outs(str);
}


void
outf(str)
  uschar *str;
{
  outz(str);
  prints("%*s\033[m", d_cols, "");
}


void
prints(char *fmt, ...)
{
  va_list args;
  uschar buf[512], *str;	/* �̪��u��L 512 �r */
  int cc;

  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);
  for (str = buf; cc = *str; str++)
    outc(cc);
}


void
scroll()
{
  scrollcnt++;
  if (++roll > b_lines)
    roll = 0;
  move(b_lines, 0);
  clrtoeol();
}


void
rscroll()
{
  scrollcnt--;
  if (--roll < 0)
    roll = b_lines;
  move(0, 0);
  clrtoeol();
}


/* ----------------------------------------------------- */


static int old_col, old_row, old_roll;
static int old_pos; /* Thor.990401: �h�s�@�� */


/* static void */
void		/* Thor.981028: ���F�� talk.c ���H�I�s�ɷ|show�r */
cursor_save()
{
  old_col = cur_col;
  old_row = cur_row;

  old_pos = cur_pos; /* Thor.990401: �h�s�@�� */
}


/* static void */
void		/* Thor.981028: ���F�� talk.c ���H�I�s�ɷ|show�r */
cursor_restore()
{
  move(old_row, old_col);
  
  cur_pos = old_pos; /* Thor.990401: �h�٭�@�� */
}


void
save_foot(slp)
  screenline *slp;
{
  int i;
  int lines[3] = {0, b_lines, b_lines - 1};	/* �x�s�o�T�C */

  for (i = 0; i < 3; i++)
  {
    move(lines[i], 0);
    memcpy(slp + i, cur_slp, sizeof(screenline));
    slp[i].smod = 0;
    slp[i].emod = ANSILINELEN;	/* Thor.990125: ���׳̫�@������, �����nø�W */
    slp[i].oldlen = ANSILINELEN;
    slp[i].mode |= SL_MODIFIED;
  }
}


void
restore_foot(slp, line)
  screenline *slp;
  int line;		/* �n��_ lines[] �̭����e�X�C */
{
  int i;
  int lines[3] = {0, b_lines, b_lines - 1};	/* ��_�o�T�C */

  for (i = 0; i < line; i++)
  {
    move(lines[i], 0);
    memcpy(cur_slp, slp + i, sizeof(screenline));
  }
}


int
vs_save(slp)
  screenline *slp;
{
  old_roll = roll;
  memcpy(slp, vbuf, sizeof(screenline) * (b_lines + 1));
  return old_roll;	/* itoc.030723: �Ǧ^�ثe�� roll */
}


void
vs_restore(slp)
  screenline *slp;
{
  memcpy(vbuf, slp, sizeof(screenline) * (b_lines + 1));
  roll = old_roll;
  vs_redraw();
}


#if 0
int
imsg(msg)			/* itoc.010827: ���n�T����� important message */
  char *msg;			/* length <= 54 */
{
  int i;
  time_t now;
  char scroller[128], spacebar[60], buf[80];
  char alphabet[26][3] = 
  {
    "��", "��", "��", "��", "��", "��", "��", "��", "��", 
    "��", "��", "��", "��", "��", "��", "��", "��", "��", 
    "��", "��", "��", "��", "�@", "�A", "�B", "�C"
  };

  time(&now);				/* �ήɶ��� random */

#if 1	/* �Q�ζ]���O�ĪG�Ӵ��ܭ��n�T�� */
  i = now % 6 + 1;			/* �C��X */
  sprintf(spacebar, "\033[%d;H", b_lines + 1);	/* ����X */
  sprintf(buf, "\033[3%dm�j�k�l�m�n�o\033[1;37;4%dm   ���n�T���Ъ`�N   \033[m", i, i);

  /* ���M�� b_lines�A�Ϧ��H�X���ĪG */
  for (i = 1; i <= 47; i += 2)
  {
    sprintf(scroller, "%s%s", spacebar, buf);		/* scroller �]���O */
    strcat(spacebar, "  ");				/* �@�����G��A�W�[�t�� */
    telnet_flush(scroller, strlen(scroller) + 1);	/* �Y�ɿ�X�A�]���O�ĪG */
    usleep(1000);
  }
#endif

  i = now % 26;		/* 'a' ~ 'z' �è��@�� */

  if (msg)
  {
    move(b_lines, 0);
    clrtoeol();
    prints(COLOR1 " �� %-55s " COLOR2 " [�Ы� %s ���~��] \033[m", msg, alphabet[i]);
  }
  else
  {
    move(b_lines, 27);
    prints(COLOR1 " �� �Ы� %s ���~�� �� \033[m", alphabet[i]);
  }

  i += 'a';
  while (vkey() != i)
    ;

  return i;
}
#endif	/* VIEW_IMSG */


#ifdef POPUP_MESSAGE
int
vmsg(msg)
  char *msg;			/* length <= 54 */
{
  if (msg)
    return pmsg(msg);

  move(b_lines, 0);
  outs(VMSG_NULL);
  move(b_lines, 0);	/* itoc.010127: �ץ��b�������k����ΤU�A������|�����G�h��檺���D */
  return vkey();
}
#else
int
vmsg(msg)
  char *msg;			/* length <= 54 */
{
  if (msg)
  {
    move(b_lines, 0);
    clrtoeol();
    prints(COLOR1 " �� %-55s " COLOR2 " [�Ы����N���~��] \033[m", msg);
  }
  else
  {
    move(b_lines, 0);
    outs(VMSG_NULL);
    move(b_lines, 0);	/* itoc.010127: �ץ��b�������k����ΤU�A������|�����G�h��檺���D */
  }
  return vkey();
}
#endif


static inline void
zkey()				/* press any key or timeout */
{
  /* static */ struct timeval tv = {1, 100};  
  /* Thor.980806: man page ���] timeval struct�O�|���ܪ� */

  int rset;

  rset = 1;
  select(1, (fd_set *) &rset, NULL, NULL, &tv);

#if 0
  if (select(1, &rset, NULL, NULL, &tv) > 0)
  {
    recv(0, &rset, sizeof(&rset), 0);
  }
#endif
}


void
zmsg(msg)			/* easy message */
  char *msg;
{
  outz(msg);
  move(b_lines, 0);	/* itoc.031029: �ץ��b�������k����ΤU�A������|�����G�h��檺���D */

  refresh();
  zkey();
}


void
vs_bar(title)
  char *title;
{
  clear();
  prints("\033[1;33;44m�i %s �j\033[m\n", title);
}


static void
vs_line(msg)
  char *msg;
{
  int head, tail;

  if (msg)
    head = (strlen(msg) + 1) >> 1;
  else
    head = 0;

  tail = head;

  while (head++ < 38)
    outc('-');

  if (tail)
  {
    outc(' ');
    outs(msg);
    outc(' ');
  }

  while (tail++ < 38)
    outc('-');
  outc('\n');
}


/* ----------------------------------------------------- */
/* input routines					 */
/* ----------------------------------------------------- */


static uschar vi_pool[VI_MAX];
static int vi_size;
static int vi_head;


static int vio_fd;


#ifdef EVERY_Z

static int holdon_fd;		 /* Thor.980727: ���Xchat&talk�Ȧsvio_fd�� */


void
vio_save()
{
  holdon_fd = vio_fd;
  vio_fd = 0;
}


void
vio_restore()
{
  vio_fd = holdon_fd;
  holdon_fd = 0;
}


int
vio_holdon()
{
  return holdon_fd;
}
#endif


#if 0
struct timeval
{
  int tv_sec;		/* timeval second */
  int tv_usec;		/* timeval micro-second */
};
#endif

static struct timeval vio_to = {60, 0};


void
add_io(fd, timeout)
  int fd;
  int timeout;
{
  vio_fd = fd;
  vio_to.tv_sec = timeout;
}


static inline int
iac_count(current)
  uschar *current;
{
  switch (*(current + 1))
  {
  case DO:
  case DONT:
  case WILL:
  case WONT:
    return 3;

  case SB:			/* loop forever looking for the SE */
    {
      uschar *look = current + 2;

      /* fuse.030518: �u�W�վ�e���j�p�A���� b_lines */
      if ((*look) == TELOPT_NAWS)
      {
	b_lines = ntohs(* (short *) (look + 3)) - 1;
	b_cols = ntohs(* (short *) (look + 1)) - 1;
	if (b_lines >= T_LINES)
	  b_lines = T_LINES - 1;
	else if (b_lines < 23)
	  b_lines = 23;
	if (b_cols >= T_COLS)
	  b_cols = T_COLS - 1;
	else if (b_cols < 79)
	  b_cols = 79;
	d_cols = b_cols - 79;
      }

      for (;;)
      {
	if ((*look++) == IAC)
	{
	  if ((*look++) == SE)
	  {
	    return look - current;
	  }
	}
      }
    }
  }
  return 1;
}


int
igetch()
{

#define	IM_TRAIL	0x01
#define	IM_REPLY	0x02	/* ^R */
#define	IM_TALK		0x04

  static int imode = 0;
  static int idle = 0;

  int cc, fd, nfds, rset;
  uschar *data;

  data = vi_pool;
  nfds = 0;

  for (;;)
  {
    if (vi_size <= vi_head)
    {
      if (nfds == 0)
      {
	refresh();
	fd = (imode & IM_REPLY) ? 0 : vio_fd;
	nfds = fd + 1;
	if (fd)
	  fd = 1 << fd;
      }

      for (;;)
      {
	struct timeval tv = vio_to;
	/* Thor.980806: man page ���] timeval �O�|���ܪ� */

	rset = 1 | fd;
	cc = select(nfds, (fd_set *) & rset, NULL, NULL, &tv /*&vio_to*/);
			/* Thor.980806: man page ���] timeval �O�|���ܪ� */

	if (cc > 0)
	{
	  if (fd & rset)
	    return I_OTHERDATA;

	  cc = recv(0, data, VI_MAX, 0);
	  if (cc > 0)
	  {
	    vi_head = (*data) == IAC ? iac_count(data) : 0;
	    if (vi_head >= cc)
	      continue;
	    vi_size = cc;

#ifdef DETAIL_IDLETIME
	    if (cutmp)
#else
	    if (idle && cutmp)
#endif
	    {
	      idle = 0;

#ifdef DETAIL_IDLETIME
	      time(&cutmp->idle_time);	/* �Y #define DETAIL_IDLETIME�A�h idle_time ��ܶ}�l���m���ɶ�(��) */
#else
	      cutmp->idle_time = 0;	/* �Y #undef DETAIL_IDLETIME�A�h idle_time ��ܤw�g���m�F�h�[(��) */
#endif

#ifdef BMW_COUNT
	      /* itoc.010421: �����@��ᱵ�����y�Ʀ^�k 0 */
	      cutmp->bmw_count = 0;
#endif
	    }
	    break;
	  }
	  if ((cc == 0) || (errno != EINTR))
	    abort_bbs();
	}
	else if (cc == 0)
	{
	  cc = vio_to.tv_sec;
	  if (cc < 60)		/* paging timeout : �C 60 ���s�@�� idle */
	    return I_TIMEOUT;

	  idle += cc / 60;
	  vio_to.tv_sec = cc + 60;	/* Thor.980806: �C��timeout���W�[60��A�ҥH���l�U���U�C�A�n�i:p */
	  /* Thor.990201.����: ���Ftalk_rqst�Bchat���~�A�ݭn�b�ʤ@�ʤ���A���]tv_sec��60���? (�w�]��) */

#ifdef TIME_KICKER
	  if (idle > IDLE_TIMEOUT)
	  {
	    outs("�� �W�L���m�ɶ��I");
	    refresh();
	    abort_bbs();
  	  }
	  else if (idle >= IDLE_TIMEOUT - IDLE_WARNOUT)	/* itoc.001222: ���m�L�[ĵ�i */
	  {
	    bell();		/* itoc.010315: �s�@�U :p */
	    prints("\033[1;5;31mĵ�i\033[m�z�w�g���m�L�[�A�t�αN�b %d �������z�𰣡I", IDLE_WARNOUT);
	    refresh();
	  }	  
#endif

#ifndef DETAIL_IDLETIME
	  cutmp->idle_time = idle;
#endif

	  if (bbsmode < M_XMENU)	/* �b menu �̭��n�� movie */
	  {
	    movie();
	    refresh();
	  }
	}
	else
	{
	  if (errno != EINTR)
	    abort_bbs();
	}
      }
    }

    cc = data[vi_head++];
    if (imode & IM_TRAIL)
    {
      imode ^= IM_TRAIL;
      if (cc == 0 || cc == 0x0a)
	continue;
    }

    if (cc == 0x0d)
    {
      imode |= IM_TRAIL;
      return '\n';
    }

    if (cc == 0x7f)
    {
      return KEY_BKSP;
    }

    if (cc == Ctrl('L'))
    {
      vs_redraw();
      continue;
    }

    if ((cc == Ctrl('R')) && (bbstate & STAT_STARTED) && !(bbstate & STAT_LOCK) && !(imode & IM_REPLY))
						/* lkchu.990513: ��w�ɤ��i�^�T */
    {
      signal(SIGUSR1, SIG_IGN);

      /* Thor.980307: �b ^R �� talk �|�]�S�� vio_fd �ݤ��� I_OTHERDATA�A�Ӭݤ����襴���r�A�ҥH�b ^R �ɸT�� talk */
      imode |= IM_REPLY;
      bmw_reply();
      imode ^= IM_REPLY;

      signal(SIGUSR1, (void *) talk_rqst);

#ifdef BMW_COUNT
      /* itoc.010907: �����@��ᱵ�����y�Ʀ^�k 0 */
      cutmp->bmw_count = 0;
#endif
      continue;
    }

    return (cc);
  }
}


#define	MATCH_END	0x8000
/* Thor.990204.����: �N��MATCH����, �n���N�ɨ�, �n���N�����쪬, ���q�X�i�઺�ȤF */

static void
match_title()
{
  move(2, 0);
  clrtobot();
  vs_line("������T�@����");
}


static int
match_getch()
{
  int ch;

  outs("\n�� �C��(C)�~�� (Q)�����H[C] ");
  ch = vkey();
  if (ch == 'q' || ch == 'Q')
    return ch;

  move(3, 0);
  clrtobot();
  return 0;
}


/* ----------------------------------------------------- */
/* ��� board	 					 */
/* ----------------------------------------------------- */


static BRD *xbrd;


BRD *
ask_board(board, perm, msg)
  char *board;
  int perm;
  char *msg;
{
  if (msg)
  {
    move(2, 0);
    outs(msg);
  }

  if (vget(1, 0, "�п�J�ݪO�W��(���ť���۰ʷj�M)�G", board, BNLEN + 1, GET_BRD | perm))
    return xbrd;

  return NULL;
}


static int
vget_match(prefix, len, op)
  char *prefix;
  int len;
  int op;
{
  char *data, *hit;
  char newprefix[BNLEN + 1];	/* �~��ɧ����O�W */
  int row, col, match;
  int rlen;			/* �i�ɧ����Ѿl���� */

  row = 3;
  col = match = rlen = 0;

  if (op & GET_BRD)
  {
    usint perm;
    int i;
    char *bits, *n, *b;
    BRD *head, *tail;

    extern BCACHE *bshm;
    extern char brd_bits[];

    perm = op & (BRD_L_BIT | BRD_R_BIT | BRD_W_BIT);
    bits = brd_bits;
    head = bshm->bcache;
    tail = head + bshm->number;

    do
    {
      if (perm & *bits++)
      {
	data = head->brdname;

	if (str_ncmp(prefix, data, len))
	  continue;

	xbrd = head;

	if ((op & MATCH_END) && !data[len])
	{
	  strcpy(prefix, data);
	  return len;
	}

	match++;
	hit = data;

	if (op & MATCH_END)
	  continue;

	if (match == 1)
	{
	  match_title();
	  if (data[len])
	  {
	    strcpy(newprefix, data);
	    rlen = strlen(data + len);
	  }
	}
	else if (rlen)	/* LHD.051014: �٦��i�ɧ����l�a */
	{
	  n = newprefix + len;
	  b = data + len;
	  for (i = 0; i < rlen && ((*n | 0x20) == (*b | 0x20)); i++, n++, b++)
	    ;
	  *n = '\0';
	  rlen = i;
	}

	move(row, col);
	outs(data);

	col += BNLEN + 1;
	if (col > b_cols + 1 - BNLEN - 1)	/* �`�@�i�H�� (b_cols + 1) / (BNLEN + 1) �� */
	{
	  col = 0;
	  if (++row >= b_lines)
	  {
	    if (match_getch() == 'q')
	      break;

	    move(row = 3, 0);
	    clrtobot();
	  }
	}
      }
    } while (++head < tail);
  }
  else if (op & GET_USER)
  {
    struct dirent *de;
    DIR *dirp;
    int cc;
    char fpath[16];

    /* Thor.981203: USER name�ܤ֥��@�r, ��"<="�|����n��? */
    if (len == 0)
      return 0;

    cc = *prefix;
    if (cc >= 'A' && cc <= 'Z')
      cc |= 0x20;
    if (cc < 'a' || cc > 'z')
      return 0;

    sprintf(fpath, "usr/%c", cc);
    dirp = opendir(fpath);
    while (de = readdir(dirp))
    {
      data = de->d_name;
      if (str_ncmp(prefix, data, len))
	continue;

      if (!match++)
      {
	match_title();
	strcpy(hit = fpath, data);	/* �Ĥ@���ŦX����� */
      }

      move(row, col);
      outs(data);

      col += IDLEN + 1;
      if (col > b_cols + 1 - IDLEN - 1)	/* �`�@�i�H�� (b_cols + 1) / (IDLEN + 1) �� */
      {
	col = 0;
	if (++row >= b_lines)
	{
	  if (match_getch())
	    break;
	  row = 3;
	}
      }
    }

    closedir(dirp);
  }
  else /* Thor.990203.����: GET_LIST */
  {
    LinkList *list;
    extern LinkList *ll_head;

    for (list = ll_head; list; list = list->next)
    {
      data = list->data;

      if (str_ncmp(prefix, data, len))
	continue;

      if ((op & MATCH_END) && !data[len])
      {
	strcpy(prefix, data);
	return len;
      }

      match++;
      hit = data;

      if (op & MATCH_END)
	continue;

      if (match == 1)
	match_title();

      move(row, col);
      outs(data);

      col += IDLEN + 1;
      if (col > b_cols + 1 - IDLEN - 1)	/* �`�@�i�H�� (b_cols + 1) / (IDLEN + 1) �� */
      {
	col = 0;
	if (++row >= b_lines)
	{
	  if (match_getch())
	    break;
	  row = 3;
	}
      }
    }
  }

  if (match == 1)
  {
    strcpy(prefix, hit);
    return strlen(hit);
  }
  else if (rlen)
  {
    strcpy(prefix, newprefix);
    return len + rlen;
  }

  return 0;
}


char lastcmd[MAXLASTCMD][80];


/* Flags to getdata input function */
/* NOECHO  0x0000  ����ܡA�Ω�K�X���o */
/* DOECHO  0x0100  �@����� */
/* LCECHO  0x0200  low case echo�A�����p�g */
/* GCARRY  0x0400  �|��ܤW�@��/�ثe���� */

int
vget(line, col, prompt, data, max, echo)
  int line, col;
  uschar *prompt, *data;
  int max, echo;
{
  int ch, len;
  int x, y;
  int i, next;
  int vlen, hlen;

  /* itoc.010312: ��������m �]���᭱ line �M prompt ���Q���F */ 
  vlen = line;
  hlen = col + strlen(prompt);

  if (prompt)
  {
    move(line, col);
    clrtoeol();
    outs(prompt);
  }
  else
  {
    clrtoeol();
  }

  STANDOUT;

  x = cur_row;
  y = cur_col;

  if (echo & GCARRY)
  {
    if (len = strlen(data))
      outs(data);
  }
  else
  {
    len = 0;
  }

  /* --------------------------------------------------- */
  /* ���o board / userid / on-line user			 */
  /* --------------------------------------------------- */

  ch = len;
  do
  {
    outc(' ');
  } while (++ch < max);

  STANDEND;

  line = -1;
  col = len;
  max--;

  for (;;)
  {
    move(x, y + col);
    ch = vkey();
    if (ch == '\n')
    {
      data[len] = '\0';
      if ((echo & (GET_BRD | GET_LIST)) && len > 0)
      /* Thor.990204:�n�D��J���@�r�~�N��۰� match, �_�h��cancel */
      {
	ch = len;
	len = vget_match(data, len, echo | MATCH_END);
	if (len > ch)
	{
	  move(x, y);
	  outs(data);
	}
	else if (len == 0)
	{
	  data[0] = '\0';
	}
      }
      break;
    }

    if (isprint2(ch))
    {
      if (ch == ' ' && (echo & (GET_USER | GET_BRD | GET_LIST)))
      {
	ch = vget_match(data, len, echo);
	if (ch > len)
	{
	  move(x, y);
	  outs(data);
	  col = len = ch;
	}
	continue;
      }

      if (len >= max)
	continue;

      /* ----------------------------------------------- */
      /* insert data and display it			 */
      /* ----------------------------------------------- */

      prompt = &data[col];
      i = col;
      move(x, y + col);

      for (;;)
      {
	outc(echo ? ch : '*');
	next = *prompt;
	*prompt++ = ch;
	if (i >= len)
	  break;
	i++;
	ch = next;
      }
      col++;
      len++;
      continue;
    }

    /* ----------------------------------------------- */
    /* ��J password �ɥu��� BackSpace		       */
    /* ----------------------------------------------- */

    if (!echo && ch != KEY_BKSP)
      continue;

    switch (ch)
    {
    case Ctrl('D'):

      if (col >= len)
	continue;

      col++;

    case KEY_BKSP:

      if (!col)
	continue;

      /* ----------------------------------------------- */
      /* remove data and display it			 */
      /* ----------------------------------------------- */

      len--;
      col--;
#ifdef HAVE_MULTI_BYTE
      /* hightman.060504: �P�_�{�b�R������m�O�_���~�r����b�q�A�Y�O�R�G�r�� */
      if ((cuser.ufo & UFO_ZHC) && echo && col && IS_ZHC_LO(data, col))
      {
	len--;
	col--;
	next = 2;
      }
      else
#endif
	next = 1;
      move(x, y + col);
      for (i = col; i < len; i++)
      {
	data[i] = ch = data[i + next];
	outc(echo ? ch : '*');
      }
      while (next--)
	outc(' ');
      break;

    case KEY_DEL:

      if (col >= len)
	continue;

      /* ----------------------------------------------- */
      /* remove data and display it			 */
      /* ----------------------------------------------- */

      len--;
#ifdef HAVE_MULTI_BYTE
      /* hightman.060504: �P�_�{�b�R������m�O�_���~�r���e�b�q�A�Y�O�R�G�r�� */
      if ((cuser.ufo & UFO_ZHC) && col < len && IS_ZHC_HI(data[col]))
      {
	len--;
	next = 2;
      }
      else
#endif
	next = 1;
      for (i = col; i < len; i++)
      {
	data[i] = ch = data[i + next];
	outc(ch);
      }
      while (next--)
	outc(' ');
      break;

    case KEY_LEFT:
    case Ctrl('B'):
      if (col)
      {
	col--;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: �����ɸI��~�r������ */
	if ((cuser.ufo & UFO_ZHC) && col && IS_ZHC_LO(data, col))
	  col--;
#endif
      }
      break;

    case KEY_RIGHT:
    case Ctrl('F'):
      if (col < len)
      {
	col++;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: �k���ɸI��~�r������ */
	if ((cuser.ufo & UFO_ZHC) && col < len && IS_ZHC_HI(data[col - 1]))
	  col++;
#endif
      }
      break;

    case KEY_HOME:
    case Ctrl('A'):
      col = 0;
      break;

    case KEY_END:
    case Ctrl('E'):
      col = len;
      break;

    case Ctrl('C'):		/* clear / reset */
      if (len)
      {
	move(x, y);
	for (ch = 0; ch < len; ch++)
	  outc(' ');
	col = len = 0;
      }
      break;

    case KEY_DOWN:
    case Ctrl('N'):

      line += MAXLASTCMD - 2;

    case KEY_UP:
    case Ctrl('P'):

      line = (line + 1) % MAXLASTCMD;
      prompt = lastcmd[line];
      col = 0;
      move(x, y);

      do
      {
	if (!(ch = *prompt++))
	{
	  /* clrtoeol */

	  for (ch = col; ch < len; ch++)
	    outc(' ');
	  break;
	}

#if 0	/* ���ݭn�A�]�� bmtad/receive_article �| strip �� ansi code */
	if (ch == KEY_ESC)	/* itoc.020601: ���o�ϥα���X */
	  ch = '*';
#endif

	outc(ch);
	data[col] = ch;
      } while (++col < max);

      len = col;
      break;

    case Ctrl('K'):		/* delete to end of line */
      if (col < len)
      {
	move(x, y + col);
	for (ch = col; ch < len; ch++)
	  outc(' ');
	len = col;
      }
      break;

    /* itoc.030619: �� vget ���ٯ�� ^R ����y */
    case Ctrl('R'):
    case Ctrl('T'):
      if (bbsmode == M_BMW_REPLY)
      {
	if (bmw_reply_CtrlRT(ch))	/* ���h�����y */
	{
	  data[len] = '\0';
	  return ch;
	}
      }
      break;
    }
  }

  if (len >= 2 && echo)
  {
    for (line = MAXLASTCMD - 1; line; line--)
      strcpy(lastcmd[line], lastcmd[line - 1]);
    strcpy(lastcmd[0], data);
  }

  move(vlen, strlen(data) + hlen);	/* itoc.010312: �T�w����ӦC�C���A�L�X'\n' */
  outc('\n');

  ch = data[0];
  if ((echo & LCECHO) && (ch >= 'A' && ch <= 'Z'))
    data[0] = (ch |= 0x20);

  return ch;
}


int
vans(prompt)
  char *prompt;
{
  char ans[3];

  /* itoc.010812.����: �|�۰ʴ����p�g�� */
  return vget(b_lines, 0, prompt, ans, sizeof(ans), LCECHO);
}


int
vkey()
{
  int mode;
  int ch, last;

  mode = last = 0;
  for (;;)
  {
    ch = igetch();
    if (mode == 0)		/* Normal Key */
    {
      if (ch == KEY_ESC)
	mode = 1;
      else
	return ch;
    }
    else if (mode == 1)		/* Escape sequence */
    {
      if (ch == '[' || ch == 'O')
	mode = 2;
      else if (ch == '1' || ch == '4')
	mode = 3;
      else
	return Esc(ch);
    }
    else if (mode == 2)		/* Cursor key */
    {
      if (ch >= 'A' && ch <= 'D')
	return KEY_UP - (ch - 'A');
      else if (ch >= '1' && ch <= '6')
	mode = 3;
      else
	return ch;
    }
    else if (mode == 3)		/* Ins Del Home End PgUp PgDn */
    {
      if (ch == '~')
	return KEY_HOME - (last - '1');
      else
	return ch;
    }
    last = ch;
  }
}
