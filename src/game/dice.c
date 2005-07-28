/*-------------------------------------------------------*/
/* dice.c         ( NTHU CS MapleBBS Ver 3.10 )          */
/*-------------------------------------------------------*/
/* target : �Y��l�C��                                   */
/* create : 01/02/15                                     */
/* update : 01/04/20                                     */
/* author : wsyfish                                      */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME


static char *pic[6][3] = 
{
  "        ",
  "   ��   ",		/* 1 */
  "        ",

  "   ��   ",
  "        ",		/* 2 */
  "   ��   ",

  "��      ",
  "   ��   ",		/* 3 */
  "      ��",

  "��    ��",
  "        ",		/* 4 */
  "��    ��",

  "��    ��",
  "   ��   ",		/* 5 */
  "��    ��",

  "��    ��",
  "��    ��",		/* 6 */
  "��    ��"
};


static void
out_song()
{
  static int count = 0;

  /* �O�ɲM���}�@���ߵ� */
  uschar *msg[11] = 
  {
    "�}�@���ߵ�  ���۹ڪ��ͻH����",
    "�������N���F",
    "�}�@���ߵ�  ���n�b�·t���K�a",
    "���O�F�h�W�߻�",
    "���ͬ��q���L�o²��  ���W�����N�i�H�Q��",
    "�C�C�L��L�گE�v���v  �ݨ��ŤѤ��A�g��",
    "�n�h���h�m�ۥѩb��",
    "����  �����a�}�@���ߵ�",
    "�^�^�a�a�ڭ̨��X�·t  ��ӥ@�ɳ��������",
    "����  �����a�}�@���ߵ�",
    "�X�X�a�{ģ�۬��R�ڷQ  �Ҧ��w���@�P�A����"
  };
  move(b_lines - 2, 0);
  prints("\033[1;3%dm%s\033[m  �w�X�٦� %d ��", time(0) % 7, msg[count], cuser.money);
  clrtoeol();
  if (++count == 11)
    count = 0;
}


int
main_dice()
{
  int money;		/* ��� */
  int i;		/* �ü� */
  char choice;		/* �O���ﶵ */
  char dice[3];		/* �T�ӻ�l���� */
  char total;		/* �T�ӻ�l���M */
  char buf[60];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  vs_bar("���� �t�� �{�� �U�`");
  outs("\n\n\n"
    "�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\n"
    "�x  2��   1. �j      2. �p                                                �x\n"
    "�x 14��   3. �T�I    4. �|�I    5. ���I    6. ���I    7. �C�I             �x\n"
    "�x  8��   8. �K�I    9. �E�I   10. �Q�I   11. �Q�@�I 12. �Q�G�I 13. �Q�T�I�x\n"
    "�x 14��  14. �Q�|�I 15. �Q���I 16. �Q���I 17. �Q�C�I 18. �Q�K�I           �x\n"
    "�x216��  19. �@�@�@ 20. �G�G�G 21. �T�T�T 22. �|�|�| 23. ������ 24. �������x\n"
    "�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\n");

#if 0	/* �Y��l�C 216 ���U�`�ƥX�{�����ƾ��v */
�z�w�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�{
�x�`�Ƣx3 �x4 �x5 �x6 �x7 �x8 �x9 �x10�x11�x12�x13�x14�x15�x16�x17�x18�x
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x���Ƣx1 �x3 �x6 �x10�x15�x21�x25�x27�x27�x25�x21�x15�x10�x6 �x3 �x1 �x / 216 ��
�|�w�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�}
#endif

  out_song(0);

  while (1)
  {
    vget(2, 0, "�аݭn�U�`�h�֩O�H(1 ~ 50000) ", buf, 6, DOECHO);
    money = atoi(buf);
    if (money < 1 || money > 50000 || money > cuser.money)
      break;				/* ���}��� */

    vget(12, 0, "�n����@���O�H(�п�J���X) ", buf, 3, DOECHO);
    choice = atoi(buf);
    if (choice < 1 || choice > 24)
      break;				/* ���}��� */

    outs("\n�����@���Y�X��l \033[5m....\033[m\n");
    igetch();

    /* �M�w�T�ӻ�l�I�� */
    total = 0;
    for (i = 0; i < 3; i++)
    {
      dice[i] = rnd(6) + 1;
      total += dice[i];
    }

    /* �B�z���G */
    if ((choice == 1 && total > 10) || (choice == 2 && total <= 10))	/* �B�z�j�p */
    {
      sprintf(buf, "���F�I�o�좱������ %d ��", money * 2);
      addmoney(money);
    }
    else if (choice <= 18 && total == choice)				/* �B�z�`�M */
    {
      if (choice >= 8 && choice <= 13)
      {
	sprintf(buf, "���F�I�o�좷������ %d ��", money * 8);
	addmoney(money * 7);
      }
      else
      {
	sprintf(buf, "���F�I�o�좰�������� %d ��", money * 14);
	addmoney(money * 13);
      }
    }
    else if ((choice - 18) == dice[0] && (dice[0] == dice[1]) && (dice[1] == dice[2]))/* �B�z�T�Ӥ@�� */
    {
      sprintf(buf, "���F�I�o�좱���������� %d ��", money * 216);
      addmoney(money * 215);
    }
    else								/* �B�z�S�� */
    {
      strcpy(buf, "�ܥi���S���㤤�I");
      cuser.money -= money;
    }

    /* �L�X��l���G */
    outs("�~�w�w�w�w���~�w�w�w�w���~�w�w�w�w��\n");
    for (i = 0; i < 3; i++)
    {
      prints("�x%s�x�x%s�x�x%s�x\n", pic[dice[0] - 1][i], 
        pic[dice[1] - 1][i], pic[dice[2] - 1][i]);
    }
    outs("���w�w�w�w�����w�w�w�w�����w�w�w�w��\n\n");

    out_song();
    vmsg(buf);
  }

  return 0;
}

#endif	/* HAVE_GAME */
