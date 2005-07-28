/*-------------------------------------------------------*/
/* chessmj.c      ( NTHU CS MapleBBS Ver 3.10 )          */
/*-------------------------------------------------------*/
/* target : �H�ѳ±N�C��                                 */
/* create : 98/07/29                                     */
/* update : 01/04/25                                     */
/* author : weiren@mail.eki.com.tw                       */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_GAME


static int host_card[5];	/* �q��(���a) ���P */
static int guest_card[5];	/* ���a���P */
static int throw[32];		/* �Q��󪺵P */

static int flag;
static int tflag;
static int tflagA;
static int tflagB;
static int selftouch;		/* �ۺN */

static int cnum[32] = 
{
  1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
  7, 7, 7, 7, 7, 8, 9, 9, 10, 10,
  11, 11, 12, 12, 13, 13,
  14, 14, 14, 14, 14
};

static int group[32] = 
{
  1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6
};


static void
out_song()
{
  static int count = 0;

  /* Ĭ�z�ۣ����n�A���R�� */
  uschar *msg[8] = 
  {
    "�ѱ��a�ڪ��ѱ��H  �O�b�N�]�O���X��",
    " �ɶ��w�R�H�@��  �߱��]�S���u�`  ��L�h��^�A�ߤ�",
    "���n�ݧڽֹ�ֿ�  �A���R�b�ڦ^�Ф�  �o�Ǧ~�ߺD�ۥ�",
    "�]�S���Ӧh�м~  ���L�h�N���@�}��",
    "���n�A���R��  ���n�A���R��",
    "�{�b�ڭ̤���  �u����B��",
    "���U�Ӧh���\\  ���L�Ӧh�©]",
    "�{�b�ڤ@�ӤH  �R���ڤ��Q��"
  };
  move(b_lines - 2, 0);
  prints("\033[1;3%dm%s\033[m  �w�X�٦� %d ��", time(0) % 7, msg[count], cuser.money);
  clrtoeol();
  if (++count == 8)
    count = 0;
}


static void
print_Sign(x, y)
  int x, y;
{
  move(x, y);
  outs("�~����");
}


static void
print_Schess(card, x, y)
  int card, x, y;
{
  char *chess[33] = 
  {
    "��", "�K", "�K", "��", "��", "��", "��", "�X", "�X", "��", "��",
    "�L", "�L", "�L", "�L", "�L",
    "�N", "�h", "�h", "�H", "�H", "��", "��", "��", "��", "�]", "�]",
    "��", "��", "��", "��", "��", "��"
  };

  move(x, y);
  outs("�~�w��");
  move(x + 1, y);
  prints("�x%2s�x", chess[card]);
  move(x + 2, y);
  outs("���w��");
}


static inline void
clear_5card()
{
  move(15, 24);
  outs("      ");
  move(16, 24);
  outs("      ");
  move(17, 24);
  outs("      ");
}


static inline void
Phost5()
{
  move(4, 24);
  outs("�~�w��");
  move(5, 24);
  outs("�x  �x");
  move(6, 24);
  outs("���w��");
}


static inline void
Chost5()
{
  move(4, 24);
  outs("      ");
  move(5, 24);
  outs("      ");
  move(6, 24);
  outs("      ");
}


static inline void
P_allchess()
{
  char *chess[33] = {
    "��", "�K", "�K", "��", "��", "��", "��", "�X", "�X", "��", "��",
    "�L", "�L", "�L", "�L", "�L",
    "�N", "�h", "�h", "�H", "�H", "��", "��", "��", "��", "�]", "�]",
    "��", "��", "��", "��", "��", ""
  };

  int i;

  for (i = 0; i < 4; i++)
  {
    move(16, 2 + 6 * i);
    outs(chess[guest_card[i]]);
  }
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
  if (cnum[a] == cnum[b])
    return 1;
  if (cnum[a] == 1 && cnum[b] == 8)
    return 1;
  if (cnum[a] == 8 && cnum[b] == 1)
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
  if (cnum[a] == 1 && cnum[b] == 2 && cnum[c] == 3)
    return 1;			/* �ӥK�� */
  if (cnum[a] == 4 && cnum[b] == 5 && cnum[c] == 6)
    return 1;			/* ���X�� */
  if (cnum[a] == 8 && cnum[b] == 9 && cnum[c] == 10)
    return 1;			/* �N�h�H */
  if (cnum[a] == 11 && cnum[b] == 12 && cnum[c] == 13)
    return 1;			/* �����] */
  if (cnum[a] == 7 && cnum[b] == 7 && cnum[c] == 7)
    return 1;			/* �L�L�L */
  if (cnum[a] == 14 && cnum[b] == 14 && cnum[c] == 14)
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


static void
printhostall()
{
  int i;
  for (i = 0; i < 5; i++)
    print_Schess(host_card[i], 4, 6 * i);
}


static void
printhostfour()
{
  int i;
  for (i = 0; i < 4; i++)
    print_Schess(host_card[i], 4, 6 * i);
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
  } if (j == 0)
    return 1;			/* �|��L */
  j = 0;
  for (i = 0; i < 4; i++)
  {
    if (group[set[i]] != 6)
      j++;
  } if (j == 0)
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
    if ((group[p[0]] == group[p[1]]) && (cnum[p[0]] != cnum[p[1]]))
      return 1;			/* ���O pair �t��䦳ť */
    if ((group[p[0]] == group[p[1]] == 3) || (group[p[0]] == group[p[1]] == 6))
      return 1;
    if (testpair(p[0], p[1]) && mm == 1)
      return 1;
  }
  return 0;
}


static inline void
host_hula()
{
  print_Sign(11, (tflagB - 1) * 4);	/* �L�ߵP�Ÿ� */
  printhostall();
}


static inline void
host_self()
{
  printhostall();
}


static int
diecard(a)			/* �Ƕi�@�i�P, �ݬO�_���i */
  int a;
{
  int i, k = 0;
  for (i = 0; i < tflag; i++)
  {
    if (cnum[throw[i]] == cnum[a])
      k++;
    if (cnum[throw[i]] == 1 && cnum[a] == 8)
      return 1;
    if (cnum[throw[i]] == 8 && cnum[a] == 1)
      return 1;
  }
  if ((cnum[a] == 7 || cnum[a] == 14) && k == 4)
    return 1;			/* �L�򵴱i */
  if (k == 1 && (cnum[a] != 7 && cnum[a] != 14))
    return 1;
  return 0;
}


static inline int
any_throw()
{
  int i, j, k, set[5] = {0}, tmp[4] = {0};
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
	if (((cnum[host_card[i]] == cnum[tmp[k]]) || 
	  (cnum[tmp[k]] == 1 && cnum[host_card[i]] == 8) || 
	  (cnum[tmp[k]] == 8 && cnum[host_card[i]] == 1)) && 
	  cnum[host_card[i]] != 7 && cnum[host_card[i]] != 14)
	  point[i] += 10;
      }
      /* �����]�], �]�ӥ� */
    }
  }

  k = 0;
  for (i = 0; i < 5; i++)
  {
    if (cnum[host_card[i]] == 7)
      k++;			/* �⦳�X��L */
  }
  if (k == 3)		/* ���T��L: �ѤU�G�䤣�O�L���U�[ 5 �� */
  {
    for (i = 0; i < 5; i++)
    {
      if (cnum[host_card[i]] != 7)
	point[i] += 5;
    }
  }
  else if (k == 4)	/* ���|��L */
  {
    if (diecard(12))	/* ���̫�@��L�w���i: ��L */
    {
      for (i = 0; i < 5; i++)
      {
	if (cnum[host_card[i]] == 7)
	  point[i] += 9999;
      }
    }
    else		/* �̫�@��L�|�����i: �ᤣ�O�L������ */
    {
      for (i = 0; i < 5; i++)
      {
	if (cnum[host_card[i]] != 7)
	  point[i] += 9999;
      }
    }
  }

  k = 0;
  for (i = 0; i < 5; i++)
  {
    if (cnum[host_card[i]] == 14)
      k++;			/* �⦳�X��� */
  }
  if (k == 3)		/* ���T���: �ѤU�G�䤣�O�򪺦U�[ 5 �� */
  {
    for (i = 0; i < 5; i++)
    {
      if (cnum[host_card[i]] != 14)
	point[i] += 5;
    }
  }
  else if (k == 4)	/* ���|��� */
  {
    if (diecard(28))	/* ���̫�@���w���i: ��� */
    {
      for (i = 0; i < 5; i++)
      {
	if (cnum[host_card[i]] == 14)
	  point[i] += 9999;
      }
    }
    else		/* �̫�@���|�����i: �ᤣ�O�򪺨��� */
    {
      for (i = 0; i < 5; i++)
      {
	if (cnum[host_card[i]] != 14)
	  point[i] += 9999;
      }
    }
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

#if 1	/* �A��, �p�G��F�|�Q�J�N��������, �A����v 5/6 */
  for (i = 0; i < 4; i++)
    set[i] = guest_card[i];
  for (i = 0; i < 5; i++)
  {
    set[4] = host_card[i];
    if (testall(set) && rnd(6))
      point[i] = -9999;
  }
#endif

  /* ��X���Ƴ̰��� */
  j = 0;
  k = point[0];
  for (i = 1; i < 5; i++)
  {
    if (point[i] >= k)
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
    if (cnum[set[i]] == 1)
      j++;
    if (cnum[set[i]] == 8)
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
    if (cnum[set[i]] == 7)
      j++;
  }
  if (j == 5)
    yes[3] = 1;			/* ���L�X�a */
  else if (j == 3)
    yes[5] = 1;			/* �T�L�J�C */

  for (i = 0, j = 0; i < 5; i++)
  {
    /* �� �� ����� */
    if (cnum[set[i]] == 14)
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
  outs("�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\n");
  for (i = 0; i < 5; i++)
  {
    if (yes[i])
      prints("    %8s [%d �x]", name[i], tai[i]);
  }
  move(b_lines - 3, 0);
  for (i = 5; i < 10; i++)
  {
    if (yes[i])
      prints("    %8s [%d �x]", name[i], tai[i]);
  }
  move(b_lines - 2, 0);
  clrtoeol();		/* �M�� out_song() */
  prints("          �� [2 �x]          �X�p [%d �x]\n", sum += 2);
  outs("�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}");

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

  char ans[10], *msg;
  int tmp[4];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  while (1)
  {
    vs_bar("�H�ѳ±N");

    out_song();

    vget(2, 0, "�аݭn�U�`�h�֩O�H(1 ~ 50000) ", ans, 6, DOECHO);
    money = atoi(ans);
    if (money < 1 || money > 50000 || money > cuser.money)
      break;			/* ���}��� */
    cuser.money -= money;	/* ���@������A���a�p�G���~���}�N�����^��� */

    move(2, 0);
    clrtoeol();		/* �M���u�аݭn�U�`�h�֡v */
    outs("(�� ������P, ����P, �� ENTER �J�P)");

    for (i = 0; i < 32; i++)		/* �P���@�i�@�i�Ʀn�A�ǳƬ~�P */
      chesslist[i] = i;

    for (i = 0; i < 31; i++)
    {
      j = rnd(32 - i) + i;

      /* chesslist[j] �M chesslist[i] �洫 */
      m = chesslist[i];
      chesslist[i] = chesslist[j];
      chesslist[j] = m;
    }

    for (i = 0; i < 32; i++)		/* �٨S���P�Q��� */
      throw[i] = 32;

    selftouch = 0;			/* �k�s */
    mo = 0;
    pickup = 0;
    picky = 0;
    listen = 0;
    flag = 0;
    tflag = 0;
    tflagA = 0;
    tflagB = 0;

    for (i = 0; i < 4; i++)		/* �o�e�|�i�P */
    {
      host_card[i] = chesslist[flag];
      flag++;
      guest_card[i] = chesslist[flag];
      flag++;
    }

    for (i = 0; i < 4; i++)			/* �Ƨ� */
    {
      for (j = 0; j < (3 - i); j++)
      {
	if (guest_card[j] > guest_card[j + 1])
	{
	  m = guest_card[j];
	  guest_card[j] = guest_card[j + 1];
	  guest_card[j + 1] = m;
	}
	if (host_card[j] > host_card[j + 1])
	{
	  m = host_card[j];
	  host_card[j] = host_card[j + 1];
	  host_card[j + 1] = m;
	}
      }
    }

    move(4, 0);
    outs("�~�w���~�w���~�w���~�w��");
    move(5, 0);
    outs("�x  �x�x  �x�x  �x�x  �x");
    move(6, 0);
    outs("���w�����w�����w�����w��");

    for (i = 0; i < 4; i++)
      print_Schess(guest_card[i], 15, 6 * i);	/* �L�X�e�|�i�P */

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
	  outs("�����@��N�P(�� �� �ߵP)");
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

	if (ch != '\n' && flag == 32)
	{
	  msg = "�y��";
	  goto next_game;
	}
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
	  tflagB++;
	  z = 0;
	  mo = 0;
	  guest_card[jp - 1] = guest_card[4];
	  guest_card[4] = 0;
	  sortchess();
	  clear_5card();
	  P_allchess();
	  print_Schess(throw[tflag - 1], 11, (tflagB - 1) * 4);
	  picky = 0;
	  break;

	case 'p':		/* �N�P */
	  if (!mo)
	  {
	    move(18, 2 + (jp - 1) * 6);
	    outs("  ");
	    guest_card[4] = chesslist[flag];
	    flag++;
	    print_Schess(guest_card[4], 15, 24);
	    mo = 1;
	  }
	  break;

	case KEY_DOWN:
	  if (tflag > 0 && !mo)
	  {
	    guest_card[4] = throw[tflag - 1];
	    print_Sign(8, (tflagA - 1) * 4);
	    print_Schess(guest_card[4], 15, 24);
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
	    printhostfour();
	    msg = "�z���ۺN�աI";
	    addmoney(money * count_tai(guest_card));
	    goto next_game;
	  }
	  else if (picky && testall(guest_card))
	  {
	    printhostfour();
	    msg = "�ݧڪ��F�`�A�J�աI";
	    addmoney(money * count_tai(guest_card));
	    goto next_game;
	  }

	  if (tflag > 0 && !mo)
	  {
	    i = guest_card[4];
	    guest_card[4] = throw[tflag - 1];
	    if (testall(guest_card) == 1)
	    {
	      print_Sign(8, (tflagA - 1) * 4);
	      print_Schess(guest_card[4], 15, 24);
	      printhostfour();
	      msg = "�J�I";
	      addmoney(money * count_tai(guest_card));
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
	host_hula();
	msg = "�q���J�աI";
	cuser.money -= money * count_tai(host_card);	/* �q���x�ƶV�h�N�߶V�h�A�B����S�� */
	if (cuser.money < 0)
	  cuser.money = 0;
	goto next_game;
      }

      if (tflag == 32)
      {
        msg = "�y��";
        goto next_game;
      }

      host_card[4] = chesslist[flag];
      if (testall(host_card))
      {
	host_self();
	msg = "�q���ۺN�I";
	cuser.money -= money * count_tai(host_card);	/* �q���x�ƶV�h�N�߶V�h�A�B����S�� */
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
	    tflag--;
	    print_Sign(11, (tflagB - 1) * 4);	/* �L�ߵP�Ÿ� */
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
	  if (cnum[tmp[i]] == 7)
	    m++;
	if (m == 2 && cnum[throw[tflag - 1]] == 7)
	  pickup = 1;
	if (m == 3 && cnum[throw[tflag - 1]] == 7)
	{
	  pickup = 1;
	  for (i = 0; i < tflag - 1; i++)
	    if (cnum[throw[i]] == 7)
	      pickup = 0;
	}
	m = 0;
	for (i = 0; i < 4; i++)
	  if (cnum[tmp[i]] == 14)
	    m++;
	if (m == 2 && cnum[throw[tflag - 1]] == 14)
	  pickup = 1;
	if (m == 3 && cnum[throw[tflag - 1]] == 14)
	{
	  pickup = 1;
	  for (i = 0; i < tflag - 1; i++)
	    if (cnum[throw[i]] == 14)
	      pickup = 0;
	}
	if (pickup)
	{
	  host_card[4] = throw[tflag - 1];
	  tflag--;
	  print_Sign(11, (tflagB - 1) * 4);	/* �L�ߵP�Ÿ� */
	}
      }


      if (!pickup)
      {
	host_card[4] = chesslist[flag];
	flag++;
      }
      /* ���S�ߵP�{�b�N�N�P */
      Phost5();
      Chost5();

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
      tflagA++;
      host_card[xx] = host_card[4];	/* ��X�Sť���i */
      print_Schess(throw[tflag - 1], 8, (tflagA - 1) * 4);

      pickup = 0;
      listen = 0;

    }		/* for �j�鵲�� */

next_game:
    vmsg(msg);

  }		/* while �j�鵲�� */

abort_game:
  return 0;
}
#endif				/* HAVE_GAME */
