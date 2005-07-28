/*-------------------------------------------------------*/
/* bwboard.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : B/W & Chinese Chess Board			 */
/* create : 02/08/05					 */
/* update :   /  /  					 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME


/*-------------------------------------------------------*/
/* �Ҧ��ѽL���Ϊ��Ѽ�					 */
/*-------------------------------------------------------*/


extern char lastcmd[MAXLASTCMD][80];

#ifdef EVERY_Z
extern int vio_fd, holdon_fd;
#endif

enum
{
  DISCONNECT = -2, LEAVE = -1, NOTHING = 0
};


enum
{
  Binit = 0, Bupdate = 1, Ballow = 2
};


enum
{
  Empty = 0, Black = 1, White = 2, Red = 2
};


static int msgline;		/* Where to display message now */
static int cfd;			/* socket number */
static int myColor;		/* my chess color */
static int Choose;		/* -3:�¥մ� -2:���l�� -1:��� 0:�x�� 1:�t�� */
static int dark_choose;		/* �t�ѬO�_�w½�}�Ĥ@�l */

static int (**rule) ();

static int bwRow, bwCol;
static char Board[19][19];


static char *ruleStrSet[] = {"�¥մ�", "���l��", "���", "�x��", "�t��"};
static char *ruleStr;

static KeyFunc *mapTalk, *mapTurn;

static int cmdCol, cmdPos;
static char talkBuf[42] = "T";
static char *cmdBuf = &talkBuf[1];
extern KeyFunc Talk[];


static void bw_printmsg();
static void ch_printmsg();
static void do_init();


static int
do_send(buf)
  char *buf;
{
  int len;

  len = strlen(buf) + 1;	/* with trail 0 */
  return (send(cfd, buf, len, 0) == len);
}


#ifdef EVERY_BIFF
static void
check_biff()
{
  /* Thor.980805: ���H�b�����enter�~�ݭncheck biff */
  static int old_biff;
  int biff;
  char *msg = "�� ��! �l�t�ӫ��a�F!";

  biff = cutmp->status & STATUS_BIFF;
  if (biff && !old_biff)
    (Choose < 0) ? bw_printmsg(msg) : ch_printmsg(1, msg);
  old_biff = biff;
}
#endif


static int
fTAB()
{
  mapTalk = mapTalk ? NULL : Talk;
  return NOTHING;
}


static int
fNoOp()
{
  return NOTHING;
}


static int
fCtrlD()
{
  /* send Q.....\0 cmd */
  if (!do_send("Q"))
    return DISCONNECT;
  return LEAVE;
}


static int
ftkCtrlC()
{
  *cmdBuf = '\0';
  cmdCol = 0;
  move(b_lines - 2, 35);
  clrtoeol();
  return NOTHING;
}


static int
ftkCtrlH()
{
  if (cmdCol)
  {
    int ch = cmdCol--;
    memcpy(&cmdBuf[cmdCol], &cmdBuf[ch], sizeof(talkBuf) - ch - 1);
    move(b_lines - 2, cmdCol + 35);
    outs(&cmdBuf[cmdCol]);
    clrtoeol();
  }
  return NOTHING;
}


static int
ftkEnter()
{
  char msg[80];

#ifdef EVERY_BIFF
  check_biff();
#endif

  if (*cmdBuf)
  {
    for (cmdPos = MAXLASTCMD - 1; cmdPos; cmdPos--)
      strcpy(lastcmd[cmdPos], lastcmd[cmdPos - 1]);
    strcpy(lastcmd[0], cmdBuf);

    if (!do_send(talkBuf))
      return DISCONNECT;

    sprintf(msg, "\033[1;36m��%s\033[m", cmdBuf);
    (Choose < 0) ? bw_printmsg(msg) : ch_printmsg(1, msg);

    *cmdBuf = '\0';
    cmdCol = 0;
    cmdPos = -1;
    move(b_lines - 2, 35);
    clrtoeol();
  }
  return NOTHING;
}


static int
ftkLEFT()
{
  if (cmdCol)
    --cmdCol;
  return NOTHING;
}


static int
ftkRIGHT()
{
  if (cmdBuf[cmdCol])
    ++cmdCol;
  return NOTHING;
}


static int
ftkUP()
{
  cmdPos++;
  cmdPos %= MAXLASTCMD;
  str_ncpy(cmdBuf, lastcmd[cmdPos], 41);
  move(b_lines - 2, 35);
  outs(cmdBuf);
  clrtoeol();
  cmdCol = strlen(cmdBuf);
  return NOTHING;
}


static int
ftkDOWN()
{
  cmdPos += MAXLASTCMD - 2;
  return ftkUP();
}


static int
ftkDefault(ch)
  int ch;
{
  if (isprint2(ch))
  {
    if (cmdCol < 40)
    {
      if (cmdBuf[cmdCol])
      {				/* insert */
	int i;
	for (i = cmdCol; cmdBuf[i] && i < 39; i++);
	cmdBuf[i + 1] = '\0';
	for (; i > cmdCol; i--)
	  cmdBuf[i] = cmdBuf[i - 1];
      }
      else
      {				/* append */
	cmdBuf[cmdCol + 1] = '\0';
      }
      cmdBuf[cmdCol] = ch;
      move(b_lines - 2, cmdCol + 35);
      outs(&cmdBuf[cmdCol++]);
    }
    return NOTHING;
  }
}


static KeyFunc Talk[] =
{
  Ctrl('C'), ftkCtrlC,
  Ctrl('D'), fCtrlD,
  Ctrl('H'), ftkCtrlH,
  '\n', ftkEnter,
  KEY_LEFT, ftkLEFT,
  KEY_RIGHT, ftkRIGHT,
  KEY_UP, ftkUP,
  KEY_DOWN, ftkDOWN,
  KEY_TAB, fTAB,
  0, ftkDefault
};


/*-------------------------------------------------------*/
/* target : ���Z�O���{��				 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


static void
play_count(userid, total, win)	/* �^�� userid ���`���ơB�ӳ��� */
  char *userid;
  int *total, *win;
{
  char fpath[64];
  int fd;
  int grade[32];	/* �O�d 32 �� int */

  *total = 0;
  *win = 0;

  usr_fpath(fpath, userid, "bwboard");
  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    read(fd, grade, sizeof(grade));
    close(fd);

    /* �̧��x�s �¥��`,�¥ճ�; ���l�`,���l��; ���`,���; �x�`,�x��; �t�`,�t�� */
    fd = (Choose + 3) << 1;
    *total = grade[fd];
    *win = grade[fd + 1];
  }
}


static void
play_add(win)			/* �ڪ��`/�ӳ��� +1 */
  int win;
{
  char fpath[64];
  int fd, kind;
  int grade[32];	/* �O�d 32 �� int */

  usr_fpath(fpath, cuser.userid, "bwboard");
  if ((fd = open(fpath, O_RDWR | O_CREAT, 0600)) >= 0)
  {
    if (read(fd, grade, sizeof(grade)) != sizeof(grade))
      memset(grade, 0, sizeof(grade));

    /* �̧��x�s �¥��`,�¥ճ�; ���l�`,���l��; ���`,���; �x�`,�x��; �t�`,�t�� */
    kind = (Choose + 3) << 1;
    grade[kind]++;
    if (win)
      grade[kind + 1]++;

    lseek(fd, (off_t) 0, SEEK_SET);
    write(fd, grade, sizeof(grade));
    close(fd);
  }
}


/*-------------------------------------------------------*/
/* target : Black & White Chess Board �¥մ�/���l��/��� */
/* create : 99/02/20					 */
/* update : 02/08/05					 */
/* author : thor.bbs@bbs.cs.nthu.edu.tw			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


enum
{
  Deny = 3
};


extern KeyFunc myRound[], yourRound[];

static char *bw_icon[] = {"�q", "��", "��", "  "};	/* Empty, Black, White, Deny */


/* 19 * 19, standard chess board */
static char Allow[19][19];
static int numWhite, numBlack;
static int rulerRow, rulerCol;

static int yourPass, myPass;
static int yourStep, myStep;

static int GameOver;			/* end game */


#if 0

  % rules util include
  0. Board initialize
     (happen before start, and clear board action)
  1. Board update by "color" down a "chess pos"(without update screen)
     (screen by a main redraw routine, after call this)
     return value, represent win side to end(for both turn)
     (happen when state change)
  2. Board allow step checking by "color"(for my turn)
     return value, represent

  % possible step
     (happen when state change)
  if Game is over, won''t allow any step

#endif


/* numWhite, numBlack maintained by rule, 0&1 */
/* Board[19][19] Allow[19][19] maintained by rule */


static inline void
countBWnum()
{
  int i, j;

  numWhite = numBlack = 0;
  for (i = 0; i < 19; i++)
  {
    for (j = 0; j < 19; j++)
    {
      if (Board[i][j] == White)
	numWhite++;
      else if (Board[i][j] == Black)
	numBlack++;
    }
  }
}


  /*-----------------------------------------------------*/
  /* �¥մ� 8 x 8					 */
  /*-----------------------------------------------------*/

static int
othInit()
{
  int i, j;
  for (i = 0; i < 19; i++)
    for (j = 0; j < 19; j++)
      Board[i][j] = i < 8 && j < 8 ? Empty : Deny;
  Board[3][3] = Board[4][4] = Black;
  Board[3][4] = Board[4][3] = White;
  numWhite = numBlack = 2;
  rulerRow = rulerCol = 8;

  return 0;
}


static inline int
othEatable(Color, row, col, rowstep, colstep)
  int Color, row, col, rowstep, colstep;
{
  int eat = 0;
  do
  {
    row += rowstep;
    col += colstep;
    /* check range */
    if (row < 0 || row >= 8 || col < 0 || col >= 8)
      return 0;
    if (Board[row][col] == Color)
      return eat;
    eat = 1;
  } while (Board[row][col] == Deny - Color);
  return 0;
}


static int
othUpdate(Color, row, col)
  int Color, row, col;
{
  int i, j, p, q;
  int winside = Empty;

  Board[row][col] = Color;
  for (i = -1; i <= 1; i++)
  {
    for (j = -1; j <= 1; j++)
    {
      if (i != 0 || j != 0)
      {
	if (othEatable(Color, row, col, i, j))
	{
	  p = row + i;
	  q = col + j;
	  for (;;)
	  {
	    if (Board[p][q] == Color)
	      break;
	    Board[p][q] = Color;
	    p += i;
	    q += j;
	  }
	}
      }
    }
  }

  /* count numWhite & numBlack */
  countBWnum();

  /* Thor.990329.����: �U���� */
  {
    static int othAllow();
    int my = myColor;		/* Thor.990331: �ȦsmyColor */
    int allowBlack, allowWhite;
    myColor = Black;
    allowBlack = othAllow();
    myColor = White;
    allowWhite = othAllow();
    myColor = my;
    if (allowBlack == 0 && allowWhite == 0)
    {
      if (numWhite > numBlack)
	winside = White;
      else if (numWhite < numBlack)
	winside = Black;
      else
	winside = Deny;
    }
  }

  return winside;
}


static void
do_othAllow(i, j, num)
  int i, j;
  int *num;
{
  int p, q;

  Allow[i][j] = 0;
  if (Board[i][j] == Empty)
  {
    for (p = -1; p <= 1; p++)
    {
      for (q = -1; q <= 1; q++)
      {
	if (p != 0 || q != 0)
	{
	  if (othEatable(myColor, i, j, p, q))
	  {
	    Allow[i][j] = 1;
	    (*num)++;
	    return;
	  }
	}
      }
    }
  }
}


static int
othAllow()
{
  int i, j, num;

  num = 0;
  for (i = 0; i < 8; i++)
  {
    for (j = 0; j < 8; j++)
    {
      do_othAllow(i, j, &num);
    }
  }

  return num;
}


static int (*othRule[]) () =
{
  othInit, othUpdate, othAllow
};


  /*-----------------------------------------------------*/
  /* ���l�� 15 x 15					 */
  /*-----------------------------------------------------*/

static int
fivInit()
{
  int i, j;
  for (i = 0; i < 19; i++)
  {
    for (j = 0; j < 19; j++)
      Board[i][j] = i < 15 && j < 15 ? Empty : Deny;
  }
  numWhite = numBlack = 0;
  rulerRow = rulerCol = 15;
  return 0;
}


static int
fivCount(Color, row, col, rowstep, colstep)
  int Color, row, col, rowstep, colstep;
{
  int count = 0;
  for (;;)
  {
    row += rowstep;
    col += colstep;
    /* check range */
    if (row < 0 || row >= 15 || col < 0 || col >= 15)
      return count;

    if (Board[row][col] != Color)
      return count;
    count++;
  }
}


static int
fivUpdate(Color, row, col)
  int Color, row, col;
{
#if 0
  int cnt[4], n3, n4, n5, nL, i;
#endif

  int winside = Empty;
  Board[row][col] = Color;
  if (Color == Black)
    numBlack++;
  else if (Color == White)
    numWhite++;


  /* �´ѡ]���̡۪^���U�C�T�۸T��(�S�ٸT��)�I�G�T�T(�����T)�B�|�|�B���s�C */
  /* �b�s�����e�Φ��T�̡۪A���w���T�ۭt�C                                 */
  /* �մѨS���T���I�A���s�Ϊ̤T�T�]���i�H�ӡC                             */

#if 0
  cnt[0] = fivCount(Color, row, col, -1, -1) + fivCount(Color, row, col, +1, +1) + 1;
  cnt[1] = fivCount(Color, row, col, -1, 0) + fivCount(Color, row, col, +1, 0) + 1;
  cnt[2] = fivCount(Color, row, col, 0, -1) + fivCount(Color, row, col, 0, +1) + 1;
  cnt[3] = fivCount(Color, row, col, -1, +1) + fivCount(Color, row, col, +1, -1) + 1;

  n3 = 0;			/* �����T */
  n4 = 0;			/* ���| */
  n5 = 0;			/* �� */
  nL = 0;			/* ���s */

  for (i = 0; i < 4; i++)
  {
    if (cnt[i] == (3 | (LIVE_SIDE + LIVE_SIDE)))
      n3++;
    if ((cnt[i] % LIVE_SIDE) == 4)
      n4++;
    if ((cnt[i] % LIVE_SIDE) == 5)
      n5++;
    if ((cnt[i] % LIVE_SIDE) > 5)
      nL++;
  }

  if (n5 > 0)
    winside = Color;
  else
  {
    if (Color == Black)
    {
      if (n3 >= 2)
      {
	bw_printmsg("�� �¤����T�T��");
	winside = White;
      }
      if (n4 >= 2)
      {
	bw_printmsg("�� �¤����|�T��");
	winside = White;
      }
      if (nL > 0)
      {
	bw_printmsg("�� �¤���s�T��");
	winside = White;
      }
    }
    else
    {
      if (nL > 0)
	winside = Color;
    }
  }
#endif

#if 0   /* Thor.990415: �W�����q�S�g���F, �d�ݦ��ߤH�h�A�C�C�g�a :p */

    (�@)��  ��  ��  ��
              ��
            �A��i�h�N��|�|
                ��
    (�G)������      ������(�����ŤT��)

    (�T)��  ������
          ����o�̤]��
              ��
                ��
                  ��

    (�|)����  ��    ����
                ���A��N�O�|�|


    ���s����,�u�n���l�H�W�N��,���ަ���

#endif

#if 1
  if (fivCount(Color, row, col, -1, -1) + fivCount(Color, row, col, +1, +1) >= 4 ||
    fivCount(Color, row, col, -1, 0) + fivCount(Color, row, col, +1, 0) >= 4 ||
    fivCount(Color, row, col, 0, -1) + fivCount(Color, row, col, 0, +1) >= 4 ||
    fivCount(Color, row, col, -1, +1) + fivCount(Color, row, col, +1, -1) >= 4)
    winside = Color;
#endif

  return winside;
}


static int
fivAllow()
{
  int i, j, num = 0;
  for (i = 0; i < 19; i++)
  {
    for (j = 0; j < 19; j++)
      num += Allow[i][j] = (Board[i][j] == Empty);
  }
  return num;
}


static int (*fivRule[]) () =
{
  fivInit, fivUpdate, fivAllow
};


  /*-----------------------------------------------------*/
  /* ��� 19 x 19					 */
  /*-----------------------------------------------------*/

static int
blkInit()
{
  memset(Board, 0, sizeof(Board));
  numWhite = numBlack = 0;
  rulerRow = rulerCol = 18;
  return 0;
}


/* borrow Allow for traversal, and return region */
/* a recursive procedure, clear Allow before call it */
/* with row,col range check, return false if out */
static int
blkLive(Color, row, col)
  int Color, row, col;
{
  if (row < 0 || row >= 19 || col < 0 || col >= 19)
    return 0;
  if (Board[row][col] == Empty)
    return 1;
  if (Board[row][col] != Color)
    return 0;
  if (Allow[row][col])
    return 0;
  Allow[row][col] = 1;
  return blkLive(Color, row - 1, col) |
    blkLive(Color, row + 1, col) |
    blkLive(Color, row, col - 1) |
    blkLive(Color, row, col + 1);
}


static inline void
blkClear()
{
  int i, j;

  for (i = 0; i < 19; i++)
  {
    for (j = 0; j < 19; j++)
    {
      if (Allow[i][j])
	Board[i][j] = Empty;
    }
  }
}


static int
blkUpdate(Color, row, col)
  int Color, row, col;
{
  Board[row][col] = Color;

  memset(Allow, 0, sizeof(Allow));
  if (!blkLive(Deny - Color, row - 1, col))
    blkClear();

  memset(Allow, 0, sizeof(Allow));
  if (!blkLive(Deny - Color, row + 1, col))
    blkClear();

  memset(Allow, 0, sizeof(Allow));
  if (!blkLive(Deny - Color, row, col - 1))
    blkClear();

  memset(Allow, 0, sizeof(Allow));
  if (!blkLive(Deny - Color, row, col + 1))
    blkClear();

  /* check for suiside */
  memset(Allow, 0, sizeof(Allow));
  if (!blkLive(Color, row, col))
    blkClear();

  /* count numWhite & numBlack */
  countBWnum();

  return Empty;		/* Please check win side by your own */
}


static int (*blkRule[]) () =
{
  blkInit, blkUpdate, fivAllow	/* borrow fivAllow as blkAllow */
};


  /*-----------------------------------------------------*/
  /* board util						 */
  /*-----------------------------------------------------*/

#if 0				/* screen */

  [maple BWboard]
  xxxx vs yyyy
  ++++++++ talkline(you color, yellow) (40 chars)
  ++++++++ talkline(my color, cryn)
  ++++++++
  ++++++++
  ++++++++

  one line for simple help, press key to......
  one line for nth turn, myColor, num, pass < -youcolor, num, pass(35), input talk
  two line for write msg

#endif


static char *
bw_brdline(row)
  int row;
{
  static char buf[80] = "\033[30;43m";
  static char rTxtY[] = " A B C D E F G H I J K L M N O P Q R S";
  static char rTxtX[] = " 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9";
  char txt[3];
  char *ptr = buf + 8, *t;
  int i;

  for (i = 0; i < 19; i++)
  {
    t = bw_icon[Board[row][i]];
    if (t == bw_icon[Empty])
    {
      if (row == 0)
      {
	if (i == 0)
	  t = "��";
	else if (i >= 18 || Board[row][i + 1] == Deny)
	  t = "��";
	else
	  t = "��";
      }
      else if (row >= 18 || Board[row + 1][i] == Deny)
      {
	if (i == 0)
	  t = "��";
	else if (i >= 18 || Board[row][i + 1] == Deny)
	  t = "��";
	else
	  t = "��";
      }
      else
      {
	if (i == 0)
	  t = "��";
	else if (i >= 18 || Board[row][i + 1] == Deny)
	  t = "��";
      }
    }
    if (t != bw_icon[Black] && t != bw_icon[White])
    {
      if (row == rulerRow && i < rulerCol)
      {
	str_ncpy(txt, rTxtX + 2 * i, 3);
	t = txt;
      }
      else if (i == rulerCol && row < rulerRow)
      {
	str_ncpy(txt, rTxtY + 2 * row, 3);
	t = txt;
      }
    }
    strcpy(ptr, t);
    ptr += 2;
  }
  strcpy(ptr, "\033[m");
  return buf;
}


static char *
bw_coorstr(row, col)
  int row;
  int col;
{
  static char coor[10];
  sprintf(coor, "(%c,%d)", row + 'A', col + 1);
  return coor;
}


static void
bw_printmsg(msg)
  char *msg;
{
  int line;

  line = msgline;
  move(line, 0);
  outs(bw_brdline(line - 1));
  outs(msg);
  clrtoeol();
  if (++line == b_lines - 3)	/* stop line */
    line = 1;
  move(line, 0);
  outs(bw_brdline(line - 1));
  outs("��");
  clrtoeol();
  msgline = line;
}


static void
bw_draw()
{
  int i, myNum, yourNum;
  for (i = 0; i < 19; i++)
  {
    move(1 + i, 0);
    outs(bw_brdline(i));
  }
  myNum = (myColor == Black) ? numBlack : numWhite;
  yourNum = (myColor == Black) ? numWhite : numBlack;

  move(b_lines - 2, 0);
  prints("��%d�^ %s%d�l%d�� (%s) %s%d�l%d��",
    BMIN(myStep, yourStep) + 1, bw_icon[Deny - myColor], myNum, myPass,
    (mapTurn == myRound ? "��" : "��"), bw_icon[myColor], yourNum, yourPass);
  /* Thor.990219: �S�O�`�N, �b���]�C�����Y, �G�Τl��icon, ��n�P�쥻�ۤ� */
  /* nth turn,myColor,num,pass <- youcolor, num,pass */
}


static void
bw_init()
{
  yourPass = myPass = yourStep = myStep = 0;

  /* �¥մ�/���l��/��Ѭ��¤l���� */
  if (myColor == Black)
  {
    (*rule[Ballow]) ();
    mapTurn = myRound;
  }
  else
  {
    mapTurn = yourRound;
  }

  move(b_lines - 1, 0);
  outs(COLOR1 " �﫳�Ҧ� " COLOR2 " (Enter)���l (TAB)�����ѽL/��� (^P)���� (^C)���� (^D)���}          \033[m");

  bw_draw();
}


static inline void
bw_overgame()
{
  if (GameOver == Black)
    bw_printmsg("\033[1;32m�� �¤����\033[m");
  else if (GameOver == White)
    bw_printmsg("\033[1;32m�� �դ����\033[m");
  else if (GameOver == Deny)
    bw_printmsg("\033[1;32m�� ���襭��\033[m");

  play_add(myColor == GameOver);
}


#if 0	/* communication protocol */

  Ctrl('O'):

  enter BWboard mode, (pass to another)
  first hand specify rule set(pass Rule later)
  then start

  clear chess board, C.....\ 0
  talk line by line, T.....\ 0
  specify down pos, Dxxx \ 0, y = xxx / 19, x = xxx % 19
  pass one turn, P.....\ 0
  leave BWboard mode, Q.....\ 0

#endif


static inline int
bw_recv()
{
  static char buf[512];
  static int bufstart = 0;
  int cc, len;
  char *bptr, *str;
  char msg[80];
  int i;

  bptr = buf;
  cc = bufstart;
  len = sizeof(buf) - cc - 1;

  if ((len = recv(cfd, bptr + cc, len, 0)) <= 0)
    return DISCONNECT;

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
    str = bptr + 1;
    switch (*bptr)
    {
      /* clear chess board, C.....\0 */
    case 'C':
      do_init();
      break;

      /* talk line by line, T.....\0 */
    case 'T':
      sprintf(msg, "\033[1;33m��%s\033[m", str);
      bw_printmsg(msg);
      break;

      /* specify down pos, Dxxx\0 , y = xxx / 19, x = xxx % 19 */
    case 'D':
      yourStep++;
      /* get pos */
      i = atoi(str);
      sprintf(msg, "�� ��踨�l %s", bw_coorstr(i / 19, i % 19));
      /* update board */
      GameOver = (*rule[Bupdate]) (Deny - myColor, i / 19, i % 19);

      mapTurn = myRound;

      bw_draw();

      bw_printmsg(msg);

      if (GameOver)
      {
	bw_overgame();
	memset(Allow, 0, sizeof(Allow));
      }
      else
      {
	if ((*rule[Ballow]) () <= 0)
	  bw_printmsg("�� �z����L���F");
      }
      break;

      /* pass one turn, P.....\0 */
    case 'P':
      yourPass++;
      yourStep++;

      mapTurn = myRound;
      bw_draw();
      bw_printmsg("�� �������");
      if (GameOver)
      {
	memset(Allow, 0, sizeof(Allow));
      }
      else
      {
	if ((*rule[Ballow]) () <= 0)
	  bw_printmsg("�� �z����L���F");	/* Thor.990329: ending game? */
      }
      break;

      /* leave BWboard mode, Q.....\0 */
    case 'Q':
      return LEAVE;
    }

    cc -= ++len;
    if (cc <= 0)
    {
      bufstart = 0;
      break;
    }
    bptr += len;
  }

  return NOTHING;
}


static int
ftnCtrlC()
{
  if (!do_send("C"))
    return DISCONNECT;
  do_init();
  return NOTHING;
}


static int
ftnUP()
{
  if (bwRow)
    bwRow--;
  return NOTHING;
}


static int
ftnDOWN()
{
  if (bwRow < 18)
    if (Board[bwRow + 1][bwCol] != Deny)
      bwRow++;
  return NOTHING;
}


static int
ftnLEFT()
{
  if (bwCol)
    bwCol--;
  return NOTHING;
}


static int
ftnRIGHT()
{
  if (bwCol < 18)
    if (Board[bwRow][bwCol + 1] != Deny)
      bwCol++;
  return NOTHING;
}


static int
ftnPass()
{
  /* Thor.990220: for chat mode to enter ^P pass */
  if (mapTurn == myRound)
  {
    myPass++;
    myStep++;
    if (!do_send("P"))
      return DISCONNECT;
    mapTurn = yourRound;
    bw_draw();
    bw_printmsg("�� �ڤ�����");
  }
  return NOTHING;
}


static int
ftnEnter()
{
  char msg[80];
  char buf[20];

  if (!Allow[bwRow][bwCol])
    return NOTHING;

  sprintf(msg, "�� �ڤ踨�l %s", bw_coorstr(bwRow, bwCol));

  myStep++;
  sprintf(buf, "D%d", bwRow * 19 + bwCol);

  if (!do_send(buf))
    return DISCONNECT;

  /* update board */
  GameOver = (*rule[Bupdate]) (myColor, bwRow, bwCol);

  mapTurn = yourRound;

  bw_draw();

  bw_printmsg(msg);

  if (GameOver)
    bw_overgame();

  return NOTHING;
}


static KeyFunc yourRound[] =
{
  Ctrl('C'), ftnCtrlC,
  Ctrl('D'), fCtrlD,
  KEY_LEFT, ftnLEFT,
  KEY_RIGHT, ftnRIGHT,
  KEY_UP, ftnUP,
  KEY_DOWN, ftnDOWN,
  KEY_TAB, fTAB,
  0, fNoOp
};


static KeyFunc myRound[] =
{
  Ctrl('C'), ftnCtrlC,
  ' ', ftnEnter,
  '\n', ftnEnter,
  Ctrl('P'), ftnPass,
  Ctrl('D'), fCtrlD,
  KEY_LEFT, ftnLEFT,
  KEY_RIGHT, ftnRIGHT,
  KEY_UP, ftnUP,
  KEY_DOWN, ftnDOWN,
  KEY_TAB, fTAB,
  0, fNoOp
};


/*-------------------------------------------------------*/
/* target : Chinese Chess Board �x��/�t��		 */
/* create : 99/12/14					 */
/* update : 02/08/05					 */
/* author : weichung.bbs@bbs.ntit.edu.tw		 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


enum
{
  Cover = 1
};


extern KeyFunc myTurn[], yourTurn[];

static int sideline;
static int Totalch;		/* �[�t�� :p */
static int Focus;
static int youreat_index;
static int myeat_index;

static char MyEat[16], YourEat[16];
static char Appear[14];

static char *ch_icon[] = 
{
  "  ", "��", 		/* Empty, Cover */
  "��", "�K", "��", "��", "�X", "��", "�L", 
  "�N", "�h", "�H", "��", "��", "�]", "��"
};


  /*-----------------------------------------------------*/
  /* �x�� 10 x 9					 */
  /*-----------------------------------------------------*/

static int
armyInit()
{
  int i, j;

  for (i = 0; i < 10; i++)
  {
    for (j = 0; j < 9; j++)
      Board[i][j] = Empty;
  }

  Board[0][4] = 2;			/* �� */
  Board[0][3] = Board[0][5] = 3;	/* �K */
  Board[0][2] = Board[0][6] = 4;	/* �� */
  Board[0][1] = Board[0][7] = 6;	/* �X */
  Board[0][0] = Board[0][8] = 5;	/* �� */
  Board[2][1] = Board[2][7] = 7;	/* �� */
  Board[3][0] = Board[3][2] = Board[3][4] = Board[3][6] = Board[3][8] = 8;	/* �L */

  Board[9][4] = 9;			/* �N */
  Board[9][3] = Board[9][5] = 10;	/* �h */
  Board[9][2] = Board[9][6] = 11;	/* �H */
  Board[9][1] = Board[9][7] = 13;	/* �� */
  Board[9][0] = Board[9][8] = 12;	/* �� */
  Board[7][1] = Board[7][7] = 14;	/* �] */
  Board[6][0] = Board[6][2] = Board[6][4] = Board[6][6] = Board[6][8] = 15;	/* �� */

  memset(MyEat, Empty, sizeof(MyEat));
  memset(YourEat, Empty, sizeof(YourEat));

  sideline = 19;
  return 0;
}      


static int (*armyRule[]) () =
{
  armyInit
};


  /*-----------------------------------------------------*/
  /* �t�� 4 x 8						 */
  /*-----------------------------------------------------*/

static int
darkInit()
{
  int i, j;

  for (i = 0; i < 4; i++)
    for (j = 0; j < 8; j++)
      Board[i][j] = Cover;

  memset(Appear, Empty, sizeof(Appear));
  memset(MyEat, Empty, sizeof(MyEat));
  memset(YourEat, Empty, sizeof(YourEat));

  sideline = 9;
  return 0;
}      


static int (*darkRule[]) () =
{
  darkInit
};


  /*-----------------------------------------------------*/
  /* board util						 */
  /*-----------------------------------------------------*/


static void
ch_printmsg(type, msg)
  int type;
  char *msg;
{
  char buf[80];

  switch (type)
  {
  case 1:			/* for move uncover eat */
    move(msgline + 9, 37);
    outs(msg);
    clrtoeol();
    if (++msgline >= 11)
      msgline = 1;
    move(msgline + 9, 37);
    outs("��");
    clrtoeol();
    break;

  case 2:			/* for select */
    sprintf(buf, "\033[1;33m���z����F %s(%d, %c)\033[m", 
      ch_icon[Focus / 256], bwCol, bwRow + 'A');
    move(1, 37);
    outs(buf);
    clrtoeol();
    break;

  case 3:			/* for disable select */
    move(1, 37);
    clrtoeol();
    break;
  }
}


static void
ch_printeat()
{
  int i;
  /* my:4, your:7 */

  if (myeat_index)
  {
    for (i = 0; i < myeat_index; i++)
    {
      move(4, 37 + i * 2);
      outs(ch_icon[MyEat[i]]);
    }
  }

  if (youreat_index)
  {
    for (i = 0; i < youreat_index; i++)
    {
      move(7, 37 + i * 2);
      outs(ch_icon[YourEat[i]]);
    }
  }
}


static void
ch_overgame(win)
  int win;
{
  char buf[80];

  sprintf(buf, "%s����ӡI�Ы� Ctrl-C ����", win == myColor ? "��" : "��");
  ch_printmsg(1, buf);

  play_add(win == myColor);
}


static char *
ch_brdline(row)
  int row;
{
  char *t, *str, ch;
  static char buf[80];
  static char river[] = "�x��      �e          �~      �ɢx";
  static char side[] = " A B C D E F G H I J";
  int i;

  if (row > 8 && Choose)	/* �t�Ѫ� row �̦h�� 8 */
    return NULL;

  if (row == 9)			/* �x�Ѫ� row 9 �O ���e�~�� */
    return river;

  str = buf;

  /* �e��l�ή�l�W���Ѥl */

  for (i = 0; i < 17; i++)
  {
    if (!Choose && (row % 2 || i % 2))
      ch = Empty;
    else
      ch = Board[row / 2][i / 2];

    if (Choose || (ch == Empty))
    {
      if (row == 0)					/* �z�w�s�w�{ */
      {
	if (i == 0)
	  t = "�z";
	else if (i == 16)
	  t = "�{";
	else if (i % 2)
	  t = "�w";
	else
	  t = "�s";
      }
      else if (row == 18 || (row == 8 && Choose))	/* �|�w�r�w�} */
      {
	if (i == 0)
	  t = "�|";
	else if (i == 16)
	  t = "�}";
	else if (i % 2)
	  t = "�w";
	else
	  t = "�r";
      }
      else if (row % 2)					/* �x  �x  �x */
      {
	if (i % 2)
	  t = ch_icon[ch];
	else
	  t = "�x";
      }
      else						/* �u�w�q�w�t */
      {
	if (i == 0)
	  t = "�u";
	else if (i == 16)
	  t = "�t";
	else if (i % 2)
	  t = "�w";
	else
	  t = "�q";
      }
    }
    else
    {
      t = ch_icon[ch];
    }
    strcpy(str, t);		/* color */
    str += 2;
  }				/* end for loop */

  if ((Choose && (row % 2 == 1)) || (!Choose && (row % 2 == 0)))
  {
    i = row / 2;
    strncpy(str, side + i * 2, 2);
  }

  return buf;
}


static void
ch_draw()
{
  int i;

  for (i = 0; i < sideline; i++)
  {
    move(1 + i, 0);
    outs(ch_brdline(i));
  }
  move(3, 37);
  outs("���ڤ�ҦY���l");
  move(6, 37);
  outs("�����ҦY���l");
  move(9, 37);
  outs("====================================");

  move(sideline + 1, 0);
  if (Choose)
    outs("  ��  ��  ��  ��  ��  ��  ��  ��");
  else
    outs("��  ��  ��  ��  ��  ��  ��  ��  ��");

  move(b_lines - 2, 0);
  prints("�ڬO \033[47;%s�l\033[m [����%s]", 
    dark_choose ? (myColor == Black ? "30m��" : "31m��") : "30m��", 
    mapTurn == myTurn ? "�ڤF" : "���");
}


static void
ch_init()
{
  Totalch = youreat_index = myeat_index = Focus = 0;

  /* �t�Ѫ����l�C�⥼�w */
  dark_choose = (Choose == 1) ? 0 : 1;

  /* �x��/�t�Ѭ��¤l���� (�t��: �u�������l�C��|�b�Ĥ@��½�l�ɨM�w) */
  mapTurn = (myColor == Black) ? myTurn : yourTurn;

  move(b_lines - 1, 0);
  outs(COLOR1 " �﫳�Ҧ� " COLOR2 " (Enter)½�l/���� (TAB)�����ѽL/��� (^S)�{�� (^C)���� (^D)���}     \033[m");

  ch_draw();
}


static inline int
ch_recv()
{
  static char buf[256];
  char msg[80];
  int len, ch, row, col, ch2, row2, col2;

  len = sizeof(buf) + 1;
  if ((len = recv(cfd, buf, len, 0)) <= 0)
    return DISCONNECT;

  switch (*buf)
  {
    /* clear chess board, C.....\0 */
  case 'C':
    do_init();
    break;

  case 'D':
    ch = atoi(buf + 1);
    row = (ch / 16) % 16;
    col = ch % 16;
    ch = ch / 256;

    Board[row][col] = ch;
    Appear[ch - 2]++;
    Totalch++;
    mapTurn = myTurn;
    sprintf(msg, "\033[1;32m�����½�} %s(%d, %c)\033[m", ch_icon[ch], col, row + 'A');
    ch_printmsg(1, msg);
    ch_draw();
    break;

  case 'T':
    sprintf(msg, "\033[1;33m��%s\033[m", buf + 1);
    ch_printmsg(1, msg);
    break;

  case 'E':
    ch = atoi(strtok(buf + 1, ":"));
    row = (ch / 16) % 16;
    col = ch % 16;
    ch = ch / 256;
    ch2 = atoi(strtok(NULL, ":"));
    row2 = (ch2 / 16) % 16;
    col2 = ch2 % 16;
    ch2 = ch2 / 256;

    Board[row][col] = Empty;
    Board[row2][col2] = ch;
    YourEat[youreat_index++] = ch2;
    mapTurn = myTurn;
    sprintf(msg, "\033[1;32m����貾�� %s(%d, %c) �Y %s(%d, %c)\033[m",
      ch_icon[ch], col, row + 'A',
      ch_icon[ch2], col2, row2 + 'A');
    ch_printmsg(1, msg);
    ch_draw();
    ch_printeat();

    if (Choose)
    {
      if (youreat_index == 16)
	ch_overgame((myColor == Black) ? Red : Black);
    }
    else
    {
      if (ch2 == 2 || ch2 == 9)
	ch_overgame((myColor == Black) ? Red : Black);
    }
    break;

  case 'M':
    ch = atoi(strtok(buf + 1, ":"));
    row = (ch / 16) % 16;
    col = ch % 16;
    ch = ch / 256;
    ch2 = atoi(strtok(NULL, ":"));
    row2 = (ch2 / 16) % 16;
    col2 = ch2 % 16;
    ch2 = ch2 / 256;

    Board[row][col] = Empty;
    Board[row2][col2] = ch;
    mapTurn = myTurn;
    sprintf(msg, "\033[1;37m����貾�� %s(%d, %c) �� (%d, %c)\033[m",
      ch_icon[ch], col, row + 'A', col2, row2 + 'A');
    ch_printmsg(1, msg);
    ch_draw();
    break;

  case 'F':
    dark_choose = 1;
    myColor = (atoi(buf + 1) == Black) ? Red : Black;
    ch_draw();
    break;

  case 'S':
    ch_overgame(myColor);
    break;

  case 'Q':
    return LEAVE;
  }

  return NOTHING;
}


static int 
ch_rand()
{
  int rd, i;
  char *index[] = {"1", "2", "2", "2", "2", "2", "5", "1", "2", "2", "2", "2", "2", "5"};

  if (Totalch == 31)		/* �קK�ѳ̫�@�Ӯ��٭n random */
  {
    for (i = 0; i < 14; i++)
    {
      if (Appear[i] < atoi(index[i]))
      {
	i += 2;
	return i;
      }
    }
  }

  for (;;)
  {
    rd = rnd(16);

    if (rd < 2)
      continue;
    if (Appear[rd - 2] < atoi(index[rd - 2]))
    {
      Appear[rd - 2] += 1;
      Totalch++;
      break;
    }
    else
      continue;
  }
  return rd;
}


static int
ch_count(row, col)	/* �^�ǥ]/���P�ݦY���������X�@�l */
  int row, col;
{
  int count, start, end;

  if (bwRow != row && bwCol != col)	/* �����b�P�@��ΦP�@�C�� */
    return -1;

  count = 0;
  if (bwRow != row)
  {
    if (bwRow > row)
    {
      start = row + 1;
      end = bwRow;
    }
    else
    {
      start = bwRow + 1;
      end = row;
    }
    for (; start < end; start++)
    {
      if (Board[start][bwCol] != Empty)
	count++;
    }
  }
  else
  {
    if (bwCol > col)
    {
      start = col + 1;
      end = bwCol;
    }
    else
    {
      start = bwCol + 1;
      end = col;
    }
    for (; start < end; start++)
    {
      if (Board[bwRow][start] != Empty)
	count++;
    }
  }

  return count;
}


static int		/* 0:�ۦP�C�� 1:���P�C�� */
ch_check(ch)
  char ch;
{
  if ((myColor == Red && ch < 9) ||
    (myColor == Black && ch > 8))
    return 0;

  return 1;
}


static int
ch_Mv0()			/* for �x�� */
{
  int mych, yourch, way;	/* way: 0:NOTHING  1:move  2:eat */
  int row, col, Rdis, Cdis;
  char buf[80];

  row = (Focus / 16) % 16;
  col = Focus % 16;
  mych = Focus / 256;
  yourch = Board[bwRow][bwCol];

  Rdis = abs(row - bwRow);	/* displayment */
  Cdis = abs(col - bwCol);
  way = NOTHING;

  switch (mych)
  {
  case 2:	/* �ӱN */
  case 9:
    if (((bwCol >= 3 && bwCol <= 5) && (bwRow <= 2 || bwRow >= 7) && (Rdis + Cdis == 1)) ||	/* ����b�E�c�椤�A���@�� */
      (bwCol == col && abs(mych - yourch) == 7 && !ch_count(row, col)))				/* ������ */
    {
      if (yourch == Empty)
	way = 1;
      else if (ch_check(yourch))
	way = 2;
    }
    break;

  case 3:	/* �K�h */
  case 10:
    if ((bwCol >= 3 && bwCol <= 5) && (bwRow <= 2 || bwRow >= 7) &&	/* ����b�E�c�椤 */
      (Rdis == 1 && Cdis == 1))						/* ���@�� */
    {
      if (yourch == Empty)
	way = 1;
      else if (ch_check(yourch))
	way = 2;
    }
    break;

  case 4:	/* �۶H */
  case 11:
    if (((bwRow <= 4 && myColor == Red) || (bwRow >= 5 && myColor == Black)) &&	/* �����L�e */
      (Rdis == 2 && Cdis == 2) &&					/* ���@�� */
      (Board[(bwRow + row) / 2][(bwCol + col) / 2] == Empty))		/* �����H�} */
    {
      if (yourch == Empty)
	way = 1;
      else if (ch_check(yourch))
	way = 2;
    }
    break;

  case 5:	/* �Ϩ� */
  case 12:
    if (!ch_count(row, col) && (row == bwRow || col == bwCol))	/* ���@�u */
    {
      if (yourch == Empty)
	way = 1;
      else if (ch_check(yourch))
	way = 2;
    }
    break;

  case 6:	/* �X�� */
  case 13:
    if ((Rdis == 2 && Cdis == 1 && Board[(bwRow + row) / 2][col] == Empty) ||	/* ���@�� */
      (Rdis == 1 && Cdis == 2 && Board[row][(bwCol + col) / 2] == Empty))	/* ����䰨�} */
    {
      if (yourch == Empty)
	way = 1;
      else if (ch_check(yourch))
	way = 2;
    }
    break;

  case 7:	/* ���] */
  case 14:
    if (row == bwRow || col == bwCol)				/* ���@�u */
    {
      Rdis = ch_count(row, col);	/* �ɥ� Rdis */
      if (Rdis == 0 && yourch == Empty)
	way = 1;
      else if (Rdis == 1 && yourch != Empty && ch_check(yourch))
	way = 2;
    }
    break;

  case 8:	/* �L�� */
  case 15:
    if (Rdis + Cdis != 1)	/* ���@�� */
      break;

    if (myColor == Red)
    {
      if ((bwRow < row) || 		/* ���ਫ�^�Y�B */
	(row <= 4 && col != bwCol))	/* �b�ꤺ�u�ਫ���� */
	break;
    }
    else
    {
      if ((bwRow > row) ||		/* ���ਫ�^�Y�B */
	(row >= 5 && col != bwCol))	/* �b�ꤺ�u�ਫ���� */
	break;
    }

    if (yourch == Empty)
      way = 1;
    else if (ch_check(yourch))
      way = 2;
    break;
  }		/* end switch */

  if (way == 1)
  {
    sprintf(buf, "M%d:%d", Focus, bwRow * 16 + bwCol);
    if (!do_send(buf))
      return DISCONNECT;
    sprintf(buf, "\033[1;36m���ڤ貾�� %s(%d, %c) �� (%d, %c)\033[m",
      ch_icon[mych], col, row + 'A', bwCol, bwRow + 'A');
  }
  else if (way == 2)
  {
    sprintf(buf, "E%d:%d", Focus, yourch * 256 + bwRow * 16 + bwCol);
    if (!do_send(buf))
      return DISCONNECT;
    sprintf(buf, "\033[1;32m���ڤ貾�� %s(%d, %c) �Y %s(%d, %c)\033[m",
      ch_icon[mych], col, col + 'A', ch_icon[yourch], bwCol, bwRow + 'A');
    MyEat[myeat_index++] = yourch;    
    ch_printeat();
  }

  if (way)
  {
    Board[bwRow][bwCol] = mych;
    Board[row][col] = Empty;
    mapTurn = yourTurn;
    Focus = Empty;
    ch_printmsg(1, buf);
    ch_draw();

    if (yourch == 2 || yourch == 9)
      ch_overgame(myColor);
  }

  return NOTHING;
}


static int
ch_Mv1()			/* for �t�� */
{
  int row, col;
  char mych, yourch;
  char buf[80];

  row = (Focus / 16) % 16;
  col = Focus % 16;
  mych = Focus / 256;
  yourch = Board[bwRow][bwCol];

  if (yourch == Empty)	/* ���i�Ŧa */
  {
    if (abs(bwRow - row) + abs(bwCol - col) != 1)	/* �n�b�j���~�ಾ�L�h */
      return NOTHING;

    sprintf(buf, "M%d:%d", Focus, bwRow * 16 + bwCol);
    if (!do_send(buf))
      return DISCONNECT;
    sprintf(buf, "\033[1;36m���ڤ貾�� %s(%d, %c) �� (%d, %c)\033[m",
      ch_icon[mych], col, row + 'A', bwCol, bwRow + 'A');
  }
  else			/* ���i���l���a */
  {
    if (!ch_check(yourch))		/* ���P��~�i�H�Y */
      return NOTHING;

    if (mych == 7 || mych == 14)	/* �]/���θ����~��Y */
    {
      if (ch_count(row, col) != 1)
	return NOTHING;
    }
    else
    {
      if (abs(bwRow - row) + abs(bwCol - col) != 1)	/* �@��l�n�b�j���~��Y�L�h */
	return NOTHING;

      if (myColor == Black)
      {
	if (mych != 15 || yourch != 2)		/* ��i�H�Y�� */
	{
	  if (mych - 7 > yourch)		/* �p����Y�j */
	    return NOTHING;
	  if (mych == 9 && yourch == 8)	/* �N����Y�L */
	    return NOTHING;
	}
      }
      else
      {
	if (mych != 8 || yourch != 9)		/* �L�i�H�Y�N */
	{
	  if (mych + 7 > yourch)		/* �p����Y�j */
	    return NOTHING;
	  if (mych == 2 && yourch == 15)	/* �Ӥ���Y�L */
	    return NOTHING;
	}
      }
    }

    sprintf(buf, "E%d:%d", Focus, yourch * 256 + bwRow * 16 + bwCol);
    if (!do_send(buf))
      return DISCONNECT;
    MyEat[myeat_index++] = yourch;
    sprintf(buf, "\033[1;32m���ڤ貾�� %s(%d, %c) �Y %s(%d, %c)\033[m",
      ch_icon[mych], col, row + 'A', ch_icon[yourch], bwCol, bwRow + 'A');
    ch_printeat();
  }

  Board[bwRow][bwCol] = mych;
  Board[row][col] = Empty;
  mapTurn = yourTurn;
  Focus = Empty;

  ch_printmsg(1, buf);
  ch_draw();

  if (myeat_index == 16)
    ch_overgame(myColor);

  return NOTHING;
}


static int
chCtrlS()
{
  if (!do_send("S"))
    return DISCONNECT;
  ch_overgame((myColor == Black) ? Red : Black);
  return NOTHING;
}


static int
chLEFT()
{
  if (bwCol > 0)
    bwCol--;
  return NOTHING;
}


static int
chRIGHT()
{
  if (bwCol < 8 - Choose)
    bwCol++;
  return NOTHING;
}


static int
chUP()
{
  if (bwRow > 0)
    bwRow--;
  return NOTHING;
}


static int
chDOWN()
{
  if (bwRow < 9 - Choose * 6)
    bwRow++;
  return NOTHING;
}


static int
chEnter()
{
  char buf[40], ch;

  ch = Board[bwRow][bwCol];
  if (ch == Cover)	/* �t��½�l */
  {
    ch = ch_rand();
    Board[bwRow][bwCol] = ch;
    sprintf(buf, "D%d", ch * 256 + bwRow * 16 + bwCol);
    if (!do_send(buf))
      return DISCONNECT;

    if (!dark_choose)	/* �t�ѲĤ@��½�l�M�w�C�� */
    {
      dark_choose = 1;
      myColor = (ch < 9) ? Red : Black;
      sprintf(buf, "F%d", myColor);
      if (!do_send(buf))
	return DISCONNECT;
    }

    mapTurn = yourTurn;
    Focus = Empty;
    sprintf(buf, "\033[1;32m���ڤ�½�} %s(%d, %c)\033[m",
      ch_icon[ch], bwCol, bwRow + 'A');
    ch_printmsg(1, buf);
    ch_draw();
  }
  else
  {
    if (Focus)		/* �x/�t�Ѳ��� */
    {
      if (Focus == ch * 256 + bwRow * 16 + bwCol)	/* �A��@�M��ܨ������ */
      {
	Focus = 0;
	ch_printmsg(3, NULL);
      }
      else
	return Choose ? ch_Mv1() : ch_Mv0();
    }
    else		/* �x/�t�ѿ�� */
    {
      if (ch != Empty && !ch_check(ch))
      {
	Focus = ch * 256 + bwRow * 16 + bwCol;
	ch_printmsg(2, NULL);
      }
    }
  }
  return NOTHING;
}


static KeyFunc yourTurn[] =
{
  Ctrl('C'), ftnCtrlC,
  Ctrl('D'), fCtrlD,
  KEY_LEFT, chLEFT,
  KEY_RIGHT, chRIGHT,
  KEY_UP, chUP,
  KEY_DOWN, chDOWN,
  KEY_TAB, fTAB,
  0, fNoOp
};


static KeyFunc myTurn[] =
{
  Ctrl('C'), ftnCtrlC,
  Ctrl('D'), fCtrlD,
  Ctrl('S'), chCtrlS,
  ' ', chEnter,
  '\n', chEnter,
  KEY_LEFT, chLEFT,
  KEY_RIGHT, chRIGHT,
  KEY_UP, chUP,
  KEY_DOWN, chDOWN,
  KEY_TAB, fTAB,
  0, fNoOp
};


/*-------------------------------------------------------*/
/* �Ҧ��ѽL���ΥD�{��					 */
/*-------------------------------------------------------*/


/* rule set */
static int (**ruleSet[]) () =
{
  othRule, fivRule, blkRule, armyRule, darkRule
};


static void
do_init()
{
  char *t, *mateid, msg[160], buf[80];
  int i, myTotal, yourTotal, myWin, yourWin;

  /* Initialize state */
  (*rule[Binit]) ();
  mapTalk = NULL;	/* �@�}�l�i�J�O�b�ѽL�W */

  bwRow = bwCol = 0;
  msgline = 1;

  cmdCol = 0;
  *cmdBuf = 0;
  cmdPos = -1;

  /* Initialize screen */
  clear();

  t = cuser.userid;
  mateid = cutmp->mateid;

  /* ���o���Z */
  play_count(t, &myTotal, &myWin);
  play_count(mateid, &yourTotal, &yourWin);

  sprintf(buf, "��%s(%d��%d��) vs ��%s(%d��%d��) \033[m", 
    t, myTotal, myWin, mateid, yourTotal, yourWin);

  sprintf(msg, "\033[1;33;44m�i �﫳%s �j", ruleStr);
  i = 80 - strlen(buf) + 3 - strlen(msg) + 10;
  t = str_tail(msg);
  for (; i; i--)
    *t++ = ' ';
  strcpy(t, buf);
  outs(msg);

  (Choose < 0) ? bw_init() : ch_init();
}


static int
main_board(sock, later)
  int sock, later;
{
  screenline sl[T_LINES];
  char c;
  int ch;
  KeyFunc *k;

  vs_save(sl);
  cfd = sock;

  if (!later)
  {
    /* ask for which rule set */
    /* assume: peer won't send char until setup */
    c = vans("�Q�U���ش� (1)�¥մ� (2)���l�� (3)��� (4)�x�� (5)�t�� (Q)�����H[Q] ");
    if (c >= '1' && c <= '5')
    {
      c -= '1';
    }
    else
    {
      c = -1;
      vs_restore(sl);	/* lkchu.990428: �� foot restore �^�� */
    }

    /* transmit rule set */
    if (send(cfd, &c, 1, 0) != 1)
      return DISCONNECT;

    /* �ҰʹC���̬��¤l */
    myColor = Black;
  }
  else
  {
    /* prompt for waiting rule set */
    outz("�� ���n�D�i�J�﫳�Ҧ���ܤ��A�еy�� \033[5m...\033[m");
    refresh();
    /* receive rule set */
    if (recv(cfd, &c, 1, 0) != 1)
      return DISCONNECT;

    vs_restore(sl);		/* lkchu.990428: �� foot restore �^�� */

    /* �Q�ҰʹC���̬���(��)�l */
    myColor = White;		/* White == Red */
  }

  if (c < 0)
    return LEAVE;
  rule = ruleSet[c];
  ruleStr = ruleStrSet[c];

  Choose = c - 3;		/* -3:�¥մ� -2:���l�� -1:��� 0:�x�� 1:�t�� */

  /* initialize all */
  do_init();

  for (;;)
  {
    if (mapTalk)
    {
      move(b_lines - 2, cmdCol + 35);
      k = mapTalk;
    }
    else
    {
      if (Choose < 0)
	move(bwRow + 1, bwCol * 2 + 1);
      else if (Choose == 0)
	move(bwRow * 2 + 1, bwCol * 4 + 1);
      else
	move(bwRow * 2 + 2, bwCol * 4 + 3);

      k = mapTurn;
    }

    ch = vkey();
    if (ch == I_OTHERDATA)
    {				/* incoming */
      ch = (Choose < 0) ? bw_recv() : ch_recv();
      if (ch >= NOTHING)	/* -1 for exit bwboard, -2 for exit talk */
	continue;
      vs_restore(sl);
      return ch;
    }

#ifdef EVERY_Z
    /* Thor: Chat ���� ctrl-z */
    else if (ch == Ctrl('Z'))
    {
      char buf[IDLEN + 1];
      screenline slt[T_LINES];

      /* Thor.980731: �Ȧs mateid, �]���X�h�ɥi��|�α� mateid */
      strcpy(buf, cutmp->mateid);

      holdon_fd = vio_fd;	/* Thor.980727: �Ȧs vio_fd */
      vio_fd = 0;
      vs_save(slt);
      every_Z(0);
      vs_restore(slt);
      vio_fd = holdon_fd;	/* Thor.980727: �٭� vio_fd */
      holdon_fd = 0;

      /* Thor.980731: �٭� mateid, �]���X�h�ɥi��|�α� mateid */
      strcpy(cutmp->mateid, buf);
      continue;
    }
#endif

    for (;; k++)
    {
      if (!k->key || ch == k->key)
	break;
    }

    /* -1 for exit bwboard, -2 for exit talk */
    if ((ch = k->key ? (*k->func) () : (*k->func) (ch)) >= NOTHING)
      continue;
    vs_restore(sl);
    return ch;
  }
}

#include <stdarg.h>

int
vaBWboard(pvar)
  va_list pvar;
{
  int sock, later;
  sock = va_arg(pvar, int);
  later = va_arg(pvar, int);
  return main_board(sock, later);
}
#endif		/* HAVE_GAME */
