/* ----------------------------------------------------- */
/* pip_royal.c     ( NTHU CS MapleBBS Ver 3.10 )         */
/* ----------------------------------------------------- */
/* target : �p�� royal                                   */
/* create :   /  /                                       */
/* update : 01/08/14                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/* ----------------------------------------------------- */


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/* royalset:  num name needmode needvalue addtoman maxtoman words1 words2 */

struct royalset royallist[] = 
{
  "T", "���X��H",   0,   0,   0,   0, NULL, NULL,
  "A", "�ӫ��M�L�s", 1,  10,  15, 100, "�z�u�n�A�ӳ��ڲ��..",		"�u�ìP�Ū��w���O�ܨ��W��..",
  "B", "�������S��", 1, 100,  25, 200, "�u�O§�����p��..�ڳ��w..",	"�S�ȴN�O���K�O�@�����w�����H..",
  "C", "���j�N�x", 1, 200,  30, 250, "��~���ӾԧЫܺ�m��..",	"�z�u�O���Q�u�����p��..",
  "D", "�ѿ��`�Ȫ�", 1, 300,  35, 300, "���������޲z�o�Ӱ�a��..",	"�z���n���ܦnť�C..�ګܳ��w��..:)",
  "E", "�޲z�Ư���", 1, 400,  35, 300, "�z�ܦ��оi��I�ܰ����{�ѱz..",	"�u�����z�A���������z���..",
  "F", "�t�ί���",   1, 500,  40, 350, "�z�n�i�R��..�ڳ��w�z��..",	"���..�H��n�h�h�өM�ڪ���..",
  "G", "�{������",   1, 550,  40, 350, "�i�D�z��A��z���ܫܧֳּ�..",	"�ӡA���ڽ��\\�W�Ať�����G��..",
  "H", SYSOPNICK,    1, 600,  50, 400, "�@�������d�����j�r..:)..",	"���±zť������..�H��n�h�ӳ�..",
  "I", "�ƨg����s", 2,  60,  20, 150, "������..�Z���F����..�ܥi�R..",	"�ӡA�ڭ̤@�_������a..",
  "J", "�C�~�ӪZ�x", 0,   0,   0,   0, "�z�n�A�ڬO�Z�x�A��q��Ҧ^��",	"�Ʊ�U���ٯਣ��z..:)",
  NULL, NULL,        0,   0,   0,   0, NULL, NULL
};


static int
pip_go_palace_screen(p)
  struct royalset *p;
{
  char inbuf1[128], inbuf2[20];
  char *needmode[3] = {"      ", "§����{��", "�ͦR�ޥ���"};
  int n, a, b, choice, change;
  int save[11];

  /* �q�X�Ҧ��i�H���X���H */
  clear();
  show_palace_pic(0);
  move(13, 0);
  outs("    \033[1;31m�z�w�w�w�w�w�w�t\033[37;41m �Ө��`�q�O���F   �п�ܱz�����X����H\033[0;1;31m�u�w�w�w�w�w�w�{\033[m\n");
  outs("    \033[1;31m�x                                                                  �x\033[m\n");

  for (n = 0; n < 5; n++)
  {
    a = 2 * n + 1;
    b = 2 * n + 2;
    sprintf(inbuf1, "%-10s%3d", needmode[p[a].needmode], p[a].needvalue);

    if (n == 4)	/* ���l */
      sprintf(inbuf2, "%-10s", needmode[p[b].needmode]);
    else
      sprintf(inbuf2, "%-10s%3d", needmode[p[b].needmode], p[b].needvalue);

    if ((d.seeroyalJ == 1 && n == 4) || (n != 4))
    {
      prints("    \033[1;31m�x\033[36m(\033[37m%s\033[36m)\033[33m%-10s \033[37m%-14s    \033[36m(\033[37m%s\033[36m)\033[33m%-10s \033[37m%-14s      \033[31m�x\033[m\n",
	p[a].num, p[a].name, inbuf1, p[b].num, p[b].name, inbuf2);
    }
    else
    {
      prints("    \033[1;31m�x\033[36m(\033[37m%s\033[36m)\033[33m%-10s \033[37m%-14s                                        \033[31m�x\033[0m\n", 
  	p[a].num, p[a].name, inbuf1);
    }
  }
  outs("    \033[1;31m�x                                                                  �x\033[m\n");
  outs("    \033[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");

  while (1)
  {
    sprintf(inbuf1, COLOR1 " [�ͩR�O] %6d/%6d  [�h�ҫ�] %6d                                      \033[m", d.hp, d.maxhp, d.tired);
    out_cmd(inbuf1, COLOR1 " �Ѩ���� " COLOR2 " [�r��]��ܱ����X���H�� [Q]���}�`�q�O��                             \033[m");

    choice = vkey();
    if (choice == 'q' || choice == KEY_LEFT)
    {
      vmsg("���}" BBSNAME "�`�q�O��.....");
      return 0;
    }

    /* �N�U�H���w�g���P���ƭȥ��x�s�_��*/
    save[1] = d.royalA;		/* from�u�� */
    save[2] = d.royalB;		/* from��� */
    save[3] = d.royalC;		/* from�N�x */
    save[4] = d.royalD;		/* from�j�� */
    save[5] = d.royalE;		/* from���q */
    save[6] = d.royalF;		/* from�d�m */
    save[7] = d.royalG;		/* from���m */
    save[8] = d.royalH;		/* from��� */
    save[9] = d.royalI;		/* from�p�� */
    save[10] = d.royalJ;	/* from���l */

    choice -= 'a' - 1;

    if ((choice >= 1 && choice <= 10 && d.seeroyalJ == 1) || (choice >= 1 && choice <= 9 && d.seeroyalJ == 0))
    {
      d.social += rand() % 3 + 3;
      d.hp -= rand() % 5 + 6;
      d.tired += rand() % 5 + 8;

      if ((p[choice].needmode == 0) || (p[choice].needmode == 1 && d.manners >= p[choice].needvalue) || 
        (p[choice].needmode == 2 && d.speech >= p[choice].needvalue))
      {
	if (choice >= 1 && choice <= 9 && save[choice] >= p[choice].maxtoman)
	{
	  vmsg(rand() % 2 ? "��M�o�򰶤j���z���ܯu�O�a����..." : "�ܰ����z�ӫ��X�ڡA���ڤ��൹�z����F..");
	}
	else
	{
	  if (choice >= 1 && choice <= 8)	/* �ʨ��x���A�W�[�ݤH���� */
	  {
	    switch (choice)
	    {
	    case 1:
	      change = d.character / 5;
	      break;
	    case 2:
	      change = d.character / 8;
	      break;
	    case 3:
	      change = d.charm / 5;
	      break;
	    case 4:
	      change = d.wisdom / 10;
	      break;
	    case 5:
	      change = d.belief / 10;
	      break;
	    case 6:
	      change = d.speech / 10;
	      break;
	    case 7:
	      change = d.social / 10;
	      break;
	    case 8:
	      change = d.hexp / 10;
	      break;
	    }

	    if (change > p[choice].addtoman)		/* �p�G�j��C�����W�[�̤j�q */
	      change = p[choice].addtoman;
	    else if ((change + save[choice]) >= p[choice].maxtoman)	/* �p�G�[�W���������j��ү൹���Ҧ��Ȯ� */
	      change = p[choice].maxtoman - save[choice];

	    save[choice] += change;
	    d.toman += change;
	  }
	  else if (choice == 9)			/* ��p�� */
	  {
	    save[9] = 0;
	    d.social -= 13 + rand() % 4;
	    d.affect += 13 + rand() % 4;
	  }
	  else if (choice == 10 && d.seeroyalJ == 1)	/* ���X���l */
	  {
	    save[10] += 15 + rand() % 4;
	    d.seeroyalJ = 0;
	  }

	  vmsg(rand() % 2 ? p[choice].words1 : p[choice].words2);
	}
      }
      else
      {
	vmsg(rand() % 2 ? "�ڤ��M�o�˪����͸�...." : "�S�оi�����A�A�h�Ǿ�§���a....");
      }
    }

    d.royalA = save[1];
    d.royalB = save[2];
    d.royalC = save[3];
    d.royalD = save[4];
    d.royalE = save[5];
    d.royalF = save[6];
    d.royalG = save[7];
    d.royalH = save[8];
    d.royalI = save[9];
    d.royalJ = save[10];
  }
}


int
pip_go_palace()			/* �Ѩ� */
{
  pip_go_palace_screen(royallist);
  return 0;
}
#endif	/* HAVE_GAME */

