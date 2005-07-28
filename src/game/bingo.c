/*-------------------------------------------------------*/
/* bingo.c        ( NTHU CS MapleBBS Ver 3.10 )          */
/*-------------------------------------------------------*/
/* target : ���G�C��                                     */
/* create :   /  /                                       */
/* update : 01/04/21                                     */
/* author : unknown                                      */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME


static void
out_song()
{
  static int count = 0;

  /* �i�Ǥ� & ���z�g���A�̬öQ */
  uschar *msg[9] = 
  {
    "���~�o�Ӯɶ� ���b�o�Ӧa�I",
    "�O�o�a�۪��� ���W��aô�W���",
    "�ʱ��ɨ�̬� �u����������",
    "�Ӧh���R�ȾK �S�H�k�R�A�����H �]�|���|",
    "�ڷ|�e�A���⪴�� �A�O���@�Ͳ��\\�۹�",
    "���Ӫ���l���A�~�� �ڤ~�|�u�@�I",
    "�ھǵ� �b�A�R�̨I�K �A�u�@�ۧڬ�L�©]",
    "�ڤ��M�h  ���@�N",
    "�o������ �ۦu���H �A�̬öQ"
  };
  move(b_lines - 2, 0);
  prints("\033[1;3%dm%s\033[m  �w�X�٦� %d ��", time(0) % 7, msg[count], cuser.money);
  clrtoeol();
  if (++count == 9)
    count = 0;
}


int
main_bingo()
{
  /* 25�� {x����, y����, ��} */
  int place[5][5][3] = 
  {
    {{3, 2, 0}, {3, 6, 0}, {3, 10, 0}, {3, 14, 0}, {3, 18, 0}},
    {{5, 2, 0}, {5, 6, 0}, {5, 10, 0}, {5, 14, 0}, {5, 18, 0}},
    {{7, 2, 0}, {7, 6, 0}, {7, 10, 0}, {7, 14, 0}, {7, 18, 0}},
    {{9, 2, 0}, {9, 6, 0}, {9, 10, 0}, {9, 14, 0}, {9, 18, 0}},
    {{11, 2, 0}, {11, 6, 0}, {11, 10, 0}, {11, 14, 0}, {11, 18, 0}}
  };

  /* �߲v : �q 5 ���N���\�� 20 ���A�q 6 ���N���\�� 15 �� ... �ܤ֭n�q 5 ���~�i�ন�\ */
  int rate[13 + 1] = {0, 1, 2, 3, 5, 7, 10, 15, 20, 0, 0, 0, 0, 0};
  
  char used[25];	/* 25 �ӼƦr�O�_�ιL */
  int money;		/* ��� */
  int account;		/* �٦��X���i�q */
  int success;		/* �s���@���u�F�� */

  int row, col, i;
  char buf[60];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  while (1)
  {
    vs_bar("���G�j��");
    out_song();

    vget(2, 0, "�аݭn�U�`�h�֩O�H(1 ~ 50000) ", buf, 6, DOECHO);
    money = atoi(buf);
    if (money < 1 || money > 50000 || money > cuser.money)
      break;				/* ���}��� */
    cuser.money -= money;

    /* initialize */

    for (i = 0; i < 25; i++)	/* 25 �ӼƦr���٨S�Q�� */
      used[i] = 0;

    for (row = 0; row < 5; row++)	/* �ѽL�� 25 ����k�s */
    {
      for (col = 0; col < 5; col++)
	place[row][col][2] = 0;
    }

    /* �e�ѽL */

    move(4, 0);
    for (i = 0; i < 5; i++)
    {
      outs("   \033[1;44m                      \033[m\n"
        "   \033[44m  \033[42m  \033[44m  \033[42m  \033[44m  \033[42m  "
        "\033[44m  \033[42m  \033[44m  \033[42m  \033[44m  \033[m\n");
    }
    outs("   \033[44m                      \033[m\n");

    /* game start */

    for (account = 13, success = 0; account && !success; account--)
    {
      move(b_lines - 7, 0);
      outs("\033[1;37;44m�|���}�X�����X\033[m\n");
      for (i = 1; i <= 25; i++)
      {
	if (!used[i - 1])
	  prints(" %2d", i);
      }

      prints("\n�|��\033[1;33;41m %2d \033[m�����|�i�q �U���q���i�o\033[1;37;44m %d \033[m��\n",
        account, rate[account]);

      do
      {
        vget(b_lines - 4, 0, "�п�J�z�����X�G", buf, 3, DOECHO);
        i = atoi(buf);
      } while (i <= 0 || i > 25 || used[i - 1]);

      /* �ä��O�@�}�l�N�M�w�ѽL�W 25 �Ӧ�m���ȡA�ӬO user �C�q�@�ӼƦr
         �A�h�M�w�n��o�ӼƦr��b�ѽL�W�����Ӧ�m */
      do
      {
        row = rnd(5);
        col = rnd(5);
      } while (place[row][col][2]);

      place[row][col][2] = i;
      used[i - 1] = 1;

      /* ��s���I����C�e�W�ѽL */

      move(5 + row * 2, 0);
      clrtoeol();
      outs("   \033[1m");
      for (i = 0; i < 5; i++)
      {      
	outs("\033[44m  ");
	if (place[row][i][2])
	  prints("\033[40m%2d", place[row][i][2]);
	else
	  outs("\033[42m  ");
      }
      outs("\033[44m  \033[m\n");

      if (account >= 13 - 5)	/* �ܤ֭n�q 5 ����~�ݭn�ˬd�O�_�s���@���u */
        continue;
      
      /* �ˬd�O�_�s���@���u */
      /* �Y�b�G���﨤�u�W�A�n�S�O�ˬd�﨤�u�A�_�h�u�n�ˬd�Ӧ�C�Y�i */

      if ((row == col && place[0][0][2] && place[1][1][2] && place[2][2][2] && place[3][3][2] && place[4][4][2]) ||
        (row + col == 4 && place[0][4][2] && place[1][3][2] && place[2][2][2] && place[3][1][2] && place[4][0][2]) ||
        (place[row][0][2] && place[row][1][2] && place[row][2][2] && place[row][3][2] && place[row][4][2]) ||
        (place[0][col][2] && place[1][col][2] && place[2][col][2] && place[3][col][2] && place[4][col][2]))
      {
	success = 1;
	break;
      }

    }	/* for �j�鵲���A�i��O�q�� 13 ���άO���\�F */

    if (success)
    {
      money *= rate[account];
      sprintf(buf, "���߱z...Ĺ�F %d �� ", money);
      addmoney(money);
    }
    else
    {      
      strcpy(buf, "�B�𤣨�...�A�Ӥ@�L�a�I");
    }
    vmsg(buf);

  }	/* while �j�鵲���A���}�C�� */

  return 0;
}
#endif	/* HAVE_GAME */
