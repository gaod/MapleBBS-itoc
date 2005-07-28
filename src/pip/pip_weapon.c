/* ----------------------------------------------------- */
/* pip_weapon.c     ( NTHU CS MapleBBS Ver 3.10 )        */
/* ----------------------------------------------------- */
/* target : �p�� weapon structure                        */
/* create :   /  /                                       */
/* update : 01/08/15                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/* ----------------------------------------------------- */


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/* ------------------------------------------------------- */
/* �Z���ʶR�禡                                            */
/* ------------------------------------------------------- */


/* name[11] quality cost */
static weapon p[9];		/* �O���Z�� */


/* itoc.021031: ���F�W�[�C�����h�˩ʡA�g�@��Z�����;� */
static void
weapon_generate(type)
  int type;			/* ���@�����˳� */
{
  int i, num;

  char adje[14][5] = {"�l�a", "�A��", "�G��", "����", "�콦", "����", "���K", "����", "�S��", "�O�s", "�ѱ�", "�O�s", "����", "�ǩ_"};
  char prep[13][3] = {"�}",   "��",   "��",   "��",   "�g",   "�P",   "��",   "�t",   "�]",   "�_",   "��",   "��",   ""};
  char noun[5][9][5] =
  {
    /* �Y���Z�� */    "�U",   "�Y��", "�Y�n", "�Y�y", "�Y��", "�վ�", "����", "�v�T", "����", 
    /* �ⳡ�Z�� */    "�C",   "�M",   "��",   "��",   "�j",   "��",   "�}",   "��",   "���", 
    /* �޵P�Z�� */    "��",   "��",   "�٫�", "��M", "����", "�u��", "�޵P", "�ҥ�", "���q", 
    /* ����Z�� */    "����", "�`��", "�֥�", "�ܭ�", "�M��", "�v��", "��A", "���", "���", 
    /* �}���Z�� */    "�c",   "�u",   "�j",   "�i",   "��",   "��",   "��",   "��",   "��"
  };

  for (i = 0; i < 9; i++)
  {
    /* �̯�O�Τ��Y�����ӨM�w�Z�����n�a */

    if (d.money < 12)
    {
      p[i].quality = 1;
      p[i].cost = d.money;
    }
    else
    {
      num = d.money / 1000 + 1;
      if (num > 300)
        num = 300;
      num = rand() % num + 1;
      p[i].quality = num;
      p[i].cost = 3 * num * num;
    }

    num = rand();	/* �ΦP�@�üƨӨM�w adj+prep+noun�A�ҥH mod ���Ƥ��n�@�� */
    /* �̭��@�����˳ƨӨM�w�Z���W�١A�`�N�r����� */
    sprintf(p[i].name, "%s%s%s", adje[num % 14], prep[num % 13], noun[type][num % 9]);
  }
}


void
pip_weapon_wear(type, variance)	/* �˳ƪZ���A�p���O������ */
  int type;			/* ���@�����˳� */
  int variance;			/* �s�ªZ�����~��t�� */
{
  /* �̸˳Ƴ��줣�P�ӧ��ܫ��� */
  if (type == 0)	/* �Y���Z�� */
  {
    d.speed += variance;
    d.immune += variance;
  }
  else if (type == 1)	/* �ⳡ�Z�� */
  {
    d.attack += variance;
    d.immune += variance;
  }
  else if (type == 2)	/* �޵P�Z�� */
  {
    d.attack += variance;
    d.resist += variance;
  }
  else if (type == 3)	/* ����Z�� */
  {
    d.resist += variance;
    d.immune += variance;
  }
  else if (type == 4)	/* �}���Z�� */
  {
    d.attack += variance;
    d.speed += variance;
  }
}


static int
pip_weapon_doing_menu(quality, type, name)	/* �Z���ʶR�e�� */
  int quality;			/* �ǤJ�ثe�t�� */
  int type;			/* ���@�����˳� */
  char *name;
{
  char menutitle[5][11] = {"�Y���˳ư�", "�ⳡ�˳ư�", "�޵P�˳ư�", "����˳ư�", "�}���˳ư�"};
  char buf[80];
  int n;

  /* �üƲ��ͪZ�� */
  weapon_generate(type);

  /* �L�X�Z���C�� */
  vs_head(menutitle[type], str_site);
  show_weapon_pic(0);
  move(11, 0);
  outs("  \033[1;37;41m [NO]  [�Z���W��]  [�~��]  [���] \033[m\n");

  /* �L�X�Z���涵 */    
  for (n = 0; n < 9; n++)
    prints("   %d     %-10s  %6d  %6d\n", n, p[n].name, p[n].quality, p[n].cost);

  /* ���B�z */
  while (1)
  {
    out_cmd("", COLOR1 " �ĶR " COLOR2 " (�x���c�l) [B]�ʶR�Z�� [E]�j�ƪZ�� [D]�߱�Z�� [Q]���X                 \033[m");

    switch (vkey())
    {
    case 'b':
      sprintf(buf, "�z�� %d ���A�Q�n�ʶRԣ�O�H[Q] ", d.money);
      n = ians(b_lines - 2, 1, buf) - '0';

      if (n >= 0 && n < 9)
      {
	sprintf(buf, "�T�w�n�ʶR���� %d �� ��%s��(Y/N)�H[N] ", p[n].cost, p[n].name);
	if (ians(b_lines - 2, 1, buf) == 'y')
	{
	  /* ���Z�� */
	  d.money -= p[n].cost;
	  strcpy(name, p[n].name);
	  pip_weapon_wear(type, p[n].quality - quality);
	  quality = p[n].quality;

	  sprintf(buf, "�p���w�g�˰t�W%s�F", name);
	  vmsg(buf);
	}
	else
	{
	  vmsg("����ʶR");
	}
      }
      break;

    case 'e':
      n = quality * 100;
      if (quality && d.money >= n)
      {
        sprintf(buf, "�T�w�n�� %d ���Ӵ���%s������(Y/N)�H[N] ", n, name);
        if (ians(b_lines - 2, 1, buf) == 'y')
        {
          /* �~��V�n���Z���j�Ʀ��O�V�� */
          d.money -= n;
          quality++;
          pip_weapon_wear(type, 1);
        }
      }
      break;

    case 'd':
      sprintf(buf, "�T�w�n�߱�%s��(Y/N)�H[N] ", name);
      if (ians(b_lines - 2, 1, buf) == 'y')
      {
        pip_weapon_wear(type, -quality);
        name[0] = '\0';
        quality = 0;
      }
      break;

    case 'q':
    case KEY_LEFT:
      return quality;
    }

    /* itoc.010816: ���� ians() �d�U�����e */
    move (b_lines - 2, 0);
    clrtoeol();
  }
}


/*-------------------------------------------------------*/
/* �Z���ө����: �U����                                  */
/*-------------------------------------------------------*/


int
pip_store_weapon_head()		/* �Y���Z�� */
{
  d.weaponhead = pip_weapon_doing_menu(d.weaponhead, 0, d.equiphead);
  return 0;
}


int
pip_store_weapon_hand()		/* �ⳡ�Z�� */
{
  d.weaponhand = pip_weapon_doing_menu(d.weaponhand, 1, d.equiphand);
  return 0;
}


int
pip_store_weapon_shield()	/* �޵P�Z�� */
{
  d.weaponshield = pip_weapon_doing_menu(d.weaponshield, 2, d.equipshield);
  return 0;
}


int
pip_store_weapon_body()		/* ����Z�� */
{
  d.weaponbody = pip_weapon_doing_menu(d.weaponbody, 3, d.equipbody);
  return 0;
}


int
pip_store_weapon_foot()		/* �}���Z�� */
{
  d.weaponfoot = pip_weapon_doing_menu(d.weaponfoot, 4, d.equipfoot);
  return 0;
}
#endif	/* HAVE_GAME */
