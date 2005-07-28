/*-------------------------------------------------------*/
/* edit.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : simple ANSI/Chinese editor			 */
/* create : 95/03/29					 */
/* update : 95/12/15					 */
/*-------------------------------------------------------*/


#include "bbs.h"


/* #define	VE_WIDTH	(ANSILINELEN - 1) */
/* Thor.990330: ������ި���, ">"�n�ܦ�, �@��|�W�LANSILINELEN, �G�h�d�Ŷ� */
/* itoc.010317.����: �����ڤ@�C�i�H��U�����Ħr�Ƭ� VE_WIDTH - 3 */
#define	VE_WIDTH	(ANSILINELEN - 11)


typedef struct textline
{
  struct textline *prev;
  struct textline *next;
  int len;
  uschar data[ANSILINELEN];
}	textline;


static textline *vx_ini;	/* first line */
static textline *vx_cur;	/* current line */
static textline *vx_top;	/* top line in current window */


static int ve_lno;		/* current line number */
static int ve_row;		/* cursor position */
static int ve_col;


static int ve_mode;		/* operation mode */


#ifdef HAVE_ANONYMOUS
char anonymousid[IDLEN + 1];	/* itoc.010717: �۩w�ΦW ID */
#endif


#define	VE_INSERT	0x01
#define	VE_ANSI		0x02
#define	VE_FOOTER	0x04
#define	VE_REDRAW	0x08

#ifdef EVERY_BIFF
#define VE_BIFF		0x10
#endif /* Thor.980805: �l�t��B�ӫ��a */


#define	FN_BAK		"bak"


/* ----------------------------------------------------- */
/* �O����޲z�P�s��B�z					 */
/* ----------------------------------------------------- */


#ifdef	DEBUG_VEDIT
static void
ve_abort(i)
  int i;
{
  char msg[40];

  sprintf(msg, "�Y������ %d", i);
  blog("VEDIT", msg);
}

#else

#define	ve_abort(n)	;
#endif


static void
ve_position(cur, top)
  textline *cur;
  textline *top;
{
  int row;

  row = cur->len;
  if (ve_col > row)
    ve_col = row;

  row = 0;
  while (cur != top)
  {
    row++;
    cur = cur->prev;
  }
  ve_row = row;

  ve_mode |= VE_REDRAW;
}


static inline void
ve_pageup()
{
  textline *cur, *top, *tmp;
  int lno, n;

  cur = vx_cur;
  top = vx_top;
  lno = ve_lno;
  for (n = PAGE_SCROLL; n > 0; n--)
  {
    if (!(tmp = cur->prev))
      break;

    cur = tmp;
    lno--;

    if (tmp = top->prev)
      top = tmp;
  }

  vx_cur = cur;
  vx_top = top;
  ve_lno = lno;

  ve_position(cur, top);
}


static inline void
ve_forward(n)
  int n;
{
  textline *cur, *top, *tmp;
  int lno;

  cur = vx_cur;
  top = vx_top;
  lno = ve_lno;
  while (n--)
  {
    if (!(tmp = cur->next))
      break;

    lno++;
    cur = tmp;

    if (tmp = top->next)
      top = tmp;
  }

  vx_cur = cur;
  vx_top = top;
  ve_lno = lno;

  ve_position(cur, top);
}


static inline char *
ve_strim(s)
  char *s;
{
  while (*s == ' ')
    s++;
  return s;
}


static textline *
ve_alloc()
{
  textline *p;

  if (p = (textline *) malloc(sizeof(textline)))
  {
    p->prev = NULL;
    p->next = NULL;
    p->len = 0;
    p->data[0] = '\0';
    return p;
  }

  ve_abort(13);			/* �O����Υ��F */
  abort_bbs();
}


/* ----------------------------------------------------- */
/* Thor: ansi �y���ഫ  for color �s��Ҧ�		 */
/* ----------------------------------------------------- */


static int
ansi2n(ansix, line)
  int ansix;
  textline *line;
{
  uschar *data, *tmp;
  int ch;

  data = tmp = line->data;

  while (ch = *tmp)
  {
    if (ch == KEY_ESC)
    {
      for (;;)
      {
	ch = *++tmp;
	if (ch >= 'a' && ch <= 'z' /* isalpha(ch) */ )
	{
	  tmp++;
	  break;
	}
	if (!ch)
	  break;
      }
      continue;
    }
    if (ansix <= 0)
      break;
    tmp++;
    ansix--;
  }
  return tmp - data;
}


static int
n2ansi(nx, line)
  int nx;
  textline *line;
{
  uschar *tmp, *nxp;
  int ansix;
  int ch;

  tmp = nxp = line->data;
  nxp += nx;
  ansix = 0;

  while (ch = *tmp)
  {
    if (ch == KEY_ESC)
    {
      for (;;)
      {
	ch = *++tmp;
	if (ch >= 'a' && ch <= 'z' /* isalpha(ch) */ )
	{
	  tmp++;
	  break;
	}
	if (!ch)
	  break;
      }
      continue;
    }
    if (tmp >= nxp)
      break;
    tmp++;
    ansix++;
  }
  return ansix;
}


/* ----------------------------------------------------- */
/* delete_line deletes 'line' from the list,		 */
/* and maintains the vx_ini pointers.			 */
/* ----------------------------------------------------- */


static void
delete_line(line)
  textline *line;
{
  textline *p = line->prev;
  textline *n = line->next;

  if (p || n)
  {
    if (n)
      n->prev = p;

    if (p)
      p->next = n;
    else
      vx_ini = n;

    free(line);
  }
  else
  {
    line->data[0] = line->len = 0;
  }
}


/* ----------------------------------------------------- */
/* split 'line' right before the character pos		 */
/* ----------------------------------------------------- */


static void
ve_split(line, pos)
  textline *line;
  int pos;
{
  int len = line->len - pos;

  if (len >= 0)
  {
    textline *p, *n;
    uschar *ptr;

    line->len = pos;
    p = ve_alloc();
    p->len = len;
    strcpy(p->data, (ptr = line->data + pos));
    *ptr = '\0';

    /* --------------------------------------------------- */
    /* append p after line in list. keep up with last line */
    /* --------------------------------------------------- */

    if (p->next = n = line->next)
      n->prev = p;
    line->next = p;
    p->prev = line;

    if (line == vx_cur && pos <= ve_col)
    {
      vx_cur = p;
      ve_col -= pos;
      ve_row++;
      ve_lno++;
    }
    ve_mode |= VE_REDRAW;
  }
}


/* ----------------------------------------------------- */
/* connects 'line' and the next line. returns true if:	 */
/* 1) lines were joined and one was deleted		 */
/* 2) lines could not be joined		 		 */
/* 3) next line is empty				 */
/* ----------------------------------------------------- */
/* returns false if:					 */
/* 1) Some of the joined line wrapped			 */
/* ----------------------------------------------------- */


static int
ve_join(line)
  textline *line;
{
  textline *n;
  uschar *data, *s;
  int sum, len;

  if (!(n = line->next))
    return 1;

  if (!*ve_strim(data = n->data))
    return 1;

  len = line->len;
  sum = len + n->len;
  if (sum < VE_WIDTH)
  {
    strcpy(line->data + len, data);
    line->len = sum;
    delete_line(n);
    return 1;
  }

  s = data - len + VE_WIDTH - 1;
  while (*s == ' ' && s != data)
    s--;
  while (*s != ' ' && s != data)
    s--;
  if (s == data)
    return 1;

  ve_split(n, (s - data) + 1);
  if (len + n->len >= VE_WIDTH)
  {
    ve_abort(0);
    return 1;
  }

  ve_join(line);
  n = line->next;
  len = n->len;
  if (len >= 1 && len < VE_WIDTH - 1)
  {
    s = n->data + len - 1;
    if (*s != ' ')
    {
      *s++ = ' ';
      *s = '\0';
      n->len = len + 2;
    }
  }
  return 0;
}


static void
join_up(line)
  textline *line;
{
  while (!ve_join(line))
  {
    line = line->next;
    if (line == NULL)
    {
      ve_abort(2);
      abort_bbs();
    }
  }
}


/* ----------------------------------------------------- */
/* character insert / detete				 */
/* ----------------------------------------------------- */


static void
ve_char(ch)
  int ch;
{
  textline *p;
  int col, len, mode;
  uschar *data;

  p = vx_cur;
  len = p->len;
  col = ve_col;

  if (col > len)
  {
    ve_abort(1);
    return;
  }

  data = p->data;
  mode = ve_mode;

  /* --------------------------------------------------- */
  /* overwrite						 */
  /* --------------------------------------------------- */

  if ((col < len) && !(mode & VE_INSERT))
  {
    data[col++] = ch;

    /* Thor: ansi �s��, �i�H overwrite, ���\�� ansi code */

    if (mode & VE_ANSI)
      col = ansi2n(n2ansi(col, p), p);

    ve_col = col;
    return;
  }

  /* --------------------------------------------------- */
  /* insert / append					 */
  /* --------------------------------------------------- */

  for (mode = len; mode >= col; mode--)
  {
    data[mode + 1] = data[mode];
  }
  data[col++] = ch;
  ve_col = col;
  p->len = ++len;

  if (len >= VE_WIDTH - 2)
  {
    /* Thor.980727: �ץ� editor buffer overrun ���D, ���� */

    ve_split(p, VE_WIDTH - 3);

#if 0
    uschar *str = data + len;

    while (*--str == ' ')
    {
      if (str == data)
	break;
    }

    ve_split(p, (str - data) + 1);
#endif



#if 0
 �@��  yvb (yvb)                                            �ݪO  SYSOP
 ���D  ���� editor...
 �ɶ�  Sun Jun 28 11:28:02 1998
�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w

    post �� mail �����o�� editor, �p�G�A�@�����᭱���r,
    �̦h�i�H����@�C 157 �r�a... �A���N�|�Q������.

    ���L�p�G�A������n�b�᭱�[�r... �]�N�O�A���^��~����
    �~�򥴦r... ���t�Τ��|���A�b�ӦC�~��[�@�Ӧr��, �åB
    ���X�s���@���...

    ���гo�ӨB�J, ��� 170 �r��, �A�N�|�Q�_�u�F...
    ��... ���ܺA�a :P

    ���, �q�t�@���[�I�ӻ�, �Y�o���O�t�ίS�N�o�ˤl�]�p,
    ���N��ܳo�ئ��G���õۥi�ǥ� buffer overrun ���覡,
    �i��F���J�I�t�Ϊ��M��...
--
�� �ӷ�: ����˪L �� From: bamboo.Dorm6.NCTU.edu.tw

#endif

  }
}


static void
delete_char(cur, col)
  textline *cur;
  int col;
{
  uschar *dst, *src;

  cur->len--;
  dst = cur->data + col;
  for (;;)
  {
    src = dst + 1;
    if (!(*dst = *src))
      break;
    dst = src;
  }
}


static void
ve_string(str)
  uschar *str;
{
  int ch;

  while (ch = *str)
  {
    if (isprint2(ch) || ch == KEY_ESC)
    {
      ve_char(ch);
    }
    else if (ch == '\t')
    {
      do
      {
	ve_char(' ');
      } while (ve_col & TAB_WIDTH);
    }
    else if (ch == '\n')
    {
      ve_split(vx_cur, ve_col);
    }
    str++;
  }
}


static void
ve_ansi()
{
  int fg, bg, mode;
  char ans[4], buf[16], *apos, *color, *tmp;
  static char t[] = "BRGYLPCW";

  mode = ve_mode | VE_REDRAW;
  color = str_ransi;

  if (mode & VE_ANSI)
  {
    move(b_lines - 1, 55);
    outs("\033[1;33;40mB\033[41mR\033[42mG\033[43mY\033[44mL\033[45mP\033[46mC\033[47mW\033[m");
    if (fg = vget(b_lines, 0, "�п�J  �G��/�e��/�I��[���`�զr�©�][0wb]�G",
	apos = ans, 4, LCECHO))
    {
      color = buf;
      strcpy(color, "\033[");
      if (isdigit(fg))
      {
	sprintf(color, "%s%c", color, *(apos++));
	if (*apos)
	  strcat(color, ";");
      }
      if (*apos)
      {
	if (tmp = strchr(t, toupper(*(apos++))))
	  fg = tmp - t + 30;
	else
	  fg = 37;
	sprintf(color, "%s%d", color, fg);
      }
      if (*apos)
      {
	if (tmp = strchr(t, toupper(*(apos++))))
	  bg = tmp - t + 40;
	else
	  bg = 40;
	sprintf(color, "%s;%d", color, bg);
      }
      strcat(color, "m");
    }
  }

  ve_mode = mode | VE_INSERT;
  ve_string(color);
  ve_mode = mode;
}


static textline *
ve_line(this, str)
  textline *this;
  uschar *str;
{
  int cc, len;
  uschar *data;
  textline *line;

  do
  {
    line = ve_alloc();
    data = line->data;
    len = 0;

    for (;;)
    {
      cc = *str;

      if (cc == '\n')
	cc = 0;
      if (cc == 0)
	break;

      str++;

      if (cc == '\t')
      {
	do
	{
	  *data++ = ' ';
	  len++;
	} while ((len & TAB_WIDTH) && (len < VE_WIDTH));
      }
      else if (cc < ' ' && cc != KEY_ESC)
      {
	continue;
      }
      else
      {
	*data++ = cc;
	len++;
      }
      if (len >= VE_WIDTH)
	break;
    }

    *data = '\0';
    line->len = len;
    line->prev = this;
    this = this->next = line;

  } while (cc);

  return this;
}


/* ----------------------------------------------------- */
/* ���ñ�W��						 */
/* ----------------------------------------------------- */


static void
show_sign()		/* itoc.000319: ���ñ�W�ɪ����e */
{
  int fd, len, i;
  char fpath[64], buf[10], ch, *str;

  clear();

  sprintf(buf, "%s.0", FN_SIGN);
  usr_fpath(fpath, cuser.userid, buf);	/* itoc.020123: �U��ñ�W���ɮפ��} */
  len = strlen(fpath) - 1;

  for (ch = '1'; ch <= '3'; ch++)	/* �T��ñ�W�� */
  {
    fpath[len] = ch;

    fd = open(fpath, O_RDONLY);
    if (fd >= 0)
    {
      mgets(-1);
      move((ch - '1') * (MAXSIGLINES + 1), 0);
      prints("\033[1;36m�i ñ�W�� %c �j\033[m\n", ch);

      for (i = 1; i <= MAXSIGLINES; i++)
      {
	if (!(str = mgets(fd)))
	  break;
	prints("%s\n", str);
      }
    }
  }
}


/* ----------------------------------------------------- */
/* �Ÿ���J�u�� Input Tools				 */
/* ----------------------------------------------------- */


#ifdef INPUT_TOOLS
static void
input_tools()   /* itoc.000319: �Ÿ���J�u�� */
{
  char msg1[] = {"1.�A�Ť��  2.�u�����  3.�ƾǲŸ� (PgDn/N:�U�@��)�H[Q] "};
  char msg2[] = {"4.�Ϯ׼Ʀr  5.��þ�r��  6.�`�����I (PgUp/P:�W�@��)�H[Q] "};

  char ansi[6][100] =
  {
    {	/* 1.�A�Ť�� */
      "�b�c�d�e�f�g�h�i����"
      "�j�k�l�m�n�o�p������"
      "�]�^�a�b�e�f�i�j�m�n"
      "�q�r�u�v�y�z�k�l�o�p"
      "�_�`�c�d�g�h�w�x�{�|"
    }, 

    {	/* 2.�u����� */
      "�������������w�x����"
      "�z�s�{�u�q�t�|�r�}��"
      "������������������\\"
      "������������~������"
      "�b�v�j�y������������"
    }, 

    {	/* 3.�ƾǲŸ� */
      "�ϡСѡҡӡסա֡ء�"
      "�ڡݡ�����ۣk��"
      "�U�S�������G�X"
      "���������I�C�D�F�G�H"
      "�J�K�P�Q�R�T�U�V�W��"
    }, 

    {	/* 4.�Ϯ׼Ʀr */
      "��������������������"
      "���������̢͢Ρ��"
      "��������������������"
      "��������������������"
      "ƵƶƷƸƹƺƻƼƽƾ"
    }, 

    {	/* 5.��þ�r�� */
      "�D�E�F�G�H�I�J�K�L�M"
      "�N�O�P�Q�R�S�T�U�V�W"
      "�X�Y�Z�[�\\�]�^�_�`�a"
      "�b�c�d�e�f�g�h�i�j�k"
      "�l�m�n�o�p�q�r�s�A�B"
    }, 

    {	/* 6.�`�����I */
      "�t�u�v�w�x�y�z�{�|�}"
      "�~������������������"
      "��������������������"
      "��������������������"
      "���A�C�B�I�H�G�F�u�v"
    }
  };

  char buf[80], tmp[6];
  char *ptr, *str;
  int ch, page;

  /* ����� */
  ch = KEY_PGUP;
  do
  {
    outz("���X��J�u��G");
    outs((ch == KEY_PGUP || ch == 'P') ? msg1 : msg2);
    ch = vkey();
  } while (ch == KEY_PGUP || ch == 'P' || ch == KEY_PGDN || ch == 'N');

  if (ch < '1' || ch > '6')
    return;

  ptr = ansi[ch - '1'];
  page = 0;

  for (;;)
  {
    buf[0] = '\0';
    str = ptr + page * 20;	/* �C page ���Q�Ӥ���r�A�C�Ӥ���r�O 2 char */

    for (ch = 0; ch < 10; ch++)
    {
      sprintf(tmp, "%d.%2.2s ", ch, str + ch * 2);	/* �C�Ӥ���r�O 2 char */
      strcat(buf, tmp);
    }
    strcat(buf, "(PgUp/P:�W  PgDn/N:�U)[Q] ");
    outz(buf);
    ch = vkey();

    if (ch == KEY_PGUP || ch == 'P')
    {
      if (page)
	page--;
    }
    else if (ch == KEY_PGDN || ch == 'N')
    {
      if (page != 4)
	page++;
    }
    else if (ch < '0' || ch > '9')
    {
      break;
    }
    else
    {
      str += (ch - '0') * 2;
      ve_char(*str);		/* �L�X�G�� char ����r */
      ve_char(*++str);
      break;
    }
  }
}
#endif


/* ----------------------------------------------------- */
/* �Ȧs�� TBF (Temporary Buffer File) routines		 */
/* ----------------------------------------------------- */


char *
tbf_ask()
{
  static char fn_tbf[] = "buf.1";
  int ch;

  do
  {
    ch = vget(b_lines, 0, "�п�ܼȦs��(1-5)�G", fn_tbf + 4, 2, GCARRY);
  } while (ch < '1' || ch > '5');
  return fn_tbf;
}


FILE *
tbf_open()
{
  int ans;
  char fpath[64], op[4];
  struct stat st;

  usr_fpath(fpath, cuser.userid, tbf_ask());
  ans = 'a';

  if (!stat(fpath, &st))
  {
    ans = vans("�Ȧs�ɤw����� (A)���[ (W)�мg (Q)�����H[A] ");
    if (ans == 'q')
      return NULL;

    if (ans != 'w')
    {
      /* itoc.030208: �ˬd�Ȧs�ɤj�p */
      if (st.st_size >= 100000)		/* 100KB ���Ӱ��F */
      {
	zmsg("�Ȧs�ɤӤj�A�L�k���[");
	return NULL;
      }

      ans = 'a';
    }
  }

  op[0] = ans;
  op[1] = '\0';

  return fopen(fpath, op);
}


static textline *
ve_load(this, fd)
  textline *this;
  int fd;
{
  uschar *str;
  textline *next;

  next = this->next;

  mgets(-1);
  while (str = mgets(fd))
  {
    this = ve_line(this, str);
  }

  this->next = next;
  if (next)
    next->prev = this;

  return this;
}


static inline void
tbf_read()
{
  int fd;
  char fpath[80];

  usr_fpath(fpath, cuser.userid, tbf_ask());

  fd = open(fpath, O_RDONLY);
  if (fd >= 0)
  {
    ve_load(vx_cur, fd);
    close(fd);
  }
}


static inline void
tbf_write()
{
  FILE *fp;
  textline *p;
  uschar *data;

  if (fp = tbf_open())
  {
    for (p = vx_ini; p;)
    {
      data = p->data;
      p = p->next;
      if (p || *data)
	fprintf(fp, "%s\n", data);
    }
    fclose(fp);
  }
}


static inline void
tbf_erase()
{
  char fpath[64];

  usr_fpath(fpath, cuser.userid, tbf_ask());
  unlink(fpath);
}


/* ----------------------------------------------------- */
/* �s�边�۰ʳƥ�					 */
/* ----------------------------------------------------- */


void
ve_backup()
{
  textline *p, *n;

  if (p = vx_ini)
  {
    FILE *fp;
    char bakfile[64];

    vx_ini = NULL;
    usr_fpath(bakfile, cuser.userid, FN_BAK);
    if (fp = fopen(bakfile, "w"))
    {
      do
      {
	n = p->next;
	fprintf(fp, "%s\n", p->data);
	free(p);
      } while (p = n);
      fclose(fp);
    }
  }
}


void
ve_recover()
{
  char fpbak[80], fpath[80];
  int ch;

  usr_fpath(fpbak, cuser.userid, FN_BAK);
  if (dashf(fpbak))
  {
    ch = vans("�z���@�g�峹�|�������A(M)�H��H�c (S)�g�J�Ȧs�� (Q)��F�H[M] ");
    if (ch == 's')
    {
      usr_fpath(fpath, cuser.userid, tbf_ask());
      rename(fpbak, fpath);
      return;
    }
    else if (ch != 'q')
    {
      mail_self(fpbak, cuser.userid, "���������峹", 0);
    }
    unlink(fpbak);
  }
}


/* ----------------------------------------------------- */
/* �ޥΤ峹						 */
/* ----------------------------------------------------- */


static int
is_quoted(str)
  char *str;			/* "--\n", "-- \n", "--", "-- " */
{
  if (*str == '-')
  {
    if (*++str == '-')
    {
      if (*++str == ' ')
	str++;
      if (*str == '\n')
	str++;
      if (!*str)
	return 1;
    }
  }
  return 0;
}


static inline int
quote_line(str, qlimit)
  char *str;
  int qlimit;			/* ���\�X�h�ި��H */
{
  int qlevel = 0;
  int ch;

  while ((ch = *str) == QUOTE_CHAR1 || ch == QUOTE_CHAR2)
  {
    if (*(++str) == ' ')
      str++;
    if (qlevel++ >= qlimit)
      return 0;
  }
  while ((ch = *str) == ' ' || ch == '\t')
    str++;
  if (qlevel >= qlimit)
  {
    if (!memcmp(str, "�� ", 3) || !memcmp(str, "==>", 3) ||
      strstr(str, ") ����:\n"))
      return 0;
  }
  return (*str != '\n');
}


static void
ve_quote(this)
  textline *this;
{
  int fd, op;
  FILE *fp;
  textline *next;
  char *str, buf[ANSILINELEN];
  static char msg[] = "���ñ�W�� (1/2/3 0=���[ r=�ü�)[0]�G";

  next = this->next;

  /* --------------------------------------------------- */
  /* �ި�						 */
  /* --------------------------------------------------- */

  if (*quote_file)
  {
    op = vans("�O�_�ޥέ�� Y)�ޥ� N)���ޥ� A)�ޥΥ��� R)���K���� 1-9)�ޥμh�ơH[Y] ");

    if (op != 'n')
    {
      if (fp = fopen(quote_file, "r"))
      {
	str = buf;

	if ((op >= '1') && (op <= '9'))
	  op -= '1';
	else if ((op != 'a') && (op != 'r'))
	  op = 1;		/* default : 2 level */

	if (op != 'a')		/* �h�� header */
	{
	  if (*quote_nick)
	    sprintf(buf + 128, " (%s)", quote_nick);
	  else
	    buf[128] = '\0';
	  sprintf(str, "�� �ޭz�m%s%s�n���ʨ��G", quote_user, buf + 128);
	  this = ve_line(this, str);

	  while (fgets(str, ANSILINELEN, fp) && *str != '\n');

	  if (curredit & EDIT_LIST)	/* �h�� mail list �� header */
	  {
	    while (fgets(str, ANSILINELEN, fp) && (!memcmp(str, "�� ", 3)));
	  }
	}

	if (op == 'r')
	{
	  op = 'a';
	}
	else
	{
	  *str++ = QUOTE_CHAR1;
	  *str++ = ' ';
	}

	if (op == 'a')
	{
	  while (fgets(str, ANSILINELEN - 2, fp))	/* �d�Ŷ��� "> " */
	    this = ve_line(this, buf);
	}
	else
	{
	  while (fgets(str, ANSILINELEN - 2, fp))	/* �d�Ŷ��� "> " */
	  {
	    if (is_quoted(str))	/* "--\n" */
	      break;
	    if (quote_line(str, op))
	      this = ve_line(this, buf);
	  }
	}
	fclose(fp);
      }
    }
    *quote_file = '\0';
  }

  this = ve_line(this, "");

  /* --------------------------------------------------- */
  /* ñ�W��						 */
  /* --------------------------------------------------- */

#ifdef HAVE_ANONYMOUS
  if (!(currbattr & BRD_ANONYMOUS) && !(cuser.ufo & UFO_NOSIGN))	/* �b�ΦW�O���L�׬O�_�ΦW�A�����ϥ�ñ�W�� */
#else
  if (!(cuser.ufo & UFO_NOSIGN))					/* itoc.000320: ���ϥ�ñ�W�� */
#endif
  {    
    if (cuser.ufo & UFO_SHOWSIGN)	/* itoc.000319: ���ñ�W�ɪ����e */
      show_sign();

    msg[33] = op = cuser.signature + '0';
    if (fd = vget(b_lines, 0, msg, buf, 3, DOECHO))
    {
      if (op != fd && ((fd >= '0' && fd <= '3') || fd == 'r'))
      {
	cuser.signature = fd - '0';
	op = fd;
      }
    }

    if (op == 'r')
      op = (time(0) % 3) + '1';

    if (op != '0')
    {
      char fpath[64];

      sprintf(buf, "%s.%c", FN_SIGN, op);
      usr_fpath(fpath, cuser.userid, buf);	/* itoc.020123: �U��ñ�W���ɮפ��} */

      if ((fd = open(fpath, O_RDONLY)) >= 0)
      {
	op = 0;
	mgets(-1);
	while ((str = mgets(fd)) && (op < MAXSIGLINES))
	{
	  if (!op)
	    this = ve_line(this, "--");

	  this = ve_line(this, str);
	  op++;
	}
	close(fd);
      }
    }
  }

  this->next = next;
  if (next)
    next->prev = this;
}


/* ----------------------------------------------------- */
/* �f�d user �ި����ϥ�					 */
/* ----------------------------------------------------- */


static int
quote_check()
{
  textline *p;
  char *str;
  int post_line;
  int quot_line;
  int in_quote;

  post_line = quot_line = in_quote = 0;
  for (p = vx_ini; p; p = p->next)
  {
    str = p->data;

    /* itoc.030305: �����@����§���Añ�W�ɤ]��ި� :p */

    if (in_quote)		/* ñ�W�� */
    {
      quot_line++;
    }
    else if (is_quoted(str))	/* ñ�W�ɶ}�l */
    {
      in_quote = 1;
      quot_line++;
    }
    else if (str[0] == QUOTE_CHAR1 && str[1] == ' ')	/* �ި� */
    {
      quot_line++;
    }
    else			/* �@�뤺�� */
    {
      /* �ťզ椣�� post_line */
      while (*str == ' ')
	str++;
      if (*str)
	post_line++;
    }
  }

  if ((quot_line >> 2) <= post_line)   /* �峹��ƭn�h��ި���ƥ|�����@ */
    return 0;

  if (HAS_PERM(PERM_ALLADMIN))
    return (vans("�ި��L�h (E)�~��s�� (W)�j��g�J�H[E] ") != 'w');

  vmsg("�ި��Ӧh�A�Ы� Ctrl+Y �ӧR�������n���ި�");
  return 1;
}


/* ----------------------------------------------------- */
/* �f�d user �o��峹�r��/�`���媺�ϥ�			 */
/* ----------------------------------------------------- */


int wordsnum;		/* itoc.010408: ��峹�r�� */


#ifdef ANTI_PHONETIC
static int
words_check()
{
  textline *p; 
  uschar *str, *pend;
  int phonetic;		/* �`����ƥ� */

  wordsnum = phonetic = 0;

  for (p = vx_ini; p; p = p->next)
  {
    if (is_quoted(str = p->data))	/* ñ�W�ɶ}�l */
      break;

    if (!(str[0] == QUOTE_CHAR1 && str[1] == ' ') && strncmp(str, "�� ", 3)) /* �D�ޥΥL�H�峹 */
    {
      wordsnum += p->len;

      pend = str + p->len;
      while (str < pend)
      {
	if (str[0] >= 0x81 && str[0] < 0xFE && str[1] >= 0x40 && str[1] <= 0xFE && str[1] != 0x7F)	/* ����r BIG5+ */
	{
	  if (str[0] == 0xA3 && str[1] >= 0x74 && str[1] <= 0xBA)	/* �`���� */
	    phonetic++;
	  str++;	/* ����r���줸�A�n�h�[�@�� */
	}
	str++;
      }

    }
  }
  return phonetic;
}

#else

static void
words_check()
{
  textline *p; 
  char *str;

  wordsnum = 0;

  for (p = vx_ini; p; p = p->next)
  {
    if (is_quoted(str = p->data))	/* ñ�W�ɶ}�l */
      break;

    if (!(str[0] == QUOTE_CHAR1 && str[1] == ' ') && strncmp(str, "�� ", 3)) /* �D�ޥΥL�H�峹 */
      wordsnum += p->len;
  }
}
#endif


/* ----------------------------------------------------- */
/* �ɮ׳B�z�GŪ�ɡB�s�ɡB���D�Bñ�W��			 */
/* ----------------------------------------------------- */


void
ve_header(fp)
  FILE *fp;
{
  time_t now;
  char *title;

  title = ve_title;
  title[72] = '\0';
  time(&now);

  if (curredit & EDIT_MAIL)
  {
    fprintf(fp, "%s %s (%s)\n", str_author1, cuser.userid, cuser.username);
  }
  else
  {   
#ifdef HAVE_ANONYMOUS
    if (currbattr & BRD_ANONYMOUS && !(curredit & EDIT_RESTRICT))
    {
      if (!vget(b_lines, 0, "�п�J�z�Q�Ϊ�ID�A�]�i������[Enter]�A�άO��[r]�ίu�W�G", anonymousid, IDLEN, DOECHO))
      {											/* �d 1 byte �[ "." */
	strcpy(anonymousid, STR_ANONYMOUS);
	curredit |= EDIT_ANONYMOUS;
      }
      else if (strcmp(anonymousid, "r"))
      {
	strcat(anonymousid, ".");		/* �Y�O�۩wID�A�n�b�᭱�[�� . ��ܤ��P */
	curredit |= EDIT_ANONYMOUS;
      }
    }
#endif

    if (!(currbattr & BRD_NOSTAT) && !(curredit & EDIT_RESTRICT))	/* ���p�峹�g�� �� �[�K�s�� ���C�J�έp�g�� */
    {
      /* ���Ͳέp��� */

      POSTLOG postlog;

#ifdef HAVE_ANONYMOUS
      /* Thor.980909: anonymous post mode */
      if (curredit & EDIT_ANONYMOUS)
	strcpy(postlog.author, anonymousid);
      else
#endif
	strcpy(postlog.author, cuser.userid);

      strcpy(postlog.board, currboard);
      str_ncpy(postlog.title, str_ttl(title), sizeof(postlog.title));
      postlog.date = now;
      postlog.number = 1;

      rec_add(FN_RUN_POST, &postlog, sizeof(POSTLOG));
    }

#ifdef HAVE_ANONYMOUS
    /* Thor.980909: anonymous post mode */
    if (curredit & EDIT_ANONYMOUS)
    {
      fprintf(fp, "%s %s (%s) %s %s\n",
	str_author1, anonymousid, STR_ANONYMOUS,
	curredit & EDIT_OUTGO ? str_post1 : str_post2, currboard);
    }
    else
#endif

    {
      fprintf(fp, "%s %s (%s) %s %s\n", 
	str_author1, cuser.userid, cuser.username,
	curredit & EDIT_OUTGO ? str_post1 : str_post2, currboard);
    }
  }
  fprintf(fp, "���D: %s\n�ɶ�: %s\n\n", title, Btime(&now));
}


void
ve_banner(fp, modify)		/* �[�W�ӷ����T�� */
  FILE *fp;
  int modify;			/* 1:�ק� 0:��� */
{
  /* itoc: ��ĳ banner ���n�W�L�T��A�L������ñ�i��|�y���Y�ǨϥΪ̪��ϷP */

  if (!modify)
  {
    fprintf(fp, EDIT_BANNER, 
#ifdef HAVE_ANONYMOUS
      (curredit & EDIT_ANONYMOUS) ? STR_ANONYMOUS : 
#endif
      cuser.userid, 
#ifdef HAVE_ANONYMOUS
      (curredit & EDIT_ANONYMOUS) ? "���P�s������ ^O^||" : 
#endif
      fromhost);
  }
  else
  {
    fprintf(fp, MODIFY_BANNER, cuser.userid, Now());
  }
}


static int
ve_filer(fpath, ve_op)
  char *fpath;
  int ve_op;	/* 1: �� header  0,2: �L header  -1: �����x�s */
{
  int ans;
  FILE *fp;
  textline *p, *v;
  char buf[80], *msg;

#ifdef POPUP_ANSWER
  char **menu;
#  ifdef HAVE_REFUSEMARK
  char *menu1[] = {"EE", "Abort    ���", "Title    ����D", "Edit     �~��s��", "Read     Ū���Ȧs��", "Write    �g�J�Ȧs��", "Delete   �R���Ȧs��", NULL};
  char *menu2[] = {"SE", "Save     �s��", "Abort    ���", "Title    ����D", "Edit     �~��s��", "Read     Ū���Ȧs��", "Write    �g�J�Ȧs��", "Delete   �R���Ȧs��", NULL};
  char *menu3[] = {"SE", "Save     �s��", "Local    �s��������", "XRefuse  �[�K�s��", "Abort    ���", "Title    ����D", "Edit     �~��s��", "Read     Ū���Ȧs��", "Write    �g�J�Ȧs��", "Delete   �R���Ȧs��", NULL};
  char *menu4[] = {"LE", "Local    �s��������", "Save     �s��", "XRefuse  �[�K�s��", "Abort    ���", "Title    ����D", "Edit     �~��s��", "Read     Ū���Ȧs��", "Write    �g�J�Ȧs��", "Delete   �R���Ȧs��", NULL};
#  else
  char *menu1[] = {"EE", "Abort    ���", "Title    ����D", "Edit     �~��s��", "Read     Ū���Ȧs��", "Write    �g�J�Ȧs��", "Delete   �R���Ȧs��", NULL};
  char *menu2[] = {"SE", "Save     �s��", "Abort    ���", "Title    ����D", "Edit     �~��s��", "Read     Ū���Ȧs��", "Write    �g�J�Ȧs��", "Delete   �R���Ȧs��", NULL};
  char *menu3[] = {"SE", "Save     �s��", "Local    �s��������", "Abort    ���", "Title    ����D", "Edit     �~��s��", "Read     Ū���Ȧs��", "Write    �g�J�Ȧs��", "Delete   �R���Ȧs��", NULL};
  char *menu4[] = {"LE", "Local    �s��������", "Save     �s��", "Abort    ���", "Title    ����D", "Edit     �~��s��", "Read     Ū���Ȧs��", "Write    �g�J�Ȧs��", "Delete   �R���Ȧs��", NULL};
#  endif
#else
#  ifdef HAVE_REFUSEMARK
  char *msg1 = "[E]�~�� (A)��� (R/W/D)Ū�g�R�Ȧs�ɡH";
  char *msg2 = "[S]�s�� (A)��� (T)����D (E)�~�� (R/W/D)Ū�g�R�Ȧs�ɡH";
  char *msg3 = "[S]�s�� (L)���� (X)�K�� (A)��� (T)����D (E)�~�� (R/W/D)Ū�g�R�Ȧs�ɡH";
  char *msg4 = "[L]���� (S)�s�� (X)�K�� (A)��� (T)����D (E)�~�� (R/W/D)Ū�g�R�Ȧs�ɡH";
#  else
  char *msg1 = "[E]�~�� (A)��� (R/W/D)Ū�g�R�Ȧs�ɡH";
  char *msg2 = "[S]�s�� (A)��� (T)����D (E)�~�� (R/W/D)Ū�g�R�Ȧs�ɡH";
  char *msg3 = "[S]�s�� (L)���� (A)��� (T)����D (E)�~�� (R/W/D)Ū�g�R�Ȧs�ɡH";
  char *msg4 = "[L]���� (S)�s�� (A)��� (T)����D (E)�~�� (R/W/D)Ū�g�R�Ȧs�ɡH";
#  endif
#endif

  ans = 0;

#ifdef POPUP_ANSWER
  if (ve_op < 0)			/* itoc.010301: �s�W ve_op = -1 �����x�s */
    menu = menu1;
  else if (bbsmode != M_POST)		/* �g�H */
    menu = menu2;
  else if (curredit & EDIT_OUTGO)	/* ��H�O�o�� */
    menu = menu3;
  else
    menu = menu4;

  switch (pans(3, 20, "�s�ɿﶵ", menu))
#else
  if (ve_op < 0)			/* itoc.010301: �s�W ve_op = -1 �����x�s */
    msg = msg1;
  else if (bbsmode != M_POST)		/* �g�H */
    msg = msg2;
  else if (curredit & EDIT_OUTGO)	/* ��H�O�o�� */
    msg = msg3;
  else
    msg = msg4;

  switch (vans(msg))
#endif

  {
  case 's':
    if (ve_op < 0)		/* itoc.010301: �����x�s */
      return VE_FOOTER;
    /* Thor.990111: ����H�h���~�y */
    if (HAS_PERM(PERM_INTERNET) && !(currbattr & BRD_NOTRAN))
      curredit |= EDIT_OUTGO;
    break;

  case 'a':
    ans = -1;
    break;

  case 'l':
    if (ve_op < 0)		/* itoc.010301: �����x�s */
      return VE_FOOTER;
    curredit &= ~EDIT_OUTGO;
    break;

#ifdef HAVE_REFUSEMARK
  case 'x':
   if (ve_op < 0)		/* itoc.010301: �����x�s */
     return VE_FOOTER;
   curredit |= EDIT_RESTRICT;
   curredit &= ~EDIT_OUTGO;	/* �[�K���O local save */
   break;
#endif

  case 'r':
    tbf_read();
    return VE_REDRAW;

  case 'e':
    return VE_FOOTER;

  case 'w':
    tbf_write();
    return VE_FOOTER;

  case 'd':
    tbf_erase();
    return VE_FOOTER;

  case 't':
    if (ve_op > 0)		/* itoc.010301: �����x�s */
    {
      strcpy(buf, ve_title);
      if (!vget(b_lines, 0, "���D�G", ve_title, TTLEN + 1, GCARRY))
	strcpy(ve_title, buf);
    }
    return VE_FOOTER;

  default:
    if (ve_op < 0)		 /* itoc.010301: �����x�s */
      return VE_FOOTER;    
  }

  if (!ans)
  {
    if (ve_op == 1 && !(curredit & EDIT_MAIL) && quote_check())
      return VE_FOOTER;

#ifdef ANTI_PHONETIC
    if (words_check() > 2)
    {
      vmsg("�ФŨϥΪ`����");
      return VE_FOOTER;
    }
#endif

    if (!*fpath)
    {
      usr_fpath(fpath, cuser.userid, fn_note);
    }

    if ((fp = fopen(fpath, "w")) == NULL)
    {
      ve_abort(5);
      abort_bbs();
    }

#ifndef ANTI_PHONETIC
    words_check();	/* itoc.010408: ��峹�r�� */
#endif

    if (ve_op == 1)
      ve_header(fp);
  }

  if (p = vx_ini)
  {
    vx_ini = NULL;

    do
    {
      v = p->next;
      if (!ans)
      {
	msg = p->data;
	str_trim(msg);
	fprintf(fp, "%s\n", msg);
      }
      free(p);
    } while (p = v);
  }

  if (!ans)
  {
    if (bbsmode == M_POST || bbsmode == M_SMAIL)
      ve_banner(fp, 0);
    fclose(fp);
  }

  return ans;
}


/* ----------------------------------------------------- */
/* �ù��B�z�G���U�T���B��ܽs�褺�e			 */
/* ----------------------------------------------------- */


static void
ve_outs(text)
  uschar *text;
{
  int ch;
  uschar *tail;

  tail = text + SCR_WIDTH;
  while (ch = *text)
  {
    switch (ch)
    {
    case KEY_ESC:
      ch = '*';
      break;
    }
    outc(ch);

    if (++text >= tail)
      break;
  }
}


int
ve_subject(row, topic, dft)
  int row;
  char *topic;
  char *dft;
{
  char *title;

  title = ve_title;

  if (topic)
  {
    sprintf(title, "Re: %s", str_ttl(topic));
    title[TTLEN] = '\0';
  }
  else
  {
    if (dft)
      strcpy(title, dft);
    else
      *title = '\0';
  }

  return vget(row, 0, "���D�G", title, TTLEN + 1, GCARRY);
}


/* ----------------------------------------------------- */
/* �s��B�z�G�D�{���B��L�B�z				 */
/* ----------------------------------------------------- */

/* ----------------------------------------------------- */
/* vedit �^�� -1:�����s�� 0:�����s��			 */
/* ----------------------------------------------------- */
/* ve_op:						 */
/*  0 => �º�s���ɮ�					 */
/* -1 => �s��������x�s�A�Φb�s��@�̤��O�ۤv���峹	 */
/*  1 => �ޤ�B�[ñ�W�ɡA�å[�W���Y�A�Φb�o��峹/�����H */
/*  2 => �ޤ�B�[ñ�W�ɡA���[�W���Y�A�Φb�H���~�H	 */
/* ----------------------------------------------------- */
/* �Y ve_op �O 1 �� 2 �ɡA�i�J vedit �e�ٱo���w curredit */
/* ----------------------------------------------------- */

int		/* -1:�����s�� 0:�����s�� */
vedit(fpath, ve_op)
  char *fpath;
  int ve_op;	/* 0:�º�s���ɮ�  -1:�s��������x�s  1:quote/header  2:quote */
{
  textline *vln, *tmp;
  int cc, col, mode, margin, pos;

  /* --------------------------------------------------- */
  /* ��l�]�w�G���J�ɮסB�ޥΤ峹�B�]�w�s��Ҧ�		 */
  /* --------------------------------------------------- */

  tmp = vln = ve_alloc();

  if (*fpath)
  {
    cc = open(fpath, O_RDONLY);
    if (cc >= 0)
    {
      vln = ve_load(vln, cc);
    }
    else
    {
      cc = open(fpath, O_WRONLY | O_CREAT, 0600);
      if (cc < 0)
      {
	ve_abort(4);
	abort_bbs();
      }
    }
    close(cc);
  }

  /* if (ve_op) */
  if (ve_op > 0)	/* itoc.010301: �s�W ve_op = -1 �ɤ����x�s */
  {
    ve_quote(vln);
  }

  if (vln = tmp->next)
  {
    free(tmp);
    vln->prev = NULL;
  }
  else
  {
    vln = tmp;
  }

  vx_cur = vx_top = vx_ini = vln;

  ve_col = ve_row = margin = 0;
  ve_lno = 1;
  ve_mode = VE_INSERT | VE_REDRAW | VE_FOOTER;

  /* --------------------------------------------------- */
  /* �D�j��G�ù���ܡB��L�B�z�B�ɮ׳B�z		 */
  /* --------------------------------------------------- */

  clear();

  for (;;)
  {
    vln = vx_cur;
    mode = ve_mode;
    col = ve_col;
    /* itoc.031123.����: �p�G�W�L SCR_WIDTH�A���򭶭����k½�A�ëO�d�������̫� 4 �r */
    cc = (col < SCR_WIDTH) ? 0 : (col / (SCR_WIDTH - 4)) * (SCR_WIDTH - 4);
    if (cc != margin)
    {
      mode |= VE_REDRAW;
      margin = cc;
    }

    if (mode & VE_REDRAW)
    {
      ve_mode = (mode ^= VE_REDRAW);

      tmp = vx_top;

      for (pos = 0;; pos++)
      {
	move(pos, 0);
	clrtoeol();
	if (pos == b_lines)
	  break;
	if (tmp)
	{
	  if (mode & VE_ANSI)
	    outx(tmp->data);
	  else if (tmp->len > margin)
	    ve_outs(tmp->data + margin);
	  tmp = tmp->next;
	}
	else
	{
	  outc('~');
	}
      }
#ifdef EVERY_BIFF
      if (!(mode & VE_BIFF))
      {
	if (HAS_STATUS(STATUS_BIFF))
	  ve_mode = mode |= VE_BIFF;
      }
#endif
    }
    else
    {
      move(ve_row, 0);
      if (mode & VE_ANSI)
	outx(vln->data);
      else if (vln->len > margin)
	ve_outs(vln->data + margin);
      clrtoeol();
    }

    /* ------------------------------------------------- */
    /* ��ܪ��A�BŪ����L				 */
    /* ------------------------------------------------- */

    if (mode & VE_ANSI)		/* Thor: �@ ansi �s�� */
      pos = n2ansi(col, vln);	/* Thor: ansi ���|�Ψ�cc */
    else			/* Thor: ���Oansi�n�@margin shift */
      pos = col - margin;

    if (mode & VE_FOOTER)
    {
      move(b_lines, 0);
      clrtoeol();

      if (cuser.ufo & UFO_VEDIT)
      {
	ve_mode = (mode ^= VE_FOOTER);
      }
      else
      {
	prints(FOOTER_VEDIT,
#ifdef EVERY_BIFF
	  mode & VE_BIFF ? "�l�t�ӤF" : "�s��峹",
#else
	  "�s��峹", 
#endif
	  mode & VE_INSERT ? "���J" : "���N",
	  mode & VE_ANSI ? "ANSI" : "�@��",
	  ve_lno, 1 + (mode & VE_ANSI ? pos : col));
      }
    }

    move(ve_row, pos);

ve_key:

    cc = vkey();

    if (isprint2(cc))
    {
      ve_char(cc);
    }
    else
    {
      switch (cc)
      {
      case '\n':

	ve_split(vln, col);
	break;

      case KEY_TAB:

	do
	{
	  ve_char(' ');
	} while (ve_col & (TAB_STOP - 1));
	break;

      case KEY_INS:		/* Toggle insert/overwrite */

	ve_mode = mode ^ VE_INSERT;
	continue;

      case KEY_BKSP:		/* backspace */

	/* Thor: �b ANSI �s��Ҧ��U, ���i�H���˰h, ���M�|�ܥi��.... */

	if (mode & VE_ANSI)
	{	
#if 0
	  goto ve_key;	/* ����h��N��S�� */	  
#endif

	  /* itoc.010322: ANSI �s��ɫ���h��^��D ANSI �Ҧ� */
  	  mode ^= VE_ANSI;
	  clear();
	  ve_mode = mode | VE_REDRAW;
	  continue;	  
	}

	if (col)
	{
	  delete_char(vln, ve_col = --col);
	  continue;
	}

	if (!(tmp = vln->prev))
	  goto ve_key;

	ve_row--;
	ve_lno--;
	vx_cur = tmp;
	ve_col = tmp->len;
	if (*ve_strim(vln->data))
	  join_up(tmp);
	else
	  delete_line(vln);
	ve_mode = mode | VE_REDRAW;
	break;

      case Ctrl('D'):
      case KEY_DEL:		/* delete current character */

	cc = vln->len;
	if (cc == col)
	{
	  join_up(vln);
	  ve_mode = mode | VE_REDRAW;
	}
	else
	{
	  if (cc == 0)
	    goto ve_key;
	  delete_char(vln, col);
	  if (mode & VE_ANSI)	/* Thor: ���M�W�[ load, ���Ledit �ɷ|����n�� */
	    ve_col = ansi2n(n2ansi(col, vln), vln);
	}
	continue;

      case KEY_LEFT:

	if (col)
	{
	  ve_col = (mode & VE_ANSI) ? ansi2n(pos - 1, vln) : col - 1;
	  continue;
	}

	if (!(tmp = vln->prev))
	  goto ve_key;

	ve_row--;
	ve_lno--;
	ve_col = tmp->len;
	vx_cur = tmp;
	break;

      case KEY_RIGHT:

	if (vln->len != col)
	{
	  ve_col = (mode & VE_ANSI) ? ansi2n(pos + 1, vln) : col + 1;
	  continue;
	}

	if (!(tmp = vln->next))
	  goto ve_key;

	ve_row++;
	ve_lno++;
	ve_col = 0;
	vx_cur = tmp;
	break;

      case KEY_HOME:
      case Ctrl('A'):

	ve_col = 0;
	continue;

      case KEY_END:
      case Ctrl('E'):

	ve_col = vln->len;
	continue;

      case KEY_UP:
      case Ctrl('P'):

	if (!(tmp = vln->prev))
	  goto ve_key;

	ve_row--;
	ve_lno--;
	if (mode & VE_ANSI)
	{
	  ve_col = ansi2n(pos, tmp);
	}
	else
	{
	  cc = tmp->len;
	  if (col > cc)
	    ve_col = cc;
	}
	vx_cur = tmp;
	break;

      case KEY_DOWN:
      case Ctrl('N'):

	if (!(tmp = vln->next))
	  goto ve_key;

	ve_row++;
	ve_lno++;
	if (mode & VE_ANSI)
	{
	  ve_col = ansi2n(pos, tmp);
	}
	else
	{
	  cc = tmp->len;
	  if (col > cc)
	    ve_col = cc;
	}
	vx_cur = tmp;
	break;

      case KEY_PGUP:
      case Ctrl('B'):

	ve_pageup();
	continue;

      case KEY_PGDN:
      case Ctrl('F'):
      case Ctrl('T'):		/* tail of file */

	ve_forward(cc == Ctrl('T') ? -1 : PAGE_SCROLL);
	continue;

      case Ctrl('S'):		/* start of file */

	vx_cur = vx_top = vx_ini;
	ve_col = ve_row = 0;
	ve_lno = 1;
	ve_mode = mode | VE_REDRAW;
	continue;

      case Ctrl('V'):		/* Toggle ANSI color */

	mode ^= VE_ANSI;
	clear();
	ve_mode = mode | VE_REDRAW;
	continue;

      case Ctrl('X'):		/* Save and exit */

	/* cc = ve_filer(fpath, ve_op & 1); */
	cc = ve_filer(fpath, ve_op);	/* itoc.010301: �s�W ve_op = -1 �ɤ����x�s */
	if (cc <= 0)
	  return cc;
	ve_mode = mode | cc;
	continue;

      case Ctrl('Z'):

	cutmp->status |= STATUS_EDITHELP;
	xo_help("post");
	cutmp->status ^= STATUS_EDITHELP;
	ve_mode = mode | VE_REDRAW;
	continue;

      case Ctrl('C'):

	ve_ansi();
	break;

      case Ctrl('O'):		/* delete to end of file */

	/* vln->len = ve_col = cc = 0; */
	tmp = vln->next;
	vln->next = NULL;
	while (tmp)
	{
	  vln = tmp->next;
	  free(tmp);
	  tmp = vln;
	}
	ve_mode = mode | VE_REDRAW;
	continue;

      case Ctrl('Y'):		/* delete current line */

	vln->len = ve_col = 0;
	vln->data[0] = '\0'; /* Thor.981001: �N���e�@�ֲM�� */

      case Ctrl('K'):		/* delete to end of line */

	if (cc = vln->len)
	{
	  if (cc != col)
	  {
	    vln->len = col;
	    vln->data[col] = '\0';
	    continue;
	  }

	  join_up(vln);
	}
	else
	{
	  tmp = vln->next;
	  if (!tmp)
	  {
	    tmp = vln->prev;
	    if (!tmp)
	      break;

	    if (ve_row > 0)
	    {
	      ve_row--;
	      ve_lno--;
	    }
	  }
	  if (vln == vx_top)
	    vx_top = tmp;
	  delete_line(vln);
	  vx_cur = tmp;
	}

	ve_mode = mode | VE_REDRAW;
	break;

      case Ctrl('U'):

	ve_char(KEY_ESC);
	break;

#ifdef SHOW_USER_IN_TEXT
      case Ctrl('Q'):
	cc = vans("��ܨϥΪ̸��(1)id (2)�ʺ١H");
	if (cc >= '1' && cc <= '2')
	{
	  ve_char(KEY_ESC);
	  ve_char('*');
	  ve_char("sn"[cc - '1']);
	}
	ve_mode = mode | VE_FOOTER;
	break;
#endif

#ifdef INPUT_TOOLS
      case Ctrl('W'):

	input_tools();
	ve_mode = mode | VE_FOOTER;
	break;
#endif

      default:

	goto ve_key;
      }
    }

    /* ------------------------------------------------- */
    /* ve_row / ve_lno �վ�				 */
    /* ------------------------------------------------- */

    cc = ve_row;
    if (cc < 0)
    {
      ve_row = 0;
      if (vln = vx_top->prev)
      {
	vx_top = vln;
	rscroll();
      }
      else
      {
	ve_abort(6);
      }
    }
    else if (cc >= b_lines)
    {
      ve_row = b_lines - 1;
      if (vln = vx_top->next)
      {
	vx_top = vln;
	scroll();
      }
      else
      {
	ve_abort(7);
      }
    }
  }
}
