/*-------------------------------------------------------*/
/* marie.c         ( NTHU CS MapleBBS Ver 3.10 )         */
/*-------------------------------------------------------*/
/* target : �p�����ֶ�C��                               */
/* create :   /  /                                       */
/* update : 01/04/26                                     */
/* author : unknown                                      */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#define MAX_MARIEBET	50000	/* �̦h��� 50000 �� */


static inline int
get_item()		/* �üƿ���n�������� */
{

#if 0
�s���G   0     1     2     3     4     5     6     7     8     9
���v�G 200     2     5    10     1    20    40   100   500   122   /1000
�߲v�G   5   500   200   100  1000    50    25    10     2     0
#endif

  int randnum = rnd(1000);	/* �߲v * ���v = ����� (�C�� 1 ���Ҧ^�������B) */

  if (randnum < 500)		/* 2 * 0.500 = 1 */
    return 8;
  if (randnum < 700)		/* 5 * 0.200 = 1 */
    return 0;
  if (randnum < 800)		/* 10 * 0.100 = 1 */
    return 7;
  if (randnum < 840)		/* 25 * 0.040 = 1 */
    return 6;
  if (randnum < 860)		/* 50 * 0.020 = 1 */
    return 5;
  if (randnum < 870)		/* 100 * 0.010 = 1 */
    return 3;
  if (randnum < 875)		/* 200 * 0.005 = 1 */
    return 2;
  if (randnum < 877)		/* 500 * 0.002 = 1 */
    return 1;
  if (randnum < 878)		/* 1000 * 0.001 = 1 */
    return 4;

  return 9;			/* ���´f�U���v�O 0.122 */
}


int
main_marie()
{
  int c_flag[7] = {1, 5, 10, 50, 100, 500, 1000};		/* ���v */
  int price[10] = {5, 500, 200, 100, 1000, 50, 25, 10, 2, 0};	/* ��v */
  int x[9] = {0};						/* �U����� */

  int w;		/* ���v������ */
  int flag;		/* flag = c_flag[w] */
  int item;		/* ���������� */
  int xtotal;		/* �`��� */
  int i, ch;
  FILE *fp;
  char buf[STRLEN];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  if (!(fp = fopen("etc/game/marie", "r")))
    return XEASY;

  vs_bar("�p�����ֶ�");
  move(1, 0);
  while (fgets(buf, STRLEN, fp))	/* �L�X��������\�] */
    outs(buf);
  fclose(fp);

  w = 0;			/* �Ĥ@���i�J��ܤ@���A�ĤG���H��h��W�����@�� */
  flag = c_flag[w];
  item = 0;
  xtotal = 0;

  while (1)
  {
    move(9, 44);
    prints("\033[1m�z���W�٦��w�X %8d ��\033[m", cuser.money);
    move(10, 44);
    prints("\033[1m�ثe��`�����v�O \033[46m%6d ��\033[m", flag);

    move(b_lines - 3, 0);
    for (i = 0; i < 9; i++)
      prints("  %5d", x[i]);

    ch = igetch();		/* ���ݥΨ� vkey() */
    switch (ch)
    {
    case 'w':			/* �������v */
      w = (w + 1) % 7;
      flag = c_flag[w];		/* �������v�ɤ~�ݭn���] flag */
      break;

    case 'a':			/* ���� */
      i = 9 * flag;
      if ((xtotal + i <= MAX_MARIEBET) && (cuser.money >= i))
      {
        cuser.money -= i;
        xtotal += i;
	for (i = 0; i <= 8; i++)
	  x[i] += flag;
      }
      break;

    case '1':			/* ���O�U�` */
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if ((xtotal + flag <= MAX_MARIEBET) && (cuser.money >= flag))
      {
        cuser.money -= flag;
        xtotal += flag;
	x[ch - '1'] += flag;
      }
      break;

    case 's':			/* �}�l�C�� */
    case '\n':
      if (x[0] || x[1] || x[2] || x[3] || x[4] || x[5] || x[6] || x[7] || x[8])
      {		/* ���U�`�~��}�l�� */
	move(15, 5 + 7 * item);
	outs("  ");   		/* �M���W���������� */
	item = get_item();
	if (item != 9)		/* item=9 �O���´f�U */
	{
	  move(15, 5 + 7 * item);
	  outs("��");		/* ø�W�o���������� */

	  if (x[item])
	  {
	    i = x[item] * price[item];
	    addmoney(i);
	    sprintf(buf, "�z�i�o %d ��", i);
	    vmsg(buf);
	  }
	  else
	  {
	    vmsg("�ܩ�p�A�z�S���㤤");
	  }
	}
	else
	{
	  vmsg("�ܩ�p�A���´f�U");
	}

	move(b_lines, 0);
	clrtoeol();		/* �M�� vmsg() */

	/* �U������k�s */
	xtotal = 0;
	for (i = 0; i < 9; i++)
 	  x[i] = 0;
      }
      else
      {
        goto abort_game;
      }
      break;

    case 'q':			/* ���} */
      goto abort_game;

    }	/* switch ���� */

  }	/* while �j�鵲�� */

abort_game:
  return 0;
}
#endif	/* HAVE_GAME */
