/*-------------------------------------------------------*/
/* pip_play.c         ( NTHU CS MapleBBS Ver 3.10 )      */
/*-------------------------------------------------------*/
/* target : ���ֿ��                                     */
/* create :   /  /                                       */
/* update : 01/08/15                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* ���ֿ��:���B �ȹC �B�� ���| �q��			 */
/*-------------------------------------------------------*/


int
pip_play_stroll()		/* ���B */
{
  /* �w�]���ܭȡA�Y�����o�ƥ�A�t�~�[����U */
  count_tired(3, 3, 1, 100, 0);	/* �W�[�h�� */
  d.happy += rand() % 3 + 3;
  d.satisfy += rand() % 2 + 1;
  d.shit += rand() % 3 + 2;
  d.hp -= rand() % 3 + 2;

  switch (rand() % 10)
  {
  case 0:
    d.happy += 6;
    d.satisfy += 6;
    show_play_pic(1);
    vmsg("�J��B���o  �u�n.... ^_^");
    break;

  case 1:
    d.happy += 4;
    d.satisfy += 8;
    show_play_pic(2);
    vmsg(d.sex == 1 ? "�ݨ�}�G���k���o  �u�n.... ^_^" : "�ݨ�^�T���k���o  �u�n.... ^_^");
    break;

  case 2:
    d.money += 100;
    d.happy += 4;
    show_play_pic(3);
    vmsg("�ߨ�F100���F..�C�C�C....");
    break;

  case 3:
    d.happy -= 10;
    d.satisfy -= 3;
    show_play_pic(4);
    if (d.money > 50)
    {
      d.money -= 50;
      vmsg("���F50���F..����....");
    }
    else
    {
      d.money = 0;
      vmsg("���������F..����....");
    }
    break;

  case 4:
    d.happy += 3;
    show_play_pic(5);
    if (d.money > 50)
    {
      d.money -= 50;
      vmsg("�ΤF50���F..���i�H�|�ڳ�....");
    }
    else
    {
      d.money = 0;
      vmsg("���Q�ڰ��Υ����F..:p");
    }
    break;

  case 5:
    d.toy++;
    show_play_pic(6);
    vmsg("�n�γ�A�ߨ쪱��F��.....");
    break;

  case 6:
    d.cookie++;
    show_play_pic(7);
    vmsg("�n�γ�A�ߨ�氮�F��.....");
    break;

  case 7:
    d.satisfy -= 5;
    d.shit += 5;
    show_play_pic(9);
    vmsg("�u�O�˷�  �i�H�h�R�R�����");
    break;

  default:
    show_play_pic(8);
    vmsg("�S���S�O���Ƶo�Ͱ�.....");
    break;
  }

  if (d.happy > 100)
    d.happy = 100;
  if (d.satisfy > 100)
    d.satisfy = 100;

  return 0;
}


int
pip_play_sport()		/* �B�� */
{
  count_tired(3, 8, 1, 100, 1);
  d.speed += 2 + rand() % 3;
  d.weight -= rand() % 3 + 2;
  d.shit += rand() % 5 + 10;
  d.hp -= rand() % 2 + 8;
  d.satisfy += rand() % 2 + 3;
  if (d.satisfy > 100)
    d.satisfy = 100;

  show_play_pic(10);
  vmsg("�B�ʦn�B�h�h��...");

  return 0;
}


int
pip_play_date()			/* ���| */
{
  if (d.money < 150)
  {
    vmsg("�������h�աI���|�`�o���I����");
  }
  else
  {
    count_tired(3, 6, 1, 100, 1);
    d.money -= 150;
    d.shit += rand() % 3 + 5;
    d.hp -= rand() % 4 + 8;
    d.character += rand() % 3 + 1;
    d.happy += rand() % 5 + 12;
    d.satisfy += rand() % 5 + 7;

    if (d.happy > 100)
      d.happy = 100;
    if (d.satisfy > 100)
      d.satisfy = 100;

    show_play_pic(11);
    vmsg("���|�h  �I�I");
  }
  return 0;
}


int
pip_play_outing()		/* ���C */
{
  if (d.money < 250)
  {
    vmsg("�������h�աI�ȹC�`�o���I����");
  }
  else
  {
    count_tired(10, 45, 0, 100, 0);
    d.money -= 250;
    d.weight += rand() % 2 + 1;
    d.hp -= rand() % 7 + 15;
    d.character += rand() % 5 + 5;
    d.happy += rand() % 10 + 12;
    d.satisfy += rand() % 10 + 10;

    if (d.happy > 100)
      d.happy = 100;
    if (d.satisfy > 100)
      d.satisfy = 100;

    switch (rand() % 4)
    {
    case 0:
      d.art += rand() % 2;
      show_play_pic(12);
      vmsg(rand() % 2 ? "�ߤ����@�ѲH�H���Pı  �n�ΪA��...." : "���� �~�� �߱��n�h�F.....");
      break;

    case 1:
      d.art += rand() % 3;
      show_play_pic(13);
      vmsg(rand() % 2 ? "���s����������  �Φ��@�T���R���e.." : "�ݵ۬ݵ�  �����h�γ������o..");
      break;

    case 2:
      d.love += rand() % 3;
      show_play_pic(14);
      vmsg(rand() % 2 ? "��  �Ӷ��֨S�J�����o..." : "�u�O�@�T����");
      break;

    case 3:
      d.hp += d.maxhp;
      show_play_pic(15);
      vmsg(rand() % 2 ? "���ڭ̺ƨg�b�]�̪����y�a....�I�I.." : "�D�n�������ﭱŧ��  �̳��w�o�طPı�F....");
    }

    /* �H���J��Ѩ� */
    if (rand() % 301 == 0)
      pip_meet_angel();
  }

  return 0;
}


int
pip_play_kite()			/* ���� */
{
  count_tired(4, 4, 1, 100, 0);
  d.weight += (rand() % 2 + 2);
  d.shit += rand() % 5 + 6;
  d.hp -= rand() % 2 + 7;
  d.affect += rand() % 4 + 6;
  d.happy += rand() % 5 + 10;
  d.satisfy += rand() % 3 + 12;

  if (d.happy > 100)
    d.happy = 100;
  if (d.satisfy > 100)
    d.satisfy = 100;

  show_play_pic(16);
  vmsg("�񭷺�u�n����...");
  return 0;
}


int
pip_play_KTV()			/* KTV */
{
  if (d.money < 250)
  {
    vmsg("�������h�աI�ۺq�`�o���I����");
  }
  else
  {
    count_tired(10, 10, 1, 100, 0);
    d.money -= 250;
    d.shit += rand() % 5 + 6;
    d.hp += rand() % 2 + 6;
    d.art += rand() % 4 + 3;
    d.happy += rand() % 3 + 20;
    d.satisfy += rand() % 2 + 20;

    if (d.happy > 100)
      d.happy = 100;
    if (d.satisfy > 100)
      d.satisfy = 100;

    show_play_pic(17);
    vmsg("�G���Ѫ�..�G���Ѫ�..�]�o��..�]�o��..");
  }
  return 0;
}


static void
guess_pip_lose()
{
  d.winn++;
  d.shit += rand() % 3 + 2;
  d.hp -= rand() % 2 + 3;
  d.satisfy--;
  d.happy -= 2;
  outs("�p����F....~>_<~");
  show_guess_pic(2);
}


static void
guess_pip_tie()
{
  d.tiee++;
  count_tired(2, 2, 1, 100, 1);
  d.shit += rand() % 3 + 2;
  d.hp -= rand() % 2 + 3;
  d.satisfy++;
  d.happy++;
  outs("����........-_-");
  show_guess_pic(3);
}


static void
guess_pip_win()
{
  d.losee++;
  count_tired(2, 2, 1, 100, 1);
  d.shit += rand() % 3 + 2;
  d.hp -= rand() % 2 + 3;
  d.satisfy += rand() % 3 + 2;
  d.happy += rand() % 3 + 5;
  outs("�p��Ĺ�o....*^_^*");
  show_guess_pic(1);
}


int
pip_play_guess()		/* �q���{�� */
{
  int mankey;		/* �ڥX���� */
  int pipkey;		/* �p���X���� */
  char msg[3][5] = {"�ŤM", "���Y", "��  "};

  out_cmd("", COLOR1 " �q�� " COLOR2 " [1]�ڥX�ŤM [2]�ڥX���Y [3]�ڥX���� [Q]���X                            \033[m");

  /* itoc.010814: �i�H�@���q�� */
  while (1)
  {
    /* �ڥ��X */
    mankey = vkey() - '1';
    if (mankey < 0 || mankey > 2)
      return 0;

    /* �p���A�X */
    pipkey = rand() % 3;

    /* �b b_lines - 2 �q�������ӭt�T�� */
    move(b_lines - 2, 0);
    prints("�z�G%s   �p���G%s    ", msg[mankey], msg[pipkey]);

    /* �P�w�ӭt */
    if (mankey == pipkey)	/* ���� */
      guess_pip_tie();
    else if (pipkey == mankey + 1 || pipkey == mankey - 2)	/* �p���� */
      guess_pip_win();
    else			/* �p���� */
      guess_pip_lose();
  }
}
#endif		/* HAVE_GAME */
