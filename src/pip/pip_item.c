/* ----------------------------------------------------- */
/* pip_item.c     ( NTHU CS MapleBBS Ver 3.10 )          */
/* ----------------------------------------------------- */
/* target : �p�� item                                    */
/* create :   /  /                                       */
/* update : 01/08/14                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */  
/* ----------------------------------------------------- */


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


struct itemset pipfoodlist[] = 
{
  /*  name          msgbuy           msgfeed                        price */
  0, "���~�W",     "�ʶR����",      "�ϥζ���",                        0, 
  1, "�n�Y������", "��O��_ 50",   "�C�Y�@�������|��_��O 50 ��",   50, 
  2, "�������s��", "��O��_ 100",  "���F��_��O�A�p���]�|��ּ�",  120, 
  0, NULL, NULL, NULL, 0
};


struct itemset pipmedicinelist[] = 
{
  /*  name          msgbuy           msgfeed                        price */
  0, "���~�W",     "�ʶR����",      "�ϥζ���",                        0, 
  1, "�ɦ�j�٤�", "��O��_ 1000", "��_�j�q�y����O���}��",       1000, 
  2, "�öQ���F��", "�k�O��_ 1000", "��_�j�q�y���k�O���}��",       1000, 
  3, "�n�Τj�ɤY", "���ʫ�_ 1000", "��_�j�q�y�����ʪ��}��",       1000, 
  4, "�d�~�H�Ѥ�", "���O��_ 1000", "��_�j�q�y�����O���}��",       1000, 
  5, "�¥��_��I", "��O������_",  "�ǻ�����N�Ҧ����˫�_���ħ�", 5000, 
  6, "�Ѥs����",   "���A������_",  "�F�_�Ѥs�~�������l",          10000, 
  0, NULL, NULL, NULL, 0
};


struct itemset pipotherlist[] = 
{
  /*  name          msgbuy           msgfeed                        price */
  0, "���~�W",     "�ʶR����",      "�ϥζ���",                        0, 
  1, "�ʬ����",   "���Ѫ��ӷ�",    "�ѥ����p�����o���󦳮���",   3000, 
  2, "�ְ������", "�ֺּ��N��",    "�������p����ְּ�",            300, 
  3, "�ռ����x",   "�������ַP",    "�Ѥ��ۦ��C�p�ɰ�",              500, 
  0, NULL, NULL, NULL, 0
};


/* ------------------------------------------------------- */
/* ���~�ʶR�禡                                            */
/* ------------------------------------------------------- */


int
pip_buy_item(mode, p, oldnum)
  int mode;
  int oldnum[];
  struct itemset *p;
{
  char *shopname[4] = {"���W", "�K�Q�ө�", "���K�ľQ", "�]�̮ѧ�"};
  char buf[128], genbuf[20];
  int oldmoney;		/* �i�ө��e�즳�� */
  int total;		/* �ʶR/�c��Ӽ� */
  int ch, choice;

  oldmoney = d.money;

  /* �q�X���~�C�� */
  clrfromto(6, 18);
  outs("\033[1;31m  �w\033[41;37m �s��\033[0;1;31m�w\033[41;37m ��      �~\033[0;1;31m�w�w\033[41;37m ��            ��\033[0;1;31m�w�w\033[41;37m ��     ��\033[0;1;31m�w\033[37;41m �֦��ƶq\033[0;1;31m�w\033[m\n\n");
  for (ch = 1; ch <= oldnum[0]; ch++)
  {
    prints("    \033[1;35m[\033[37m%2d\033[35m]    \033[36m%-10s     \033[37m%-14s       \033[1;33m%-10d  \033[1;32m%-9d   \033[m\n",
      p[ch].num, p[ch].name, p[ch].msgbuy, p[ch].price, oldnum[ch]);
  }

  do
  {
    sprintf(buf, COLOR1 " �ĶR " COLOR2 " (%8s) [B]�R�J���~ [S]��X���~ [Q]���X                             \033[m", shopname[mode]);
    out_cmd("", buf);

    switch (ch = vkey())
    {
    case 'b':
      sprintf(buf, "�Q�n�R�Jԣ�O�H[0]���R�J [1��%d]���~�Ӹ��G", oldnum[0]);
      choice = ians(b_lines - 2, 0, buf) - '0';
      if (choice >= 1 && choice <= oldnum[0])
      {
	sprintf(buf, "�z�n�R�J���~ [%s] �h�֭өO(1-%d)�H[Q] ", p[choice].name, d.money / p[choice].price);
	vget(b_lines - 2, 0, buf, genbuf, 6, DOECHO);
	total = atoi(genbuf);

	if (total <= 0)
	{
	  vmsg("���R�J...");
	}
	else if (d.money < total * p[choice].price)
	{
	  vmsg("�z�����S������h��..");
	}
	else
	{
	  sprintf(buf, "�T�w�R�J�`���� %d �����~ [%s] �ƶq %d �Ӷ�(Y/N)�H[N] ", total * p[choice].price, p[choice].name, total);
	  if (ians(b_lines - 2, 0, buf) == 'y')
	  {
	    oldnum[choice] += total;
	    d.money -= total * p[choice].price;

	    /* itoc.010816: ��s�֦��ƶq */
	    move(7 + choice, 0);
	    prints("    \033[1;35m[\033[37m%2d\033[35m]    \033[36m%-10s     \033[37m%-14s       \033[1;33m%-10d  \033[1;32m%-9d   \033[m",
	      p[choice].num, p[choice].name, p[choice].msgbuy, p[choice].price, oldnum[choice]);

	    vmsg(p[choice].msguse);
	  }
	  else
	  {
	    vmsg("���R�J...");
	  }
	}
      }
      else
      {
	sprintf(buf, "���R�J.....");
	vmsg(buf);
      }
      break;

    case 's':
      sprintf(buf, "�Q�n��Xԣ�O�H[0]����X [1��%d]���~�Ӹ�: ", oldnum[0]);
      choice = ians(b_lines - 2, 0, buf) - '0';
      if (choice >= 1 && choice <= oldnum[0])
      {
	sprintf(buf, "�z�n��X���~ [%s] �h�֭өO(1-%d)�H[Q] ", p[choice].name, oldnum[choice]);
	vget(b_lines - 2, 0, buf, genbuf, 6, DOECHO);
	total = atoi(genbuf);

	if (total <= 0)
	{
	  vmsg("����X...");
	}
	else if (total > oldnum[choice])
	{
	  sprintf(buf, "�z�� [%s] �S������h�ӳ�", p[choice].name);
	  vmsg(buf);
	}
	else
	{
	  sprintf(buf, "�T�w��X�`���� %d �����~ [%s] �ƶq %d �Ӷ�(Y/N)�H[N] ", total * p[choice].price * 4 / 5, p[choice].name, total);
	  if (ians(b_lines - 2, 0, buf) == 'y')
	  {
	    oldnum[choice] -= total;
	    d.money += total * p[choice].price * 8 / 10;

	    /* itoc.010816: ��s�֦��ƶq */
	    move(7 + choice, 0);
	    prints("    \033[1;35m[\033[37m%2d\033[35m]    \033[36m%-10s     \033[37m%-14s       \033[1;33m%-10d  \033[1;32m%-9d   \033[m",
	      p[choice].num, p[choice].name, p[choice].msgbuy, p[choice].price, oldnum[choice]);

	    sprintf(buf, "���󮳨��F�z�� %d ��%s", total,  p[choice].name);
	    vmsg(buf);
	  }
	  else
	  {
	    vmsg("����X...");
	  }
	}
      }
      else
      {
	sprintf(buf, "����X.....");
	vmsg(buf);
      }
      break;

    case 'q':
    case KEY_LEFT:
      sprintf(buf, "��������@ %d ��,���} %s ", oldmoney - d.money, shopname[mode]);
      vmsg(buf);
      break;
    }

    /* itoc.010816: ���� ians() vget() �d�U�����e */
    move (b_lines - 2, 0);
    clrtoeol();

  } while (ch != 'q' && ch != KEY_LEFT);

  return 0;
}


/*-------------------------------------------------------*/
/* �ө����:���� �s�� �j�ɤY ���� �ѥ�			 */
/*-------------------------------------------------------*/

/*-------------------------------------------------------*/
/* �禡�w                      				 */
/*-------------------------------------------------------*/

int
pip_store_food()
{
  int num[3];
  num[0] = 2;
  num[1] = d.food;
  num[2] = d.cookie;
  pip_buy_item(1, pipfoodlist, num);
  d.food = num[1];
  d.cookie = num[2];
  return 0;
}


int
pip_store_medicine()
{
  int num[7];
  num[0] = 6;
  num[1] = d.pill;  
  num[2] = d.medicine;
  num[3] = d.burger;
  num[4] = d.ginseng;
  num[5] = d.paste;
  num[6] = d.snowgrass;
  pip_buy_item(2, pipmedicinelist, num);
  d.pill = num[1];
  d.medicine = num[2];
  d.burger = num[3];
  d.ginseng = num[4];
  d.paste = num[5];
  d.snowgrass = num[6];
  return 0;
}


int
pip_store_other()
{
  int num[4];
  num[0] = 3;
  num[1] = d.book;
  num[2] = d.toy;
  num[3] = d.playboy;
  pip_buy_item(3, pipotherlist, num);
  d.book = num[1];
  d.toy = num[2];
  d.playboy = num[3];
  return 0;
}
#endif		/* HAVE_GAME */
