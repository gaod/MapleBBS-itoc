/*-------------------------------------------------------*/
/* bj.c          ( NTHU CS MapleBBS Ver 3.10 )           */
/*-------------------------------------------------------*/
/* target : �³ǧJ�G�Q�@�I�C��                           */
/* create :   /  /                                       */
/* update : 01/04/23                                     */
/* author : unknown                                      */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_GAME


enum
{				/* ���v */

  SEVEN = 5,			/* 777 */
  SUPERAJ = 5,			/* spade A+J */
  AJ = 4,			/* A+J */
  FIVE = 3,			/* �L���� */
  JACK = 3,			/* ���e��i�N 21 �I */
  WIN = 2			/* Ĺ */
};


static char *flower[4] = {"��", "��", "��", "��"};
static char *poker[53] = 
{
  "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��",
  "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��",
  "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��",
  "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��", "��",
  "��", "��", "��", "��", "  "
};


static void
out_song()
{
  static int count = 0;

  /* �\���壻�R�u�Ѥ@�� */
  uschar *msg[9] = 
  {
    "�t���ַ��F  ���e�O����  �٩��e��  �����W��R",
    "���Q���֩�@�A��  �A�@�ä�  �ڴN�h��  �I�l���F",
    "�R������n  ������  �A��W�@�s�����u�@�A��������",
    "����  ���֥u�Ѥ@��  �A�ٸ��\\�D��",
    "�n�D�ک񱼡@�����ӻ�����~�n",
    "�ۤv�`�R���H  �����p�ġ@�L�z���x",
    "�ͩR�@�p�G�u�Ѥ@��  �ڥu�Q�n��a",
    "���b�A�h��  �ڸ̦۴M�ѯ�a�ѡ@�j�@��߸���",
    "�A���p�N�@�A�ˤ��F�@�ڪ��n"
  };
  move(b_lines - 2, 0);
  prints("\033[1;3%dm%s\033[m  �w�X�٦� %d ��", time(0) % 7, msg[count], cuser.money);
  clrtoeol();
  if (++count == 9)
    count = 0;
}


static void
print_card(int card, int x, int y)
{

  move(x, y);
  outs("�~�w�w�w��");
  move(x + 1, y);
  prints("�x%s    �x", poker[card]);
  move(x + 2, y);
  prints("�x%s    �x", card != 52 ? flower[card % 4] : "  ");
  move(x + 3, y);
  outs("�x      �x");
  move(x + 4, y);
  outs("�x      �x");
  move(x + 5, y);
  outs("�x      �x");
  move(x + 6, y);
  outs("���w�w�w��");
}


static char
get_newcard(mode)
  int mode;			/* 0:���s�~�P  1:�o�P */
{
  static char card[10];	/* �̦h�u�|�Ψ� 10 �i�P */
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
main_bj()
{
  int money;			/* ��� */

  /* �q���O���a */
  char host_card[12];		/* �q�����P�i */
  char guest_card[12];		/* ���a���P�i */

  int host_count;		/* �q�����F�X�i�P */
  int guest_count;		/* ���a���F�X�i�P */

  int host_A;			/* �q���� A ���i�� */
  int guest_A;			/* ���a�� A ���i�� */
  int host_point;		/* �q�������`�I�� */
  int guest_point;		/* ���a�����`�I�� */

  int doub;			/* �O�_�w�g�[����� */
  int ch;			/* ���� */

  int card;
  char buf[60];

  int num[52] =		/* �U�i�P���I�� */
  {
    11, 11, 11, 11, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6,
    7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10
  };

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  while (1)
  {
    vs_bar("�³ǧJ�j��");
    out_song();

    vget(2, 0, "�аݭn�U�`�h�֩O�H(1 ~ 50000) ", buf, 6, DOECHO);
    money = atoi(buf);
    if (money < 1 || money > 50000 || money > cuser.money)
      break;			/* ���}��� */

    cuser.money -= money;
    doub = 0;

    move(3, 0);
    outs("(�� y ��P, n ����P, d double)");

    out_song();

    get_newcard(0);	/* �~�P */

    guest_card[0] = get_newcard(1);
    guest_card[1] = get_newcard(1);
    host_card[0] = get_newcard(1);
    host_card[1] = get_newcard(1);

    host_count = 2;		/* ���ɡA����U���G�i�P */
    guest_count = 2;
    host_A = 0;			/* ���ɡA���賣�S�� Ace */
    guest_A = 0;

    /* �ˬd A ���i�ơA�ӨM�w�O�_�� Black Jack �άO�� A �� 11 �I */

    if (host_card[0] < 4)
      host_A++;
    if (host_card[1] < 4)
      host_A++;
    if (guest_card[0] < 4)
      guest_A++;
    if (guest_card[1] < 4)
      guest_A++;

    print_card(52, 5, 0);		/* �L�X�q�����Ĥ@�i�P(����ܪť�) */
    print_card(host_card[1], 5, 4);	/* �L�X�q�����ĤG�i�P */
    print_card(guest_card[0], 14, 0);	/* �L�X���a���Ĥ@�i�P */
    print_card(guest_card[1], 14, 4);	/* �L�X���a���ĤG�i�P */

    host_point = num[host_card[1]];	/* �q�����Ĥ@�i�P�Ȯɤ����I(�H�K���S����) */
    guest_point = num[guest_card[0]] + num[guest_card[1]];

    move(12, 0);
    prints("\033[1;32m�I�ơG\033[33m%2d\033[m", host_point);

    for (;;)
    {
      /* �ˬd�P�� */
check_condition:

      /* itoc.011025: ���a�C���@�i�P�N�n��ø�@���I�� */
      move(13, 0);
      prints("\033[1;35m�I�ơG\033[36m%2d\033[m", guest_point);

      if (guest_count == 3 &&	/* �n�ˬd guest_count �H�K�ĤT�i�P�O�W���P */
	(guest_card[0] >= 24 && guest_card[0] <= 27) &&
	(guest_card[1] >= 24 && guest_card[1] <= 27) &&
	(guest_card[2] >= 24 && guest_card[2] <= 27))
      {
	money *= SEVEN;
	sprintf(buf, "\033[1;41;33m     ������     �o���� %d �Ȩ�   \033[m", money);
	goto next_game;
      }

      else if ((guest_card[0] == 40 && guest_card[1] == 0) ||
	(guest_card[0] == 0 && guest_card[1] == 40))
      {
        money *= SUPERAJ;
	sprintf(buf, "\033[1;41;33m���� BLACK JACK �o���� %d �Ȩ�   \033[m", money);
	goto next_game;
      }

      else if (((guest_card[0] <= 3 && guest_card[0] >= 0) && 
        (guest_card[1] <= 43 && guest_card[1] >= 40)) ||
	((guest_card[1] <= 3 && guest_card[1] >= 0) && 
	(guest_card[0] <= 43 && guest_card[0] >= 40)))
      {
	money *= AJ;
	sprintf(buf, "\033[1;41;33m  BLACK JACK    �o���� %d �Ȩ�   \033[m", money);
	goto next_game;
      }

      else if (guest_point == 21 && guest_count == 2)	/* �e��i�N 21 �I */
      {
	money *= JACK;
	sprintf(buf, "\033[1;41;33m     JACK       �o���� %d �Ȩ�   \033[m", money);
	goto next_game;
      }

      else if (guest_count == 5 && guest_point <= 21)
      {
	money *= FIVE;
	sprintf(buf, "\033[1;41;33m    �L����      �o���� %d �Ȩ�   \033[m", money);
	goto next_game;
      }

      else if (guest_point > 21 && guest_A)
      {
	guest_point -= 10;
	guest_A--;
        move(13, 0);
        prints("\033[1;35m�I�ơG\033[36m%2d\033[m", guest_point);
	goto check_condition;
      }

      /* �@��P�� */

      else
      {
	/* �z�F */

	if (guest_point > 21)
	{
	  money = 0;
	  strcpy(buf, "\033[1;41;33m     �z�F       ���Q�Y���F   \033[m");
	  goto next_game;
	}

	/* �S�z */

	do
	{
	  ch = igetch();

	  if (ch == 'd' && !doub && guest_count == 2)	/* �u��b�ĤG�i�P��� double */
	  {
	    doub = 1;	/* �w double�A����A double */

	    if (cuser.money >= money)
	    {
	      cuser.money -= money;
	      money *= 2;
	    }
	    else
	    {
	      money += cuser.money;	/* �������G���A�N�����F */
	      cuser.money = 0;
	    }
	    out_song();
	  }
	} while (ch != 'y' && ch != 'n');

	if (ch == 'y' && guest_point != 21)	/* ���a��P */
	{
	  card = get_newcard(1);
	  guest_card[guest_count] = card;
	  if (card < 4)
	    guest_A++;

          guest_point += num[card];
          print_card(card, 14, 4 * guest_count);
          guest_count++;
	  move(13, 0);
	  prints("\033[1;35m�I�ơG\033[36m%2d\033[m", guest_point);
	  goto check_condition;
	}
	else					/* ���a����P */
	{
	  /* ����ܲ��a���Ĥ@�i�P */
          move(6, 2);
          outs(poker[host_card[0]]);
          move(7, 2);
          outs(flower[host_card[0] % 4]);
          host_point += num[host_card[0]];	/* ���}�l�S�[���Ĥ@�i�P�I�[�^�� */

	  /* ����q��(���a)���P */

	  if (host_A == 2)	/* �S��: �q���G�i���P��n���O A �ɷ|�P�_���~ */
	  {
	    /* ��@�i A �q 11 �I���� 1 �I */
	    host_point -= 10;
	    host_A--;
	  }

	  while (host_point < guest_point)	/* ���a�p�G�I����֡A�N�j�����P */
	  {
	    card = get_newcard(1);
	    host_card[host_count] = card;
	    if (card < 4)
	      host_A++;

	    host_point += num[card];
	    print_card(card, 5, 4 * host_count);
	    host_count++;

	    if (host_point > 21)
	    {
	      if (host_A)
	      {
	        host_point -= 10;
	        host_A--;
	        continue;	/* �~����U�@�i�P */
	      }

	      move(12, 0);
	      prints("\033[1;32m�I�ơG\033[33m%2d\033[m", host_point);

	      /* �q��(���a)�z�F */

	      money *= WIN;
	      sprintf(buf, "\033[1;41;33m    WINNER      �o���� %d �Ȩ�   \033[m", money);
	      goto next_game;
	    }
	  }

	  /* �q��(���a)���P�����A�B�S���z���A�q����� */

	  move(12, 0);
	  prints("\033[1;32m�I�ơG\033[33m%2d\033[m", host_point);

	  money = 0;
	  strcpy(buf, "\033[1;41;33m     ��F       ���Q�Y���F   \033[m");
	  goto next_game;
	}

      }				/* if �ˬd�P������ */

    }				/* for �j�鵲���A�����o�^�X */

next_game:
    move(18, 3);
    outs(buf);
    addmoney(money);
    vmsg(NULL);
  }				/* while �j�鵲���A���}�C�� */

  return 0;
}
#endif				/* HAVE_GAME */
