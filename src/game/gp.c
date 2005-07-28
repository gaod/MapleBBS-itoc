/*-------------------------------------------------------*/
/* gp.c		( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : �����J�����C��                               */
/* create : 98/10/24                                     */
/* update : 01/04/21                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#if 0
             -=== �����J�����C�� ===-

        1. ���k���������A��q����j�A�i�[���I
        2. �i�H�N������U�@������`�C

        �j�p�G
        �P�ᶶ���K�K�ָ�Ī�֦P��ֶ��l�֤T���֨߭F�ֳ�F�ֳ�i

        �S��[���G
        �P�ᶶ  ������
        �|  �i  ������
        ���@Ī�@�@����

#endif


#include "bbs.h"


#ifdef HAVE_GAME

#define MAX_CHEAT	2	/* �q���@���h���P���� (0:���@���A�̦h�i�@�� 6 ��) */

static char mycard[5];		/* �ڪ� 5 �i�P */
static char cpucard[5];		/* �q�� 5 �i�P */


static void
out_song()
{
  static int count = 0;

  /* �P�ذ����B�� */
  uschar *msg[7] = 
  {
    "�o�Ǧ~  �@�ӤH  ���]�L  �B�]��",
    "���L�\\  ���L��  �ٰO�o�������",
    "�u�R�L  �~�|��  �|�I��  �|�^��",
    "�צ���  �צ��A  �b�ߤ�",
    "�B�ͤ@�ͤ@�_��  ���Ǥ�l���A��",
    "�@�y��  �@���l  �@�ͱ�  �@�M�s",
    "�B�ͤ����t��L  �@�n�B�ͧA�|��"
  };
  move(b_lines - 2, 0);
  prints("\033[1;3%dm%s\033[m  �w�X�٦� %d ��", time(0) % 7, msg[count], cuser.money);
  clrtoeol();
  if (++count == 7)
    count = 0;
}


static void
show_card(isDealer, c, x)
  int isDealer;		/* 1:�q��  2:���a */
  char c;		/* �P�i */
  int x;		/* �ĴX�i�P */
{
  int beginL;
  char *suit[4] = {"��", "��", "��", "��"};
  char *num[13] = {"��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��"};

  beginL = (isDealer) ? 2 : 12;
  move(beginL, x * 4);
  outs("�~�w�w�w��");
  move(beginL + 1, x * 4);
  prints("�x%2s    �x", num[c % 13]);
  move(beginL + 2, x * 4);
  prints("�x%2s    �x", suit[c / 13]);
  move(beginL + 3, x * 4);
  outs("�x      �x");
  move(beginL + 4, x * 4);
  outs("�x      �x");
  move(beginL + 5, x * 4);
  outs("�x      �x");
  move(beginL + 6, x * 4);
  outs("���w�w�w��");
}


/* �P�ᶶ�B�K�K�B���B�P��B���B�T���B�߭F�B�F�B�@�� */
static void
show_style(my, cpu)
  int my, cpu;
{
  char *style[9] = {"�P�ᶶ", "�|�i", "��Ī", "�P��", "���l", "�T��", "�߭F", "��F", "�@�i"};

  move(5, 26);
  prints("\033[41;37;1m%s\033[m", style[cpu - 1]);
  move(15, 26);
  prints("\033[41;37;1m%s\033[m", style[my - 1]);
}


static int
card_cmp(a, b)
  char *a, *b;
{
  /* 00~12: C, KA23456789TJQ
     13~25: D, KA23456789TJQ
     26~38: H, KA23456789TJQ
     39~51: S, KA23456789TJQ */

  char c = (*a) % 13;
  char d = (*b) % 13;

  if (c == 0)
    c = 13;
  else if (c == 1)
    c = 14;
  if (d == 0)
    d = 13;
  else if (d == 1)
    d = 14;

  /* �����I�ơA�A���� */
  if (c == d)
    return *a - *b;
  return c - d;
}


/* a �O�I�� .. b �O��� */
static void
tran(a, b, c)
  char *a, *b, *c;
{
  int i;
  for (i = 0; i < 5; i++)
  {
    a[i] = c[i] % 13;
    if (!a[i])
      a[i] = 13;
  }

  for (i = 0; i < 5; i++)
    b[i] = c[i] / 13;
}


static void
check(p, q, r, cc)
  char *p, *q, *r, *cc;
{
  char i;

  for (i = 0; i < 13; i++)
    p[i] = 0;
  for (i = 0; i < 5; i++)
    q[i] = 0;
  for (i = 0; i < 4; i++)
    r[i] = 0;

  for (i = 0; i < 5; i++)
    p[cc[i] % 13]++;

  for (i = 0; i < 13; i++)
    q[p[i]]++;

  for (i = 0; i < 5; i++)
    r[cc[i] / 13]++;
}


/* �P�ᶶ�B�K�K�B���B�P��B���B�T���B�߭F�B�F�B�@�� */
static int
complex(cc, x, y)
  char *cc, *x, *y;
{
  char p[13], q[5], r[4];
  char a[5], b[5], c[5], d[5];
  int i, j, k;

  tran(a, b, cc);
  check(p, q, r, cc);

  /* �P�ᶶ */
  if ((a[0] == a[1] - 1 && a[1] == a[2] - 1 && a[2] == a[3] - 1 && a[3] == a[4] - 1) &&
    (b[0] == b[1] && b[1] == b[2] && b[2] == b[3] && b[3] == b[4]))
  {
    *x = a[4];
    *y = b[4];
    return 1;
  }

  if (a[4] == 1 && a[0] == 2 && a[1] == 3 && a[2] == 4 && a[3] == 5 &&
    (b[0] == b[1] && b[1] == b[2] && b[2] == b[3] && b[3] == b[4]))
  {
    *x = a[3];
    *y = b[4];
    return 1;
  }

  if (a[4] == 1 && a[0] == 10 && a[1] == 11 && a[2] == 12 && a[3] == 13 &&
    (b[0] == b[1] && b[1] == b[2] && b[2] == b[3] && b[3] == b[4]))
  {
    *x = 1;
    *y = b[4];
    return 1;
  }

  /* �K�K */
  if (q[4] == 1)
  {
    for (i = 0; i < 13; i++)
    {
      if (p[i] == 4)
	*x = i ? i : 13;
    }
    return 2;
  }

  /* ��Ī */
  if (q[3] == 1 && q[2] == 1)
  {
    for (i = 0; i < 13; i++)
    {
      if (p[i] == 3)
	*x = i ? i : 13;
    }
    return 3;
  }

  /* �P�� */
  for (i = 0; i < 4; i++)
  {
    if (r[i] == 5)
    {
      *x = i;
      return 4;
    }
  }

  /* ���l */
  memcpy(c, a, 5);
  memcpy(d, b, 5);
  for (i = 0; i < 4; i++)
  {
    for (j = i; j < 5; j++)
    {
      if (c[i] > c[j])
      {
	k = c[i];
	c[i] = c[j];
	c[j] = k;
	k = d[i];
	d[i] = d[j];
	d[j] = k;
      }
    }
  }

  if (10 == c[1] && c[1] == c[2] - 1 && c[2] == c[3] - 1 && c[3] == c[4] - 1 && c[0] == 1)
  {
    *x = 1;
    *y = d[0];
    return 5;
  }

  if (c[0] == c[1] - 1 && c[1] == c[2] - 1 && c[2] == c[3] - 1 && c[3] == c[4] - 1)
  {
    *x = c[4];
    *y = d[4];
    return 5;
  }

  /* �T�� */
  if (q[3] == 1)
  {
    for (i = 0; i < 13; i++)
    {
      if (p[i] == 3)
      {
	*x = i ? i : 13;
	return 6;
      }
    }
  }

  /* �߭F */
  if (q[2] == 2)
  {
    for (*x = 0, i = 0; i < 13; i++)
    {
      if (p[i] == 2)
      {
	if ((i > 1 ? i : i + 13) > (*x == 1 ? 14 : *x))
	{
	  *x = i ? i : 13;
	  *y = 0;
	  for (j = 0; j < 5; j++)
	  {
	    if (a[j] == i && b[j] > *y)
	      *y = b[j];
	  }
	}
      }
    }
    return 7;
  }

  /* ��F */
  if (q[2] == 1)
  {
    for (i = 0; i < 13; i++)
    {
      if (p[i] == 2)
      {
	*x = i ? i : 13;
	*y = 0;
	for (j = 0; j < 5; j++)
	  if (a[j] == i && b[j] > *y)
	    *y = b[j];
	return 8;
      }
    }
  }

  /* �@�i */
  *x = 0;
  *y = 0;
  for (i = 0; i < 5; i++)
  {
    if ((a[i] = a[i] ? a[i] : 13 > *x || a[i] == 1) && *x != 1)
    {
      *x = a[i];
      *y = b[i];
    }
  }
  return 9;
}


static int	/* <0:���aĹ�P <-1000:���a�S��Ĺ�P >0:�q��Ĺ�P */
gp_win(my, cpu)
  int *my, *cpu;	/* �Ǧ^���a�M�q�����P�� */
{
  int ret;
  char myX, myY, cpuX, cpuY;

  *my = complex(mycard, &myX, &myY);
  *cpu = complex(cpucard, &cpuX, &cpuY);

  if (*my != *cpu)		/* �p�G�P�����P�A��������P���j�p */
    ret = *my - *cpu;
  else if (myX == 1 && cpuX != 1)
    ret = -1;
  else if (myX != 1 && cpuX == 1)
    ret = 1;
  else if (myX != cpuX)
    ret = cpuX - myX;
  else if (myY != cpuY)
    ret = cpuY - myY;

  if (ret < 0)		/* �p�G���aĹ�P */
  {
    switch (*my)
    {
    case 1:		/* �P�ᶶ */
      ret = -1001;
      break;
    case 2:		/* �K�K */
      ret = -1002;
      break;
    case 3:		/* ��Ī */
      ret = -1003;
      break;
    }
  }

  return ret;
}


static char
get_newcard(mode)
  int mode;		/* 0:���s�~�P  1:�o�P */
{
  static char card[20 + 5 * MAX_CHEAT];	/* �̦h�u�|�Ψ� 20+5*MAX_CHEAT �i�P */
  static int now;			/* �o�X�� now �i�P */
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


static int
cpu_doing()
{
  int my, cpu;
  int i, j, k;
  char hold[5];
  char p[13], q[5], r[4];
  char a[5], b[5];

  for (i = 0; i < 5; i++)
  {
    cpucard[i] = get_newcard(1);
    hold[i] = 0;
  }
  qsort(cpucard, 5, sizeof(char), card_cmp);
  for (i = 0; i < 5; i++)
    show_card(1, cpucard[i], i);

  tran(a, b, cpucard);
  check(p, q, r, cpucard);

  /* �Y���S��P���A�h�O�d */
  k = 0;	/* 1:���S��P�� */
  for (j = 0; j < 13; j++)
  {
    if (p[j] > 1)
    {
      for (i = 0; i < 5; i++)
      {
	if (j == cpucard[i] % 13)
	{
	  hold[i] = 1;
	  k = 1;
	}
      }
    }
  }

  for (i = 0; i < 5; i++)
  {
    /* �p�G�S���S��P���A����O�d A�BK�A�_�h�������O�d */
    if (!k && (a[i] == 13 || a[i] == 1))
      hold[i] = 1;

    move(6, i * 4 + 2);
    outs(hold[i] ? "�O" : "  ");
    move(7, i * 4 + 2);
    outs(hold[i] ? "�d" : "  ");
  }

  vmsg("�q�����P�e..");

  for (j = 0; j < 1 + MAX_CHEAT; j++)	/* ���P�@���B�@�� MAX_CHEAT �� */
  {
    /* �q�����P */
    for (i = 0; i < 5; i++)
    {
      if (!hold[i])
	cpucard[i] = get_newcard(1);
    }
    qsort(cpucard, 5, sizeof(char), card_cmp);

    if ((k = gp_win(&my, &cpu)) > 0)	/* �Y�q��Ĺ�A���}�@���j�� */
      break;
  }

  for (i = 0; i < 5; i++)
    show_card(1, cpucard[i], i);

  show_style(my, cpu);

  return k;
}


int
main_gp()
{
  int money;		/* ���`���B */
  int cont;		/* �~�����`������ */
  int doub;		/* �O�_�䭿 */
  char hold[5];		/* ���O�d���P */

  char buf[60];
  int i, x, xx;

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  cont = 0;		/* ���`�����k�s */

  while (1)
  {
    vs_bar("�����J����");
    out_song();

    if (!cont)		/* �Ĥ@�����` */
    {
      vget(b_lines - 3, 0, "�аݭn�U�`�h�֩O�H(1 ~ 50000) ", buf, 6, DOECHO);
      money = atoi(buf);
      if (money < 1 || money > 50000 || money > cuser.money)
        break;		/* ���}��� */
      cuser.money -= money;
      move(b_lines - 4, 0);
      prints(COLOR1 " (��)(��)���ܿ�P  (d)Double  (SPCAE)���ܴ��P  (Enter)�T�w                    \033[m");
    }
    else		/* �~��W�@�LĹ������A�N���i�H�A double �F */
    {
      move(b_lines - 4, 0);
      prints(COLOR1 " (��)(��)���ܿ�P  (SPCAE)���ܴ��P  (Enter)�T�w                               \033[m");
    }

    out_song();

    get_newcard(0);	/* �~�P */

    doub = 0;
    for (i = 0; i < 5; i++)
    {
      mycard[i] = get_newcard(1);
      hold[i] = 1;
    }
    qsort(mycard, 5, sizeof(char), card_cmp);

    for (i = 0; i < 5; i++)
      show_card(0, mycard[i], i);

    x = xx = 0;
    do
    {
      for (i = 0; i < 5; i++)
      {
	move(16, i * 4 + 2);
	outs(hold[i] < 0 ? "�O" : "  ");
	move(17, i * 4 + 2);
	outs(hold[i] < 0 ? "�d" : "  ");
      }
      move(11, xx * 4 + 2);
      outs("  ");
      move(11, x * 4 + 2);
      outs("��");
      move(11, x * 4 + 3);	/* �קK���ΰ��� */
      xx = x;

      switch (i = vkey())
      {
      case KEY_LEFT:
	x = x ? x - 1 : 4;
	break;

      case KEY_RIGHT:
	x = (x == 4) ? 0 : x + 1;
	break;

      case ' ':
	hold[x] *= -1;
	break;

      case 'd':
	if (!cont && !doub && cuser.money >= money)
	{
	  doub = 1;
	  cuser.money -= money;
	  money *= 2;
          move(b_lines - 4, 0);
	  prints(COLOR1 " (��)(��)���ܿ�P  (SPCAE)���ܴ��P  (Enter)�T�w                               \033[m");
	  out_song();
	}
	break;
      }
    } while (i != '\n');

    for (i = 0; i < 5; i++)
    {
      if (hold[i] == 1)
	mycard[i] = get_newcard(1);
    }
    qsort(mycard, 5, sizeof(char), card_cmp);
    for (i = 0; i < 5; i++)
      show_card(0, mycard[i], i);
    move(11, x * 4 + 2);
    outs("  ");

    i = cpu_doing();

    if (i < 0)		/* ���aĹ�P */
    {
      switch (i)
      {      
      /* �S��P�����S�O���߲v */
      case -1001:
        money *= 16;
	break;
      case -1002:
	money *= 11;
	break;
      case -1003:
        money *= 6;
	break;
      default:
	money <<= 1;
	break;
      }
      sprintf(buf, "�z�I�n�γ�I�o�� %d ���� :)", money);
      vmsg(buf);

      if (vans("�z�n������~�����`��(Y/N)�H[N] ") == 'y')
      {
        cont++;
      }
      else
      {
        cont = 0;
        addmoney(money);	/* �@��P���hĹ�@���A�S��P���h 15/10/5 �� */
      }
    }
    else			/* ��P */
    {
      vmsg("��F..:~~~");
      cont = 0;
    }
  }
  return 0;
}
#endif	/* HAVE_GAME */
