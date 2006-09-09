/*-------------------------------------------------------*/
/* chessmj.c      ( NTHU CS MapleBBS Ver 3.10 )          */
/*-------------------------------------------------------*/
/* target : �H�ѳ±N�C��                                 */
/* create : 98/07/29                                     */
/* update : 05/06/30                                     */
/* author : weiren@mail.eki.com.tw                       */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/* modify : yiting.bbs@bbs.cs.tku.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_GAME


static int host_card[5];	/* �q��(���a) ���P */
static int guest_card[5];	/* ���a���P */
static int throw[50];		/* �Q��󪺵P�A�h�d�@�Ǧ�m��Q�Y���P */

static int flag;
static int tflag;
static int selftouch;		/* �ۺN */

static int cnum[32] = 
{
  1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
  7, 7, 7, 7, 7, 8, 9, 9, 10, 10,
  11, 11, 12, 12, 13, 13,
  14, 14, 14, 14, 14
};

static int group[14] = 
{
  1, 1, 1, 2, 2, 2, 3,
  4, 4, 4, 5, 5, 5, 6
};

static char *chess[15] = 
{
  "\033[1;30m  \033[m",
  "\033[1;31m��\033[m", "\033[1;31m�K\033[m", "\033[1;31m��\033[m", "\033[1;31m��\033[m", "\033[1;31m�X\033[m", "\033[1;31m��\033[m", "\033[1;31m�L\033[m",
  "\033[1;37m�N\033[m", "\033[1;37m�h\033[m", "\033[1;37m�H\033[m", "\033[1;37m��\033[m", "\033[1;37m��\033[m", "\033[1;37m�]\033[m", "\033[1;37m��\033[m"
};


static void
out_song(money)
  int money;
{
  move(b_lines - 2, 0);
  prints("\033[1;37;44m�{���w�X: %-20d", cuser.money);
  if(money)
    prints("��`���B: %-38d\033[m", money);
  else
    outs("                                                \033[m");
}


static inline void
print_sign(host)
  int host;
{
  int y = (tflag-1) / 2 * 4; 
  move((host * 3 + 8), y);
  outs("�~����");
}


static inline void
print_chess(x, y)
  int x, y;
{
  move(x, y);
  outs("�~�w��");
  move(x + 2, y);
  outs("���w��");
}


static inline void
clear_chess(x, y)
  int x, y;
{
  move(x, y);
  outs("      ");
  move(x + 2, y);
  outs("      ");
}


static void
print_all(cards, x)
  int cards[5], x;
{
  int i;

  move(x + 1, 0);
  clrtoeol();

  for (i = 0; i < 4; i++)
    prints("�x%s�x", chess[cards[i]]);

  if (cards[4] == 0)
    clear_chess(x, 24);
  else
  {
    prints("�x%s�x", chess[cards[4]]);
    print_chess(x, 24);
  }
}


static inline void
print_guest()
{
  print_all(guest_card, 15);
}


static inline void
print_host()
{
  print_all(host_card, 4);
}


static void
print_throw(host)
  int host;
{
  int x = 11 - host * 3;

  move(x + 1, 0);
  outs("�x");

  for(; host < tflag; host += 2)
    prints("%s�x", chess[throw[host] & 0x0f]);
  
  print_chess(x, (host-2) / 2 * 4);
}


static inline void
sortchess()
{
  int i, j, x;

  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < (3 - i); j++)
    {
      if (guest_card[j] > guest_card[j + 1])
      {
	x = guest_card[j];
	guest_card[j] = guest_card[j + 1];
	guest_card[j + 1] = x;
      }
      if (host_card[j] > host_card[j + 1])
      {
	x = host_card[j];
	host_card[j] = host_card[j + 1];
	host_card[j + 1] = x;
      }
    }
  }
}


static int
testpair(a, b)
  int a, b;
{
  if (a == b)
    return 1;
  if (a == 1 && b == 8)
    return 1;
  if (a == 8 && b == 1)
    return 1;
  return 0;
}


static int
testthree(a, b, c)
  int a, b, c;
{
  int tmp;
  if (a > b)
  {
    tmp = a;
    a = b;
    b = tmp;
  }
  if (b > c)
  {
    tmp = b;
    b = c;
    c = tmp;
  }
  if (a > b)
  {
    tmp = a;
    a = b;
    b = tmp;
  }
  if (a == 1 && b == 2 && c == 3)
    return 1;			/* �ӥK�� */
  if (a == 4 && b == 5 && c == 6)
    return 1;			/* ���X�� */
  if (a == 8 && b == 9 && c == 10)
    return 1;			/* �N�h�H */
  if (a == 11 && b == 12 && c == 13)
    return 1;			/* �����] */
  if (a == 7 && b == 7 && c == 7)
    return 1;			/* �L�L�L */
  if (a == 14 && b == 14 && c == 14)
    return 1;			/* ���� */
  return 0;
}


static int
testall(set)
  int set[5];
{
  int i, j, k, m, p[3];

  for (i = 0; i < 4; i++)
  {
    for (j = i + 1; j < 5; j++)
    {
      m = 0;
      for (k = 0; k < 5; k++)
      {
	if (k != i && k != j)
	{
	  p[m] = set[k];
	  m++;
	}
      }
      if (testpair(set[i], set[j]) != 0 && testthree(p[0], p[1], p[2]) != 0)
	return 1;
    }
  }
  return 0;
}


static int
testlisten(set)
  int set[4];
{
  int i, j, k, p[2] = {0}, m = 0, mm = 0;

  j = 0;
  for (i = 0; i < 4; i++)
  {
    if (group[set[i]] != 3)
      j++;
  }
  if (j == 0)
    return 1;			/* �|��L */

  j = 0;
  for (i = 0; i < 4; i++)
  {
    if (group[set[i]] != 6)
      j++;
  }
  if (j == 0)
    return 1;			/* �|��� */

  if (testthree(set[1], set[2], set[3]) != 0)
    return 1;
  if (testthree(set[0], set[2], set[3]) != 0)
    return 1;
  if (testthree(set[0], set[1], set[3]) != 0)
    return 1;
  if (testthree(set[0], set[1], set[2]) != 0)
    return 1;			/* �T�䦨�Ϋhť */

  for (i = 0; i < 3; i++)
  {
    for (j = i + 1; j < 4; j++)
    {
      if (testpair(set[i], set[j]))
      {				/* ��䦳�F�ݥt��䦳�S��ť */
	m = 0;
	for (k = 0; k < 4; k++)
	{
	  if (k != i && k != j)
	  {
	    p[m] = set[k];
	    m++;
	  }
	  if (group[set[i]] == 3 || group[set[i]] == 6)
	    mm = 1;		/* ���F���O�L�Ψ� */
	}
      }
    }
  }
  if (m != 0)
  {
    if ((group[p[0]] == group[p[1]]) && (p[0] != p[1]))
      return 1;			/* ���O pair �t��䦳ť */
    if ((group[p[0]] == group[p[1]] == 3) || (group[p[0]] == group[p[1]] == 6))
      return 1;
    if (testpair(p[0], p[1]) && mm == 1)
      return 1;
  }
  return 0;
}

static int
diecard(a)			/* �Ƕi�@�i�P, �ݬO�_���i */
  int a;
{
  int i, k = 0;
  for (i = 0; i < tflag; i++)
  {
    if (throw[i] == a)
      k++;
    if (throw[i] == 1 && a == 8)
      return 1;
    if (throw[i] == 8 && a == 1)
      return 1;
  }
  if ((a == 7 || a == 14) && k == 4)
    return 1;			/* �L�򵴱i */
  if (k == 1 && (a != 7 && a != 14))
    return 1;
  return 0;
}


static inline int
any_throw()
{
  int i, j, k = 0, set[5] = {0}, tmp[4] = {0};
  int point[5] = {0};	/* point[5] �������t��, �ݥ���i�P����n, ���ư����u���� */

  /* ���ձN��W���䮳���@�� */
  for (i = 0; i < 5; i++)
  {
    k = 0;
    for (j = 0; j < 5; j++)
    {
      if (i != j)
      {
	tmp[k] = host_card[j];
	k++;
      }
    }
    if (testlisten(tmp))	/* �Y�Ѿl���|��wť�P, ��h�����i */
    {
      point[i] += 10;		/* ��ť�N�[ 10 �� */
      if (diecard(host_card[i]))
	point[i] += 5;		/* ���i��ӥ� */
      for (k = 0; k < 4; k++)
      {
	if (((host_card[i] == tmp[k])
	    || (tmp[k] == 1 && host_card[i] == 8)
	    || (tmp[k] == 8 && host_card[i] == 1))
	  && host_card[i] != 7 && host_card[i] != 14)
	  point[i] += 10;
      }
      /* �����]�], �]�ӥ� */
    }
  }
  k = 0;
  for (i = 0; i < 5; i++)	/* �⦳�X��L */
  {
    if (host_card[i] == 7)
      k++;
  }
  if (k == 3)			/* ���T��L: �ѤU�G�䤣�O�L���U�[ 5 �� */
  {
    for (i = 0; i < 5; i++)
      if (host_card[i] != 7)
	point[i] += 5;
  }
  else if (k == 4)		/* ���|��L���� */
  {
    if (diecard(7))		/* ���̫�@��L�w���i: ��L */
    {
      for (i = 0; i < 5; i++)
	if (host_card[i] == 7)
	  point[i] += 999;
    }
    else			/* �̫�@��L�|�����i: �ᤣ�O�L������ */
    {
      for (i = 0; i < 5; i++)
	if (host_card[i] != 7)
	  point[i] += 999;
    }
  }
  k = 0;
  for (i = 0; i < 5; i++)	/* �⦳�X��� */
  {
    if (host_card[i] == 14)
      k++;
  }
  if (k == 3)			/* ���T���: �ѤU�G�䤣�O�򪺦U�[ 5 �� */
  {
    for (i = 0; i < 5; i++)
      if (host_card[i] != 14)
	point[i] += 5;
  }
  else if (k == 4)		/* ���|��򪺸� */
  {
    if (diecard(14))		/* ���̫�@���w���i: ��� */
    {
      for (i = 0; i < 5; i++)
	if (host_card[i] == 14)
	  point[i] += 999;
    }
    else			/* �̫�@���|�����i: �ᤣ�O�򪺨��� */
    {
      for (i = 0; i < 5; i++)
	if (host_card[i] != 14)
	  point[i] += 999;
    }
  }

  for (i = 0; i < 5; i++)
  {
    if (host_card[i] == 7)
      point[i] -= 1;
    if (host_card[i] == 14)
      point[i] -= 1;		/* �L��ɶq���� */
  }

  for (i = 0; i < 4; i++)
  {
    for (j = i + 1; j < 5; j++)
    {
      if (group[host_card[i]] == group[host_card[j]])
      {
	point[i] -= 2;
	point[j] -= 2;		/* �t�@�䦨�T������ */
      }
      if (testpair(host_card[i], host_card[j]))
      {
	point[i] -= 2;
	point[j] -= 2;		/* ���F������ */
      }
    }
  }

#if 1	/* �A��, �p�G��F�|�Q�J�N��������, �A����v 1/2 */
  for (i = 0; i < 4; i++)
    set[i] = guest_card[i];
  for (i = 0; i < 5; i++)
  {
    set[4] = host_card[i];
    if (testall(set) && rnd(2))
      point[i] = -999;
  }
#endif

  /* ��X���Ƴ̰��� */
  j = 0;
  k = point[0];
  for (i = 1; i < 5; i++)
  {
    if (point[i] > k)
    {
      k = point[i];
      j = i;
    }
  }
  return j;
}


static int
count_tai(set)
  int set[5];		/* Ĺ�����i�P */
{
  char *name[10] = 
  {
    "�N�ӹ�", "�N�h�H", "�ӥK��",
    "���L�X�a", "����s��", "�T�L�J�C", "�T��J�C",
    "����", "�ѭJ", "�ۺN"
  };

  int tai[10] = 	/* �x�ƹ����W�����ԭz */
  {
    2, 1, 1,
    5, 5, 2, 2,
    3, 5, 1
  };

  int yes[10] = {0};
  int i, j, k, sum;

  if (selftouch)
    yes[9] = 1;			/* �ۺN */

  if (flag == 32)
    yes[7] = 1;			/* ���� */
  else if (tflag <= 1)
    yes[8] = 1;			/* �ѭJ */

  for (i = 0, j = 0, k = 0; i < 5; i++)
  {
    /* �� ��/�N ����� */
    if (set[i] == 1)
      j++;
    if (set[i] == 8)
      k++;
  }
  if (j)
  {
    if (k)
      yes[0] = 1;		/* ���ӤS���N�N�O �N�ӹ� */
    else
      yes[2] = 1;		/* ���ӨS���N�N�O �ӥK�� */
  }
  else if (k)
  {
    yes[1] = 1;			/* ���N�S���ӴN�O �N�h�H */
  }

  for (i = 0, j = 0; i < 5; i++)
  {
    /* �� �L ����� */
    if (set[i] == 7)
      j++;
  }
  if (j == 5)
    yes[3] = 1;			/* ���L�X�a */
  else if (j == 3)
    yes[5] = 1;			/* �T�L�J�C */

  for (i = 0, j = 0; i < 5; i++)
  {
    /* �� �� ����� */
    if (set[i] == 14)
      j++;
  }
  if (j == 5)
    yes[4] = 1;			/* ����s�� */
  else if (j == 3)
    yes[6] = 1;			/* �T��J�C */

  /* ��x�� */
  sum = 0;
  for (i = 0; i < 10; i++)
  {
    if (yes[i])
      sum += tai[i];
  }

  /* �C�L�X���� */
  move(b_lines - 5, 0);
  outs("�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\n");
  for (i = 0; i < 10; i++)
  {
    if (yes[i])		/* �̦h�|�� */
      prints("  %8s [%d �x] ", name[i], tai[i]);
  }
  move(b_lines - 2, 0);
  clrtoeol();		/* �M�� out_song() */
  prints("        �� [2 �x]       �X�p [%d �x]\n", sum += 2);
  outs("�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}");

  return sum;
}


int
main_chessmj()
{
  int money;			/* ��� */
  int mo;			/* 1:�w�N�P 0:���N�P */
  int picky;			/* 1:�ߧO�H���P 0:�ۤv�N�� */
  int pickup;
  int listen;			/* 1:ť�P 0:�S��ť�P */
  int chesslist[32];		/* 32 �i�P�� */

  int i, j, k, m;
  int jp, x, xx, ch, z;

  char ans[10], msg[40];
  int tmp[4];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  while (1)
  {
    vs_bar("�H�ѳ±N");

    out_song(0);

    vget(2, 0, "�аݭn�U�`�h�֩O�H(1 ~ 50000) ", ans, 6, DOECHO);
    money = atoi(ans);
    if (money < 1 || money > 50000 || money > cuser.money)
      break;			/* ���}��� */
    cuser.money -= money;	/* ���@������A���a�p�G���~���}�N�����^��� */

    out_song(money);
    move(2, 0);
    clrtoeol();		/* �M���u�аݭn�U�`�h�֡v */
    outs("(�� ������P, ����P, �� ENTER �J�P)");

    for (i = 0; i < 32; i++)		/* �P���@�i�@�i�Ʀn�A�ǳƬ~�P */
      chesslist[i] = cnum[i];

    for (i = 0; i < 31; i++)
    {
      j = rnd(32 - i) + i;

      /* chesslist[j] �M chesslist[i] �洫 */
      m = chesslist[i];
      chesslist[i] = chesslist[j];
      chesslist[j] = m;
    }

    selftouch = 0;			/* �k�s */
    mo = 0;
    pickup = 0;
    picky = 0;
    listen = 0;
    flag = 0;
    tflag = 0;

    for (i = 0; i < 4; i++)		/* �o�e�|�i�P */
    {
      host_card[i] = chesslist[flag];
      flag++;
      guest_card[i] = chesslist[flag];
      flag++;
    }
    guest_card[4] = 0;

    sortchess();			/* �Ƨ� */

    move(4, 0);
    outs("�~�w���~�w���~�w���~�w��");
    move(5, 0);
    outs("�x  �x�x  �x�x  �x�x  �x");
    move(6, 0);
    outs("���w�����w�����w�����w��");
    move(15, 0);
    outs("�~�w���~�w���~�w���~�w��");

	print_guest();

    move(17, 0);
    outs("���w�����w�����w�����w��");  /* �L�X�e�|�i�P */

    for (;;)
    {
      jp = 5;
      x = 0;
      z = 1;
      move(18, 26);
      do
      {
	if (!mo)
	{
	  move(14, 24);
	  outs("���ť���N�P(�� �� �ߵP)");
	}
	else
	{
	  move(14, 0);
	  clrtoeol();
	}
	move(18, 2 + (jp - 1) * 6);
	outs("��");
	move(18, 3 + (jp - 1) * 6);		/* �קK���ΰ��� */

	ch = vkey();

	if (!mo && ch != KEY_DOWN && ch != '\n')
	{
	  ch = 'p';		/* �|�i�P�h�j��N�P */
	}

	switch (ch)
	{
	case KEY_RIGHT:
	  move(18, 2 + (jp - 1) * 6);
	  outs("  ");
	  jp += 1;
	  if (jp > 5)
	    jp = 5;
	  move(18, 2 + (jp - 1) * 6);
	  outs("��");
	  move(18, 3 + (jp - 1) * 6);	/* �קK���ΰ��� */
	  break;

	case KEY_LEFT:
	  move(18, 2 + (jp - 1) * 6);
	  outs("  ");
	  jp -= 1;
	  if (jp < 1)
	    jp = 1;
	  move(18, 2 + (jp - 1) * 6);
	  outs("��");
	  move(18, 3 + (jp - 1) * 6);	/* �קK���ΰ��� */
	  break;

	case KEY_UP:		/* �X�P */
	  move(18, 2 + (jp - 1) * 6);
	  outs("  ");
	  throw[tflag] = guest_card[jp - 1];
	  tflag++;
	  z = 0;
	  mo = 0;
	  guest_card[jp - 1] = guest_card[4];
	  guest_card[4] = 0;
	  sortchess();
	  print_guest();
	  print_throw(0);
	  picky = 0;
	  break;

	case 'p':		/* �N�P */
	  if (!mo)
	  {
        if (flag == 32)
        {
          strcpy(msg, "�y��");
          goto next_game;
        }
	    move(18, 2 + (jp - 1) * 6);
	    outs("  ");
	    guest_card[4] = chesslist[flag];
	    flag++;
	    print_guest();
	    mo = 1;
	  }
	  break;

	case KEY_DOWN:
	  if (tflag > 0 && !mo)
	  {
	    guest_card[4] = throw[tflag - 1];
	    throw[tflag - 1] |= 0x80;
	    print_sign(0);
	    print_guest();
	    mo = 1;
	    picky = 1;
	  }
	  break;

	case 'q':
	  return 0;
	  goto abort_game;
	  break;

	case '\n':
	  if (testall(guest_card) && mo && !picky)
	  {
	    selftouch = 1;
	    addmoney(money *= count_tai(guest_card));
	    sprintf(msg, "�z���ۺN�աIĹ�F%d��", money);
	    goto next_game;
	  }
	  else if (picky && testall(guest_card))
	  {
	    addmoney(money *= count_tai(guest_card));
	    sprintf(msg, "�ݧڪ��F�`�A�J�աIĹ�F%d��", money);
	    goto next_game;
	  }

	  if (tflag > 0 && !mo)
	  {
	    i = guest_card[4];
	    guest_card[4] = throw[tflag - 1];
	    if (testall(guest_card) == 1)
	    {
	      print_sign(0);
	      print_guest();
	      addmoney(money *= count_tai(guest_card));
	      sprintf(msg, "�J�IĹ�F%d��", money);
	      goto next_game;
	    }
	    guest_card[4] = i;
	  }
	  break;

	default:
	  break;
	}
      } while (z == 1);

      host_card[4] = throw[tflag - 1];
      if (testall(host_card))
      {
	print_sign(1);	/* �L�ߵP�Ÿ� */
	sprintf(msg, "�q���J�աI��F%d��", money *= count_tai(host_card));
	cuser.money -= money;	/* �q���x�ƶV�h�N�߶V�h�A�B����S�� */
	if (cuser.money < 0)
	  cuser.money = 0;
	goto next_game;
      }

      if (flag == 32)
      {
        host_card[4] = 0;
        strcpy(msg, "�y��");
        goto next_game;
      }

      host_card[4] = chesslist[flag];
      if (testall(host_card))
      {
	selftouch = 1;
	sprintf(msg, "�q���ۺN�I��F%d��", money *= count_tai(host_card));
	cuser.money -= money;	/* �q���x�ƶV�h�N�߶V�h�A�B����S�� */
	if (cuser.money < 0)
	  cuser.money = 0;
	goto next_game;
      }

      for (i = 0; i < 4; i++)
	tmp[i] = host_card[i];

      if (!testlisten(tmp))
      {				/* �Sť���� */
	for (i = 0; i < 4; i++)
	{
	  k = 0;
	  for (j = 0; j < 4; j++)
	  {
	    if (i != j)
	    {
	      tmp[k] = host_card[j];
	      k++;
	    }
	  }
	  tmp[3] = throw[tflag - 1];	/* ��߰_���i���W���P��� */
	  if (testlisten(tmp))
	  {			/* �ߵP��ť���� */
	    listen = 1;
	    host_card[4] = throw[tflag - 1];
	    throw[tflag - 1] |= 0x80;
	    print_sign(1);	/* �L�ߵP�Ÿ� */
	    xx = i;		/* �����U�n�᪺���i�P */
	    pickup = 1;
	    break;		/* ���X i loop */
	  }
	}
      }

      for (i = 0; i < 4; i++)
	tmp[i] = host_card[i];
      if (testlisten(tmp) && !pickup)
      {				/* ��ť�B���S�� */
	m = 0;
	for (i = 0; i < 4; i++)
	  if (tmp[i] == 7)
	    m++;
	if (m == 2 && throw[tflag - 1] == 7)
	  pickup = 1;
	if (m == 3 && throw[tflag - 1] == 7)
	{
	  pickup = 1;
	  for (i = 0; i < tflag - 1; i++)
	    if (throw[i] == 7)
	      pickup = 0;
	}
	m = 0;
	for (i = 0; i < 4; i++)
	  if (tmp[i] == 14)
	    m++;
	if (m == 2 && throw[tflag - 1] == 14)
	  pickup = 1;
	if (m == 3 && throw[tflag - 1] == 14)
	{
	  pickup = 1;
	  for (i = 0; i < tflag - 1; i++)
	    if (throw[i] == 14)
	      pickup = 0;
	}
	if (pickup)
	{
	  host_card[4] = throw[tflag - 1];
	  throw[tflag - 1] |= 0x80;
	  print_sign(1);	/* �L�ߵP�Ÿ� */
	}
      }


      if (!pickup)
      {
	host_card[4] = chesslist[flag];
	flag++;
      }
      /* ���S�ߵP�{�b�N�N�P */

      if (!pickup)
      {
	for (i = 0; i < 4; i++)
	{
	  k = 0;
	  for (j = 0; j < 4; j++)
	  {
	    if (i != j)
	    {
	      tmp[k] = host_card[j];
	      k++;
	    }
	  }
	  tmp[3] = host_card[4];
	  if (testlisten(tmp))
	  {			/* �N�P��ť���� */
	    listen = 1;
	    xx = i;		/* �����U�n�᪺���i�P */
	    break;		/* ���X i loop */
	  }
	}
      }

      for (i = 0; i < 4; i++)
	tmp[i] = host_card[i];

      xx = any_throw();

      throw[tflag] = host_card[xx];
      tflag++;
      host_card[xx] = host_card[4];	/* ��X�Sť���i */
      print_throw(1);

      host_card[4] = 0;
      pickup = 0;
      listen = 0;

    }		/* for �j�鵲�� */

next_game:
    print_host();
    move(b_lines, 0);
    clrtoeol();
    prints("\033[1;37;44m �� %-55s \033[1;33;46m [�Ы����N���~��] \033[m", msg);
    vkey();

  }		/* while �j�鵲�� */

abort_game:
  return 0;
}
#endif				/* HAVE_GAME */
