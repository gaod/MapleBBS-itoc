/*-------------------------------------------------------*/
/* reversi.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : �¥մѹC��					 */
/* create : 01/07/24					 */
/* update :   /  /					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

enum
{
  /* GRAY_XPOS + MAP_X �n�p�� b_lines - 2 = 21    *
   * GRAY_YPOS + MAP_Y * 2 �n�p�� STRLEN - 1 = 79 *
   * GRAY_YPOS �n������ out_prompt() ��J */

  GRAY_XPOS = 2,	/* �� x ��V */
  GRAY_YPOS = 17,	/* �� y ��V */

  MAP_X = 8,		/* �n�O���� */
  MAP_Y = 8,		/* �n�O���� */

  /* These are flags for "map, tile" bitwise operators */
  TILE_BLANK= 0,	/* �S���аO */
  TILE_CPU = 1,		/* �q���֦� */
  TILE_USR = 2,		/* ���a�֦� */

  /* These are flags for "level" bitwise operators */
  LEVEL_USR_FIRST = 1,	/* ���a���U */
  LEVEL_1 = 2,		/* �@�� */
  LEVEL_2 = 4,		/* �G�� */
  LEVEL_3 = 8		/* �T�� */
};


static char piece[3][3] = {"��", "��", "��"};
static char map[MAP_X][MAP_Y];	/* �a�ϤW�C�檺�֦��� */
static int cx, cy;		/* current (x, y) */
static int EndGame;		/* -1: ���}�C�� 1: �C������ 0: �٦b�� */

#define	mouts(x,y,t)	{ move(GRAY_XPOS + x, GRAY_YPOS + (y) * 2); outs(piece[t]); }


/* int count##(x, y, tile)     */
/* ##     : ���ժ���V N E W S */
/* x, y   : ��m�� (x,y) �y��  */
/* tile   : �֤U���l	       */
/* return : ��Y�X�l	       */


static int
countN(x, y, tile)
  int x, y;
  int tile;
{
  int i;

  for (i = x - 1; i > 0;)
  {
    if (map[i][y] == TILE_BLANK || map[i][y] & tile)
      break;
    i--;
  }

  if (i != x - 1 && map[i][y] & tile)
    return x - i - 1;
  return 0;
}


static int
countS(x, y, tile)
  int x, y;
  int tile;
{
  int i;

  for (i = x + 1; i < MAP_X - 1;)
  {
    if (map[i][y] == TILE_BLANK || map[i][y] & tile)
      break;
    i++;
  }

  if (i != x + 1 && map[i][y] & tile)
    return i - x - 1;
  return 0;
}


static int
countE(x, y, tile)
  int x, y;
  int tile;
{
  int j;

  for (j = y + 1; j < MAP_Y - 1;)
  {
    if (map[x][j] == TILE_BLANK || map[x][j] & tile)
      break;
    j++;
  }

  if (j != y + 1 && map[x][j] & tile)
    return j - y - 1;
  return 0;
}


static int
countW(x, y, tile)
  int x, y;
  int tile;
{
  int j;

  for (j = y - 1; j > 0;)
  {
    if (map[x][j] == TILE_BLANK || map[x][j] & tile)
      break;
    j--;
  }

  if (j != y - 1 && map[x][j] & tile)
    return y - j - 1;
  return 0;
}


static int
countNE(x, y, tile)
  int x, y;
  int tile;
{
  int i, j;

  for (i = x - 1, j = y + 1; i > 0 && j < MAP_Y - 1;)
  {
    if (map[i][j] == TILE_BLANK || map[i][j] & tile)
      break;
    i--;
    j++;
  }

  if (i != x - 1 && map[i][j] & tile)
    return x - i - 1;
  return 0;
}


static int
countNW(x, y, tile)
  int x, y;
  int tile;
{
  int i, j;

  for (i = x - 1, j = y - 1; i > 0 && j > 0;)
  {
    if (map[i][j] == TILE_BLANK || map[i][j] & tile)
      break;
    i--;
    j--;
  }

  if (i != x - 1 && map[i][j] & tile)
    return x - i - 1;
  return 0;
}


static int
countSE(x, y, tile)
  int x, y;
  int tile;
{
  int i, j;

  for (i = x + 1, j = y + 1; i < MAP_X - 1 && j < MAP_Y - 1;)
  {
    if (map[i][j] == TILE_BLANK || map[i][j] & tile)
      break;
    i++;
    j++;
  }

  if (i != x + 1 && map[i][j] & tile)
    return i - x - 1;
  return 0;
}


static int
countSW(x, y, tile)
  int x, y;
  int tile;
{
  int i, j;

  for (i = x + 1, j = y - 1; i < MAP_X - 1 && j > 0;)
  {
    if (map[i][j] == TILE_BLANK || map[i][j] & tile)
      break;
    i++;
    j--;
  }

  if (i != x + 1 && map[i][j] & tile)
    return i - x - 1;
  return 0;
}


static int 		/* �`�@�i�H�Y�X�l 0: ����Y */
do_count(x, y, tile)
{
  if (map[x][y] != TILE_BLANK)
    return 0;

  return countN(x, y, tile) + countS(x, y, tile) + countE(x, y, tile) + countW(x, y, tile) + 
    countNE(x, y, tile) + countNW(x, y, tile) + countSE(x, y, tile) + countSW(x, y, tile);
}


/* void eat##(x, y, tile, num) */
/* ##   : ���Y����V N E W S   */
/* x, y : ��m�� (x,y) �y��    */
/* tile : �֤U���l	       */
/* num  : �Y�X�l	       */


static inline void
eatN(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int i;

  for (i = x - 1; i >= x - num; i--)
  {
    map[i][y] = tile;
    mouts(i, y, tile);
  }
}


static inline void
eatS(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int i;

  for (i = x + 1; i <= x + num; i++)
  {
    map[i][y] = tile;
    mouts(i, y, tile);
  }
}


static inline void
eatE(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int j;

  for (j = y + 1; j <= y + num; j++)
  {
    map[x][j] = tile;
    mouts(x, j, tile);
  }
}


static inline void
eatW(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int j;

  for (j = y - 1; j >= y - num; j--)
  {
    map[x][j] = tile;
    mouts(x, j, tile);
  }
}


static inline void
eatNE(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int i, j;

  for (i = x - 1, j = y + 1; i >= x - num; i--, j++)
  {
    map[i][j] = tile;
    mouts(i, j, tile);
  }
}


static inline void
eatNW(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int i, j;

  for (i = x - 1, j = y - 1; i >= x - num; i--, j--)
  {
    map[i][j] = tile;
    mouts(i, j, tile);
  }
}


static inline void
eatSE(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int i, j;

  for (i = x + 1, j = y + 1; i <= x + num; i++, j++)
  {
    map[i][j] = tile;
    mouts(i, j, tile);
  }
}


static inline void
eatSW(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int i, j;

  for (i = x + 1, j = y - 1; i <= x + num; i++, j--)
  {
    map[i][j] = tile;
    mouts(i, j, tile);
  }
}


static void
do_eat(x, y, tile)
{
  /* �Y�U��V��Y�� */
  eatN(x, y, tile, countN(x, y, tile));
  eatS(x, y, tile, countS(x, y, tile));
  eatE(x, y, tile, countE(x, y, tile));
  eatW(x, y, tile, countW(x, y, tile));
  eatNE(x, y, tile, countNE(x, y, tile));
  eatNW(x, y, tile, countNW(x, y, tile));
  eatSE(x, y, tile, countSE(x, y, tile));
  eatSW(x, y, tile, countSW(x, y, tile));

  /* �Y�ҤU���o�� */
  map[x][y] = tile;
  mouts(x, y, tile);
}


/* ������סA�ܲʲL���H�u���z�A�ݧﵽ */

/* ���Y��X���� */
static int
count_edge(x, y, tile)
  int x, y;
  int tile;
{
  /* �����O��A�~�]�i��Y��W�U�Υ��k���� */
  if (x == 0 || x == MAP_X - 1)
  {
    return 1 + countE(x, y, tile) + countW(x, y, tile);	/* �]�A�ۤv�@���� */
  }
  if (y == 0 || y == MAP_Y - 1)
  {
    return 1 + countN(x, y, tile) + countS(x, y, tile);	/* �]�A�ۤv�@���� */
  }
  return 0;
}


static inline int
find_best(x, y, level)	/* �Ǧ^ (x, y) �q���ҩ�m�̦n����m */
  int *x, *y;
  int level;
{
  int i, j, bestx, besty, tmp;
  int score = 0;

  if (level & LEVEL_1)			/* �@��: �Y�V�h�V�n */
  {
    for (i = 0; i < MAP_X; i++)
    {
      for (j = 0; j < MAP_Y; j++)
      {
	if (tmp = do_count(i, j, TILE_CPU))
	{
	  if (tmp > score)
	  {
	    score = tmp;
	    bestx = i;
	    besty = j;
	  }
	}
      }
    }
  }
  else if (level & LEVEL_2)		/* �G��: ²�ƪ��������� */
  {
    for (i = 0; i < MAP_X; i++)
    {
      for (j = 0; j < MAP_Y; j++)
      {
	if (tmp = do_count(i, j, TILE_CPU))
	{
	  /* �� +100  �� +50  �@�� +1 */
	  if (i == 0 || i == MAP_X - 1)
	  {
	    if (j == 0 || j == MAP_Y - 1)
	      tmp += 100;
	    else
	      tmp += 50;
	  }
	  else if (j == 0 || j == MAP_Y - 1)
	  {
	    tmp += 50;
	  }

	  if (tmp > score)
	  {
	    score = tmp;
	    bestx = i;
	    besty = j;
	  }
	}
      }
    }	  
  }
  else /* if (level & LEVEL_3) */	/* �T��: �������� */
  {
    for (i = 0; i < MAP_X; i++)
    {
      for (j = 0; j < MAP_Y; j++)
      {
	if (tmp = do_count(i, j, TILE_CPU))
	{
	  /* �� +100  �@���� +10  �@�� +1 */
	  if (i == 0 || i == MAP_X - 1)
	  {
	    if (j == 0 || j == MAP_Y - 1)
	      tmp += 100;
	    else
	      tmp += 10 * count_edge(i, j, TILE_CPU);
	  }
	  else if (j == 0 || j == MAP_Y - 1)
	  {
	    tmp += 10 * count_edge(i, j, TILE_CPU);
	  }

	  if (tmp > score)
	  {
	    score = tmp;
	    bestx = i;
	    besty = j;
	  }
	}
      }
    }	  
  }

  *x = bestx;
  *y = besty;
  return score;
}


/* �]�w�ѽL */

static inline void
init_map()
{
  int i, j;

  for (i = 0; i < MAP_X; i++)
  {
    for (j = 0; j < MAP_Y; j++)
    {
      map[i][j] = TILE_BLANK;
    }
  }

  map[MAP_X / 2 - 1][MAP_Y / 2 - 1] = TILE_CPU;
  map[MAP_X / 2][MAP_Y / 2] = TILE_CPU;
  map[MAP_X / 2 - 1][MAP_Y / 2] = TILE_USR;
  map[MAP_X / 2][MAP_Y / 2 - 1] = TILE_USR;
}



/* �ù����� */

static inline void
out_prompt()
{
  /* ���o�W�L GRAY_YPOS�A�_�h�|���� */
  move(3, 0);
  outs("���仡���G");
  move(5, 0);
  outs("����     ��V��");
  move(6, 0);
  outs("����     �ť���");
  move(7, 0);
  outs("����     Enter");
  move(8, 0);
  outs("���}     Esc / q");
  move(10, 0);
  outs("���a     ");
  outs(piece[TILE_USR]);
  move(11, 0);
  outs("�q��     ");
  outs(piece[TILE_CPU]);
}


static inline void
out_song()
{
  uschar *msg[5] = 
  {
    "�G���Ѫ�  �G���Ѫ�",
    "�]�o��  �]�o��",
    "�@���S������",
    "�@���S������",
    "�u�_��  �u�_��"
  };
  move(b_lines - 2, 0);
  prints("\033[1;3%dm%s\033[m", time(0) % 7, msg[time(0) % 5]);
  clrtoeol();
}


static inline void
out_map()
{
  int i, j;

  vs_bar("�¥մ�");

  out_prompt();
  out_song();

  for (i = 0; i < MAP_X; i++)
  {
    move(GRAY_XPOS + i, GRAY_YPOS);
    for (j = 0; j < MAP_Y; j++)
      outs(piece[TILE_BLANK]);
  }

  mouts(MAP_X / 2 - 1, MAP_Y / 2 - 1, TILE_CPU);
  mouts(MAP_X / 2, MAP_Y / 2, TILE_CPU);
  mouts(MAP_X / 2 - 1, MAP_Y / 2, TILE_USR);
  mouts(MAP_X / 2, MAP_Y / 2 - 1, TILE_USR);

  move(GRAY_XPOS + cx, GRAY_YPOS + cy * 2 + 1);	/* move to (0, 0) */
}


/* �C���D�{�� */

static inline void
result(msg)
  char *msg;
{
  int i, j;
  int sumCPU, sumUSR;

  sumCPU = sumUSR = 0;
  for (i = 0; i < MAP_X; i++)
  {
    for (j = 0; j < MAP_Y; j++)
    {
      if (map[i][j] & TILE_CPU)
	sumCPU++;
      else if(map[i][j] & TILE_USR)
	sumUSR++;
    }
  }

  sprintf(msg, "[%s] ���a�G�q�� = %d�G%d", 
    (sumUSR > sumCPU) ? "�ӧQ" : (sumUSR < sumCPU ? "����" : "����"),
    sumUSR, sumCPU);
}


static inline void
play_reversi(level)
  int level;
{
  int i, j;
  int ch;
  int usr_turn;		/* 1: �Ӫ��a  0: �ӹq�� */
  int pass;		/* 0: �S���Hpass  1: �@�ӤHpass  2:�s��G�ӤHpass */
  int bestx, besty;	/* �q���̨ΤU�l�B */

  pass = 0;
  if (!(level & LEVEL_USR_FIRST))
    goto cpu_first;

  while (!EndGame)
  {
    /* ���⪱�a�٦��S���l�i�H�U */
    for (i = 0; i < MAP_X; i++)
    {
      for (j = 0; j < MAP_Y; j++)
      {
	if (do_count(i, j, TILE_USR))
	{
	  usr_turn = 1;
	  i = MAP_X;	/* ���} for �j�� */
	  j = MAP_Y;
	}
      }	
    }

    if (!usr_turn)
    {
      pass++;

      /* �ˬd�O�_���G�H passout */
      if (pass == 2)
      {
	EndGame = 1;	/* �C������ */
	return;
      }
    }
    else if (pass)
    {
      vmsg("�q���L�l�i�U�A����z�F");
      move(b_lines, 0);
      clrtoeol();	/* ���� vmsg() */
    }

    while (usr_turn && (ch = vkey()))	/* �Ӫ��a�U */
    {
      switch (ch)
      {
      case KEY_ESC:
      case 'q':
      case 'Q':
	EndGame = -1;
	return;

      case KEY_UP:
	if (cx)
	{
	  cx--;
	  move(GRAY_XPOS + cx, GRAY_YPOS + cy * 2 + 1);
	}
	break;

      case KEY_DOWN:
	if (cx < MAP_X - 1)
	{
	  cx++;
	  move(GRAY_XPOS + cx, GRAY_YPOS + cy * 2 + 1);
	}
        break;

      case KEY_LEFT:
	if (cy)
	{
	  cy--;
	  move(GRAY_XPOS + cx, GRAY_YPOS + cy * 2 + 1);
	}
	break;

      case KEY_RIGHT:
	if (cy < MAP_Y - 1)
	{
	  cy++;
	  move(GRAY_XPOS + cx, GRAY_YPOS + cy * 2 + 1);
	}
	break;

      case '\n':
      case ' ':
	if (do_count(cx, cy, TILE_USR))
	{
          do_eat(cx, cy, TILE_USR);
          usr_turn = 0;
          pass = 0;
	}
	break;

      default:
        break;
      }
    }		/* ���a�U while �j�鵲�� */

cpu_first:

    /* �� CPU �U */
    if (!find_best(&bestx, &besty, level))
    {
      pass++;
    }
    else
    {
      do_eat(bestx, besty, TILE_CPU);
      pass = 0;
      cx = bestx;		/* ���� CPU �ҤU����m */
      cy = besty;
    }
    move(GRAY_XPOS + cx, GRAY_YPOS + cy * 2 + 1);

    /* �ˬd�O�_���G�H passout */
    if (pass == 2)
      EndGame = 1;	/* �C������ */

  }	/* while (!EndGame) �j�鵲�� */
}


int
main_reversi()
{
  int level;

  level = vans("�п�� 1)���p�ϴx 2)�D�`²�� 3)���q���סA�Ϋ� [Q] ���}�G") - '1';
  if (level >= 0 && level <= 2)
  {
    level = LEVEL_1 << level;	/* �]�w���� */
    if (vans("���a���U��(Y/N)�H[Y] ") != 'n')
      level |= LEVEL_USR_FIRST;
  }
  else
  {
    /* vmsg(MSG_QUITGAME); */	/* itoc.010312: ���n�F */
    return XEASY;
  }

  cx = MAP_X / 2 - 1;
  cy = MAP_Y / 2 - 1;
  EndGame = 0;

  init_map();
  out_map();
  play_reversi(level);

  if (EndGame < 0)
  {
    vmsg(MSG_QUITGAME);
  }
  else
  {
    char buf[60];
    result(buf);
    vmsg(buf);
  }
  return 0;
}
#endif	/* HAVE_GAME */
