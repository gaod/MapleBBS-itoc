/* ----------------------------------------------------- */
/* pip_quest.c        ( NTHU CS MapleBBS Ver 3.10 )      */
/* ----------------------------------------------------- */
/* target : �԰����                                     */
/* create : 01/12/22                                     */
/* update :   /  /  		  			 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */  
/* ----------------------------------------------------- */


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* �Ҧ�����: �^�� 1 ��ܧ���   �^�� 0 ��ܨS������       */
/*-------------------------------------------------------*/


/* �g���ȫD�`²��G�g�@�Ө禡 pip_quest_??()�A�å[�J quest_cb�A
   �ç� PIPQUEST_NUM�A�̫�s��@�� etc/game/pip/quest/pic?? �Y�i */


  /*-----------------------------------------------------*/
  /* pip_quest_1~99 ���o���~������			 */
  /*-----------------------------------------------------*/


static int
pip_quest_1()		/* ���o��¦�˳� */
{
  /* �����C�ӳ��쳣��۸˳ƴN��X�� */
  if (d.weaponhead && d.weaponhand && d.weaponshield && d.weaponbody && d.weaponfoot)
  {
    /* ��˳Ʋ��� */
    d.weaponhead = d.weaponhand = d.weaponshield = d.weaponbody = d.weaponfoot = 0;
    d.equiphead[0] = d.equiphand[0] = d.equipshield[0] = d.equipbody[0] = d.equipfoot[0] = '\0';

    d.tired = 0;
    vmsg("�z���A�P��h�ΤF");
    return 1;
  }

  return 0;
}


static int
pip_quest_2()		/* ���o�����˳� */
{
  /* �����C�ӳ��쪺�˳ƦW�ٳ��]�t�u���v�o�r�N��X�� */
  if (strstr(d.equiphead, "��") && strstr(d.equiphand, "��") &&
    strstr(d.equipshield, "��") && strstr(d.equipbody, "��") && strstr(d.equipfoot, "��"))
  {
    /* ��˳Ʋ��� */
    d.weaponhead = d.weaponhand = d.weaponshield = d.weaponbody = d.weaponfoot = 0;
    d.equiphead[0] = d.equiphand[0] = d.equipshield[0] = d.equipbody[0] = d.equipfoot[0] = '\0';

    d.happy = 100;
    d.tired = 0;
    vmsg("�z���믫�ʭ�");
    return 1;
  }

  return 0;
}


static int
pip_quest_3()		/* ���o�ħ� */
{
  /* ���o�j�٤��B�F�ۡB�j�ɤY�B�d�~�H�x�B�¥��_��I�B�Ѥs�����U�@ */
  if (d.pill && d.medicine && d.burger && d.ginseng && d.paste && d.snowgrass)
  {
    d.pill--;
    d.medicine--;
    d.burger--;
    d.ginseng--;
    d.paste--;
    d.snowgrass--;
    d.happy = 100;
    vmsg("�U�H���ּ֤���");
    return 1;
  }

  return 0;
}


  /*-----------------------------------------------------*/
  /* pip_quest_101~199 �ݩʹF��������			 */
  /*-----------------------------------------------------*/


static int
pip_quest_101()		/* �������d */
{
  /* �F�줣�h�֡B���ͯf�B����ż */
  if (d.tired == 0 && d.sick == 0 && d.shit == 0)
  {
    d.food++;
    d.cookie++;
    vmsg("�u�ΡA�k����z�@�ǭ���");
    return 1;
  }

  return 0;
}


  /*-----------------------------------------------------*/
  /* pip_quest_201~299 ���ѩǪ�������			 */
  /*-----------------------------------------------------*/


static int
pip_quest_201()		/* ���˯f�r */
{
  /* ���ˤ@����O��ۤv�����Ǫ� */

  int level;
  playrule m;

  strcpy(m.name, "�ܺدf�r");  
  level = d.level + 5;
  m.hp = m.maxhp = 100 + level * level;
  m.attack = m.spirit = m.magic = m.armor = m.dodge = level * 15;
  m.money = 0;
  m.exp = 0;
  m.attribute = +7;		/* �M��: �s���k�N */ 
  m.pic = 004;

  if (pip_vs_man(m, 0))		/* �p����ԼĤH (�ɥΪZ�N�j�|) */
  {
    d.hexp += d.level;
    d.mexp += d.level;
    vmsg("�z���ѤF�o���ܺدf�r�A��������");
    return 1;
  }

  return 0;
}


static int
pip_quest_202()		/* �Q�j�c�H */
{
  /* �̧ǥ��˼ư��Ǫ� */
  /* �Y���ѤQ�j�c�H�A�W�n�����F�Ϥ��A�m�����篫�\ */

  int i;
  struct playrule badmanlist[] =	/* �Q�j�c�H�۷�󵥯� 40 ~ 50 ���Ǫ� */
  {
    /* name[13] attribute hp maxhp attack spirit magic armor dodge money exp pic */
    /* �R�Χl�� */	"���j�L", +3, 1875, 1875, 500, 460, 350, 500, 500, 0, 220, 001,
    /* �R�� blitz */	"���E��", +2, 1850, 1850, 520, 470, 320, 420, 630, 0, 220, 002,
    /* �R�ηt�� */	"������", +7, 1750, 1750, 450, 450, 370, 400, 520, 0, 220, 003,
    /* �R�Ϊ��t�]�k */	"�O�b�b", -4, 1635, 1635, 430, 340, 640, 360, 470, 0, 220, 004,
    /* �����O�S�j */	"��  ��",  0, 2400, 2400, 610, 570, 280, 550, 500, 0, 250, 005,
  };

  if (ians(b_lines - 1, 0, "�Q�j�c�H�ݰ_�ӶW�j�A�z�n�������(Y/N)�H[Y] ") != 'n')
  {
    if (ians(b_lines - 1, 0, "�нַ�����H(1)�P�F�� (2)�P��� (3)�P�n�� (4)�P�_�� ") == '3')
    {
      /* itoc.050320: �Y���ŤӧC�ɱ���o�ӥ��ȷ|����Ĺ�A�ҥH�n���ѽ�� :p */
      vmsg("�b�j�L�P�n�Ѫ����U���U�A�z���\\�a���h�Q�j�c�H");
      return 1;
    }
    else
    {
      vmsg("�L�ڵ��F�z���ШD�A�ݨӱz�u�n�ۤv�W�F");
    }
  }

  for (i = 0; i< 5; i++)
  {
    if (!pip_vs_man(badmanlist[i], 0))	/* �p����ԼĤH (�ɥΪZ�N�j�|) */
    {
      vmsg("�z�Q�Q�j�c�H���A������߭��_");
      d.hp = 1;
      d.mp = 0;
      d.vp = 0;
      d.sp = 0;

      /* ���篫�\: �� maxmp �h�� maxsp */
      i = rand() % 10;
      if (d.maxmp > i)
      {
        d.maxmp -= i;
        d.maxsp += i;
        vmsg("�b����U�K�y���v�����U�A�z�Ͻm�����篫�\\");
      }

      return 0;
    }
  }

  d.hexp += 100;
  d.mexp += 100;
  vmsg("�z���\\�a���h�c�H�����Ҧ��a�J�A�W�n�j�T����");
  return 1;
}


  /*-----------------------------------------------------*/
  /* pip_quest_301~399 �ƾǭp�⪺����			 */
  /*-----------------------------------------------------*/


static int
pip_quest_301()		/* ���t�]�_ */
{
  char ans[3];

  vget(b_lines, 0, "�����ӥi�H����X�ӯ]�_�O�H", ans, 3, DOECHO);
  if (atoi(ans) == 2)
  {
    d.social += 10;
    vmsg("���ڷQ�Q�ݰڡI���A�S���A�z��b���o���F");
    return 1;
  }

  return 0;
}


static int
pip_quest_302()		/* 0.9999... �`���p�� */
{
  int num;
  char ans1[4], ans2[4];

  if (vget(b_lines, 0, "�`���p�� 0.9999... �Ƭ����ơA�����O ", ans1, 4, DOECHO) &&
    vget(b_lines, 0, "�`���p�� 0.9999... �Ƭ����ơA���l�O ", ans2, 4, DOECHO))
  {
    if ((num = atoi(ans1)) && (num == atoi(ans2)))	/* �������ର 0 */
    {
      d.wisdom += 10;
      vmsg("�S���A0.9999... �N�O 1");
      return 1;
    } 
  }

  return 0;
}


  /*-----------------------------------------------------*/
  /* pip_quest_401~499 �樥�Y�઺����			 */
  /*-----------------------------------------------------*/


static int
pip_quest_401()		/* �A�צʤ��� */
{
  if (ians(b_lines - 1, 0, "1)���� 2)�V�O 3)�A�� ") == '3')
  {
    d.affect += 5;
    d.toman += 5;
    vmsg("�O���A�ߦ��ʤ��ʪ��A�פ~����o���H���L�q");
    return 1;
  }

  return 0;
}


  /*-----------------------------------------------------*/
  /* quest_cb[] ���ȦC��			 	 */
  /*-----------------------------------------------------*/


#define PIPQUEST_NUM	9


/* static KeyFunc quest_cb[] = */
static KeyFunc quest_cb[PIPQUEST_NUM + 1] =	/* �� PIPQUEST_NUM ���w�i�h�A�p�G�����A�i�H�b compile ���ݥX */
{
  /* �M�䪫�~ */
  1, pip_quest_1,
  2, pip_quest_2,
  3, pip_quest_3,

  /* �ݩʹF�� */
  101, pip_quest_101,

  /* ���ѩǪ� */
  201, pip_quest_201,
  202, pip_quest_202,

  /* �ƾǰ��D */
  301, pip_quest_301,
  302, pip_quest_302,

  /* �樥�Y�� */
  401, pip_quest_401,
  
  0, NULL		/* �������ȦC�� */
};
    

/*-------------------------------------------------------*/
/* ���Ȩ禡                                              */
/*-------------------------------------------------------*/


static int		/* 1:�d�ߥ���  0:�S������ */
pip_quest_query(quest)	/* �d���¥��� */
  int quest;		/* ���Ƚs�� */
{
  if (!quest && !(quest = d.quest))
  {
    vmsg("�z�ثe�S�����Ȧb��");
    return 0;
  }
  show_quest_pic(quest);
  return 1;
}


int			/* 1:���o�s����  0:�������o�Τw������ */
pip_quest_new()		/* ���o�s���� */
{
  if (ians(b_lines - 1, 0, "�z�w�F�ɯżзǡA�@�N�������ѫ������ȶ�(Y/N)�H[N] ") == 'y')
  {
    d.quest = quest_cb[rand() % PIPQUEST_NUM].key;
    pip_quest_query(d.quest);
    vmsg("�h�a�A����o�ӥ��ȵ�����ڴN�ᤩ�z�󰪪���O");
    return 1;
  }
  return 0;
}


static int		/* 1:���ȧ���  0:���ȥ��� */
pip_quest_done()	/* �������� */
{
  KeyFunc *cb;
  int key;

  if (!d.quest)
  {
    vmsg("�z�ثe�S�����Ȧb��");
    return 0;
  }

  /* itoc.����: �O���O�Ҽ{�� binary search? */
  for (cb = quest_cb; (key = cb->key); cb++)
  {
    if (key == d.quest)
    {
      key = (*(cb->func)) ();	/* 1:���� 0:���� */
      pip_levelup(key);
      return key;
    }
  }

  vmsg("�Чi�D�����A�䤣�즹���Ȫ��{��");	/* ���Ӥ��i��X�{ */
  return 0;
}


static int		/* 1:����¥���  0:�������ΨS������ */
pip_quest_abort()	/* ����¥��� */
{
  if (!d.quest)
  {
    vmsg("�z�ثe�S�����Ȧb��");
  }
  else if (ians(b_lines - 1, 0, "�z�T�w�n���{�������ȶ�(Y/N)�H[N] ") == 'y')
  {  
    pip_levelup(0);
    return 1;
  }
  else
  {
    vmsg("�٬O���n���n�F");
  }
  return 0;
}


/*-------------------------------------------------------*/
/* ���ȥD���                                            */
/*-------------------------------------------------------*/


int
pip_quest_menu()
{
  while (1)
  {
    out_cmd("", COLOR1 " ���� " COLOR2 " [1]���� [2]�d�� [3]��� [Q]���X                                        \033[m");

    switch (vkey())
    {
      case 'q':
        return 0;

      case '1':
	pip_quest_done();
	break;
	
      case '2':
	pip_quest_query(0);
	break;

      case '3':
	pip_quest_abort();
	break;
    }
  }
}
#endif	/* HAVE_GAME */
