/*-------------------------------------------------------*/
/* bar.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : BAR �x�C��                                   */
/* create :   /  /                                       */
/* update : 01/04/27                                     */
/* author : unknown                                      */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME


#if 0
  �~�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w��
  �x��x�Y�x���x���x���x77�x��x��x���x���x�Y�x�[�x���x77�x���x��x��x���x
  �u�w�q�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�q�w�t
  �x�Y�x                                                              �x���x
  �u�w�t                                                              �u�w�t
  �x���x                                                              �x���x
  �u�w�t                                                              �u�w�t
  �x�[�x                                                              �x�[�x
  �u�w�t                                                              �u�w�t
  �x�Y�x                                                              �x��x
  �u�w�t                                                              �u�w�t
  �x77�x                                                              �x���x
  �u�w�t                      ���j           ���p                     �u�w�t
  �x���x                                                              �x�Y�x
  �u�w�q�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�q�w�t
  �x���x��x77�x�Y�x���x���x��x���x��x�Y�x���x�[�x���x77�x�Y�x��x���x77�x
  ���w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w��
#endif

static char *itemlist[10] =  {"  ", "�[", "��", "��", "��", "��", "��", "77", "��", "�Y"};	/* ���ئW */

static int bar[49] = 	/* �W�� 48 �椤(�ѥ��W"��"�}�l���ɰw¶)�ҹ����� itemlist[] */
{
  0, 3, 9, 8, 4, 6, 7, 2, 3, 5,
  8, 9, 1, 8, 7, 6, 3, 2, 8, 4,
  5, 1, 2, 6, 9, 7, 5, 3, 9, 7,
  8, 1, 4, 9, 2, 6, 3, 5, 8, 9,
  7, 2, 4, 8, 7, 9, 1, 5, 9
};				/* �O����} */

static int money[10];		/* ��� */


static int
total_money()			/* �O�_���U�` */
{
  if (money[1] || money[2] || money[3] || money[4] || money[5] ||
    money[6] || money[7] || money[8] || money[9])
  {
    return 1;
  }
  return 0;
}


static int
run(step, last, freq)
  int step;		/* ��m */
  int last;		/* 1: �̫�@�B  0: �����B */
  int freq;		/* frequency: Hz */
{
  int x1, y1, x2, y2;

  if (step == 1)
  {
    x1 = 6;
    y1 = 4;
    x2 = 4;
    y2 = 4;
  }

  else if (step > 1 && step < 19)
  {
    x1 = 4;
    y1 = 4 * step - 4;
    x2 = 4;
    y2 = 4 * step;
  }

  else if (step > 18 && step < 26)
  {
    x1 = 2 * step - 34;
    y1 = 72;
    x2 = 2 * step - 32;
    y2 = 72;
  }

  else if (step > 25 && step < 43)
  {
    x1 = 18;
    y1 = 176 - 4 * step;
    x2 = 18;
    y2 = 172 - 4 * step;
  }

  else if (step > 42 && step < 49)
  {
    x1 = 104 - 2 * step;
    y1 = 4;
    x2 = 102 - 2 * step;
    y2 = 4;
  }

  move(x1, y1);
  if (step == 1)
    outs(itemlist[9]);
  else
    outs(itemlist[bar[step - 1]]);

  move(x2, y2);
  if (last)
    outs(itemlist[bar[step]]);
  else
    outs("��");

  refresh();
  usleep(1000000 / freq);	/* ���� */
}


static inline int
get_item()		/* �üƿ���n�������� */
{

#if 0
�s���G   1     2     3     4     5     6     7     8     9
���ءG "�[", "��", "��", "��", "��", "��", "77", "��", "�Y"
���v�G  13    16    21    26    32    43    64   127   308    /650
�߲v�G  50    40    30    25    20    15    10     5     2
#endif

  int randnum = rnd(650);	/* �߲v * ���v = ����� (�C�� 1 ���Ҧ^�������B) */

  if (randnum < 308)		/* 2 * 308 / 650 = 0.948 */
    return 9;
  if (randnum < 435)		/* 5 * 127 / 650 = 0.978 */
    return 8;
  if (randnum < 499)		/* 10 * 64 / 650 = 0.985 */
    return 7;
  if (randnum < 542)		/* 15 * 43 / 650 = 0.992 */
    return 6;
  if (randnum < 574)		/* 20 * 32 / 650 = 0.985 */
    return 5;
  if (randnum < 600)		/* 25 * 26 / 650 = 1.000 */
    return 4;
  if (randnum < 621)		/* 30 * 21 / 650 = 0.969 */
    return 3;
  if (randnum < 637)		/* 40 * 16 / 650 = 0.985 */
    return 2;

  return 1;			/* 50 * 13 / 650 = 1.000 */
}


static inline int	/* �Ǧ^ map �W�� 1~48 �䤤�@�ӼƦr */
get_dst(item)		/* �ǤJ�������ؿ���̫ᰱ�d����m */
  int item;
{
  int dst, randnum;

  randnum = rnd(48) + 1;	/* �q�������e�����A�קK�C�����q 1 ��|�����k�W�������� */

  for (dst = randnum; dst <= 48; dst++)
  {
    if (bar[dst] == item)
      return dst;
  }

  for (dst = randnum; dst >= 1; dst--)
  {
    if (bar[dst] == item)
      return dst;
  }
}


static inline void
print_total()
{
  outs("\n\n");
  outs("  �~�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w��\n");
  outs("  �x��x�Y�x���x���x���x77�x��x��x���x���x�Y�x�[�x�Y�x77�x���x��x��x���x\n");
  outs("  �u�w�q�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�q�w�t\n");
  outs("  �x�Y�x                                                              �x���x\n");
  outs("  �u�w�t                                                              �u�w�t\n");
  outs("  �x���x                                                              �x���x\n");
  outs("  �u�w�t                                                              �u�w�t\n");
  outs("  �x�[�x                                                              �x�[�x\n");
  outs("  �u�w�t                                                              �u�w�t\n");
  outs("  �x�Y�x                                                              �x��x\n");
  outs("  �u�w�t                                                              �u�w�t\n");
  outs("  �x77�x                                                              �x���x\n");
  outs("  �u�w�t                      ���j           ���p                     �u�w�t\n");
  outs("  �x���x                                                              �x�Y�x\n");
  outs("  �u�w�q�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�q�w�t\n");
  outs("  �x���x��x77�x�Y�x���x���x��x���x��x�Y�x���x�[�x���x77�x�Y�x��x���x77�x\n");
  outs("  ���w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w��\n");
  outs("    \033[1;31m�x�[�x  \033[32m�x��x  \033[33m�x��x  \033[34m�x���x  \033[35m�x���x  \033[36m�x���x  \033[37m�x77�x  \033[0;36m�x���x  \033[33m�x�Y�x \033[m\n");
  outs("    \033[1;31m�x50�x  \033[32m�x40�x  \033[33m�x30�x  \033[34m�x25�x  \033[35m�x20�x  \033[36m�x15�x  \033[37m�x10�x  \033[0;36m�x 5�x  \033[33m�x 2�x \033[m");
}


int
main_bar()
{
  int price[10] = {0, 50, 40, 30, 25, 20, 15, 10, 5, 2};	/* ���v */

  int item;			/* ���ؽs�� */
  int ogn, dst;			/* OriGiN: �_�I  DeStinaTion: ���I */  
  int ch, i, j;
  char buf[80];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  vs_bar("BAR �x");
  print_total();		/* �L�O�� */
  ogn = 1;			/* �_�I�b�Ĥ@�� */

  while (1)
  {
    for (i = 1; i < 10; i++)	/* money �k�s */
      money[i] = 0;

    for (;;)
    {
      /* �M�w�U����` */

      ch = vget(2, 0, "�z�n�����(1-9)�H[S]�}�l [Q]���}�G", buf, 3, DOECHO);
      if (!ch || ch == 's')
      {
	if (total_money())
	  break;
	addmoney(money[1] + money[2] + money[3] + money[4] + money[5] +	money[6] + money[7] + money[8] + money[9]);	/* �ٿ� */
	goto abort_game;
      }
      else if (ch < '1' || ch > '9')
      {
	addmoney(money[1] + money[2] + money[3] + money[4] + money[5] +	money[6] + money[7] + money[8] + money[9]);	/* �ٿ� */
	goto abort_game;
      }

      if (!(vget(2, 0, "�n��h�ֽ���H", buf, 6, DOECHO)))
      {
	if (total_money())
	  break;
	addmoney(money[1] + money[2] + money[3] + money[4] + money[5] +	money[6] + money[7] + money[8] + money[9]);	/* �ٿ� */
	goto abort_game;
      }

      j = atoi(buf);
      if (j < 1 || j > cuser.money)
      {
	addmoney(money[1] + money[2] + money[3] + money[4] + money[5] +	money[6] + money[7] + money[8] + money[9]);	/* �ٿ� */
	goto abort_game;
      }

      cuser.money -= j;
      money[ch - '0'] += j;

      move(b_lines - 1, 0);
      clrtoeol();
      outs("\033[1m   ");
      for (i = 1; i < 10; i++)
	prints("\033[3%dm%6d  ", i, money[i]);  
      prints("\n\033[m         �w�X�٦� %d ��     ", cuser.money);
    }

    /* �}�l�]�F */

    item = get_item();		/* �üƿ���������� */
    dst = get_dst(item);	/* �ѩҤ������بӨM�w�̫ᰱ�d����m */

    for (i = ogn; i <= 48; i++)	/* �Ĥ@�� */
    {
      run(i, 0, 10 + i / 10);	/* �_�l�[�t�A�C�� 10 - 14 ���A�V�]�V�� */
    }

    for (j = 0; j < 2; j++)	/* �����]�G��N�n�F */
    {
      for (i = 1; i < 49; i++)	/* �������� */
      {
	run(i, 0, 15);		/* ���q���t�A�����C�� 15 �� */
      }
    }

    for (i = 1; i <= dst; i++)	/* �̫�@�� */
    {
      run(i, 0, 14 - i / 10);	/* �̫��t�A�C�� 14 - 10 �� */
    }

    for (j = 0; j < 2; j++)	/* �b�̫ᰱ�d����m�{�G�� */
    {
      run(dst, 0, 2);
      run(dst, 1, 2);
    }

    move(2, 0);
    clrtoeol();
    prints("�������O \033[37m%s\033[m�A", itemlist[item]);

    if (!money[item])
    {
      outs("ݢ�t +_+");
    }
    else
    {
      money[0] = money[item] * price[item];
      prints("���߱z�㤤�F�A��o���� \033[32m%d\033[m", money[0]);

      for (;;)		/* �i�H�@����j�p���n */
      {
	sprintf(buf, "�ثe����: %d �z�٭n��j�p��(Y/N)�H[N] ", money[0]);

	if (vans(buf) != 'y')	/* ����j�p */
	{
	  move(2, 0);
	  clrtoeol();
	  prints("�o���� %d", money[0]);
	  break;
	}
	else			/* ��j�p */
	{
	  sprintf(buf, "�z�n�㤰��H [1]�j (2)�p ");
	  ch = vans(buf) - '1';		/* ch = 0:�j  1:�p */

	  for (i = 16; i <= 20; i++)
	  {
	    move(15, 30);
	    outs("���j           ���p");
	    refresh();
	    usleep(6000 * (i ^ 2));

	    move(15, 30);
	    outs("���j           ���p");
	    usleep(6000 * (i ^ 2));
	    refresh();
	  }

	  price[0] = rnd(2);
	  move(15, 30);
	  if (price[0])
	    outs("���j           ���p");
	  else
	    outs("���j           ���p");

	  if (price[0] == ch)
	  {
	    money[0] *= 2;
	    move(2, 0);
	    clrtoeol();
	    outs("�ڡI�㤤�F�I�����ܦ��G���I");
	  }
	  else
	  {
	    money[0] = 0;
	    move(2, 0);
	    clrtoeol();
	    outs("�����F�I�s���I");
	    break;	/* ��j�p���� */
	  }
	}
      }
      addmoney(money[0]);
    }		/* ��j�p���� */

    vmsg("�~��U�@�L�j��");
    move(b_lines, 0);
    clrtoeol();			/* �M���Ы����N���~�� */

    ogn = dst;			/* �U���_�I�O�W�������I */
  }

abort_game:
  return 0;
}
#endif	/* HAVE_GAME */
