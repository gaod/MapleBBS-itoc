/*-------------------------------------------------------*/
/* pip_race.c          ( NTHU CS MapleBBS Ver 3.10 )     */
/*-------------------------------------------------------*/
/* target : ��ì�u����                                   */
/* create :   /  /                                       */
/* update : 03/03/31                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* ���ɪ̦W��	 				         */
/*-------------------------------------------------------*/


static char racename[4][9] = {"�Z���j�|", "���N�j�i", "�Ӯa�R�|", "�i���j��"};


/* name[13] attribute hp maxhp attack spirit magic armor dodge money exp pic */
/* �Ѧ� badman_generate() �������ѡA���Ǫ����Ū������ */
struct playrule racemanlist[] = 
{
  /* �۷�󵥯� 05 ���Ǫ� */	"���R����", 0,   30,   30,  50,  50,  50,  50,  50,  50,  25, 001,
  /* �۷�󵥯� 10 ���Ǫ� */	"��ڧQ�R", 0,   95,   95,  99,  99,  99,  99,  99,  99,  50, 002,
  /* �۷�󵥯� 20 ���Ǫ� */	"���g����", 0,  300,  300, 350,  40,  40, 200, 200, 200, 100, 003, 
  /* �۷�󵥯� 40 ���Ǫ� */	"���h�p��", 0, 1200, 1200, 350, 600, 200, 600, 100, 400, 200, 004, 
  /* �۷�󵥯� 60 ���Ǫ� */	"�d���Ԭ�", 0, 2700, 2700, 600, 600, 600, 600, 600, 600, 300, 005, 
  /* �۷�󵥯� 90 ���Ǫ� */ 	"���j�Դ�", 0, 6100, 6100, 900, 799, 999, 799, 999, 900, 450, 006, 
};

static int player[3];		/* �T�쳭�ɿ�⪺���X�A�j�׬O player[2] > player[1] > player[0] */


/*-------------------------------------------------------*/
/* �Z���j�|	 				         */
/*-------------------------------------------------------*/


/* itoc.030331.�C���]�p: �I�s pip_vs_man() �o�禡�i�J�԰��e���A��Ĺ�h�֤H�Ӻ⦨�Z */

static int			/* �^��: Ĺ�F�X�ӤH >=3:�a�x 2:�ȭx 1:�u�x <=0:�̫�@�W */
pip_race_eventA()
{
  int i, winorlost;
  char buf[80];

  /* �q racemanlist ���줤�D�X�T�ӳ��ɪ� */
  player[0] = rand() % 2;
  player[1] = rand() % 2 + 2;
  player[2] = rand() % 2 + 4;

  winorlost = 0;
  for (i = 0; i < 3; i++)
  {
    sprintf(buf, "�z���� %d �ӹ��O%s", i + 1, racemanlist[player[i]].name);
    vmsg(buf);

    if (pip_vs_man(racemanlist[player[i]], 0))	/* �p����ԼĤH */
      winorlost++;	/* ��� */
  }

  return winorlost;
}


/*-------------------------------------------------------*/
/* ���N�j�i	 				         */
/*-------------------------------------------------------*/


/* itoc.030331.�C���]�p: ������ d.art �M d.charm �ӨM�w���N�j�i�����Z */

static int			/* �^��: Ĺ�F�X�ӤH >=3:�a�x 2:�ȭx 1:�u�x <=0:�̫�@�W */
pip_race_eventB()
{
  /* �q racemanlist ���줤�D�X�T�ӳ��ɪ� */
  player[0] = rand() % 2 + 4;
  player[1] = rand() % 2;
  player[2] = rand() % 2 + 2;

  /* �����ݯ�O�A�S�����ɹL�{ */
  return ((d.art * 2 + d.character) / 600);
}


/*-------------------------------------------------------*/
/* �Ӯa�R�|	 				         */
/*-------------------------------------------------------*/


/* itoc.030331.�C���]�p: ������ d.art �M d.charm �ӨM�w�Ӯa�R�|�����Z */

static int			/* �^��: Ĺ�F�X�ӤH >=3:�a�x 2:�ȭx 1:�u�x <=0:�̫�@�W */
pip_race_eventC()
{
  /* �q racemanlist ���줤�D�X�T�ӳ��ɪ� */
  player[0] = rand() % 2 + 2;
  player[1] = rand() % 2 + 4;
  player[2] = rand() % 2;

  /* �����ݯ�O�A�S�����ɹL�{ */
  return ((d.art * 2 + d.charm) / 600);
}


/*-------------------------------------------------------*/
/* �i���j��	 				         */
/*-------------------------------------------------------*/


/* itoc.030331.�C���]�p: ��h�W�O�� d.cook �M d.affect �ӨM�w�i���j�ɪ����Z�A
   ���O�p�G���M�ثe�����A�k�X���ܦ��[���ĪG�A�Ϥ��h�������ĪG */

static int			/* �^��: Ĺ�F�X�ӤH >=3:�a�x 2:�ȭx 1:�u�x <=0:�̫�@�W */
pip_race_eventD()
{
  int winorlost;

  /* �q racemanlist ���줤�D�X�T�ӳ��ɪ� */
  player[0] = rand() % 2 + 4;
  player[1] = rand() % 2 + 2;
  player[2] = rand() % 2;

  winorlost = ians(b_lines - 1, 0, "�z�Q�N���ؤf�������H0)�a�` 1)�� 2)�� 3)�W 4)�� [0] ");
  if (winorlost == '1')
    winorlost = 70 - d.satisfy * 2;	/* �V�������N�X�Ӫ���~�V�a���L�� (�������|�Y�L) */
  else if (winorlost == '2')
    winorlost = d.happy * 2 - 130;	/* �V�O�ֵּN�X�Ӫ���~�V�a������ (�ܧַּ|���e) */
  else if (winorlost == '3')
    winorlost = d.shit * 2 - 130;	/* �V�O��ż�N�X�Ӫ���~�V�a���W�B (�j�K�O�W��:p) */
  else if (winorlost == '4')
    winorlost = d.sick * 2 - 130;	/* �V�O�ͯf�N�X�Ӫ���~�V�a������ (�ͯf��ı���F) */
  else
    winorlost = 0;			/* �a�`��P���A�L�� */

  winorlost += d.cook * 2 + d.affect;

  return (winorlost / 600);
}


/*-------------------------------------------------------*/
/* ���ɵ��G	 				         */
/*-------------------------------------------------------*/


static void
pip_race_ending(winorlost, mode)
  int winorlost;		/* Ĺ�F�X�ӤH >=3:�a�x 2:�ȭx 1:�u�x <=0:�̫�@�W */
  int mode;			/* �ѥ[���@�ؤ��� */
{
  char *name1, *name2, *name3, *name4;
  char buf[80];

  if (winorlost <= 0)		/* �̫�@�W */
  {
    name1 = racemanlist[player[2]].name;
    name2 = racemanlist[player[1]].name;
    name3 = racemanlist[player[0]].name;
    name4 = d.name;
  }
  else if (winorlost == 1)	/* �u�x���� 2000 */
  {
    name1 = racemanlist[player[2]].name;
    name2 = racemanlist[player[1]].name;
    name3 = d.name;
    name4 = racemanlist[player[0]].name;
    d.money += 2000;
  }
  else if (winorlost == 2)	/* �ȭx���� 5000 */
  {
    name1 = racemanlist[player[2]].name;
    name2 = d.name;
    name3 = racemanlist[player[1]].name;
    name4 = racemanlist[player[0]].name;
    d.money += 5000;
  }
  else				/* �a�x���� 10000 */
  {
    name1 = d.name;
    name2 = racemanlist[player[2]].name;
    name3 = racemanlist[player[1]].name;
    name4 = racemanlist[player[0]].name;
    d.money += 10000;
  }

  clear();
  move(6, 13);
  prints("\033[1;37m����\033[32m���� %s ���G����\033[37m����\033[m", racename[mode - 1]);
  move(8, 15);
  prints("\033[1;41m �a�x \033[0;1m��\033[1;33m%-10s\033[36m  ���� %d\033[m", name1, 10000);
  move(10, 15);
  prints("\033[1;41m �ȭx \033[0;1m��\033[1;33m%-10s\033[36m  ���� %d\033[m", name2, 5000);
  move(12, 15);
  prints("\033[1;41m �u�x \033[0;1m��\033[1;33m%-10s\033[36m  ���� %d\033[m", name3, 2000);
  move(14, 15);
  prints("\033[1;41m �̫� \033[0;1m��\033[1;33m%-10s\033[36m\033[0m", name4);
  sprintf(buf, "���~��%s�����o ��~�A�ӧa..", racename[mode - 1]);
  vmsg(buf);
}


/*-------------------------------------------------------*/
/* ��ì�u����					         */
/*-------------------------------------------------------*/


int			/* !=0:�ѥ[������ 0:���ѥ[ */
pip_race_main()		/* ��ì�u */
{
  int ch;
  int winorlost;		/* Ĺ�F�X�ӤH >=3:�a�x 2:�ȭx 1:�u�x <=0:�̫�@�W */

  clear();
  move(10, 14);
  outs("\033[1;33m�m�N�m�N�� ���W���l�t���ڭ̰e�H�ӤF��...\033[m");
  vmsg("��  ��H���}�ݬݧa...");

  show_resultshow_pic(0);

  move(b_lines - 2, 0);
  prints("[A]%s [B]%s [C]%s [D]%s [Q]���G", racename[0], racename[1], racename[2], racename[3]);
  do
  {
    ch = vkey();
  } while (ch != 'q' && (ch < 'a' || ch > 'd'));

  if (ch == 'q')
  {
    vmsg("���~���ѥ[��.....:(");
    d.happy -= rand() % 10 + 10;
    d.satisfy -= rand() % 10 + 10;
    d.relation -= rand() % 10;
    return 0;
  }

  ch -= 'a' - 1;
  show_resultshow_pic(ch);
  vmsg("���~�@���|�H���ɡ�{�b���ɶ}�l");

  switch (ch)
  {
  case 1:					/* �Z���j�| */
    winorlost = pip_race_eventA();
    d.hexp += rand() % 10 + 20 * winorlost;
    d.exp += rand() % 10 + d.level * winorlost;
    break;

  case 2:					/* ���N�j�i */
    winorlost = pip_race_eventB();
    d.art += rand() % 10 + 20 * winorlost;
    d.character += rand() % 10 + 20 * winorlost;
    break;

  case 3:					/* �Ӯa�R�| */
    winorlost = pip_race_eventC();
    d.art += rand() % 10 + 20 * winorlost;
    d.charm += rand() % 10 + 20 * winorlost;
    break;

  case 4:					/* �i���j�� */
    winorlost = pip_race_eventD();
    d.cook += rand() % 10 + 20 * winorlost;
    d.family += rand() % 10 + 20 * winorlost;
    break;
  }

  pip_race_ending(winorlost, ch);

  /* �p�G�ѥ[���ܡA��_�Ҧ��ݩ� */
  d.tired = 0;
  d.hp = d.maxhp;
  d.happy += rand() % 20;
  d.satisfy += rand() % 20;
  d.relation += rand() % 10;

  return ch;
}
#endif	/* HAVE_GAME */
