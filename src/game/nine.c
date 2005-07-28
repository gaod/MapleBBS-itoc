/*-------------------------------------------------------*/
/* nine.c           ( NTHU CS MapleBBS Ver 3.10 )        */
/*-------------------------------------------------------*/
/* target : �Ѧa�E�E�C��                                 */
/* create : 98/11/26                                     */
/* update : 01/04/24                                     */
/* author : dsyan.bbs@Forever.twbbs.org                  */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_GAME


#undef	NINE_DEBUG


static char AI_score[13] = {7, 6, 5, 4,10, 9, 3, 2, 1, 0,11, 8,12};
/* �q�� AI �Ҧb :           K  A  2  3  4  5  6  7  8  9  T  J  Q  �C�I�P�ҹ��������� */
/* AI_score 6 �H�U���O�Ʀr�P�A7 �H�W���O�S��P */
/* �q���ɦV�� AI_score �p���P��X�h */

static char str_dir[4][3] = {"��", "��", "��", "��"};

/* hand[] �̭����ȩҥN���P�i 0~12:CK~CQ 13~25:DK~DQ 26~38:HK~KQ 39~51:SK~SQ */
static char str_suit[4][3] = {"��", "��", "��", "��"};
static char str_num[13][3] = {"��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��"};

static char hand[4][5];		/* �F��n�_�|�a���P�i */
static char now;		/* �{�b��W�I�� */
static char dir;		/* �ثe���઺��V 1:�f�ɰw -1:���ɰw */
static char turn;		/* �ثe������@�a 0:�ۤv 1-3:�q�� */
static char live;		/* �q���٦��X�a���� 0~3  0:���a�ӧQ */
static int sum;			/* ��W�w�g���X�i�P�F */


static void
out_song()
{
  static int count = 0;

  /* �B�Y�^���ܷR�ܷR�A */
  uschar *msg[12] = 
  {
    "�Q���A�����  ���A��ּ֪���",
    "�n�b�A���ߤ�  �I�U�ڪ��W�r",
    "�D�ɶ�  �X�ۧA  ���`�N���ɭ�",
    "�����a  ��o�ؤl  �C���G��",
    "�ڷQ�o���T�O  ��A�X�A���k�l",
    "�ڤӤ����ŬX�u����������",
    "�p�G��  �h�^��  �n�B�ͪ���m",
    "�A�]�N  ���A�ݭn  �������o�ˤl",
    "�ܷR�ܷR�A  �ҥH�@�N  �˱o���A",
    "����h���֪��a�譸�h",
    "�ܷR�ܷR�A  �u�����A  �֦��R��",
    "�ڤ~�w��"    
  };
  move(b_lines - 2, 0);
  prints("\033[1;3%dm%s\033[m  �w�X�٦� %d ��", time(0) % 7, msg[count], cuser.money);
  clrtoeol();
  if (++count == 12)
    count = 0;
}


static int
score_cmp(a, b)
  char *a, *b;
{
  return AI_score[(*a) % 13] - AI_score[(*b) % 13];
}


static void
show_mycard()	/* �q�X�ڤ�W�����i�P */
{
  int i;
  char t;

  for (i = 0; i < 5; i++)
  {
    t = hand[0][i];
    move(16, 30 + i * 4);
    outs(str_num[t % 13]);
    move(17, 30 + i * 4);
    outs(str_suit[t / 13]);
  }
}


static void
show_seacard(t)	/* �q�X�������P */
  char t;	/* �X�F���i�P */
{
  char x, y;

  /* �F�n��_�|�a���X�P�m�� (x, y) ���� */
  /*               �n  �F  �_  �� */
  char coorx[4] = { 8,  6,  4,  6};
  char coory[4] = {30, 38, 30, 22};

#ifdef NINE_DEBUG
  /* �q�X�q�����P */
  move(b_lines - 1, 5);  
  for (x = 3; x > 0; x--)
  {
    if (hand[x][0] == -1)	/* �o�a�w�g�G�D�^�O�A�����L�X */
      continue;
    qsort(hand[x], 5, sizeof(char), score_cmp);
    for (y = 0; y < 5; y++)
      outs(str_num[hand[x][y] % 13]);
    outs("  ");
  }
#endif

  x = coorx[turn];
  y = coory[turn];
  move(x, y);
  outs("�~�w�w�w��");
  move(x + 1, y);
  prints("�x%s    �x", str_num[t % 13]);
  move(x + 2, y);
  prints("�x%s    �x", str_suit[t / 13]);
  move(x + 3, y);
  outs("�x      �x");
  move(x + 4, y);
  outs("�x      �x");
  move(x + 5, y);
  outs("�x      �x");
  move(x + 6, y);
  outs("���w�w�w��");

  move(8, 50);
  prints("%s  %s", dir == 1 ? "��" : "��", dir == 1 ? "��" : "��");
  move(10, 50);
  prints("%s  %s", dir == 1 ? "��" : "��", dir == 1 ? "��" : "��");

  move(13, 46);
  prints("�I�ơG%-2d", now);
  /* prints("�I�ơG%c%c%c%c", (now / 10) ? 162 : 32, (now / 10) ? (now / 10 + 175) : 32, 162, now % 10 + 175); */ /* �L�ݴ������� */

  move(14, 46);
  prints("�i�ơG%d", sum);

  refresh();

  sleep(1);		/* �����a�ݲM���X�P���p */
}


static void
ten_or_twenty(t)	/* �[�δ� 10/20 */
  char t;
{
  if (now < t)			/* �����[ */
  {
    now += t;
  }
  else if (now > 99 - t)	/* ������ */
  {
    now -= t;
  }
  else				/* �߰ݭn�[�٬O�n�� */
  {
    int ch;

    move(b_lines - 4, 0);
    clrtoeol();
    prints("     (��)(+)�[%d  (��)(-)��%d   ", t, t);

    while (1)
    {
      if (turn)		/* �q�� */
	ch = rnd(2) + KEY_LEFT;	/* KEY_RIGHT == KEY_LEFT + 1 */
      else		/* ���a */
	ch = vkey();

      switch (ch)
      {
      case KEY_LEFT:
      case '+':
	now += t;
	prints("\033[32;1m�[ %d\033[m", t);
	return;

      case KEY_RIGHT:
      case '-':
	now -= t;
	prints("\033[32;1m�� %d\033[m", t);
	return;
      }
    }
  }
}


static void
next_turn()
{
  while (1)
  {
    turn = (turn + 4 + dir) % 4;
    if (hand[turn][0] >= 0)		/* �p�G�U�@�a�w�D�^�O�A���A�U�@�a */
      break;
  }
}


static char
get_newcard()
{
  /* 0~12:C)KA23456789TJQ 13~25:D))KA23456789TJQ 26~38:H))KA23456789TJQ 39~51:S)KA23456789TJQ */

  /* itoc.020929: ���D�{��²��A�P�Y�ϭ��ХX�{�]�L���A�ϥ��Ѧa�E�E�o�C��
     ���ӬO���ܦh�ƾ�J�P�V�_�Ӫ� */

  return rnd(52);

  /* itoc.����: �p�Gı�o�Ѧa�[�[���פӰ��A����i�H�b���üư��վ�A
     ���O now ���� 99 �ɡA���a���P�|�ܦn�F�άO�q������S��P�����v�ܤp */
}


static int	/* -1:�z�� */
lead_card(t)	/* �X�P */
  char *t;	/* �ǤJ�X�F���i�P�A�]�n�ǥX�@�i�s���P���N�o�i�P */
{
  int ch;
  char m;

  m = *t % 13;
  switch (m)
  {
  case 4:	/* �j�� */
    dir = -dir;
    break;

  case 5:	/* ���w */
    move(b_lines - 4, 0);
    clrtoeol();
    outs("     ���w���@�a�H");
    for (ch = 3; ch >= 0; ch--)
    {
      if (turn != ch && hand[ch][0] >= 0)
	prints("(%s) ", str_dir[ch]);
    }

    /* ���w�@�a�|���^�O�� */
    while (1)
    {

#if 0		/* from global.h */
#define KEY_UP          -1
#define KEY_DOWN        -2
#define KEY_RIGHT       -3 
#define KEY_LEFT        -4
#endif

      if (turn || live == 1)	/* �p�G�O�q���X���P�άO�u�ѤU�@�ӹq�����A�N�۰ʫ��w */
	ch = rnd(4) + KEY_LEFT;
      else
	ch = vkey();

      /* �Q���w�����a����w�g�D�^�O */
      if (turn != 3 && hand[3][0] >= 0 && ch == KEY_LEFT)
	ch = 3;
      else if (turn != 2 && hand[2][0] >= 0 && (ch == KEY_UP || ch == KEY_DOWN))
	ch = 2;
      else if (turn != 1 && hand[1][0] >= 0 && ch == KEY_RIGHT)
	ch = 1;
      else if (turn != 0 && hand[0][0] >= 0)
	ch = 0;
      else
	continue;

      break;
    }

    prints("\033[32;1m(%s)\033[m", str_dir[ch]);
    break;

  case 10:	/* �[�δ�10 */
    ten_or_twenty(10);
    break;

  case 11:	/* Pass */
    break;

  case 12:	/* �[�δ�20 */
    ten_or_twenty(20);
    break;

  case 0:	/* ���W��99 */
    now = 99;
    break;

  default:	/* �@��Ʀr�P */
    if (now + m > 99)
      return -1;
    else
      now += m;
    break;
  }

  show_seacard(*t);

  /* ���@�i�s���P */
  *t = get_newcard();

  /* ����U�@�a */
  if (m == 5)
    turn = ch;
  else
    next_turn();

  return 0;
}


static void
cpu_die()
{
  char buf[20];

  switch (turn)
  {
  case 1:
    move(9, 55);
    break;
  case 2:
    move(7, 52);
    break;
  case 3:
    move(9, 49);
    break;
  }
  outs("  ");
  live--;

  sprintf(buf, "�q�� %d �z�F", turn);
  vmsg(buf);
  sleep(1);		/* �����a�ݲM���X�P���p */

  hand[turn][0] = -1;	/* �z�������a�� hand[turn][0] �]�� -1 */
  next_turn();
}


static void
online_help(t)
  char t;
{
  char m;

  move(b_lines - 4, 0);
  clrtoeol();

  m = t % 13;
  switch (m)
  {
  case 0:
    outs("     �E�E�G�I�ư��W�ܦ�����");
    break;

  case 4:
    outs("     �j��G�C���i���V�ۤ�");
    break;

  case 5:
    outs("     ���w�G�ۥѫ��w�U�@�Ӫ��a");
    break;

  case 11:
    outs("     PASS�G�i pass �@��");
    break;

  case 10:
    outs("     �I�ƥ[�δ� 10");
    break;

  case 12:
    outs("     �I�ƥ[�δ� 20");
    break;

  default:
    prints("     �I�ƥ[ %d", m);
    break;
  }
}


int
main_nine()
{
  int money;		/* ��� */

  int i, j;
  char m;
  char buf[STRLEN];
  FILE *fp;

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  while (1)
  {
    vs_bar("�Ѧa�E�E");
    out_song();

    vget(2, 0, "�аݭn�U�`�h�֩O�H(1 ~ 50000) ", buf, 6, DOECHO);
    money = atoi(buf);
    if (money < 1 || money > 50000 || money > cuser.money)
      break;			/* ���}��� */

    /* �L�X��������\�] */

    if (!(fp = fopen("etc/game/99", "r")))
      break;			/* ���}��� */

    move(2, 0);
    clrtoeol();		/* �M���u�аݭn�U�`�h�֩O�H�v���ݾl */

    move(1, 0);
    while (fgets(buf, STRLEN, fp))
      outs(buf);

    fclose(fp);

    cuser.money -= money;		/* ���T�w��}�ɦA���� */

    for (i = 0; i < 4; i++)
    {
      for (j = 0; j < 5; j++)
	hand[i][j] = get_newcard();
    }

    sum = 0;		/* �ثe�P�त���S���P */
    now = 0;		/* �ثe�P�त�����I�ƬO 0 */
    turn = 0;		/* ���a�}�l�Ĥ@�B */
    dir = 1;		/* �@�}�l�w�] �f�ɰw */
    live = 3;		/* �|���T�a�q������ */

    /* ���a���P�n�ƧǡA�q�����P������q���ɦA�Ƨ� */
    qsort(hand[0], 5, sizeof(char), score_cmp);
    show_mycard();

    /* �C���}�l */
    for (;;)
    {
      move(9, 52);
      outs(str_dir[turn]);

      /* ����q���X�P */

      if (turn)
      {
	qsort(hand[turn], 5, sizeof(char), score_cmp);

	for (i = 0; i < 5; i++)
	{
	  m = hand[turn][i] % 13;
	  if (AI_score[m] >= 7)		/* �� AI_score[] �ӧP�_�O�_���S��P */
	    break;
	  if (now + m <= 99)
	    break;
	}

	if (i == 5)	/* �S������@�i�P��X */
	{
	  cpu_die();
	}
	else
	{
	  /* �ѩ� qsort �|�� AI_score ���P��b�̥���A�N�����ߥX�̥���o�i�P�Y�i */
	  sum++;
	  lead_card(&(hand[turn][i]));
	}

	if (rnd(5) == 0)
	  out_song();

	continue;
      }

      /* ���쪱�a�X�P */

      if (!live)	/* �T�a�q���������F */
      {
	if (sum < 25)
	  money *= 15;
	else if (sum < 50)
	  money *= 10;
	else if (sum < 100)
	  money *= 5;
	else if (sum < 150)
	  money *= 3;
	else if (sum < 200)
	  money *= 2;
	/* �W�L 200 ���~Ĺ�A�u�٭��� */

	addmoney(money);
	sprintf(buf, "�b�g�L %d �^�X���r���A�z��o�ӥX�A��o���� %d", sum, money);
	vmsg(buf);
	break;
      }

      /* ���p���P/�S��P��b�k��A�Y�̫�@�i���O�S��P�B�[�W�h�|�W�L 99�A�N�O�z�F */
      m = hand[0][4] % 13;
      if (AI_score[m] < 7 && now + m > 99)
      {
        sprintf(buf, "����..�b %d �i�P�Q�q���q�z���F.. :~", sum);
        vmsg(buf);
	break;
      }

      i = 0;		/* �b�H�U while(j) �j�餤�A�� i �ӷ�ĴX�i�P */
      while (1)		/* ���� vkey() == '\n' �� ' ' �ɤ~���}�j�� */
      {
	m = hand[0][i] % 13;
	move(18, i * 4 + 30);

	if (AI_score[m] < 7)	/* �� AI_score[] �ӧP�_�O�_���S��P */
	{
	  if (now + m > 99)
	    outs("�I");		/* ĵ�i�|�z�� */
	  else
	    outs("��");		/* �@��P */
	}
	else
	{
	  outs("��");		/* �S��P */
	}

	move(18, i * 4 + 31);	/* �קK�������k����� */

	j = vkey();

	if (j == KEY_LEFT)
	{
	  move(18, i * 4 + 30);
	  outs("  ");
	  i = i ? i - 1 : 4;
	}
	else if (j == KEY_RIGHT)
	{
	  move(18, i * 4 + 30);
	  outs("  ");
	  i = (i == 4) ? 0 : i + 1;
	}
#ifdef NINE_DEBUG
	else if (j == KEY_UP)
	{
	  if (vget(b_lines - 4, 5, "��P�����G", buf, 3, DOECHO))
	  {
	    m = atoi(buf);
	    if (m >= 0 && m < 52)
	    {
  	      hand[0][i] = m;
	      qsort(hand[0], 5, sizeof(char), score_cmp);
	      show_mycard();
	    }
	  }
	}
#endif
	else if (j == KEY_DOWN)
	{
	  online_help(hand[0][i]);
	}
	else if (j == '\n' || j == ' ')
	{
	  move(18, i * 4 + 30);
	  outs("  ");
	  sum++;
	  break;
	}
	else if (j == 'q')
	{
	  return 0;
	}
      }

      if (lead_card(&(hand[0][i])) < 0)
      {
	vmsg("����..�����z�F!!.. :~");
	break;
      }

      qsort(hand[0], 5, sizeof(char), score_cmp);
      show_mycard();
    }
  }
  return 0;
}

#endif	/* HAVE_GAME */
