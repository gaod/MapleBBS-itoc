/*-------------------------------------------------------*/
/* liteon.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : �}�O�C��					 */
/* create : 02/05/23					 */
/* update :   /  /                                       */
/* author : Gein.bbs@csdc.twbbs.org			 */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_GAME

#define MAX_LEVEL	(b_lines - 3)

enum
{
  TL_XPOS = 2,
  TL_YPOS = 5,

  /* �� bitwise operators */
  TILE_BLANK = 0,	/* �t�� */
  TILE_LIGHT = 1	/* �G�� */
};


static int cx, cy;	/* �ثe�Ҧb��� */
static int level;	/* ���šA�P�ɤ]�O board ����� */
static int onturn;	/* ���X�ӿO���}�F */
static int candle;	/* �I�F�X������ */
static int tl_board[T_LINES - 4][T_LINES - 4];


static void 
tl_setb()			/* set board all 0 */
{
  int i, j;

  move(1, 0);
  clrtobot();

  for (i = 0; i < level; i++)
  {
    move(i + TL_XPOS, TL_YPOS);
    for (j = 0; j < level; j++)
    {
      tl_board[i][j] = TILE_BLANK;
      outs("��");
    }
  }

  cx = cy = onturn = candle = 0;
  move(TL_XPOS, TL_YPOS + 1);	/* move back to (0, 0) */
}


static void
tl_draw(x, y)			/* set/reset and draw a tile */
  int x, y;
{
  tl_board[x][y] ^= TILE_LIGHT;
  move(x + TL_XPOS, y * 2 + TL_YPOS);
  if (tl_board[x][y] == TILE_BLANK)	/* on-turn -> off-turn */
  {
    onturn--;
    outs("��");
  }
  else					/* off-turn -> on-turn */
  {
    onturn++;
    outs("��");
  }
}


static void 
tl_turn()			/* turn light and light arround it */
{
  tl_draw(cx, cy);

  if (cx > 0)
    tl_draw(cx - 1, cy);

  if (cx < level - 1)
    tl_draw(cx + 1, cy);

  if (cy > 0)
    tl_draw(cx, cy - 1);

  if (cy < level - 1)
    tl_draw(cx, cy + 1);
}


static void 
tl_candle()			/* cheat: use candle */
{
  /* itoc.����: �]���j�a���}���F�o�C���A�ҥH���Ѥ@�U�@���Ϊ��I���� */
  tl_draw(cx, cy);
  candle++;
}

static int			/* 1:win 0:lose */
tl_play()			/* play turn_light */
{
  tl_setb();

  while (onturn != level * level)
  {
    switch (vkey())
    {
    case KEY_LEFT:
      cy--;
      if (cy < 0)
	cy = level - 1;
      break;

    case KEY_RIGHT:
      cy++;
      if (cy == level)
	cy = 0;
      break;

    case KEY_UP:
      cx--;
      if (cx < 0)
	cx = level - 1;
      break;

    case KEY_DOWN:
      cx++;
      if (cx == level)
	cx = 0;
      break;

    case 'c':
      tl_candle();
      break;

    case ' ':
    case '\n':
      tl_turn();
      break;

    case 'r':
      tl_setb();
      break;

    case 'q':
      return 0;
    }
    move(cx + TL_XPOS, cy * 2 + TL_YPOS + 1);	/* move back to current (x, y) */
  }
  return 1;
}


int
main_liteon()
{
  char ans[5], buf[80];

  sprintf(buf, "�п�ܵ���(1��%d)�A�Ϋ� [Q] ���}�G", MAX_LEVEL);
  level = vget(b_lines, 0, buf, ans, 3, DOECHO);
  if (level == 'q' || level == 'Q')
  {
    return XEASY;
  }
  else
  {
    level = atoi(ans);
    if (level < 1 || level > MAX_LEVEL)
      return XEASY;
  }

  vs_bar("�}�O�C��");
  move(4, 13);
  outs("�e�����n�G");
  move(5, 15);
  outs("���@�ѡA�p�ئ^��a�o�{�O���Q���F�C");
  move(6, 15);
  outs("�i�O�L�a���O���@�ӯS�ʡA���N�O�G");
  move(7, 15);
  outs("��@���O�Q���U�}���H��A�L�P�򪺿O");
  move(8, 15);
  outs("�쥻�G���A�N�|�ܷt�A�쥻�t���A�N�|�ܫG�C -____-#");
  move(9, 15);
  outs("�{�b�N���o�����z���L��Ҧ��O���}�a�I");

  move(11, 13);
  outs("���仡���G");
  move(12, 15);
  outs("��������     ���ʤ�V");
  move(13, 15);
  outs("Enter/Space  �����}��");
  move(14, 15);
  outs("c            �I�U���� [�K��]");
  move(15, 15);
  outs("r            ���s�ӹL");
  move(16, 15);
  outs("q            ���}�C��");

  vmsg(NULL);

  if (tl_play())		/* if win */
  {
    sprintf(buf, "���߱z���\\�F  (�ΤF %d ������)", candle);
    vmsg(buf);
  }

  return 0;
}
#endif				/* HAVE_GAME */
