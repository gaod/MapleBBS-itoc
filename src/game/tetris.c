/*-------------------------------------------------------*/
/* tetris.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : �Xù�����					 */
/* create :   /  /  					 */
/* update : 02/10/15					 */
/* author : zhch.bbs@bbs.nju.edu.cn			 */
/* modify : hightman.bbs@bbs.hightman.net		 */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#define MAX_MAP_WIDTH	10	/* �ѽL���e�צ@ 10 �� */
#define MAX_MAP_HEIGHT	20	/* �ѽL�����צ@ 20 �C */
#define MAX_STYLE	7	/* �`�@�� 7 ����������� */


/* �C�ؤ�����O 4*4 ���A�M��i�H�� 4 �Ӥ�V */

static int style_x[MAX_STYLE][4][4] =
{
  0, 1, 0, 1,    0, 1, 0, 1,   0, 1, 0, 1,   0, 1, 0, 1,
  0, 1, 2, 3,    1, 1, 1, 1,   0, 1, 2, 3,   1, 1, 1, 1,
  0, 1, 1, 2,    1, 0, 1, 0,   0, 1, 1, 2,   1, 0, 1, 0,
  1, 2, 0, 1,    0, 0, 1, 1,   1, 2, 0, 1,   0, 0, 1, 1,
  0, 1, 2, 0,    0, 1, 1, 1,   2, 0, 1, 2,   0, 0, 0, 1,
  0, 1, 2, 2,    1, 1, 0, 1,   0, 0, 1, 2,   0, 1, 0, 0,
  0, 1, 2, 1,    1, 0, 1, 1,   1, 0, 1, 2,   0, 0, 1, 0
};

static int style_y[MAX_STYLE][4][4] =
{
  0, 0, 1, 1,    0, 0, 1, 1,   0, 0, 1, 1,   0, 0, 1, 1,
  1, 1, 1, 1,    0, 1, 2, 3,   1, 1, 1, 1,   0, 1, 2, 3,
  0, 0, 1, 1,    0, 1, 1, 2,   0, 0, 1, 1,   0, 1, 1, 2,
  0, 0, 1, 1,    0, 1, 1, 2,   0, 0, 1, 1,   0, 1, 1, 2,
  0, 0, 0, 1,    0, 0, 1, 2,   0, 1, 1, 1,   0, 1, 2, 2,
  0, 0, 0, 1,    0, 1, 2, 2,   0, 1, 1, 1,   0, 0, 1, 2,
  0, 0, 0, 1,    0, 1, 1, 2,   0, 1, 1, 1,   0, 1, 1, 2
};


static int map[MAX_MAP_HEIGHT + 1][MAX_MAP_WIDTH + 2];

static int my_lines;		/* �`���h���� */
static int my_scores;		/* �`�o�� */
static int level;		/* ���d */
static int delay;		/* ����۰ʤU�^���ɶ����Z (���:usec) */
static int cx, cy;		/* �ثe�Ҧb (x, y) */
static int style;		/* �ثe��������� */
static int dir;			/* �ثe�������V */
static int last_dir;		/* �W����V����V */


static void
move_map(x, y)
  int x, y;
{
  move(x + 2, 2 * y);
}


static void
tetris_welcome()
{
  move(4, 33);
  outs("���仡���G");
  move(5, 35);
  outs("������       ���ʤ�V");
  move(6, 35);
  outs("Space        �ֳt���U");
  move(7, 35);
  outs("��           180 �ױ���");
  move(8, 35);
  outs("k            ���ɰw����");
  move(9, 35);
  outs("j            �f�ɰw����");
  move(10, 35);
  outs("^S           �Ȱ��C��");
  move(11, 35);
  outs("q            ���}�C��");

  move(13, 35);
  outs("�Y��Ц����ʿ��~���{�H");
  move(14, 35);
  outs("�ȮɱN������V����������Y�i");

  move(16, 33);
  outs("�C���������ɤ@��");
  move(17, 33);
  outs("���@��o�����B�G�梲���B�T�梶���B�|�械����");

  vmsg(NULL);
}


static void
tetris_init()		/* initialize map[][] */
{
  int i, j, line;

  line = MAX_MAP_HEIGHT - level;	/* �̵��ŨӨM�w���X�榳�F�� */

  /* map[][] �̭����� 0:�ť�(  ) 1:����(�i) 2:���k��(�x) 3:����(�w) 4:��������(�|) 5:�k������(�}) */

  for (i = 0; i < MAX_MAP_HEIGHT; i++)
  {
    for (j = 1; j <= MAX_MAP_WIDTH; j++)
    {
      if (i >= line)
	map[i][j] = rnd(2);			/* ���� */
      else
	map[i][j] = 0;				/* �ť� */
    }
    map[i][0] = map[i][MAX_MAP_WIDTH + 1] = 2;	/* ���k�� */
  }

  for (j = 1; j <= MAX_MAP_WIDTH; j++)
    map[MAX_MAP_HEIGHT][j] = 3;			/* ���� */

  map[MAX_MAP_HEIGHT][0] = 4;			/* �������� */
  map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH + 1] = 5;	/* �������� */
}


static void
tetris_mapshow()
{
  int i, j;
  char piece[6][3] = {"  ", "�i", "�x", "�w", "�|", "�}"};

  /* map[][] �̭����� 0:�ť�(  ) 1:����(�i) 2:���k��(�x) 3:����(�w) 4:��������(�|) 5:�k������(�}) */

  for (i = 0; i <= MAX_MAP_HEIGHT; i++)
  {
    move_map(i, 0);
    for (j = 0; j <= MAX_MAP_WIDTH + 1; j++)
      outs(piece[map[i][j]]);
  }
}


static void
tetris_blineshow()
{
  move(b_lines, 0);
  clrtoeol();
  prints("���šG\033[1;32m%d\033[m  �`���h���ơG\033[1;32m%d\033[m  �`�o���G\033[1;32m%d\033[m",
    level, my_lines, my_scores);
}


static void
block_show(x, y, s, d, f)
  int x, y;		/* (x, y) */
  int s;		/* style */
  int d;		/* dir */
  int f;		/* 1:�[�W��� 0:������� */
{
  int n;

  if (d == -1)
    return;

  for (n = 0; n <= 3; n++)
  {
    move_map(x + style_x[s][d][n], y + style_y[s][d][n]);
    if (f)
      outs("�i");	/* piece[1] */
    else
      outs("  ");	/* piece[0] */
  }
  move(1, 0);
}


static void
block_move()
{
  static int last_x = -1;
  static int last_y = -1;
  static int last_style = -1;

  if (last_x == cx && last_y == cy && last_style == style && last_dir == dir)
    return;

  block_show(last_x, last_y, last_style, last_dir, 0);
  last_x = cx;
  last_y = cy;
  last_style = style;
  last_dir = dir;
  block_show(cx, cy, style, dir, 1);
}


static void
tune_delay()
{
  /* delay �n�b 1 ~ 999999 ���� */
  delay = 999999 / (level + 1);
}


static void
check_lines()		/* �ˬd�ݬO�_����h�@�� */
{
  int i, j, s;

  s = 1;
  for (i = 0; i < MAX_MAP_HEIGHT; i++)
  {
    for (j = 1; j <= MAX_MAP_WIDTH; j++)
    {
      if (map[i][j] == 0)	/* �ť� */
	break;
    }

    if (j == MAX_MAP_WIDTH + 1)
    {
      int n;

      s *= 2;
      /* ���h���W�������U�� */
      for (n = i; n > 0; n--)
      {
	for (j = 1; j <= MAX_MAP_WIDTH; j++)
	  map[n][j] = map[n - 1][j];
      }
      for (j = 1; j <= MAX_MAP_WIDTH; j++)
	map[0][j] = 0;

      if (++my_lines % 30 == 0)	/* �C�� 30 �����d�[�@ */
      {
	level++;
	tune_delay();
      }
    }
  }

  if (--s > 0)	/* �����h�ܤ֤@�� */
  {
    s = s * (10 + level) / 10;
    my_scores += s;
    tetris_mapshow();
    tetris_blineshow();
  }
}


static int	/* 1:�I���ê�� 0:�q��L�� */
crash(x, y, s, d)
  int x, y;	/* (x, y) */
  int s;	/* style */
  int d;	/* dir */
{
  int n;

  for (n = 0; n <= 3; n++)
  {
    if (map[x + style_x[s][d][n]][y + style_y[s][d][n]])	/* �w���� */
      return 1;
  }
  return 0;
}


static void
arrived()
{
  int n;

  for (n = 0; n <= 3; n++)
    map[cx + style_x[style][dir][n]][cy + style_y[style][dir][n]] = 1;	/* ���� */

  check_lines();
}


static int
getkey()
{
  int fd;
  struct timeval tv;

  fd = 1;
  tv.tv_sec = 0;
  tv.tv_usec = delay;

  /* �Y������A�^�ǩҫ�����F�Y delay ���ɶ���F���S������A�^�� 0 */

  if (select(1, (fd_set *) &fd, NULL, NULL, &tv) > 0)
    return vkey();

  return 0;
}


int
main_tetris()
{
  int ch;
  int next_style;

  vs_bar("�Xù�����");
  tetris_welcome();

start_game:

  level = vans("�q�ĴX�Ŷ}�l��(0-9)�H[0] ") - '0';
  if (level < 0 || level > 9)
    level = 0;

  tetris_init();
  tetris_mapshow();

  vmsg("�C���}�l");
  tetris_blineshow();

  style = 0;
  next_style = rnd(MAX_STYLE);
  my_lines = my_scores = 0;
  tune_delay();

  while (1)
  {
    style = next_style;
    next_style = rnd(MAX_STYLE);/* �üƨM�w�U�@�ӥX�Ӫ�������� */
    last_dir = -1;		/* �C���s����X�ӳ��n���]�� -1 */
    dir = 0;			/* ����@�X�ӬO�¤W�� */
    cx = 0;			/* ����@�X�Ӫ���m */
    cy = MAX_MAP_WIDTH / 2;

    /* ��W�@�Ӥw�X�Ӫ��M���A��U�@�ӭn�X�Ӫ��e�b�k�W�� */
    block_show(0, MAX_MAP_WIDTH + 2, style, dir, 0);
    block_show(0, MAX_MAP_WIDTH + 2, next_style, dir, 1);

    block_move();

    if (crash(cx, cy, style, dir))	/* �s����@�X�ӴN crash�Agame over */
      break;

    for (;;)
    {
      switch (ch = getkey())
      {
      case Ctrl('S'):
	vmsg("�C���Ȱ��A�����N���~��");
	tetris_blineshow();
	break;

      case KEY_LEFT:
	if (!crash(cx, cy - 1, style, dir))
	{
	  cy--;
	  block_move();
	}
	break;

      case KEY_RIGHT:
	if (!crash(cx, cy + 1, style, dir))
	{
	  cy++;
	  block_move();
	}
	break;

      case KEY_DOWN:
	if (!crash(cx + 1, cy, style, dir))
	{
	  cx++;
	  block_move();
	}
	else
	{
	  arrived();
	  ch = '#';
	}
	break;

      case KEY_UP:
	if (!crash(cx, cy, style, (dir + 2) % 4))
	{
	  dir = (dir + 2) % 4;
	  block_move();
	}
	break;

      case 'k':
	if (!crash(cx, cy, style, (dir + 1) % 4))
	{
	  dir = (dir + 1) % 4;
	  block_move();
	}
	break;

      case 'j':
	if (!crash(cx, cy, style, (dir + 3) % 4))
	{
	  dir = (dir + 3) % 4;
	  block_move();
	}
	break;

      case ' ':
	while (!crash(cx + 1, cy, style, dir))
	  cx++;
	block_move();
	arrived();
	ch = '#';
	break;

      case 0:		/* �H delay �ɶ��@��|�۰ʩ��U�� */
	if (!crash(cx + 1, cy, style, dir))
	{
	  cx++;
	  block_move();
	}
	else
	{
	  arrived();
	  ch = '#';
	  break;
	}
	break;
      }

      if (ch == 'q' || ch == '#')
	break;

      refresh();
    }		/* end of for (;;) */

    if (ch == 'q')
      break;
  }		/* end of while (1) */

  if (vans("���������I�z�٭n�~�򪱶�(Y/N)�H[N] ") == 'y')
    goto start_game;

  return 0;
}
#endif	/* HAVE_GAME */
