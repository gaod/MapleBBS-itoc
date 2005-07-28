/*-------------------------------------------------------*/
/* dragon.c	( YZU WindTopBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : ���s�C��					 */
/* create : 01/01/12					 */
/* update : 03/07/23					 */
/* author : verit.bbs@bbs.yzu.edu.tw			 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

static int cards[52];
static int seven[7];		/* �C��P�[�U�٦��X�i�P */
static int points[7];


/*-------------------------------------------------------*/
/* �e�X�P						 */
/*-------------------------------------------------------*/


static void
draw_card(x, y, card)
  int x, y;
  int card;	/* >=0:�L�X��i�P�Φ��i�P�����X  -1:�u�L�P�~�� */
{
  char flower[4][3] = {"��", "��", "��", "��"};
  char number[13][3] = {"��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��"};

  move(x, y);
  outs("�~�w�w�w��");

  if (card < 0)
    return;

  move(x + 1, y);
  prints("�x%s�@�@�x", number[card % 13]);
  move(x + 2, y);
  prints("�x%s�@�@�x", flower[card % 4]);
  move(x + 3, y);
  outs("�x�@�@�@�x");
  move(x + 4, y);
  outs("�x�@�@�@�x");
  move(x + 5, y);
  outs("���w�w�w��");
}


/*-------------------------------------------------------*/
/* �C������						 */
/*-------------------------------------------------------*/


static int
draw_explain()
{
  vs_bar("���s�C��");

  move(4, 10);
  outs("�i�C�������j");
  move(6, 15);
  outs("(1) �b�ù��W�謰�P�[�A�i�H�Q�� ���B�� �����C");
  move(8, 15);
  outs("(2) �b�ù����U�謰���P , �i�H�Q�� c �����C");
  move(10, 15);
  outs("(3) ��P�[���P�O���P���U�@�i�ΤW�@�i�A�Y�i�Q�� Enter �Y�P�C");
  move(11, 15);
  outs("   (�u���I�ơA���ݪ��)");
  move(13, 15);
  outs("(4) ��P�[���P���Y���A�ιC����ӡC");
  move(15, 15);
  outs("(5) ����P�������B�|���Y���P�[���P�A�Y�C�����ѡC");
  vmsg(NULL);
}


/*-------------------------------------------------------*/
/* �e�X�C���P���t�m					 */
/*-------------------------------------------------------*/


static void
draw_screen()
{
  int i, j;
  vs_bar("���s�C��");

  for (i = 0; i < 7; i++)
  {
    for (j = 0; j <= i; j++)
      draw_card(4 + j, 5 + i * 10, (i == j) ? cards[i] : -1);
  }
}


/*-------------------------------------------------------*/
/* �e�X���						 */
/*-------------------------------------------------------*/


static void
draw_cursor(location, mode)
  int location;
  int mode;		/* 1:�W��  0:�M�� */
{
  int x, y;

  x = 8 + seven[location];
  y = 9 + location * 10;
  move(x, y);
  outs(mode ? "��" : "�@");
  if (mode)
    move(x, y + 1);		/* �קK�۰ʰ������� */
}


/*-------------------------------------------------------*/
/* �M���ù��W���P					 */
/*-------------------------------------------------------*/


static void 
clear_card(location)
  int location;
{
  move(9 + seven[location], 5 + location * 10);
  outs("          ");
}


/*-------------------------------------------------------*/
/* �C���Ѽƪ�l��					 */
/*-------------------------------------------------------*/


static int
init_dragon()
{
  int i, j, num;

  for (i = 0; i < 52; i++)	/* �P���@�i�@�i�Ʀn�A�ǳƬ~�P */
    cards[i] = i;

  for (i = 0; i < 51; i++)
  {
    j = rnd(52 - i) + i;

    /* cards[j] �M cards[i] �洫 */
    num = cards[i];
    cards[i] = cards[j];
    cards[j] = num;
  }

  for (i = 0; i < 7; i++)
  {
    seven[i] = i;
    points[i] = cards[i];
  }

  return 0;
}


/*-------------------------------------------------------*/
/* �P�_�C���O�_����					 */
/*-------------------------------------------------------*/


static int	/* 1:���\ */
gameover()
{
  int i;

  for (i = 0; i < 7; i++)
  {
    if (seven[i] != -1)
      return 0;
  }
  return 1;
}


/*-------------------------------------------------------*/
/* �C���D�{��						 */
/*-------------------------------------------------------*/


int			/* >=0:���\ -1:���� -2:���} */
play_dragon()
{
  int i;
  int location = 0;	/* �ثe��Ъ���m */
  int now = 7;		/* �ثe�Ψ� cards[] �ĴX�i�P */
  int have_card = 22;	/* 22 �����P���| */
  int point;		/* �ثe��W���o�i�P */

  clear();

  draw_screen();
  draw_cursor(location, 1);
  point = cards[now];
  draw_card(14, 5, cards[now++]);
  move(19, 40);
  prints("�z�٦� %2d �����|�i�H���P", have_card);
  move(b_lines, 0);
  outs("�� �ާ@�����G(��)���� (��)�k�� (Enter)�Y�P (c)���P (q)���}");

  for (;;)
  {
    switch (vkey())
    {
    case 'c':
      if (have_card <= 0)
	return -1;
      have_card--;
      move(19, 47);
      prints("%2d", have_card);
      point = cards[now];
      draw_card(14, 5, cards[now++]);
      break;

    case KEY_RIGHT:
      draw_cursor(location, 0);
      do
      {
	location = (location + 1) % 7;
      } while (seven[location] == -1);
      draw_cursor(location, 1);
      break;

    case KEY_LEFT:
      draw_cursor(location, 0);
      do
      {
	location = (location == 0) ? 6 : location - 1;
      } while (seven[location] == -1);
      draw_cursor(location, 1);
      break;

    case '\n':
    case ' ':
      if (points[location] % 13 - point % 13 == 1 ||
	points[location] % 13 - point % 13 == -1 ||
	points[location] % 13 - point % 13 == 12 ||
	points[location] % 13 - point % 13 == -12)
      {
	point = points[location];
	draw_card(14, 5, point);
	clear_card(location);
	draw_cursor(location, 0);
	seven[location]--;
	if (seven[location] >= 0)
	{
	  points[location] = cards[now];
	  draw_card(4 + seven[location], 5 + location * 10, cards[now++]);
	  draw_cursor(location, 1);
	}
	else
	{
	  for (i = 0; i < 5; i++)
	  {
	    move(4 + i, 5 + location * 10);
	    outs("          ");
	  }
	  if (gameover() == 1)
	    return have_card;
	  do
	  {
	    location = (location + 1) % 7;
	  } while (seven[location] == -1);
	  draw_cursor(location, 1);
	}
      }
      break;

    case 'q':
      return -2;
    }
  }
}


int
main_dragon()
{
  draw_explain();

  while (1)
  {
    init_dragon();

    switch (play_dragon())
    {
    case -1:
      vmsg("�D�ԥ��ѡI");
      break;

    case -2:
      vmsg(MSG_QUITGAME);
      return 0;

    default:
      vmsg("���߱z�L���աI");
    }

    if (vans("�O�_�n�~��(Y/N)�H[N] ") != 'y')
      break;
  }

  return 0;
}
#endif	/* HAVE_GAME */
