/*-------------------------------------------------------*/
/* pushbox.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : �ܮw�f					 */
/* create : 98/11/11					 */
/* update : 02/09/05					 */
/* author : period.bbs@smth.org				 */
/*          cityhunter.bbs@smth.org			 */
/* modify : zhch.bbs@bbs.nju.edu.cn			 */
/*          hightman.bbs@bbs.hightman.net		 */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#define MAX_MAP_WIDTH		20
#define MAX_MAP_HEIGHT		16
#define MAX_MOVE_TIMES		1000	/* �̦h���ʴX�B */

#define move2(x, y)		move(x+6, y*2)
#define move3(x, y)		move(x+6, y*2+1)	/* �קK���ΰ��� */

static int cx, cy;		/* �ثe�Ҧb��m */
static int stage;		/* �ĴX�� */
static int NUM_TABLE;		/* �`�@���X�� */
static int total_line;		/* �������X�C */
static usint map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH];


static char *
map_char(n)
  usint n;
{
  if (n & 8)
    return "��";
  if (n & 4)
    return "��";
  if (n & 2)
    return "��";
  if (n & 1)
    return "�O";
  return "  ";
}


static usint
map_item(item)
  char *item;
{
  if (!str_ncmp(item, map_char(1), 2))
    return 1;
  if (!str_ncmp(item, map_char(2), 2))
    return 2;
  if (!str_ncmp(item, map_char(4), 2))
    return 4;
  if (!str_ncmp(item, map_char(8), 2))
    return 8;
  return 0;
}


static int	/* 0:����  !=0:�@���X�C */
map_init(fp)
  FILE *fp;
{
  int i, j;
  char buf[80], level[8];

  sprintf(level, "--%03d", stage);
  while (fgets(buf, 10, fp))
  {
    if (!str_ncmp(buf, level, 5))	/* ��쥿�T�����d */
      break;
  }

  memset(map, 0, sizeof(map));

  i = 0;
  while (fgets(buf, 80, fp))
  {
    /* ���J�a�� */
    if (buf[0] == '-')
      break;

    for (j = 0; j < MAX_MAP_WIDTH; j++)
      map[i][j] = map_item(buf + j * 2);

    if (++i >= MAX_MAP_HEIGHT)
      break;

    memset(buf, 0, sizeof(buf));
  }

  return i;
}


static int
map_line(x)	/* �L�X�� x �C */
  int x;
{
  int j;
  usint n;

  move2(x, 0);
  clrtoeol();
  for (j = 0; j < MAX_MAP_WIDTH; j++)
  {
    n = map[x][j];
    if (n & 5)		/* �[�j�C��ϲM�� */
      prints("\033[1;32m%s\033[m", map_char(n));
    else
      outs(map_char(n));
  }
}


static void
map_show()
{
  int i;

  for (i = 0; i < total_line; i++)
    map_line(i);

  move3(cx, cy);
}


static void
map_move(x0, y0, x1, y1)	/* (x0, y0) -> (x1, y1) */
  int x0, y0, x1, y1;
{
  usint m;

  m = map[x0][y0];

  map[x1][y1] = (m & 6) | (map[x1][y1] & 1);	/* �ت���[�J '��' �� '��' */
  map[x0][y0] = m & 1;				/* �Ҧb��M�� '��' �� '��' */

  map_line(x0);
  if (x1 != x0)
    map_line(x1);
}


static int	/* 1:���\ */
check_win()
{
  int i, j;
  for (i = 0; i < total_line; i++)
  {
    for (j = 0; j < MAX_MAP_WIDTH; j++)
    {
      if ((map[i][j] & 1) && !(map[i][j] & 4))	/* �٦� '�O' �W�S�� '��' */
	return 0;
    }
  }
  return 1;
}


static int
find_cxy()		/* ���l��m */
{
  int i, j;
  for (i = 0; i < total_line; i++)
  {
    for (j = 0; j < MAX_MAP_WIDTH; j++)
    {
      if (map[i][j] & 2)
      {
	cx = i;
	cy = j;
	return 1;
      }
    }
  }
  return 0;
}


static int
select_stage()
{
  int count;
  char buf[80], ans[4];
  FILE *fp;

  if (!(fp = fopen("etc/game/pushbox.map", "r")))
    return 0;

  if (stage < 0)	/* �Ĥ@���i�J�C�� */
  {
    fgets(buf, 4, fp);
    NUM_TABLE = atoi(buf);	/* etc/game/pushbox.map �Ĥ@��O�����d�� */
    sprintf(buf, "�п�ܽs�� [1-%d]�A[0] �H���X�D�A�Ϋ� [Q] ���}�G", NUM_TABLE);
    if (vget(b_lines, 0, buf, ans, 4, DOECHO) == 'q')
    {
      fclose(fp);
      return 0;
    }
    stage = atoi(ans);
    if (stage <= 0 || stage > NUM_TABLE)	/* �H���X�D */
      stage = 1 + time(0) % NUM_TABLE;
  }

  count = map_init(fp);
  fclose(fp);

  return count;
}


int
main_pushbox()
{
  int dx, dy;		/* �U�@�B�Ҧ��� */
  int valid;
  usint n;

  stage = -1;

start_game:

  if (!(total_line = select_stage()))
    return XEASY;

  vs_bar("�ܮw�f");
  move(2, 0);
  prints("�� \033[1;32m%03d\033[m ���G��Ҧ��� '��' ������ '�O' �W���h(�|�ܦ�\033[1;32m���\033[m)�N�L���F\n", stage);
  outs("���仡���G(��������)���� (s)���� (q)���} (^L)�ù���ø");

  if (!find_cxy())
  {
    vmsg("�o�i�a�Ϧ��G����l�I");
    return 0;
  }

  map_show();

  while (1)
  {
    dx = dy = 0;

    switch (vkey())
    {
    case KEY_UP:
      dx = -1;
      break;

    case KEY_DOWN:
      dx = 1;
      break;

    case KEY_LEFT:
      dy = -1;
      break;

    case KEY_RIGHT:
      dy = 1;
      break;

    case Ctrl('L'):
      map_show();
      break;

    case 'q':
      goto game_over;

    case 's':
      goto start_game;
    }

    if (!dx && !dy)
      continue;

    /* �}�l���� */
    valid = 0;
    n = map[cx + dx][cy + dy];	/* �ت��� */

    if (n <= 1)		/* �ت���O�Ū��A�������J�Y�i */
    {
      map_move(cx, cy, cx + dx, cy + dy);
      valid = 1;
    }
    else if (n & 4)	/* �ت��榳 '��'�A���h�U�U�� */
    {
      if (map[cx + dx * 2][cy + dy * 2] <= 1)	/* �U�U��O�Ū� */
      {
	map_move(cx + dx, cy + dy, cx + dx * 2, cy + dy * 2);
	map_move(cx, cy, cx + dx, cy + dy);
	valid = 1;
      }
    }

    if (valid)	/* ���Ĳ��� */
    {
      if (check_win())
	break;

      cx += dx;
      cy += dy;
      move3(cx, cy);
    }
  }

  vmsg("���P�z�I���\\�L��");

  if (++stage > NUM_TABLE)
    stage = 1;

  goto start_game;

game_over:
  return 0;
}
#endif	/* HAVE_GAME */
