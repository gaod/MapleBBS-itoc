/*-------------------------------------------------------*/
/* seven.c      ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : �䫰�C�i�C��                                 */
/* create : 98/07/29                                     */
/* update : 01/04/26                                     */
/* author : weiren@mail.eki.com.tw                       */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_GAME


static char *kind[9] = {"�Q�s", "��F", "�߭F", "�T��", "���l", "�P��", "��Ī","�K��", "�h�B"};
static char *poker[52] = 
{
  "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", 
  "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", 
  "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", 
  "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", 
  "��", "��", "��", "��"
};


static void
out_song()
{
  static int count = 0;

  /* ��Ķ�壻�߸� */
  uschar *msg[8] =
  {
    "�A�S��S��  ����������  �ڲq���X  �]������A���@��",
    "�]�b���b��  �߭W�W����  �ڨ�����  �w���i�Y�@�ئM�I",
    "��G�C�C�o���y�o�S  �I�l�ϩ��n���F",
    "�����եդ��O��³��w  ���O�R  ���O�R��",
    "ť��ۤv�߸�  ���֤߸�  ���n�몺�_��  ����ۧڤ����",
    "�h�Q  ��ߥ洫  �A�N��ƺ�  �A�N�|���A  �߸�  ������",
    "��G�C�C�o���y�o�S  �I�l�ϩ��n���F",
    "�����եդ��O��³��w  ���D�N�O�R  ���D�O�R"
  };
  move(b_lines - 2, 0);
  prints("\033[1;3%dm%s\033[m  �w�X�٦� %d ��", time(0) % 7, msg[count], cuser.money);
  clrtoeol();
  if (++count == 8)
    count = 0;
}


static inline int
find_pair(set)			/* �� Pair �N�Ǧ^ 1 */
  int set[6];
{
  int i, j;

  for (i = 0; i < set[5] - 1; i++)
  {
    for (j = i + 1; j < set[5]; j++)
    {
      if (set[j] / 4 == set[i] / 4)
	return 1;
    }
  }
  return 0;
}


static inline int
find_tpair(set)			/* Two Pair �Ǧ^ 1 */
  int set[6];
{
  int i, j, k;
  int z[13] = {0};

  for (i = 0; i < 13; i++)
  {
    for (j = 0; j < 5; j++)
    {
      if (set[j] / 4 == i)
	z[i]++;
    }
  }
  k = 0;
  for (i = 0; i < 13; i++)
  {
    if (z[i] >= 2)
      k++;
  }
  if (k == 2)
    return 1;
  return 0;
}


static inline int
find_triple(set)		/* �T���Ǧ^ 3, �K��Ǧ^ 4 */
  int set[6];
{
  int i, j, k;

  for (i = 0; i < 13; i++)
  {
    k = 0;
    for (j = 0; j < 5; j++)
    {
      if (set[j] / 4 == i)
	k++;
    }
    if (k == 4)
      return 4;
    if (k == 3)
      return 3;
  }
  return 0;
}


static inline int
find_dragon(set)		/* ���Ǧ^ 1, �_�h�Ǧ^ 0 */
  int set[6];
{
  int i;
  int test[6];

  for (i = 0; i < 5; i++)
    test[i] = set[i] / 4;

  for (i = 0; i < 3; i++)
  {
    if (test[i] + 1 != test[i + 1])
      return 0;
  }

  if (test[4] == 12 && test[0] == 0)
    return 1;			/* A2345 �� */

  if (test[3] + 1 == test[4])
    return 1;			/* �@�붶 */
  return 0;
}


static inline int
find_flush(set)			/* �P��Ǧ^ 1, �_�h�Ǧ^ 0 */
  int set[6];
{
  int i;
  int test[6];

  for (i = 0; i < 5; i++)
    test[i] = set[i] % 4;

  for (i = 1; i < 5; i++)
  {
    if (test[0] != test[i])
      return 0;
  }

  return 1;
}


static int
find_all(set)
  int set[6];
{
  int i;
  int a[9];		/* �Q�s, �F , �߭F, �T��, ��, �P��, �J�c, �K��, �P�ᶶ */

  a[0] = 1;		/* a[0]  1    2     3     4    5    6     7     a[8]   */

  for (i = 1; i < 9; i++)
    a[i] = 0;

  a[1] = find_pair(set);
  a[2] = find_tpair(set);

  switch (find_triple(set))
  {
  case 3:
    a[3] = 1;
    break;

  case 4:
    a[7] = 1;
    break;
  }

  a[4] = find_dragon(set);
  a[5] = find_flush(set);

  if (a[2] && a[3])
    a[6] = 1;			/* �߭F + �T�� = �J�c */
  if (a[4] && a[5])
    a[8] = 1;			/* �P�� + �� = �P�ᶶ */

  for (i = 8; i >= 0; i--)
    if (a[i])
      return i;
}


static inline int
diedragon(set, a, b)
  int set[6];
  int a, b;
{
  int card[13] = {0};
  int first[2];
  int i, z;

  first[0] = a;
  first[1] = b;
  z = find_all(set);

  if (!z)
  {				/* �ĤG���Q�s */
    if (first[0] / 4 == first[1] / 4)
      return 1;
    if (first[1] / 4 > set[4] / 4)
      return 1;
    if (first[1] / 4 == set[4] / 4)
    {
      if (first[0] / 4 > set[3] / 4)
	return 1;
    }				/* ���s */
  }

  else if (z == 1)		/* �F */
  {
    for (i = 0; i < 5; i++)
      card[set[i] / 4]++;

    for (i = 0; i < 13; i++)
    {
      if (card[i] == 2 && first[0] / 4 == first[1] / 4 && first[0] / 4 > i)
	return 1;		/* �������F�B���s */
    }
  }
  return 0;
}


static inline int
bigsmall(h, g, key, gm)
  int h[7], g[7], key, gm[2];
{
  int hm[2];
  int i, j, k = 0, tmp = 0, tmp2 = 0, x, a, b;
  int duA = 0, duB = 0;		/* duA duB �O����P�w��Ĺ�Ѽ�, 1 �O�q��Ĺ */
  int hset[6], gset[6];	/* host, guest */
  int gc[13] = {0}, hc[13] = {0};

  for (i = 0; i < 6; i++)
  {
    for (j = i + 1; j < 7; j++)
    {
      if (key == k)
      {
	hm[0] = i;
	hm[1] = j;
      };
      k++;
    }
  }

  if (hm[1] < hm[0])
  {
    k = hm[1];
    hm[1] = hm[0];
    hm[0] = k;
  }

  if (gm[1] < gm[0])
  {
    k = gm[1];
    gm[1] = gm[0];
    gm[0] = k;
  }

  if (h[hm[0]] / 4 == h[hm[1]] / 4)
    tmp = 1;
  if (g[gm[0]] / 4 == g[gm[1]] / 4)
    tmp2 = 1;
  if (tmp == tmp2)
  {
    if (h[hm[1]] / 4 > g[gm[1]] / 4)
      duA = 1;			/* duA=1 ��ܲĤ@�����aĹ */
    if (h[hm[1]] / 4 == g[gm[1]] / 4 && tmp == 1)
      duA = 1;			/* �Ĥ@�����F�B����, ���aĹ */
    if (h[hm[1]] / 4 == g[gm[1]] / 4 && tmp == 0 && h[hm[0]] / 4 >= g[gm[0]] / 4)
      duA = 1;
  }
  if (tmp > tmp2)
    duA = 1;
  if (tmp < tmp2)
    duA = 0;
  k = 0;
  j = 0;

  for (i = 0; i < 7; i++)
  {
    if (i != hm[0] && i != hm[1])
    {
      hset[j] = h[i];
      j++;
    }
    if (i != gm[0] && i != gm[1])
    {
      gset[k] = g[i];
      k++;
    }
  }

  hset[5] = 5;
  gset[5] = 5;
  tmp = find_all(hset);
  tmp2 = find_all(gset);

  if (tmp > tmp2)
  {
    duB = 1;
  }
  else if (tmp == tmp2)
  {
    for (i = 0; i < 5; i++)
    {
      gc[gset[i] / 4]++;
      hc[hset[i] / 4]++;
    }
    switch (tmp)
    {
    case 0:
      i = 12;
      x = 0;
      duB = 1;			/* ��賣�O�Q�s */
      do
      {
	if (hc[i] > gc[i])
	{
	  duB = 1;
	  x = 1;
	}
	if (hc[i] < gc[i])
	{
	  duB = 0;
	  x = 1;
	}
	i--;
	if (i < 0)
	  x = 1;
      } while (x == 0);
      break;

    case 1:
      for (i = 0; i < 12; i++)
      {
	if (hc[i] == 2)
	  a = i;
	if (gc[i] == 2)
	  b = i;
      }
      if (a > b)
      {
	duB = 1;		/* ��賣�O�F */
      }
      else if (a == b)
      {
	i = 12;
	j = 12;
	x = 0;
	duB = 1;
	do
	{
	  if (hc[i] == 2)
	    i--;
	  if (hc[j] == 2)
	    j--;
	  if (hc[i] > gc[j])
	  {
	    duB = 1;
	    x = 1;
	  }
	  if (hc[i] < gc[j])
	  {
	    duB = 0;
	    x = 1;
	  }
	  i--;
	  j--;
	  if (i < 0 || j < 0)
	    x = 1;
	} while (x == 0);
      }
      break;

    case 2:
      i = 12;
      x = 0;
      duB = 2;			/* ��賣�O�߭F */
      do
      {
	if (hc[i] > gc[i] && hc[i] != 1)
	{
	  duB = 1;
	  x = 1;
	};
	if (hc[i] < gc[i] && gc[i] != 1)
	{
	  duB = 0;
	  x = 1;
	};
	i--;
	if (i < 0)
	  x = 1;
      } while (x == 0);
      if (duB == 2)
      {
	for (i = 0; i < 12; i++)
	{
	  if (hc[i] == 1)
	    a = i;
	  if (gc[i] == 1)
	    b = i;
	}
	duB = 1;
	if (a < b)
	  duB = 0;
      }
      break;

    case 3:
    case 6:
      for (i = 0; i < 12; i++)
      {
	if (hc[i] == 3)
	  a = i;
	if (gc[i] == 3)
	  b = i;
      }
      if (a > b)
	duB = 1;		/* ��賣�O�T��(�J�c) */
      else if (a < b)
	duB = 0;
      break;

    case 4:
      i = 12;
      x = 0;
      a = 0;
      b = 0;			/* ��賣�O���l */
      do
      {
	if (hc[i] > gc[i])
	{
	  duB = 1;
	  x = 1;
	}
	if (hc[i] < gc[i])
	{
	  duB = 0;
	  x = 1;
	}
	i--;
	if (i < 0)
	{
	  duB = 1;
	  x = 1;
	}
      } while (x == 0);

      if (hc[12] == hc[0] && hc[0] == 1)
	a = 1;
      if (gc[12] == gc[0] && gc[0] == 1)
	b = 1;
      if (a > b)
	duB = 0;
      if (a < b)
	duB = 1;
      if (a == b && b == 1)
	duB = 1;
      break;

    case 5:
      if (hset[0] % 4 > gset[0] % 4)
	duB = 1;		/* ��賣�O�P�� */
      if (hset[0] % 4 < gset[0] % 4)
	duB = 0;
      if (hset[0] % 4 == gset[0] % 4)
      {
	if (hset[4] > gset[4])
	  duB = 1;
	if (hset[4] < gset[4])
	  duB = 0;
      }
      break;

    case 7:
      for (i = 0; i < 12; i++)
      {
	if (hc[i] == 4)
	  a = i;
	if (gc[i] == 4)
	  b = i;
      }
      if (a > b)
	duB = 1;		/* ��賣�O�K�� */
      if (a < b)
	duB = 0;
      break;

    case 8:
      if (hset[0] % 4 > gset[0] % 4)
	duB = 1;		/* ��賣�O�P�ᶶ */
      if (hset[0] % 4 < gset[0] % 4)
	duB = 0;
      if (hset[0] % 4 == gset[0] % 4)
      {
	i = 12;
	x = 0;
	do
	{
	  if (hc[i] > gc[i])
	  {
	    duB = 1;
	    x = 1;
	  }
	  if (hc[i] < gc[i])
	  {
	    duB = 0;
	    x = 1;
	  }
	  i--;
	  if (i < 0)
	  {
	    duB = 1;
	    x = 1;
	  }
	} while (x == 0);
      }
      break;
    }
  }
  return 2 * duA + duB;
}


static void
print_Scard(card, x, y)
  int card, x, y;
{
  char *flower[4] = {"��", "��", "��", "��"};

  move(x, y);
  outs("�~�w�w�w��");
  move(x + 1, y);
  prints("�x%s    �x", poker[card]);
  move(x + 2, y);
  prints("�x%s    �x", flower[card % 4]);
  move(x + 3, y);
  outs("�x      �x");
  move(x + 4, y);
  outs("�x      �x");
  move(x + 5, y);
  outs("�x      �x");
  move(x + 6, y);
  outs("���w�w�w��");
}


static inline void
print_hostcard(card, x)		/* x ����i���զX key */
  int card[7];
  int x;
{
  int i, j, k = 0;
  int tmp, tmp2;
  int set[6];

  for (i = 1; i < 6; i++)
  {
    move(5 + i, 0);
    clrtoeol();
  }

  for (i = 0; i < 6; i++)
  {
    for (j = i + 1; j < 7; j++)
    {
      if (x == k)
      {
	tmp = i;
	tmp2 = j;
      };
      k++;
    }
  }

  print_Scard(card[tmp], 3, 0);
  print_Scard(card[tmp2], 3, 4);

  j = 0;
  for (i = 0; i < 7; i++)
  {
    if (i != tmp && i != tmp2)
    {
      print_Scard(card[i], 3, 16 + j * 4);
      set[j] = card[i];
      j++;
    }
  }
  set[5] = 5;
  move(7, 4);
  if (card[tmp] / 4 == card[tmp2] / 4)
    x = 1;
  prints("\033[1;44;33m   %s%s   \033[m  �x  �x  \033[1;44;33m         %s         \033[m",
    poker[card[tmp]], x == 1 ? "�F" : poker[card[tmp2]] ,kind[find_all(set)]);
}


static inline int
score(first, set)	/* �^�Ǥ�����᪺����(AI), �q���|�� 21 �صP������X��, ���������� */
  int first[2], set[6];
{
  int i, z;
  int points = 0;
  int card[13] = {0};

  z = find_all(set);
  if (z == 0)
  {
    if (first[0] / 4 == first[1] / 4)
      return 0;
    if (first[1] / 4 >= set[4] / 4)
      return 0;			/* ���s */
  }
  else if (z == 1)
  {
    for (i = 0; i < 5; i++)
      card[set[i] / 4]++;
    for (i = 0; i < 13; i++)
    {
      if (card[i] == 2 && first[0] / 4 == first[1] / 4 && first[0] / 4 >= i)
	return 0;		/* �������F�B���s */
    }
  }

  points = z + 2;		/* �ĤG���Q�s�N����, �H�W���W */
  if (points >= 5)
    points ++;			/* �ĤG���Y�����H�W�A�[�@�� */
  if (first[0] / 4 == first[1] / 4)
    points += 3;		/* �Ĥ@�����F���ƥ[�T */
  if (first[0] / 4 != first[1] / 4 && first[1] / 4 >= 10)
    points++;
  /* �Ĥ@���L�F�� Q �H�W�[�@�� */
  if (first[0] / 4 == 12 || first[1] / 4 == 12)
    points += 1;		/* �Ĥ@���� A ���ƦA�[�@ */
  return points;
}


static inline int 
find_host(h)			/* �Ǧ^��i���զX key */
  int h[7];
{
  int i, j, k, x = 0, z = 0;
  int tmp = 0, tmp2 = 0;  
  int result[21] = {0}, set[6] = {0}, first[2];

  for (i = 0; i < 6; i++)
  {
    for (j = i + 1; j < 7; j++)
    {
      first[0] = h[i];
      first[1] = h[j];
      x = 0;
      for (k = 0; k < 7; k++)
      {
	if (i != k && j != k)
	{
	  set[x] = h[k];
	  x++;
	}
      }
      set[5] = 5;
      result[z] = score(first, set);
      z++;
    }
  }
  for (i = 0; i < 21; i++)
  {
    if (result[i] >= tmp)
    {
      tmp = result[i];
      tmp2 = i;
    }
  }
  return tmp2;
}


static int
get_newcard(mode)
  int mode;			/* 0:���s�~�P  1:�o�P */
{
  static int card[14];	/* �̦h�u�|�Ψ� 14 �i�P */
  static int now;	/* �o�X�� now �i�P */
  char num;
  int i;

  if (!mode)	/* ���s�~�P */
  {
    now = 0;
    return -1;
  }

rand_num:		/* random �X�@�i�M���e�����P���P */
  num = rnd(52);
  for (i = 0; i < now; i++)
  {
    if (num == card[i])	/* �o�i�P�H�e random �L�F */
      goto rand_num;
  }

  card[now] = num;
  now++;

  return num;
}


int 
main_seven()
{
  int money;		/* ��� */
  int host_card[7];	/* �q���� 7 �i�P�i */
  int guest_card[7];	/* ���a�� 7 �i�P�i */
  int mark[2];		/* ���a�аO�� */
  int set[6];

  int i, j, win;
  char buf[10];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  while (1)
  {
    vs_bar("�䫰�C�i");
    out_song();

    vget(2, 0, "�аݭn�U�`�h�֩O�H(1 ~ 50000) ", buf, 6, DOECHO);
    money = atoi(buf);
    if (money < 1 || money > 50000 || money > cuser.money)
      break;			/* ���}��� */

    cuser.money -= money;

    out_song();

    move(2, 0);
    clrtoeol();		/* �M���u�аݭn�U�`�h�֡v */
    outs("(�� ���� ���ʡA�� ���� ����G�i�P�A��n�� enter �u�P)");

    get_newcard(0);	/* �~�P */

    mark[0] = mark[1] = 123;	/* mark[?] = 123 ��ܨS�� mark */

    /* �o�Q�|�i�P */
    for (i = 0; i < 7; i++)
    {
      host_card[i] = get_newcard(1);
      guest_card[i] = get_newcard(1);
    }

    /* �Ƨ� */
    for (i = 0; i < 7; i++)
    {
      for (j = 0; j < (6 - i); j++)
      {
        /* �ɥ� win */
	if (guest_card[j] > guest_card[j + 1])
	{
	  win = guest_card[j];
	  guest_card[j] = guest_card[j + 1];
	  guest_card[j + 1] = win;
	}
	if (host_card[j] > host_card[j + 1])
	{
	  win = host_card[j];
	  host_card[j] = host_card[j + 1];
	  host_card[j + 1] = win;
	}
      }
    }

    /* �L�X��W���P */
    move(3, 0);
    outs("�~�w�~�w�~�w�~�w�~�w�~�w�~�w�w�w��\n");
    outs("�x  �x  �x  �x  �x  �x  �x      �x\n");
    outs("�x  �x  �x  �x  �x  �x  �x      �x\n");
    outs("�x  �x  �x  �x  �x  �x  �x      �x\n");
    outs("�x  �x  �x  �x  �x  �x  �x      �x\n");
    outs("�x  �x  �x  �x  �x  �x  �x      �x\n");
    outs("���w���w���w���w���w���w���w�w�w��");
    for (i = 0; i < 7; i++)
      print_Scard(guest_card[i], 11, 0 + 4 * i);

    i = j = 0;
    for (;;)	/* ��X�G�i�P */
    {
      /* �b���j�餺�Ai �O�ĴX�i�P�Aj �O�w����F�X�i�P */
      move(15, 1 + i * 4);
      switch (vkey())
      {
      case KEY_RIGHT:
	if (i < 6)
	  i++;
	break;

      case KEY_LEFT:
	if (i > 0)
	  i--;
	break;

      case KEY_UP:		/* ����o�i�P */
        if (j < 2 && mark[0] != i && mark[1] != i)	/* ���୫�� mark �B�̦h mark �G�i */
	{
	  if (mark[0] == 123)
	    mark[0] = i;
	  else
	    mark[1] = i;
	  j++;
	  move(15, 2 + i * 4);
	  outs("��");
	}
	break;

      case KEY_DOWN:		/* ��������o�i�P */
	if (mark[0] == i)
	{
	  mark[0] = 123;
	  j--;
	  move(15, 2 + i * 4);
	  outs("  ");
	}
	else if (mark[1] == i)
	{
	  mark[1] = 123;
	  j--;
	  move(15, 2 + i * 4);
	  outs("  ");
	}
	break;

      case '\n':		/* ��X��i��� enter */
	if (j == 2)
	  goto end_choose;
	break;
      }
    }
  end_choose:

    if (mark[0] > mark[1])
    {
      i = mark[0];
      mark[0] = mark[1];
      mark[1] = i;
    }

    /* �L�X���a���n����᪺�P */
    for (i = 1; i < 18; i++)
    {
      move(i, 0);
      clrtoeol();
    }
    print_Scard(guest_card[mark[0]], 11, 0);
    print_Scard(guest_card[mark[1]], 11, 4);

    j = 0;
    for (i = 0; i < 7; i++)
    {
      if (i != mark[0] && i != mark[1])
      {
	print_Scard(guest_card[i], 11, 16 + j * 4);
	set[j] = guest_card[i];
	j++;
      }
    }

    /* �P�_�O�_���s�A�Y���s�h���� */
    set[5] = 5;
    if (diedragon(set, guest_card[mark[0]], guest_card[mark[1]]))
    {
      vmsg("���s");
      continue;
    }

    /* �P�_�ӭt */
    i = find_host(host_card);
    print_hostcard(host_card, i);
    win = bigsmall(host_card, guest_card, i, mark);

    /* �q�X���G */
    switch (win)
    {
      /* �ɥ� i �ӷ� color1; �ɥ� j �ӷ� color2 */

    case 0:	/* ���a duA duB ��Ĺ  */
      win = 2;
      i = 41;
      j = 41;
      break;

    case 1:	/* ���a duA Ĺ duB �� */
      win = 1;
      i = 41;
      j = 47;
      break;

    case 2:	/* ���a duA �� duB Ĺ */
      win = 1;
      i = 47;
      j = 41;
      break;

    case 3:	/* ���a duA duB �ҿ� */
      win = 0;
      i = 47;
      j = 47;
      break;
    }

    move(15, 4);
    prints("\033[1;%d;%dm   %s%s   \033[m  �x  �x  \033[1;%d;%dm         %s         \033[m",
      i, i == 41 ? 33 : 30, poker[guest_card[mark[0]]], 
      (guest_card[mark[0]] / 4 == guest_card[mark[1]] / 4) ? "�F" : poker[guest_card[mark[1]]],
      j, j == 41 ? 33 : 30, kind[find_all(set)]);

    switch (win)
    {
    case 2:
      vmsg("�zĹ�F");
      money *= 2;
      break;

    case 1:
      vmsg("����");
      break;

    case 0:
      vmsg("�z��F");
      money = 0;
      break;
    }
    addmoney(money);
  }
  return 0;
}
#endif	/* HAVE_GAME */
