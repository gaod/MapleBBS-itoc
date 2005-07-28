/*-------------------------------------------------------*/
/* mine.c         ( NTHU CS MapleBBS Ver 3.10 )          */
/*-------------------------------------------------------*/
/* target : ��a�p�C��                                   */
/* create : 01/02/15                                     */
/* update : 01/03/01                                     */
/* author : piaip.bbs@sob.twbbs.org                      */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#define _CHINESE_	/* ����r symbol */

enum
{
  /* MINE_XPOS + MAP_MAX_X �n�p�� b_lines - 2 = 21    *
   * MINE_YPOS + MAP_MAX_Y * 2 �n�p�� STRLEN - 1 = 79 *
   * MINE_YPOS �n������ out_prompt() ��J             */

  MINE_XPOS = 0,
  MINE_YPOS = 17,
  MAP_MAX_X = 20,		/* �� x ��V */
  MAP_MAX_Y = 30,		/* �� y ��V */

  /* These are flags for bitwise operators */
  TILE_BLANK = 0,		/* �S���a�p */
  TILE_MINE = 1,		/* ���a�p */
  TILE_TAGGED = 0x10,		/* �Q�аO */
  TILE_EXPAND = 0x20		/* �w�Q�i�} */
};


static char MineMap[MAP_MAX_X + 2][MAP_MAX_Y + 2];	/* �a�ϤW�C�檺�ݩ� */
static char MineNei[MAP_MAX_X + 2][MAP_MAX_Y + 2];	/* �a�ϤW�C��F�~���h�֦a�p */

static int MAP_X, MAP_Y;	/* �ѽL�j�p */
static int cx, cy;		/* current (x, y) */
static int TotalMines;		/* �w�аO���a�p�� */
static int TaggedMines;		/* �w�аO���a�p�� */
static time_t InitTime;		/* �}�l�����ɶ� */
static int LoseGame;		/* 1: ��F     0: �٦b�� */
static int EndGame;		/* 1: ���}�C�� 0: �٦b�� */


#ifdef _CHINESE_
static char symTag[3] = "��";	/* �аO�a�p/���B���a�p�B���Q�Х� */
static char symMine[3] = "��";	/* /���B���a�p���S�Q�Х� */
static char symWrong[3] = "��";	/* /���B�S�a�p�����Q�Х� */
static char symBlank[3] = "��";	/* ���Q�i�}/���B�S�a�p�B�S�Q�i�} */
static char *strMines[9] = {"�@", "��", "��", "��", "��", "��", "��", "��", "��"};	/* ���䦳�X���a�p */
#else
static char symTag[3] = " M";
static char symMine[3] = " m";
static char symWrong[3] = " X";
static char symBlank[3] = " o";
static char *strMines[9] = {" _", " 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8"};
#endif


static inline int
count_neighbor(x, y, bitmask)
  int x, y, bitmask;
{
  return (((MineMap[x - 1][y - 1] & bitmask) + (MineMap[x - 1][y] & bitmask) +
    (MineMap[x - 1][y + 1] & bitmask) + (MineMap[x][y - 1] & bitmask) +
    (MineMap[x][y] & bitmask) + (MineMap[x][y + 1] & bitmask) +
    (MineMap[x + 1][y - 1] & bitmask) + (MineMap[x + 1][y] & bitmask) +
    (MineMap[x + 1][y + 1] & bitmask)) / bitmask);
}


static void
init_map()
{
  int x, y, i;

  /* �]�w���ĴѽL */

  for (x = 0; x < MAP_X + 2; x++)
  {
    for (y = 0; y < MAP_Y + 2; y++)
    {
      MineMap[x][y] = TILE_BLANK;
      if (x == 0 || y == 0 || x == MAP_X + 1 || y == MAP_Y + 1)
	MineMap[x][y] |= TILE_EXPAND;
    }
  }

  /* �]�w�a�p�Ҧb */

  for (i = 0; i < TotalMines;)
  {
    x = rnd(MAP_X) + 1;
    y = rnd(MAP_Y) + 1;

    if (MineMap[x][y] == TILE_BLANK)
    {
      MineMap[x][y] = TILE_MINE;
      i++;
    }
  }

  /* ��X�Ҧ��檺�F�~���h�֦a�p */

  for (x = 1; x <= MAP_X; x++)
  {
    for (y = 1; y <= MAP_Y; y++)
    {
      MineNei[x][y] = count_neighbor(x, y, TILE_MINE);
    }
  }

  /* �{�b�}�l�p�ɡA�]�� out_map() �� out_info() �n�Ψ� */
  InitTime = time(0);
}


static void
out_prompt()
{
  /* outs() �̭����ԭz���o�W�L MINE_YPOS�A�_�h�|���� */
  move(3, 0);
  outs("���仡���G");
  move(5, 0);
  outs("����     ��V��");
  move(7, 0);
  outs("½�}     �ť���");
  move(9, 0);
  outs("�аO�a�p   ��");
  move(11, 0);
  outs("���p       ��");
  move(13, 0);
  outs("���}       ��");
}


static inline void
out_song()
{
  uschar *msg[8] = 
  {
    "�D�F�y�L�H���ӡA�D���R�L�H�P��",			/* �Ѹ��G�|�l�� */
    "�@���@���A���o�Ӥ����F�b���b�\\�A������O���}",	/* ���l�P�a�樥 */
    "�y���B�Ӻ��[�A���{���ӱ���",			/* ���l�P�a�樥 */
    "�L�������A�ֲ�ּw",				/* �j�� */
    "�j�Ǥ��D�A�b�����w�A�b�˥��A�b���ܵ�",		/* �j�� */
    "�ѩR���שʡA�v�ʤ��w�D�A��D���ױ�",		/* ���e */
    "���w�H�������v�A�w�����H�]",			/* �׻y���Ǧ� */
    "�»D�D�A�i���i�o"					/* �׻y������ */
  };
  move(b_lines - 2, 0);
  prints("\033[1;3%dm%s\033[m", time(0) % 7, msg[time(0) % 8]);
  clrtoeol();
}


static void
out_info()
{
  move(b_lines - 1, 0);
  prints("�Ҫ�ɶ�: %.0lf ��A�ѤU %d �Ӧa�p���аO�C",
    difftime(time(0), InitTime), TotalMines - TaggedMines);
  clrtoeol();

  out_song();
}


static inline void
out_map()
{
  int x, y;

  vs_bar("�l�z��a�p");

  for (x = 1; x <= MAP_X; x++)
  {
    move(MINE_XPOS + x, MINE_YPOS + 2);
    for (y = 1; y <= MAP_Y; y++)
      outs(symBlank);
  }

  out_prompt();
  out_info();
  move(MINE_XPOS + cx, MINE_YPOS + cy * 2 + 1);	/* move to (0, 0) */
}


static void
draw_map()			/* �e�X���㵪�� */
{
  int x, y;

  vs_bar("�l�z��a�p");

  for (x = 1; x <= MAP_X; x++)
  {
    move(MINE_XPOS + x, MINE_YPOS + 2);
    for (y = 1; y <= MAP_Y; y++)
    {
      if (MineMap[x][y] & TILE_TAGGED)
      {
	if (!(MineMap[x][y] & TILE_MINE))
	  outs(symWrong);
	else
	  outs(symTag);
      }
      else if (MineMap[x][y] & TILE_EXPAND)
	outs(strMines[MineNei[x][y]]);
      else if (MineMap[x][y] & TILE_MINE)
	outs(symMine);
      else
	outs(symBlank);
    }
  }
}


static void
expand_map(x, y)
  int x, y;
{
  if (MineMap[x][y] & TILE_TAGGED || MineMap[x][y] & TILE_EXPAND)
    return;

  if (MineMap[x][y] & TILE_MINE && !(MineMap[x][y] & TILE_TAGGED))
  {
    LoseGame = 1;
    return;
  }

  MineMap[x][y] |= TILE_EXPAND;
  move(MINE_XPOS + x, MINE_YPOS + y * 2);
  outs(strMines[MineNei[x][y]]);

  if (MineNei[x][y] == 0)
  {
    if ((MineMap[x - 1][y] & TILE_EXPAND) == 0)
      expand_map(x - 1, y);
    if ((MineMap[x][y - 1] & TILE_EXPAND) == 0)
      expand_map(x, y - 1);
    if ((MineMap[x + 1][y] & TILE_EXPAND) == 0)
      expand_map(x + 1, y);
    if ((MineMap[x][y + 1] & TILE_EXPAND) == 0)
      expand_map(x, y + 1);
    if ((MineMap[x - 1][y - 1] & TILE_EXPAND) == 0)
      expand_map(x - 1, y - 1);
    if ((MineMap[x + 1][y - 1] & TILE_EXPAND) == 0)
      expand_map(x + 1, y - 1);
    if ((MineMap[x - 1][y + 1] & TILE_EXPAND) == 0)
      expand_map(x - 1, y + 1);
    if ((MineMap[x + 1][y + 1] & TILE_EXPAND) == 0)
      expand_map(x + 1, y + 1);
  }
}


static inline void
trace_map(x, y)
  int x, y;
{
  if (MineMap[x][y] & TILE_EXPAND &&
    MineNei[x][y] == count_neighbor(x, y, TILE_TAGGED))
  {
    expand_map(x - 1, y);
    expand_map(x, y - 1);
    expand_map(x + 1, y);
    expand_map(x, y + 1);
    expand_map(x - 1, y - 1);
    expand_map(x + 1, y - 1);
    expand_map(x - 1, y + 1);
    expand_map(x + 1, y + 1);
  }
}


static inline void
play_mine()
{
  int ch;

  LoseGame = 0;
  EndGame = 0;

  while (!LoseGame && (ch = vkey()))
  {
    switch (ch)
    {
    case KEY_ESC:
    case 'q':
      EndGame = 1;
      return;

      /* ������ʧ@���n���Ц^�_���Ӫ���m */

    case KEY_UP:
      if (cx > 1)
      {
	cx--;
	move(MINE_XPOS + cx, MINE_YPOS + cy * 2 + 1);
      }
      break;

    case KEY_DOWN:
      if (cx < MAP_X)
      {
	cx++;
	move(MINE_XPOS + cx, MINE_YPOS + cy * 2 + 1);
      }
      break;

    case KEY_LEFT:
      if (cy > 1)
      {
	cy--;
	move(MINE_XPOS + cx, MINE_YPOS + cy * 2 + 1);
      }
      break;

    case KEY_RIGHT:
      if (cy < MAP_Y)
      {
	cy++;
	move(MINE_XPOS + cx, MINE_YPOS + cy * 2 + 1);
      }
      break;

    case 'd':
    case '\n':
      trace_map(cx, cy);
      out_info();
      move(MINE_XPOS + cx, MINE_YPOS + cy * 2 + 1);
      break;

    case ' ':
      expand_map(cx, cy);
      out_info();
      move(MINE_XPOS + cx, MINE_YPOS + cy * 2 + 1);
      break;

    case 'f':
      if (MineMap[cx][cy] & TILE_EXPAND)
      {
	if (MineMap[cx][cy] & TILE_TAGGED)	/* ���ӳQ�аO, ���� */
	{
	  TaggedMines--;
	  MineMap[cx][cy] ^= TILE_EXPAND;
	  MineMap[cx][cy] ^= TILE_TAGGED;
	  move(MINE_XPOS + cx, MINE_YPOS + cy * 2);
	  outs(symBlank);
	}
      }
      else		/* ���ӨS�аO, �W�аO */
      {
	TaggedMines++;
	MineMap[cx][cy] ^= TILE_EXPAND;
	MineMap[cx][cy] ^= TILE_TAGGED;
	move(MINE_XPOS + cx, MINE_YPOS + cy * 2);
	outs(symTag);
	if (TaggedMines == TotalMines)
	  return;
      }
      out_info();
      move(MINE_XPOS + cx, MINE_YPOS + cy * 2 + 1);
      break;

    default:
      break;
    }
  }
}


static inline int
win()
{
  int x, y;

  for (x = 1; x <= MAP_X; x++)
  {
    for (y = 1; y <= MAP_Y; y++)
    {
      if (((MineMap[x][y] & TILE_TAGGED) && !(MineMap[x][y] & TILE_MINE)) ||
        (!(MineMap[x][y] & TILE_TAGGED) && (MineMap[x][y] & TILE_MINE)))
      {
        return 0;		/* �п��a�� */
      }
    }
  }
  return 1;
}


int
main_mine()
{
  int level;
  char ans[4];

  level = vans("�п�� [1-5] ���šA[0] �۩w�A�Ϋ� [Q] ���}�G");
  if (level == 'q')
  {
    return XEASY;
  }
  else if (level < '1' || level > '5')	/* �۩w�ѽL���o�j�� 60 * 20 */
  {
    vget(b_lines, 0, "�п�J�a�Ϫ����G", ans, 3, DOECHO);
    MAP_Y = atoi(ans) > MAP_MAX_Y ? MAP_MAX_Y : atoi(ans);

    vget(b_lines, 0, "�п�J�a�Ϫ��e�G", ans, 3, DOECHO);
    MAP_X = atoi(ans) > MAP_MAX_X ? MAP_MAX_X : atoi(ans);

    vget(b_lines, 0, "�п�J�a�p�ơG", ans, 3, DOECHO);
    level = atoi(ans);
    TotalMines = MAP_Y * MAP_X / 3;
    if (TotalMines > level)
      TotalMines = level;
    /* ����a�p�Ƥ��o�W�L MAP_Y * MAP_X / 3�A�H�K init_map() �üƨ��Ӥ[ */

    if (MAP_Y < 1 || MAP_X < 1 || TotalMines < 1)
      return 0;
    level = 0;
  }
  else
  {
    level -= '0';
    MAP_Y = 5 * level;		/* ���o�W�L MAP_MAX_Y */
    MAP_X = (level < 4) ? 5 * level : MAP_MAX_X;	/* ���o�W�L MAP_MAX_X */
    TotalMines = MAP_Y * MAP_X / 10;
  }

  TaggedMines = 0;
  cx = MAP_X / 2 + 1;
  cy = MAP_Y / 2 + 1;

  init_map();
  out_map();
  play_mine();

  if (!EndGame)
  {
    if (LoseGame)
    {
      draw_map();
      vmsg("�I�I���a�p�F�I");
    }
    else	/* �аO�� == �a�p�� */
    {
      if (win())	/* itoc.010711: �n�ˬd�O�_�}���A�H�K�H�K�üаO�A��аO��=�a�p�ƴN���L���F */
      {
	char buf[STRLEN];
	sprintf(buf, "�z��F %.0lf �� �}�� %d �� �n�R�� ^O^", difftime(time(0), InitTime), level);
	vmsg(buf);
	addmoney(level * 75);
      }
      else
      {
        draw_map();
        vmsg("�z�п��a�p�F�� =.=");
      }
    }
  }
  else
  {
    vmsg(MSG_QUITGAME);
  }

  return 0;
}
#endif	/* HAVE_GAME */
