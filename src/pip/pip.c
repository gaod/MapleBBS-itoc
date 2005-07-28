/*-------------------------------------------------------*/
/* pip.c         ( NTHU CS MapleBBS Ver 3.10 )           */
/*-------------------------------------------------------*/
/* target : �i�p���C��                                   */
/* create :   /  /                                       */
/* update : 01/08/14                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME


#define	_PIPMAIN_C_	/* �� define _PIPMAIN_C_ �A�޶i pip.h */

#include "pip.h"


/*-------------------------------------------------------*/
/* �D�{��                                                */
/*-------------------------------------------------------*/

#define ALIVE   (9/10)          /* �_���H�᪺��O�O��Ӫ� 90% */

static void
pip_live_again()
{
  vs_head("�p���_����N��", str_site);

  /* itoc.010814: �_���H��n�����a�٪����U�h�A�ҥH�����u�}�a�ʡv
     �����ݩʡA�� maxhp..������ */

  /* �򥻪���� */
  d.death = 0;
  d.liveagain++;

  /* ���A���ƭ� */
  d.relation = 0;	/* �H�M�d�������ʴc�� */  
  d.happy = 20;		/* �ܱo���ּ� */
  d.satisfy = 20;	/* �ܱo�����N */

  /* ���骺�Ѽ� */
  d.weight = (rand() % 10) + 55 + (d.bbtime / 180);
  d.tired = 20;
  d.sick = 20;
  d.shit = 20;

  /* ������� */
  d.social = d.social * ALIVE;
  d.family = d.family * ALIVE;
  d.hexp = d.hexp * ALIVE;
  d.mexp = d.mexp * ALIVE;

  /* �k�s */
  d.hp = d.maxhp;
  d.mp = 0;
  d.vp = 0;
  d.sp = 0;

  vmsg("�p���S�_���F�A���n�A��e�i���F��I");

  pip_write_file();
}


static int			/* 1:�ӽЦ��\ 0:���� */
pip_apply()			/* �s�p���ӽ� */
{
  time_t now;
  struct tm *ptime;
  
  memset(&d, 0, sizeof(d));

  vs_head(BBSNAME PIPNAME, str_site);

  /* �p���R�W */
  if (!vget(2, 0, "   ���p�����Ӧnť���W�r�a�G", d.name, IDLEN + 1, DOECHO))
    return 0;

  /* 1:�� 2:�� */
  d.sex = (ians(4, 3, "�ʧO�G(1) �p������  (2) �p������ (1/2)�H[1] ") == '2') ? 2 : 1;

  move(6, 0);
  outs("   " BBSNAME PIPNAME "���C���{��������ت��k\n");
  outs("   �靈�����|�b�p�� 20 ���ɵ����C���A�çi���p�����򪺵o�i\n");
  outs("   ��S�������h�@���i��p�����`�~�����C��....");

  /* 1:���n�B���B  4:�n�B���B */
  d.wantend = (ians(9, 0, "   �z�Ʊ�p���C���O�_�n�� 20 ������(Y/N)�H[Y] ") == 'n') ? 1 : 4;

  /* �}�Y�e�� */
  show_basic_pic(0);
  vmsg("�p���ש�ϥͤF�A�Цn�n�R�L....");

  /* �}�Y�]�w�G�S���]�w����쳣�O�w�] 0 */
  time(&now);
  ptime = localtime(&now);
  sprintf(d.birth, "%02d/%02d/%02d", ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);

  /* �򥻸�� */
  d.year = ptime->tm_year;
  d.month = ptime->tm_mon + 1;
  d.day = ptime->tm_mday;

  /* ����Ѽ� */
  d.weight = rand() % 10 + 50;

  /* �����Ѽ� : �w�] 0 */

  /* �԰��Ѽ� */
  d.level = 1;
  d.hp = d.maxhp = rand() % 20 + 40;
  d.mp = d.maxmp = rand() % 20 + 40;
  d.vp = d.maxvp = rand() % 20 + 40;
  d.sp = d.maxsp = rand() % 20 + 40;

  /* �ޯ�Ѽ� : �w�] 0 */
  /* �Z���Ѽ� : �w�] 0 */
  /* ��O�Ѽ� : �w�] 0 */

  /* ���A�ƭ� */
  d.happy = rand() % 10 + 45;
  d.satisfy = rand() % 10 + 45;

  /* �����Ѽ� */
  d.food = 20;
  d.cookie = 2;

  /* ���~�Ѽ� : �w�] 0 */
  d.money = 1000;

  /* �q���Ѽ� : �w�] 0 */

  /* �Ѩ����� */
  d.seeroyalJ = 1;

  /* �����D�B�R�H : �w�] 0 */
  /* �u�@���� : �w�] 0 */

  /* �@�ͥu��Ǳo�@���S��ޯ� */
  pip_learn_skill(0);

  pip_write_file();
  return 1;
}


static int
pip_reborn()			/* �p������ */
{
  vs_head(BBSNAME PIPNAME, str_site);
  move(4, 0);
  outs("   �w��Ө�\033[1;33m" BBSNAME "�ͪ���ެ�s�|\033[m\n\n");
  outs("   �g�ڭ̽լd���  ���e�z���i�L�p����  �i�O�Q�z�i���F\n\n");

  if (ians(7, 3, "�z�n�ڭ����e���Ͷ�(Y/N)�H[N] ") == 'y')
  {
    pip_live_again();
    return 1;
  }	
  else
  {
    if (pip_apply())
      return 1;
  }

  return 0;
}  


/* �C���D�{�� */
int
main_pip()
{
  int ch;
  char fpath[64];

  /* more(PIP_PICHOME "pip.welcome", NULL); */
  vs_head("�q�l�i�p��", str_site);

  usr_fpath(fpath, cuser.userid, fn_pip);

  if (!dashf(fpath))	/* ���e�S���L�p�� */
  {
    show_system_pic(11);

    if (vkey() == 'q')
      return 0;

    if (!pip_apply())
      return 0;
  }
  else
  {
    show_system_pic(12);

    if ((ch = vkey()) == 'r')
    {
      if (!pip_read_backup())
        return 0;
    }
    else if (ch == 'q')
    {
      return 0;
    }
    else
    {
      pip_read_file(cuser.userid, &d);

      if (d.death == 1)		/* �����F�i��ܭ��� */
      {
        if (!pip_reborn())
          return 0;
      }
      else if (d.death)		/* ��ͩε������ܭn���s�}�l */
      {
        if (!pip_apply())
          return 0;
      }
    }
  }

  start_time = time(0);
  last_time = start_time;

  /* itoc.010816: �ѩ�Ĥ@���i�J�D���S���������p���ϡA�ҥH�n�t�~�� */
  clrfromto(5, 18);
  outs("\033[34m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m");
  show_basic_pic(21);
  move(18, 0);
  outs("\033[34m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");

  pip_main_menu();

  pip_write_file();		/* ���}�C���۰ʦs�� */
  return 0;
}
#endif		/* HAVE_GAME */
