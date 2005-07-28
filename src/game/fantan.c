/*-------------------------------------------------------*/
/* fantan.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : ���s�C��					 */
/* create : 98/08/04					 */
/* update : 01/03/01					 */
/* author : dsyan.bbs@Forever.twbbs.org			 */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

enum
{
  SIDE_UP = 1,				/* �b�W�� */
  SIDE_DOWN = 0				/* �b�U�� */
};


static inline int
cal_kind(card)				/* ���� */
  int card;
{
  /* card:  1 - 13 �ġ��ت�� ��
	   14 - 26 �Ĥ@�ت�� ��
	   27 - 39 �ĤG�ت�� ��
	   40 - 52 �ĤT�ت�� �� */

  return (card - 1) / 13;
}


static inline int 
cal_num(card)				/* ���I�� */
  int card;
{
  card %= 13;
  return card ? card : 13;
}


static void
move_cur(a, b, c, d)			/* ���ʽb�� */
  int a, b;		/* ���m */
  int c, d;		/* �s��m */
{
  move(a, b);
  outs("  ");
  move(c, d);
  outs("��");
  move(c, d + 1);	/* �קK�۰ʰ������� */
}


static inline int
ycol(y)					/* ��y�� */
  int y;
{
  return y * 11 - 8;
}


static void
draw(x, y, card)			/* �e�P */
  int x, y;
  int card;
{
  char kind[4][3] = {"��", "��", "��", "��"};			/* ��� */
  char num[13][3] = {"��", "��", "��", "��", "��", "��", "��",	/* �I�� */
		     "��", "��", "��", "��", "��", "��"};

  move(x, y);

  if (card > 0)
    prints("�u%s%s�t", kind[cal_kind(card)], num[cal_num(card) - 1]);
  else if (!card)
    outs("�u�w�w�t");
  else
    outs("        ");
}


static inline void
out_prompt()			/* ���ܦr�� */
{
  move(b_lines - 1, 0);
  outs(COLOR2 " (q)���} (r)���� (��������)���� (��)½�� (Enter)���| (Space)���w/����/½�P    \033[m");
}


static char
get_newcard(mode)
  int mode;			/* 0:���s�~�P  1:�o�P */
{
  static char card[52];		/* �̦h�u�|�Ψ� 52 �i�P */
  static int now;		/* �o�X�� now �i�P */
  int i, num;
  char tmp;

  if (!mode)   /* ���s�~�P */
  {
    for (i = 0; i < 52; i++)
      card[i] = i + 1;

    for (i = 0; i < 51; i++)
    {
      num = rnd(52 - i) + i;

      /* card[num] �M card[i] �洫 */
      tmp = card[i];
      card[i] = card[num];
      card[num] = tmp;
    }

    now = 0;
    return -1;
  }

  tmp = card[now];
  now++;
  return tmp;
}


int
main_fantan()
{
  char max[9];				/* �C�諸�P�ơA�ĤK��O���W�����P */
  char rmax[8];				/* �C�良�|�Q½�}���P�� */

  char left_stack[25];			/* ���W���� 24 �i�P */
  char up_stack[5];			/* �k�W���� 4 �i�P (�u�n�O���̤j�P�Y�i) */
  char down_stack[8][21];		/* �U���C�Ӱ��|���Ҧ��P */

  int level;				/* �@��½�X�i�P */
  int side;				/* ��Цb�W���٬O�U�� */

  int cx, cy;				/* �ثe�Ҧb (x, y) �y�� */
  int xx, yy;				/* �L�h�Ҧb (x, y) �y�� */
  int star_c, star_x, star_y;		/* �� '*' �B�� �P�B�y�� */
  int left;				/* ���W�����|½��ĴX�i */

  int i, j;

  time_t init_time;			/* �C���}�l���ɶ� */

  level = vans("�п�ܤ@��½ [1~3] �@��T �i�P�A�Ϋ� [Q] ���}�G");
  if (level > '0' && level < '4')
    level -= '0';
  else
    return XEASY;

game_start:
  vs_bar("���s");
  out_prompt();

  side = SIDE_DOWN;
  star_c = 0;
  star_x = 2;
  star_y = 79;

  for (i = 0; i <= 4; i++)			/* �W�����|�Ӱ��|�k�s */
    up_stack[i] = 0;

  get_newcard(0);	/* �~�P */

  for (i = 1; i <= 7; i++)
  {
    max[i] = i;					/* �� i ���}�l�� i �i�P */
    rmax[i] = i - 1;				/* �� i ���}�l�� i-1 �i�����} */
    for (j = 1; j <= i; j++)
    {
      down_stack[i][j] = get_newcard(1);	/* �t�m�U�����P */
      draw(j + 2, ycol(i), i != j ? 0 : down_stack[i][j]);	/* �C�若�}�̫�@�i�P */
    }
  }

  max[8] = 24;					/* ���W����}�l�� 24 �i�P */
  for (i = 1; i <= 24; i++)
    left_stack[i] = get_newcard(1);		/* �t�m���W�����P */
  draw(1, 1, 0);

  left = 0;
  cx = 1;
  cy = 1;
  xx = 1;
  yy = 1;

  init_time = time(0);		/* �}�l�O���ɶ� */

  for (;;)
  {
    if (side == SIDE_DOWN)
    {
      move_cur(xx + 2, ycol(yy) - 2, cx + 2, ycol(cy) - 2);
      xx = cx;
      yy = cy;

      switch (vkey())
      {
      case 'q':
	vmsg(MSG_QUITGAME);
	return;

      case 'r':
	goto game_start;

      case KEY_LEFT:
	cy--;
	if (!cy)
	  cy = 7;
	if (cx > max[cy] + 1)
	  cx = max[cy] + 1;
	break;

      case KEY_RIGHT:
	cy++;
	if (cy == 8)
	  cy = 1;
	if (cx > max[cy] + 1)
	  cx = max[cy] + 1;
	break;

      case KEY_DOWN:
	cx++;
	if (cx == max[cy] + 2)
	  cx--;
	break;

      case KEY_UP:
	cx--;
	if (!cx)					/* �]��W���h�F */
	{
	  side = SIDE_UP;
	  move_cur(xx + 2, ycol(yy) - 2, 1, 9);
	}
	break;

      case '\n':				/* ���P��k�W�� */
	j = down_stack[cy][cx];
	if ((cal_num(j) == up_stack[cal_kind(j)] + 1) && cx == max[cy] && cx > rmax[cy])
	{
	  up_stack[cal_kind(j)]++;
	  max[cy]--;
	  draw(1, cal_kind(j) * 10 + 40, j);
	  draw(cx + 2, ycol(cy), -1);
	  if (star_c == j)			/* �p�G���O���N���� */
	  {
	    move(star_x, star_y);
	    outc(' ');
	  }
	}
	/* �}������: �k�W���|�ӳ��O 13 */
	if (up_stack[0] & up_stack[1] & up_stack[2] & up_stack[3] == 13)
	{
	  char buf[80];
	  sprintf(buf, "�z��F %.0lf �� �}�� %d �� �n�R�� ^O^", 
	    difftime(time(0), init_time), level);
	  vmsg(buf);
	  addmoney(level * 100);
	  return;	  
	}
	break;

      case ' ':
	if (cx == max[cy] && cx == rmax[cy])	/* ½�s�P */
	{
	  rmax[cy]--;
	  draw(cx + 2, ycol(cy), down_stack[cy][cx]);
	  break;
	}
	else if (cx > rmax[cy] && cx <= max[cy])	/* �ŤU */
	{
	  move(star_x, star_y);
	  outc(' ');
	  star_c = down_stack[cy][cx];
	  star_x = cx + 2;
	  star_y = cy * 11;
	  move(star_x, star_y);
	  outc('*');
	  break;
	}
	else if (cx != max[cy] + 1)
	  break;				/* �K�W */

	if ((max[cy] && (cal_num(down_stack[cy][max[cy]]) == cal_num(star_c) + 1) && 
	  (cal_kind(down_stack[cy][max[cy]]) + cal_kind(star_c)) % 2) ||
	  (max[cy] == 0 && cal_num(star_c) == 13))
	{
	  if (star_x == 1)		/* �q�W���K�U�Ӫ� */
	  {
	    max[cy]++;
	    max[8]--;
	    star_x = 2;
	    left--;
	    for (i = left + 1; i <= max[8]; i++)
	      left_stack[i] = left_stack[i + 1];
	    down_stack[cy][max[cy]] = star_c;
	    draw(max[cy] + 2, ycol(cy), star_c);
	    move(1, 19);
	    outc(' ');
	    draw(1, 11, left ? left_stack[left] : -1);
	  }
	  else if (star_x > 2)		/* �b�U���K�ӶK�h�� */
	  {
	    int tmp;;
	    j = star_y / 11;
	    tmp = max[j];	    
	    for (i = star_x - 2; i <= tmp; i++)
	    {
	      max[cy]++;
	      max[j]--;
	      down_stack[cy][max[cy]] = down_stack[j][i];
	      draw(max[cy] + 2, ycol(cy), down_stack[cy][max[cy]]);
	      draw(i + 2, ycol(j), -1);
	    }
	    move(star_x, star_y);
	    outc(' ');
	    star_x = 2;
	  }
	}
	break;
      }

    }
    else /* side == SIDE_UP */	/* �b�W�� */
    {
      draw(1, 11, left ? left_stack[left] : -1);

      switch (vkey())
      {
      case 'q':
	vmsg(MSG_QUITGAME);
	return;

      case 'r':
	goto game_start;

      case '\n':				/* ���P��k�W�� */
	j = left_stack[left];
	if (cal_num(j) == up_stack[cal_kind(j)] + 1)
	{
	  up_stack[cal_kind(j)]++;
	  max[8]--;
	  left--;
	  draw(1, cal_kind(j) * 10 + 40, j);

	  for (i = left + 1; i <= max[8]; i++)
	    left_stack[i] = left_stack[i + 1];

	  draw(1, 11, left ? left_stack[left] : -1);

	  if (star_x == 1)	/* �p�G���O���N�M�� */
	  {
	    star_x = 2;
	    move(1, 19);
	    outc(' ');
	  }
	  /* �}������: �k�W���|�ӳ��O 13 */
	  if (up_stack[0] & up_stack[1] & up_stack[2] & up_stack[3] == 13)
	  {
	    char buf[80];
	    sprintf(buf, "�z��F %.0lf �� �}�� %d �� �n�R�� ^O^", 
	      difftime(time(0), init_time), level);
	    vmsg(buf);
	  }
	}
	break;

      case KEY_DOWN:
	side = SIDE_DOWN;
	cx = 1;
	move_cur(1, 9, cx + 2, ycol(cy) - 2);
	break;

      case KEY_UP:
	if (left == max[8])
	  left = 0;
	else
	  left += level;	/* �@���o level �i�P */
	if (left > max[8])
	  left = max[8];

	if (star_x == 1)
	{
	  star_x = 2;
	  move(1, 19);
	  outc(' ');
	}

	draw(1, 1, left == max[8] ? -1 : 0);
	break;

      case ' ':
	if (left > 0)
	{
	  move(star_x, star_y);
	  outc(' ');
	  star_c = left_stack[left];
	  star_x = 1;
	  star_y = 19;
	  move(1, 19);
	  outc('*');
	}
	break;
      }
    }
  }
}
#endif	/* HAVE_GAME */
