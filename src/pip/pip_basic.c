/*-------------------------------------------------------*/
/* pip_basic.c         ( NTHU CS MapleBBS Ver 3.10 )     */
/*-------------------------------------------------------*/
/* target : �򥻿��                                     */
/* create :   /  /                                       */
/* update : 01/08/14                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* �򥻿��:���� �M�� �˿� ��	������			 */
/*-------------------------------------------------------*/


int				/* 1: �S�Y�����A���  0: �Y�F */
pip_basic_feed()		/* ���� */
{
  int ch;
  int nodone;			/* itoc.010731: �O���O�_����� */

  nodone = 1;

  do
  {
    out_cmd(COLOR1 " �@�� " COLOR2 " [1]�Y�� [2]�s�� [3]�ѥ� [4]���� [5]Ū�� [Q]���X                        \033[m", 
      COLOR1 " �ī~ " COLOR2 " [a]�j�� [b]�F�� [c]�ɤY [d]�H�x [e]�¥� [f]���� [Q]���X                \033[m");

    switch (ch = vkey())
    {
    case '1':		/* �Y�� */
      if (d.food <= 0)
      {
	vmsg("�S�������o..�֥h�R�a�I");
	break;
      }
      d.food--;
      d.hp += 50;
      if (d.hp > d.maxhp)
      {
	d.hp = d.maxhp;
	d.weight += rand() % 2;
      }
      nodone = 0;
      if ((d.bbtime / 60 / 30) < 3)		/* �����T�� */
	show_feed_pic(11);
      else
	show_feed_pic(12);
      vmsg("�C�Y�@�������|��_��O50��!");
      break;

    case '2':		/* �s�� */
      if (d.cookie <= 0)
      {
	vmsg("�s���Y���o..�֥h�R�a�I");
	break;
      }
      d.cookie--;
      d.hp += 100;
      if (d.hp > d.maxhp)
      {
	d.hp = d.maxhp;
	d.weight += rand() % 2 + 2;
      }
      else
      {
	d.weight += (rand() % 2 + 1);
      }
      d.happy += (rand() % 3 + 4);
      d.satisfy += rand() % 3 + 2;
      nodone = 0;
      if (rand() % 2)
	show_feed_pic(21);
      else
	show_feed_pic(22);
      vmsg("�Y�s���e���D��...");
      break;

    case '3':		/* �ѥ� */
      if (d.book <= 0)
      {
	vmsg("�S���ѥ��o..�֥h�R�a�I");
	break;
      }
      d.book--;
      d.happy -= 5;
      d.wisdom+= 20;
      d.art += 20;
      nodone = 0;
      show_feed_pic(31);
      vmsg("�}�����q");
      break;

    case '4':		/* ���� */
      if (d.toy <= 0)
      {
	vmsg("�S�������o..�֥h�R�a�I");
	break;
      }
      d.toy--;
      d.happy += 20;
      d.satisfy += 20;
      nodone = 0;
      show_feed_pic(41);
      vmsg("�����㪺�p�Ĥ��|���a");
      break;

    case '5':		/* Ū�� */
      if (d.playboy <= 0)
      {
	vmsg("�S���ҥ~Ū���o..�֥h�R�a�I");
	break;
      }
      if ((d.bbtime / 60 / 30) < 5)
      {
        /* itoc.010801: �����H��~��� :p */
        vmsg("�ʭ��W�g�� 5 �T ��");
        break;
      }
      d.playboy--;
      /* itoc.010801: �W�[�o�c�A���ּ�/���N�j�q�W�� */
      d.happy = 100;
      d.satisfy = 100;
      d.art += 5;
      d.sin += 30;
      nodone = 0;
      show_feed_pic(51);
      vmsg("�I�I�A�٦n�S�H�ݨ�");
      break;

    case 'a':		/* �j�� */
      if (d.pill <= 0)
      {
	vmsg("�S���j�٤��o..�֥h�R�a�I");
	break;
      }
      d.pill--;
      d.hp += 1000;
      if (d.hp > d.maxhp)
	d.hp = d.maxhp;
      nodone = 0;
      show_feed_pic(61);
      vmsg("�C�Y�@���j�٤��|��_��O 1000 ��!");
      break;

    case 'b':
      if (d.medicine <= 0)
      {
	vmsg("�S���F���o..�֥h�R�a�I");
	break;
      }
      d.medicine--;
      d.mp += 1000;
      if (d.mp > d.maxmp)
	d.mp = d.maxmp;
      nodone = 0;
      show_feed_pic(71);
      vmsg("�C�Y�@���F�۷|��_�k�O 1000 ��!");
      break;

    case 'c':		/* �ɤY */
      if (d.burger <= 0)
      {
	vmsg("�S���j�ɤY�F�C! �֥h�R�a..");
	break;
      }
      d.burger--;
      d.vp += 1000;
      if (d.vp > d.maxvp)
	d.vp = d.maxvp;
      nodone = 0;
      show_feed_pic(81);
      vmsg("�C�Y�@���ɤY�|��_���� 1000 ��!");
      break;

    case 'd':		/* �H�x */
      if (d.ginseng <= 0)
      {
	vmsg("�S���d�~�H�x�C! �֥h�R�a..");
	break;
      }
      d.ginseng--;
      d.sp += 1000;
      if (d.sp > d.maxsp)
        d.sp = d.maxsp;
      nodone = 0;
      show_feed_pic(91);
      vmsg("�C�Y�@���H�x�|��_���O 1000 ��!");
      break;

    case 'e':		/* �¥� */
      if (d.paste <= 0)
      {
	vmsg("�S���¥��_��I�C! �֥h�R�a..");
	break;
      }
      d.snowgrass--;
      d.hp = d.maxhp;
      d.tired = 0;
      d.sick = 0;
      nodone = 0;
      show_feed_pic(101);
      vmsg("�¥��_��I..�W���Ϊ���...");
      break;

    case 'f':		/* ���� */
      if (d.snowgrass <= 0)
      {
	vmsg("�S���Ѥs�����C! �֥h�R�a..");
	break;
      }
      d.snowgrass--;
      d.hp = d.maxhp;
      d.mp = d.maxmp;
      d.vp = d.maxvp;
      d.sp = d.maxsp;
      d.tired = 0;
      d.sick = 0;
      nodone = 0;
      show_feed_pic(111);
      vmsg("�Ѥs����..�W���Ϊ���...");
      break;
    }
  } while (ch != 'q' && ch != KEY_LEFT);

  return nodone;
}


int
pip_basic_takeshower()		/* �~�� */
{
  d.shit -= 20;
  if (d.shit < 0)
    d.shit = 0;

  d.hp -= rand() % 2 + 3;

  switch(rand() % 3)
  {
  case 0:
    show_usual_pic(1);
    vmsg("�ڬO���b���p��  cccc....");
    break;

  case 1:
    show_usual_pic(7);
    vmsg("���� ����");
    break;

  case 2: 
    show_usual_pic(2);
    vmsg("�ڷR�~�� lalala....");
    break;
  }
  return 0;
}


int
pip_basic_takerest()		/* �� */
{
  count_tired(5, 20, 1, 100, 0);	/* ��_�h�� */
  d.shit++;
  d.hp += d.maxhp / 10;
  if (d.hp > d.maxhp)
    d.hp = d.maxhp;

  show_usual_pic(5);
  vmsg("�A���@�U�ڴN�_���o....");
  show_usual_pic(6);
  vmsg("�޳޳�..�Ӱ_���o......");
  return 0;
}


int
pip_basic_kiss()		/* �˿� */
{
  if (rand() % 2)
  {
    d.happy += rand() % 3 + 4;
    d.satisfy += rand() % 2 + 1;
  }
  else
  {
    d.happy += rand() % 2 + 1;
    d.satisfy += rand() % 3 + 4;
  }
  count_tired(1, 2, 0, 100, 1);
  d.shit += rand() % 5 + 4;
  d.relation += rand() % 2;

  show_usual_pic(3);

  if (d.shit < 60)
    vmsg("�ӹ�! �q�@��.....");
  else
    vmsg("�ˤӦh�]�O�|ż������....");

  return 0;
}


int
pip_money()
{
  int money;
  char buf[80];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return 0;
  }

  /* itoc.����: ���ҥH�����Ѥp�������ȹ�����]�O�]���p���i�H�x�s/Ū���i�� */

  clrfromto(6, 18);
  prints("�z���W�� %d �ȹ�,���� %d ��\n", cuser.money, d.money);
  outs("\n�@�ȹ����@������I\n");

  do
  {
    if (!vget(10, 0, "�n���h�ֻȹ��H[Q] ", buf, 10, DOECHO))
      return 0;
    money = atol(buf);
  } while (money <= 0 || money > cuser.money);

  sprintf(buf, "�O�_�n�ഫ %d �ȹ� �� %d ����(Y/N)�H[N] ", money, money);
  if (ians(11, 0, buf) == 'y')
  {
    cuser.money -= money;
    d.money += money;
    sprintf(buf, "�z���W�� %d ���ȹ�,���� %d ��", cuser.money, d.money);
    vmsg(buf);
    return 1;
  }
  vmsg("����....");
  return 0;
}
#endif		/* HAVE_GAME */
