/* ----------------------------------------------------- */
/* pip_fight.c        ( NTHU CS MapleBBS Ver 3.10 )      */
/* ----------------------------------------------------- */
/* target : �԰����                                     */
/* create :   /  /                                       */
/* update : 01/08/14		  			 */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */  
/* ----------------------------------------------------- */


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* �ɯŨ禡                                              */
/*-------------------------------------------------------*/


void
pip_levelup(success)
  int success;		/* 1:���\�ѨM�ɯť��� 0:���� */
{
  int level;

  d.quest = 0;

  level = d.level;
  d.exp -= level * 100;
  d.level = ++level;

  /* itoc.010730: ���F�W�[�԰������n�ʡAmaxhp maxmp maxvp maxsp 
     �o���ݩ����ӥu�b exp �W�[�ɯū�A�~��j�q�W�[ */

  /* itoc.010730: �b�]�w�Ǫ��ɽЪ`�N�G�b�C�������\�ѨM�ɯť��Ȥ��U
     level �O n �Ū��p���A�� maxhp/maxmp/maxvp/maxsp ����Ȭ��O 0.75*(n^2) */

  d.maxhp += rand() % level;
  d.maxmp += rand() % level;
  d.maxvp += rand() % level;
  d.maxsp += rand() % level;

  if (success)		/* �p�G���\�ѨM�ɯť��ȡA��[����h */
  {
    d.maxhp += level;
    d.maxmp += level;
    d.maxvp += level;
    d.maxsp += level;
    vmsg("���ȧ����A���Ŵ��ɤF");
  }
  else
  {
    vmsg("���ȥ��ѡA���Ŵ��ɤF");
  }

  /* �ɯū�ɺ��� */
  d.hp = d.maxhp;
  d.mp = d.maxmp;
  d.vp = d.maxvp;
  d.sp = d.maxsp;
}


/* itoc.010731: �ˬd�g��ȬO�_�w�g�F�ɯżз� */
static void
pip_check_levelup()
{
  /* itoc.010731: �C�ɤ@�ŭn (���� * 100) ���g��� */
  /* ���� n �Ǫ��� exp = n*5 (��h�W��20���Ǫ��ɤ@��) */

  /* itoc.020114: ����ŤW���A�]���~�M���H����ƦʸU�šA�C�����]�p���Q�}�a�F */
  
  if ((d.level < 100) && (d.exp >= d.level * 100))
  {
    /* itoc.021031: �F�ɯżзǮɷ|���o�@�Ӥɯť��ȡA�ѧ����Ȥ~��ɯ� */
    if (d.quest)
      vmsg("�w�F�ɯżзǡA���z��������Ω����Ȥ~��ɯ�");
    else
      pip_quest_new();
  }
}


/*-------------------------------------------------------*/
/* �԰��S��                    				 */
/*-------------------------------------------------------*/


/* itoc.010731.����: �H�U�Ҧ����� value * (110 - rand() % 20) / 100; ���F��
   �N�O�d��b value �� 90% ~ 110%�A�����Ȭ� value */

/* itoc.010731: �[�j���m */
/* resistmore = 40 ��ܹ������ܬ� 60%�Aresistmore = -20 ��ܹ������ܬ� 120% */

static int d_resistmore;	/* �p���[�j���m */
static int m_resistmore;	/* �Ǫ��[�j���m */

static int d_nodone;		/* 1:�p���٨S�ʧ@ 0:�p���w���槹�� */


  /*-----------------------------------------------------*/
  /* �Ǫ����;�            				 */
  /*-----------------------------------------------------*/


/* m.name[13] attribute hp maxhp attack spirit magic armor dodge money exp pic */
static playrule m;		/* �O���Ǫ� */


/* itoc.010731: ���F�קK�Ǫ�����Ƥ��e�j�A�Y�Ӧh�귽�A�g�@��Ǫ����;� */
static void
badman_generate(area)
  int area;		/* �ǤJ�ϰ�Ӳ��ͩǪ� */
{
  int level;		/* �Ǫ������� */

  level = rand();	/* �ɥ� level��üơA�]���P�l���Ƴ����P�A�ҥH�u�Τ@�� rand() �N�n�F */

  memset(&m, 0, sizeof(playrule));	/* ��l�� */

  /* [1]�����}�] [2]�_��B�� [3]�j�N��� [4]�H�u�q�� [5]�a������ [6]���e�s�L */
  /* itoc.010731: �U�ϩǪ��� name/attribute/pic ���P�A�H�ε��Žd��]���P */

  /* itoc.010731: ���Ŭ� n �Ū��Ǫ��A����� 
     maxhp = 0.75*(n^2)   (�M���a�@��)
     attack/spirit/magic/armor/dodge = n*10
     money = n*10
     exp = n*5 (��h�W��20���Ǫ��ɤ@��) */

  /* itoc.010731: �`�N m.name ���׬O 13 (���Ӥ���r) */
  /* itoc.010731: �{�b�U�a�ϳ��u���@�i�ϡA�ҥH m.pic �u�n�Ϋ��w�� */

  switch (area)
  {
  case '1':
    {
      char race[3][7] = {"�s�H", "���]", "���K"};
      char title[4][7] = {"�x�L", "�u��", "�h�L", "�o��"};
     
      sprintf(m.name, "%s%s", race[level % 3], title[level % 4]);
      level = d.level - 10 + level % 5;		/* ���y�C */
      if (level <= 5)
	level = 5;
      m.maxhp = level * level / 2 + 30;
      m.attack = level * 8;
      m.spirit = level * 8;
      m.magic = level * 8;
      m.armor = level * 8;
      m.dodge = level * 8;
      m.money = level * 8;
      m.exp = level * 3;			/* �Ҧ��ݩʳ�����t�A��M�g��Ȥ����Ȥ� */
      m.attribute = -4;				/* ���t */
      m.pic = 101 + rand() % 3;
    }
    break;

  case '2':
    {
      char color[7][5] = {"��", "��", "��", "��", "��", "��", "�W�j"};
      char race[4][9] = {"�B�]", "����", "�B��", "����H"};

      sprintf(m.name, "%s%s", color[level % 7], race[level % 4]);
      level = d.level - 10 + level % 10;		/* ���קC */
      if (level <= 5)
	level = 5;
      m.maxhp = level * level + 30;			/* ������Ȧh */
      m.attack = level * 10;
      m.spirit = level * 10;
      m.magic = level * 10;
      m.armor = level * 10;
      m.dodge = level * 10;
      m.money = level * 12;				/* ������h */
      m.exp = level * 5;
      m.attribute = -3;					/* �B�t */
      m.pic = 201 + rand() % 3;
    }
    break;

  case '3':
    {
      char color[5][3] = {"��", "��", "��", "��", "�g"};
      
      sprintf(m.name, "%s����", color[level % 5]);
      level = d.level - 10 + level % 20;		/* ���פ� */
      if (level <= 5)
	level = 5;
      m.maxhp = level * level * 3 / 4 + 30;
      m.attack = level * 12;				/* ���z���������ȱj */
      m.spirit = level * 8;				/* ���O���Ƥ����Ȯt */
      m.magic = level * 15;				/* �]�k�O�q�����ȱj�ܦh */
      m.armor = level * 8;				/* �@�ұj�פ����Ȯt */
      m.dodge = level * 10;
      m.money = level * 10;
      m.exp = level * 6;				/* �g��Ȥ���h */
      m.attribute = 0;
      m.pic = 301 + rand() % 3;
    }
    break;

  case '4':
    {
      char title[5][5] = {"�^�i", "���Z", "�ʾ�", "�`��", "�U��"};
      char race[8][5] = {"�M�h", "�Z�h", "�Ԫ�", "�C��", "�s��", "���Q", "�Ův", "���v"};

      sprintf(m.name, "%s%s", title[level % 5], race[level % 8]);
      level = d.level + level % 10;			/* ���װ� */
      m.maxhp = level * level * 2 + 30;			/* ������Ȧh�ܦh */
      m.attack = level * 10;
      m.spirit = level * 10;
      m.magic = level * 10;
      m.armor = level * 12;				/* �@�ұj�פ����Ȱ� */
      m.dodge = level * 12;				/* �{�׫��Ƥ����Ȱ� */
      m.money = level * 10;
      m.exp = level * 13 / 2;				/* �g��Ȥ���h */
      m.attribute = 0;
      m.pic = 401 + rand() % 3;
    }
    break;

  case '5':
    {
      char title[3][7] = {"�g�Y��", "���L��", "�R����"};
      char race[5][7] = {"���F", "�l�尭", "�µL�`", "�յL�`", "�p��"};

      sprintf(m.name, "%s%s", title[level % 3], race[level % 5]);
      level = d.level + level % 20;			/* ���װ� */
      m.maxhp = level * level * 3 / 2 + 30;		/* �������ܱj */
      m.attack = level * 15;
      m.spirit = level * 15;
      m.magic = level * 15;
      m.armor = level * 15;
      m.dodge = level * 15;
      m.money = level * 15;				/* ���h�ܦh */
      m.exp = level * 7;				/* �g��Ȧh�ܦh */
      m.attribute = 0;
      m.pic = 501 + rand() % 3;
    }
    break;

  case '6':
    {
      /* itoc.010814: ���e�s�L�� */
      int num;
      char name[27][13] = 
      {
	"���p�_", "�q�A",   "�f��",   "��Z��", "���", 
	"�p�s�k", "�J��",   "�}�e�_", "���j",   "�����s", 
	"�K�ӧ�", "���T",   "�ۯ}��", "���N�I", "�����k��", 
	"�O���R", "�i�L��", "�@�O",   "���L",   "�x�C��", 
	"���Įv", "�ڶ��p", "��p",   "���t",   "���ڦ�", 
	"�P�B�q", "�F�褣��"
      };

      /* m.attr: +1:�@�� +2:blitz +3:�l�� +4:�� +5:�C +6:�M +7:�t�� (�Ѧ� pip_attack_skill())
                 -1:�v�� -2:�p -3:�B -4:�� -5:�g -6:�� -7:�s�� */

      int attr[27] = 
      {
	+7, +3, +6, +3, +4, 
	+5, +6, -7, +6, +3, 
	+5, -1, -4, -3, -5, 
	+5, +4, +4, +5, +4, 
	-1, +2, +4, +4, +3, 
	+4, -6
      };

      num = level % 27;			/* ���w�@�� */
      strcpy(m.name, name[num]);
      m.attribute = attr[num];		/* ���w�ժ��ޯ� */
      num++;				/* �קK�ݷ|�P�l�Ƭ� 0 */

      level = d.level + num + rand() % 20;		/* �V�᭱���H���A���׶V�� */
      m.maxhp = level * level * 2 + 500 * (rand() % num);
      m.attack = level * (15 + (rand() % num));
      m.spirit = level * (15 + (rand() % num));
      m.magic = level * (15 + (rand() % num));
      m.armor = level * (15 + (rand() % num));
      m.dodge = level * (15 + (rand() % num));
      m.money = level * 20;				/* ���h�ܦh */
      m.exp = level * 10;				/* �g��Ȧh�ܦh */
      m.pic = 101 + 100 * (rand() % 5) + rand() % 3;	/* 101~501 102~502 103~503 �Q����@ */
    }
    break;
  }

  m.hp = m.maxhp;	/* ��ɺ� */
}


  /*-----------------------------------------------------*/
  /* �԰��ޯ�Ѽ�                                        */
  /*-----------------------------------------------------*/


/* skillset: smode sno sbasic name[13] needhp needmp needvp needsp addtired effect pic message[41] */

/* itoc.010820: �S�������ޯ� */

struct skillset skillXYZlist[] =
{
  /* �������ĪG�O�{���q�w             hp   mp   vp   sp  tir  eff  pic  message */
  +0, 0x0000,      7, "�S��ޯ�",      0,   0,   0,   0,   0,   0,   0, "�S��ޯ�", 
  +0, 0x0001, 0x0000, "���k���i",      0,   0,   0,   0,   0,   0,   0, "�z�b�L�ᤧ�ڡA�۳ФF����M�k�⥴�[����k", 	/* d_dr �j�q�W�� */
  +0, 0x0002, 0x0000, "�O�ޤs�e",      0,   0,   0,   0,   0,   0,   0, "�z���������z����Ƨ�", 			/* d_sr �j�q�W�� */
  +0, 0x0004, 0x0000, "�]����F",      0,   0,   0,   0,   0,   0,   0, "�z�M�Ѧa���Fñ�U�����A�I�i�]�k�¤O�j�W", 	/* d_mr �j�q�W�� */
  +0, 0x0008, 0x0000, "�ְ��[�@",      0,   0,   0,   0,   0,   0,   0, "�z�i�b�ħڤ����ֳt����A�p�J�L�H���a", 	/* d_hr �j�q�W�� */

  +0, 0x0010, 0x0000, "�M�s�Z",        0,   0,   0,   0,   0,   0,   0, "�M�s�������ǡA������Q�z���z�F", 		/* �԰����W�i�H�^�_�� */
  +0, 0x0020, 0x0000, "�o�ӳt",        0,   0,   0,   0,   0,   0,   0, "�z�b����ҫe��|�F�t�����ޥ�", 		/* �԰����Y�F�褣�Ӧ^�X�� */
  +0, 0x0040, 0x0000, "�u������",      0,   0,   0,   0,   0,   0,   0, "�q���H��A�Գ��W�z�`��֤H�@�B", 		/* �C���԰��������� */
};


/* itoc.010801: �j�׳]�w: �ѯ��I (mp/vp/sp) �`�X�ӨM�w�ĪG (effect)�Aneedmp->effect�A�����p�U
   20->10 30->20 50->40 70->100 100->150 250->350 400->600 600->900 900->1500 �оA��ϥΤ����k
   �Y�ӧޯ���I�����b�G�إH�W���I�ơA����ĪG�n���ӧ馩�A�Y�ªZ�\�ί��]�k����j */

/* �T�تZ�\ skillA ~ skillC */

struct skillset skillAlist[] =
{
  /* �@�����ĪG�O�W�[ d_resistmore    hp   mp   vp   sp  tir  eff  pic  message */
  +1, 0x0000,      4, "�@��",          0,   0,   0,   0,   0,   0,   0, "�@���C��", 
  +1, 0x0001, 0x0000, "�K���m",        0,   0,   0,  80,   6,  60, 100, "�����W�U��¶�ۤ@�}����", 
  +1, 0x0002, 0x0001, "�����n",        0,   0,   0, 150,   7,  80, 100, "�֪L���褣�a����", 

  +1, 0x0004, 0x0000, "����P��",      0,   0,  90,   5,   6,  70, 100, "����P���A�G���ƥ|�H", 
  +1, 0x0008, 0x0004, "���[�j����",    0,   0, 120, 100,   7,  90, 100, "���[�j�����A�|�H�ƤK��", 
};

struct skillset skillBlist[] =
{
  /* ���\�Ӥ��O�A�^�_���ʤO           hp   mp   vp   sp  tir  eff  pic  message */
  +2, 0x0000,      3, "���\\",         0,   0,   0,   0,   0,   0,   0, "���\\�C��", 
  +2, 0x0001, 0x0000, "�Z���ܶ���",    0,   0, -30,  15,   2,   0, 110, "�Z��̤l�Ҿժ����ܶ���", 
  +2, 0x0002, 0x0001, "�������",      0,   0,-100,  60,   3,   0, 110, "���ܧ��ܧ�������", 
  +2, 0x0004, 0x0003, "��i�L�B",      0,   0,-200, 110,   3,   0, 110, "���ӤK���A�z�ϥX��i�L�B", 
};

struct skillset skillClist[] =
{
  /* �ߪk�W�[�h�ҡA�^�_���O           hp   mp   vp   sp  tir  eff  pic  message */
  +3, 0x0000,     12, "�ߪk",          0,   0,   0,   0,   0,   0,   0, "�ߪk�C��", 
  +3, 0x0001, 0x0000, "���Ӹg",        0,   0,   0, -20,  10,  50, 120, "�g���B�_�F���ӯu�g", 
  +3, 0x0002, 0x0000, "�������\\",     0,   0,   0, -60,  20,  50, 120, "�z�B�_�������\\�A�y�����", 
  +3, 0x0004, 0x0000, "�E���u�g",      0,   0,   0,-200,  28,  50, 120, "���������o�N���ۡФE���u�g", 
  +3, 0x0008, 0x0000, "�E���u�g",      0,   0,   0,-250,  30,  50, 120, "�E�����\\�A�H�ߦӵo", 

  +3, 0x0010, 0x0000, "�p�L�ۥ\\",     0,   0,   0, -40,  15,  50, 120, "�p�L�ۥ\\�A�����թM", 
  +3, 0x0020, 0x0010, "�~��g",        0,   0,   0, -80,  23,  50, 120, "�z�I�i�X�ǻ������~��g", 
  +3, 0x0040, 0x0030, "�Q�K�d��",      0,   0,   0,-180,  27,  50, 120, "�z�q�Q�K�L�d�����Ұѳz�����\\", 
  +3, 0x0080, 0x0070, "�����g",        0,   0,   0,-360,  35,  50, 120, "�֪L���Ǥ��ǡйF������", 

  /* �Ψ�L�� sp */
  +3, 0x0100, 0x0000, "�������",     15,   0,   0, -30,   1,  50, 120, "�z�ϥX�r�A�æ����l�F���@�Ǥ��O", 
  +3, 0x0200, 0x0100, "�ƥ\\�j�k",     0,  35,   0, -60,   1,  50, 120, "�z�q�Ĥ⨭�W�ƨӤF�@�������O", 
  +3, 0x0400, 0x0300, "�l�P�j�k",      0,  55,   0,-150,   7,  50, 120, "�b�l�ǹ�褺�O���l�A�z�o�n�n�𮧤@�f", 
  +3, 0x0800, 0x0700, "�_�߯��\\",     0, 100,   0,-180,   1,  50, 120, "�L�N�����A�z�q���l���F�j�q���O", 
};

struct skillset skillDlist[] =
{
  /* ���k�D�n�Ӥ��O                   hp   mp   vp   sp  tir  eff  pic  message */
  +4, 0x0000,     16, "���k",          0,   0,   0,   0,   0,   0,   0, "���k�C��", 
  +4, 0x0001, 0x0000, "�֪L����",      0,   0,   0,  50,  10,  40, 130, "�֪L�����A���ͭ�", 		/* �֪L�t�x�k�������O */
  +4, 0x0002, 0x0001, "ù�~��",        0,   0,   0,  70,  10, 100, 130, "�֪L�Q�Kù�~", 
  +4, 0x0004, 0x0003, "��ꮱ",        0,   0,   0, 100,  10, 150, 130, "�n�s���", 
  +4, 0x0008, 0x0007, "��Y�x",        0,   0,   0, 150,  10, 200, 130, "�֪L��Y�x", 
  +4, 0x0010, 0x000F, "�j�O�����",    0,   0,   0, 250,  10, 350, 130, "�֪L�j�O�����", 
  +4, 0x0020, 0x001F, "�L�ۧT��",      0,   0,   0, 400,  10, 600, 130, "�֪L�L�ۧT��", 
  +4, 0x0040, 0x001F, "���߯��C",      0,   0, 200, 100,   7, 400, 130, "�j�z�q�a���߯��C", 
  +4, 0x0080, 0x0003, "�f�M����x",   30,   0,   0, 100,  12, 300, 130, "�z�ˤߦa���X�@�x", 		/* �ݯӦ�! */

  +4, 0x0100, 0x0000, "�Z��x�k",      0,   0,  30,  30,   6,  40, 130, "�z���X�Z���̰򥻪��x�k", 	/* �Z��t�x�k�ݯӲ��ʤO */
  +4, 0x0200, 0x0100, "�ӷ���",        0,   0, 180,  80,   6, 350, 130, "�ӷ����k�A�H�X�J��", 
  +4, 0x0400, 0x0100, "�K��",          0,   0, 120,  60,  10, 200, 130, "�ܤF�@�f�s�A�z�x�l�j�F�_��", 

  +4, 0x0800, 0x0000, "�Ѱ��y�P��",    0,  30,  30,  30,   8, 120, 130, "�ڰڡA�Ѱ��y����", 		/* �ǩ����x�k�ݯӲ��ʤO�Ϊk�O */
  +4, 0x1000, 0x0800, "�t����\\",     0,  60,  60,  60,   8, 200, 130, "�Y�ڤ@�O�t����\\", 
  +4, 0x2000, 0x1800, "�����",        0, 120, 120, 120,   8, 450, 130, "�@�����𭸥X", 
  +4, 0x4000, 0x3800, "�ɤ���",        0, 240, 240, 240,   8, 950, 130, "�ݧڤ������ɤ���", 

  +4, 0x8000, 0x7FFF, "���s�Q�K�x",    0,   0,   0, 900,  15, 1500,130, "���s�����I", 			/* �׷����k�A���O�n�`�p�� */
};

struct skillset skillElist[] =
{
  /* �C�k�D�n�Ӳ��ʤO�A�䦸���O       hp   mp   vp   sp  tir  eff  pic  message */
  +5, 0x0000,     16, "�C�k",          0,   0,   0,   0,   0,   0,   0, "�C�k�C��", 
  +5, 0x0001, 0x0000, "�Z��C�k",      0,   0,  30,  20,   6,  35, 140, "�Z��C�k�A�D�W�ѤU", 		/* �Z��t�C�k�A�������ʤO */
  +5, 0x0002, 0x0001, "��v�C�k",      0,   0,  50,  20,   6,  90, 140, "��v�C�k�A�C�ܼv��", 
  +5, 0x0004, 0x0003, "�񿪼C�k",      0,   0,  70,  20,   6, 120, 140, "�񿪳����A�C�b�H�b", 
  +5, 0x0008, 0x0007, "�ȶ��C�k",      0,   0, 100,  30,   6, 150, 140, "�ȶ������A�C�o��o", 
  +5, 0x0010, 0x000F, "�R��C�k",      0,   0, 150,  30,   6, 220, 140, "�R��C�k�A�@�����", 
  +5, 0x0020, 0x001F, "����C�k",      0,   0, 250,  30,   6, 365, 140, "����C�k�A����|�H", 
  +5, 0x0040, 0x003F, "�ӷ��C�k",      0,   0, 400,  30,   5, 620, 140, "�ӷ��C�k�A�H�X�J��", 
  +5, 0x0080, 0x003F, "�P�x��",        0,   0, 200, 200,   7, 580, 140, "�P�x�����A������n", 

  +5, 0x0100, 0x0000, "�Ѥk����",     10,   0,  30,  20,   6,  40, 140, "�Ѥk����A�Ѫ�üY", 
  +5, 0x0200, 0x0100, "�g��C�k",     40,   0, 100,  40,   7, 250, 140, "�g��C�k�A�C����o", 
  +5, 0x0400, 0x0300, "���K�C�k",     80,   0,  70, 120,   7, 350, 140, "�ȼC���C�A�O�D�Q��", 
  +5, 0x0800, 0x0700, "�W�t�E�C",      0,   0, 250,  70,  12, 400, 140, "�W�t�E�C�A�D�Ѥ���", 

  +5, 0x1000, 0x0000, "�@���C�k",    100,   0, 100, 100,  15, 450, 140, "���m���\\�A�����ۮc", 		/* �Ӥ��֦�A�ҥH�¤O�[�j */
  +5, 0x2000, 0x1000, "�����_��",    300,   0, 250, 200,  15, 1200,140, "�Y���ۮc�A�]�ন�\\", 

  +5, 0x4000, 0x3000, "�L�W�C�Z",      0,  50, 250, 200,   6, 700, 140, "�L�W�^���A�Q���Q�D", 
  +5, 0x8000, 0x7FFF, "�U�C�k�v",      0, 150, 700, 150,   5, 1500,140, "����L�C�Ӧ��C", 		/* �׷��C�k�A�ݩʭn�U��� */
};

struct skillset skillFlist[] =
{
  /* �M�k�ݯӲ��ʤO�Τ��O             hp   mp   vp   sp  tir  eff  pic  message */
  +6, 0x0000,      5, "�M�k",          0,   0,   0,   0,   0,   0,   0, "�M�k�C��", 
  +6, 0x0001, 0x0000, "��ʤM�k",      0,   0,  40,  40,   6, 100, 150, "�z�Ѥ���ʤ�����|���M�k", 
  +6, 0x0002, 0x0000, "��M�g",       15,   0,  70,  70,   7, 200, 152, "��M�g�۱z����A�g���L��", 
  +6, 0x0004, 0x0000, "�J�a�M�k",      0,   0, 170, 160,   6, 400, 152, "�J�a�a�ǤM�k", 
  +6, 0x0008, 0x0000, "�a�յ�M",      0,   0, 210, 230,   6, 600, 151, "�W�j���ҮɥN�Ҭy�ǤU�Ӫ��M�k", 
  +6, 0x0010, 0x0000, "�r���C�s��",    0,  50, 400, 150,   7, 800, 153, "�·t�Ʋz�ɳ̱j����", 
  +6, 0x0020, 0x0000, "�R�E�g�M",     25,  30, 300, 350,   7, 1000,152, "�R�E�g�M�A�@�y", 
};


struct skillset skillGlist[] =
{
  /* �t���D�n�Ӳ��ʤO�A�r�ݷl�@�I��   hp   mp   vp   sp  tir  eff  pic  message */  
  /* �t��������������@�ˡA�A�X�C��O�̨ϥΡA�P�˯��I�U�Aeffect �񥿱`�Ȱ��@�� */
  +7, 0x0000,      5, "�t�����r",      0,   0,   0,   0,   0,   0,   0, "�t���C��", 
  +7, 0x0001, 0x0000, "�X����",        5,   0,  20,   0,   5,  25, 160, "�ϤH���g�@�q�ɶ����X����", 
  +7, 0x0002, 0x0001, "�n��",         10,   0,  40,   0,   5,  55, 160, "�ϤH�W�}���n��", 
  +7, 0x0004, 0x0003, "�n�Y�Y",       40,   0, 160,   0,   5, 350, 160, "�t�ۥ��L���n�Y���l", 
  +7, 0x0008, 0x0007, "�w�D�L�R",     80,   0, 320,   0,   5, 650, 160, "�U�٦B�����w���l", 

  +7, 0x0010, 0x0000, "�S�̼C",        0,   0,  20,   5,   5,  25, 161, "�S�f���g�X�z�ƥ��æn���ּC", 
  +7, 0x0020, 0x0010, "��������",      0,   0,  40,  10,   5,  55, 161, "�z�Y�X���Ѫ����ץ�", 
  +7, 0x0040, 0x0030, "�����äM",      0,   0, 160,  40,   5, 350, 161, "�L�������A�z��MѶ�X�@�M", 
  +7, 0x0080, 0x0070, "�t�F�g�v",      0,   0, 320,  80,   5, 650, 162, "���ѭ��F�����A�u���ĤH���F�z���t��", 
};


/* �C���k�N spellA ~ spellG */
/* �Y needhp <0  �h��ܸ� hp�A�l���� */
/* itoc.010801: �U�t�� Top �k�N�����n���� :p */

struct skillset spellAlist[] =
{
  /* �v���k�N                         hp   mp   vp   sp  tir  eff  pic  message */
  -1, 0x0000,     12, "�v���k�N",      0,   0,   0,   0,   0,   0,   0, "�v���k�N", 

  /* �ɦ��]�k */
  -1, 0x0001, 0x0000, "�򥻮���",    -40,  50,   0,   0,   2,   0, 200, "�z�Pı��x�M�F��", 
  -1, 0x0002, 0x0001, "�����k��",   -150, 100,   0,   0,   2,   0, 200, "���ؾi���A�Pı�n�F�@�I", 
  -1, 0x0004, 0x0002, "���F�k��",   -350, 250,   0,   0,   2,   0, 200, "�믫�M�ߤF�_��", 
  -1, 0x0008, 0x0004, "����¤�",   -900, 600,   0,   0,   2,   0, 200, "�z�l���F�j�۵M���O�q", 

  /* �ɲ��ʤO/�h���]�k */
  -1, 0x0010, 0x0000, "�M���]�G",      0,  50,  -40,  0,  -5,   0, 200, "�z���h�ҫ�_�F", 
  -1, 0x0020, 0x0010, "�k���ȯ�",      0, 100, -150,  0, -10,   0, 200, "�z�Pı�즳�H�����a�O�@�۱z", 
  -1, 0x0040, 0x0030, "�ܸt���~",      0, 250, -350,  0, -15,   0, 200, "�@�}�t����¶����", 
  -1, 0x0080, 0x0070, "�Ѩϥ[��",      0, 600, -900,  0, -20,   0, 200, "�ѨϪ��O�q�[���b�z���W", 

  /* �ɦ�/���ʤO/�h���]�k�A�n���Ƿ|�e���G�ع������k�N */
  -1, 0x0100, 0x0011, "�t��",        -40,  120,  -40,  0, -5,   0, 200, "�z�o��ӦۤW�Ҫ�����", 
  -1, 0x0200, 0x0033, "����",       -150,  240, -150,  0, -5,   0, 200, "�z�o��Ӧۭ���������", 
  -1, 0x0400, 0x0077, "�P��",       -350,  600, -350,  0, -5,   0, 200, "�z�o��Ӧ۬P�Ū�����", 
  -1, 0x0800, 0x00FF, "�ժ�",      -9999, 2000, -9999, 0, -999, 0, 200, "�ժ�ѭ��A�ߤ��ҦV", 	/* ���� */
};

struct skillset spellBlist[] =
{
  /* �p�t�k�N                         hp   mp   vp   sp  tir  eff  pic  message */
  -2, 0x0000,      7, "�p�t�k�N",      0,   0,   0,   0,   0,   0,   0, "�p�t�k�N", 
  -2, 0x0001, 0x0000, "�p�G",          0,  30,   0,   0,   2,  20, 210, "�z�I�i�F�p�G", 
  -2, 0x0002, 0x0001, "���p�G",        0,  50,   0,   0,   2,  40, 210, "�z�I�i�F���p�G", 
  -2, 0x0004, 0x0003, "�ѹp��",        0,  70,   0,   0,   2, 100, 210, "�z�I�i�F�ѹp��", 
  -2, 0x0008, 0x0007, "�ƨg���p",      0, 100,   0,   0,   3, 150, 210, "�z�I�i�F�ƨg���p", 
  -2, 0x0010, 0x000F, "�p�����R",      0, 150,   0,   0,   3, 200, 210, "�z�I�i�F�p�����R", 
  -2, 0x0020, 0x001F, "�z�p���{",      0, 250,   0,   0,   3, 300, 210, "�z�I�i�F�z�p���{", 
  -2, 0x0040, 0x003F, "���s",          0, 400,   0,   0,   4, 600, 210, "���s�\\���A�p�q���H", 
};

struct skillset spellClist[] =
{
  /* �B�t�k�N                         hp   mp   vp   sp  tir  eff  pic  message */
  -3, 0x0000,      7, "�B�t�k�N",      0,   0,   0,   0,   0,   0,   0, "�B�t�k�N", 
  -3, 0x0001, 0x0000, "�B�G",          0,  30,   0,   0,   2,  20, 220, "�z�I�i�F�B�G", 
  -3, 0x0002, 0x0001, "�H�B�G",        0,  50,   0,   0,   2,  40, 220, "�z�I�i�F�H�B�G", 
  -3, 0x0004, 0x0003, "�ȦB�G",        0,  70,   0,   0,   2, 100, 220, "�z�I�i�F�ȦB�G", 
  -3, 0x0008, 0x0007, "�B����",        0, 100,   0,   0,   3, 150, 220, "�z�I�i�F�B����", 
  -3, 0x0010, 0x000F, "���p�B��",      0, 150,   0,   0,   3, 200, 220, "�z�I�i�F���p�B��", 
  -3, 0x0020, 0x001F, "����s��",      0, 250,   0,   0,   3, 300, 220, "�z�I�i�F����s��", 
  -3, 0x0040, 0x003F, "�B��",          0, 400,   0,   0,   4, 600, 220, "�B�����R�A���B�L��", 
};

struct skillset spellDlist[] =
{
  /* ���t�k�N                         hp   mp   vp   sp  tir  eff  pic  message */
  -4, 0x0000,      7, "���t�k�N",      0,   0,   0,   0,   0,   0,   0, "���t�k�N", 
  -4, 0x0001, 0x0000, "���G",          0,  30,   0,   0,   2,  20, 230, "�z�I�i�F���G", 
  -4, 0x0002, 0x0001, "�����G",        0,  50,   0,   0,   2,  40, 230, "�z�I�i�F�����G", 
  -4, 0x0004, 0x0003, "�Һ��u��",      0,  70,   0,   0,   2, 100, 230, "�z�I�i�F�Һ��u��", 
  -4, 0x0008, 0x0007, "�a���~��",      0, 100,   0,   0,   3, 150, 230, "�z�I�i�F�a���~��", 
  -4, 0x0010, 0x000F, "���]�a��",      0, 150,   0,   0,   3, 200, 230, "�z�I�i�F���]�a��", 
  -4, 0x0020, 0x001F, "���s�۳�",      0, 250,   0,   0,   3, 300, 230, "�z�I�i�F���s�۳�", 
  -4, 0x0040, 0x003F, "����",          0, 400,   0,   0,   4, 600, 230, "�����i�͡A�Ӫ̥���",
};

struct skillset spellElist[] =
{
  /* �g�t�k�N                         hp   mp   vp   sp  tir  eff  pic  message */
  -5, 0x0000,      7, "�g�t�k�N",      0,   0,   0,   0,   0,   0,   0, "�g�t�k�N", 
  -5, 0x0001, 0x0000, "�g�G",          0,  30,   0,   0,   2,  20, 240, "�z�I�i�F�g�G", 
  -5, 0x0002, 0x0001, "�����N",        0,  50,   0,   0,   2,  40, 240, "�z�I�i�F�����N", 
  -5, 0x0004, 0x0003, "�a���ѱY",      0,  70,   0,   0,   2, 100, 240, "�z�I�i�F�a���ѱY", 
  -5, 0x0008, 0x0007, "���s����",      0, 100,   0,   0,   3, 150, 240, "�z�I�i�F���s����", 
  -5, 0x0010, 0x000F, "�g�s�l��",      0, 150,   0,   0,   3, 200, 240, "�z�I�i�F�g�s�l��", 
  -5, 0x0020, 0x001F, "�g�a����",      0, 250,   0,   0,   3, 300, 240, "�z�I�i�F�g�a����", 
  -5, 0x0040, 0x003F, "�ȪZ",          0, 400,   0,   0,   4, 600, 240, "�ȪZ�A�{�A�����L��", 
};

struct skillset spellFlist[] =
{
  /* ���t�k�N                         hp   mp   vp   sp  tir  eff  pic  message */
  -6, 0x0000,      7, "���t�k�N",      0,   0,   0,   0,   0,   0,   0, "���t�k�N", 
  -6, 0x0001, 0x0000, "���G",          0,  30,   0,   0,   2,  20, 250, "�z�I�i�F���G", 
  -6, 0x0002, 0x0001, "�ۭ��G",        0,  50,   0,   0,   2,  40, 250, "�z�I�i�F�ۭ��G", 
  -6, 0x0004, 0x0003, "�g���N",        0,  70,   0,   0,   2, 100, 250, "�z�I�i�F�g���N", 
  -6, 0x0008, 0x0007, "�s����",        0, 100,   0,   0,   3, 150, 250, "�z�I�i�F�s����", 
  -6, 0x0010, 0x000F, "�����ݶ�",      0, 150,   0,   0,   3, 200, 250, "�z�I�i�F�����ݶ�", 
  -6, 0x0020, 0x001F, "���᳷��",      0, 250,   0,   0,   3, 300, 250, "�z�I�i�F���᳷��", 
  -6, 0x0040, 0x003F, "�C�s",          0, 400,   0,   0,   4, 600, 250, "�C�s���A�����S��", 
};

struct skillset spellGlist[] =
{
  /* �s���k�N                         hp   mp   vp   sp  tir  eff  pic  message */
  -7, 0x0000,      6, "�s���k�N",      0,   0,   0,   0,   0,   0,   0, "�s���k�N", 			/* �c�d�t���]�k :p */
  -7, 0x0001, 0x0000, "���夤��",      0,2000, 9999, 9999, 0,   0, 260, "����@�R�A�ϤH�@�U", 		/* �v�� */
  -7, 0x0002, 0x0000, "���",          0, 800,   0,   0,   0, 1200,260, "T28 �[���عq�H�A�W�j�q�Ϫi", 	/* �p */
  -7, 0x0004, 0x0000, "���N������",    0, 800,   0,   0,   0, 1200,260, "�W�W����S���D",		/* �B */
  -7, 0x0008, 0x0000, "�ּu�ĦX",      0, 800,   0,   0,   0, 1200,260, "�֤l�u�z���F", 		/* �� */
  -7, 0x0010, 0x0000, "�g�۬y",        0, 800,   0,   0,   0, 1200,260, "�x�W�W���Фg�۬y", 		/* �g */
  -7, 0x0020, 0x0000, "�s�˭�",        0, 800,   0,   0,   0, 1200,260, "�y�N�F�s�˦̯�", 		/* �� */
};


  /*-----------------------------------------------------*/
  /* �ޯ�ǲߨ禡                			 */
  /*-----------------------------------------------------*/


/* itoc.010801: �Ǩ�s�ޯ� */
int				/* 0:�S���Ǩ� 1:�Ǩ� */
pip_learn_skill(smode)
  int smode;			/* skill mode  0:����  >0:�Z�\  <0:�]�k */
{
  int num;
  char buf[80];

  /* itoc.010801: ���üƨM�w�Өt�ޯ઺�䤤�@���A�M���ˬd�p���O�_�w�g�|�o�ޯ�F */
  /* �p�G���|�ӥB�w�g�Ǩ즹�ޯध�򥻧ޯ�A����N��o���@�s�ޯ� */

  /* itoc.020129.�]�p�ޥ�: �Y skill?list[].sno = skill?list[].sbasic�A����ϥΪ̴N�L�k�q
     pip_learn_skill() �Ǩ즹�ޯ�A�ҥH�i�H�ѯS��ƥ�Ӿǲ� */

  switch (smode)
  {
  case 0:		/* �S�� */
    num = rand() % skillXYZlist[0].sbasic + 1;
    if ((d.skillXYZ & skillXYZlist[num].sno) || ((d.skillXYZ & skillXYZlist[num].sbasic) != skillXYZlist[num].sbasic))
      return 0;
    d.skillXYZ |= skillXYZlist[num].sno;
    strcpy(buf, skillXYZlist[num].msg);
    break;  

  case 1:		/* �@�� */
    num = rand() % skillAlist[0].sbasic + 1;
    if ((d.skillA & skillAlist[num].sno) || ((d.skillA & skillAlist[num].sbasic) != skillAlist[num].sbasic))
      return 0;
    d.skillA |= skillAlist[num].sno;
    sprintf(buf, "�z�⮩�F�@����%s", skillAlist[num].name);
    break;

  case 2:		/* ���\ */
    num = rand() % skillBlist[0].sbasic + 1;
    if ((d.skillB & skillBlist[num].sno) || ((d.skillB & skillBlist[num].sbasic) != skillBlist[num].sbasic))
      return 0;
    d.skillB |= skillBlist[num].sno;
    sprintf(buf, "�z�⮩�F���\\��%s", skillBlist[num].name);
    break;  
    
  case 3:		/* �ߪk */
    num = rand() % skillClist[0].sbasic + 1;
    if ((d.skillC & skillClist[num].sno) || ((d.skillC & skillClist[num].sbasic) != skillClist[num].sbasic))
      return 0;
    d.skillC |= skillClist[num].sno;
    sprintf(buf, "�z�⮩�F�ߪk��%s", skillClist[num].name);
    break;

  case 4:		/* ���k */
    num = rand() % skillDlist[0].sbasic + 1;
    if ((d.skillD & skillDlist[num].sno) || ((d.skillD & skillDlist[num].sbasic) != skillDlist[num].sbasic))
      return 0;
    d.skillD |= skillDlist[num].sno;
    sprintf(buf, "�z�⮩�F���k��%s", skillDlist[num].name);
    break;  
    
  case 5:		/* �C�k */
    num = rand() % skillElist[0].sbasic + 1;
    if ((d.skillE & skillElist[num].sno) || ((d.skillE & skillElist[num].sbasic) != skillElist[num].sbasic))
      return 0;
    d.skillE |= skillElist[num].sno;
    sprintf(buf, "�z�⮩�F�C�k��%s", skillElist[num].name);
    break;

  case 6:		/* �M�k */
    num = rand() % skillFlist[0].sbasic + 1;
    if ((d.skillF & skillFlist[num].sno) || ((d.skillF & skillFlist[num].sbasic) != skillFlist[num].sbasic))
      return 0;
    d.skillF |= skillFlist[num].sno;
    sprintf(buf, "�z�⮩�F�M�k��%s", skillFlist[num].name);
    break;

  case 7:		/* �t�� */
    num = rand() % skillGlist[0].sbasic + 1;
    if ((d.skillG & skillGlist[num].sno) || ((d.skillG & skillGlist[num].sbasic) != skillGlist[num].sbasic))
      return 0;
    d.skillG |= skillGlist[num].sno;
    sprintf(buf, "�z�⮩�t����%s", skillGlist[num].name);
    break;


  case -1:		/* �v���k�N */
    num = rand() % spellAlist[0].sbasic + 1;
    if ((d.spellA & spellAlist[num].sno) || ((d.spellA & spellAlist[num].sbasic) != spellAlist[num].sbasic))
      return 0;
    d.spellA |= spellAlist[num].sno;
    sprintf(buf, "�z�Ƿ|�F���]�k��%s", spellAlist[num].name);
    break;

  case -2:		/* �p�t�k�N */
    num = rand() % spellBlist[0].sbasic + 1;
    if ((d.spellB & spellBlist[num].sno) || ((d.spellB & spellBlist[num].sbasic) != spellBlist[num].sbasic))
      return 0;
    d.spellB |= spellBlist[num].sno;
    sprintf(buf, "�z�Ƿ|�F�p�]�k��%s", spellBlist[num].name);
    break;
    
  case -3:		/* �B�t�k�N */
    num = rand() % spellClist[0].sbasic + 1;
    if ((d.spellC & spellClist[num].sno) || ((d.spellC & spellClist[num].sbasic) != spellClist[num].sbasic))
      return 0;
    d.spellC |= spellClist[num].sno;
    sprintf(buf, "�z�Ƿ|�F�B�]�k��%s", spellClist[num].name);
    break;


  case -4:		/* ���t�k�N */
    num = rand() % spellDlist[0].sbasic + 1;
    if ((d.spellD & spellDlist[num].sno) || ((d.spellD & spellDlist[num].sbasic) != spellDlist[num].sbasic))
      return 0;
    d.spellD |= spellDlist[num].sno;
    sprintf(buf, "�z�Ƿ|�F���]�k��%s", spellDlist[num].name);
    break;
    
  case -5:		/* �g�t�k�N */
    num = rand() % spellElist[0].sbasic + 1;
    if ((d.spellE & spellElist[num].sno) || ((d.spellE & spellElist[num].sbasic) != spellElist[num].sbasic))
      return 0;
    d.spellE |= spellElist[num].sno;
    sprintf(buf, "�z�Ƿ|�F�g�]�k��%s", spellElist[num].name);
    break;

  case -6:		/* ���t�k�N */
    num = rand() % spellFlist[0].sbasic + 1;
    if ((d.spellF & spellFlist[num].sno) || ((d.spellF & spellFlist[num].sbasic) != spellFlist[num].sbasic))
      return 0;
    d.spellF |= spellFlist[num].sno;
    sprintf(buf, "�z�Ƿ|�F���]�k��%s", spellFlist[num].name);
    break;

  case -7:		/* �s���k�N */
    num = rand() % spellGlist[0].sbasic + 1;
    if ((d.spellG & spellGlist[num].sno) || ((d.spellG & spellGlist[num].sbasic) != spellGlist[num].sbasic))
      return 0;
    d.spellG |= spellGlist[num].sno;
    sprintf(buf, "�z�A�ѤF���]�k��%s", spellGlist[num].name);
    break;
  }

  vmsg(buf);
  return 1;
}


  /*-----------------------------------------------------*/
  /* �԰��ޯ���                			 */
  /*-----------------------------------------------------*/


/* ���Ƿ|�ӧޯ�A�B��B�k�O���~��I�i */
#define can_useskill(n)	((skill & p[n].sno) && (p[n].needhp < d.hp) && (p[n].needmp <= d.mp) && (p[n].needvp <= d.vp) && (p[n].needsp <= d.sp))


/* itoc.010729: �ޯ���� */
static void
pip_skill_doing_menu(p, dr, sr, mr, hr)		/* �ޯ�e�� */
  struct skillset *p;
  int dr;		/* �ˮ`�O damage rate */
  int sr;		/* ���O�j�� spirit rate */
  int mr;		/* �]�k�j�� magic rate */
  int hr;		/* �R���v hit rate */
{
  int n, ch;
  char ans[5];
  usint skill;

  switch (p[0].smode)
  {
  case 1:		/* �@�� */
    skill = d.skillA;
    break;

  case 2:		/* ���\ */
    skill = d.skillB;
    break;  
    
  case 3:		/* �ߪk */
    skill = d.skillC;
    break;

  case 4:		/* ���\ */
    skill = d.skillD;
    break;

  case 5:		/* �C�k */
    skill = d.skillE;
    break;  
    
  case 6:		/* �M�k */
    skill = d.skillF;
    break;

  case 7:		/* �t�� */
    skill = d.skillG;
    break;

  case -1:		/* �v���k�N */
    skill = d.spellA;
    break;

  case -2:		/* �p�t�k�N */
    skill = d.spellB;
    break;
    
  case -3:		/* �B�t�k�N */
    skill = d.spellC;
    break;

  case -4:		/* ���t�k�N */
    skill = d.spellD;
    break;

  case -5:		/* �g�t�k�N */
    skill = d.spellE;
    break;
    
  case -6:		/* ���t�k�N */
    skill = d.spellF;
    break;

  case -7:		/* �s���k�N */
    skill = d.spellG;
    break;

  default:		/* �H�K�N�~�o�� */
    skill = 0;
  }

  clrfromto(7, 16);
  prints("\033[1;31m�w�w�w�w�w�w�w�w�w�w�w�w�w�t\033[37;41m   �i��[%s]�@����  \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w\033[m", p[0].name);

  n = 1;
  while (n <= p[0].sbasic)	/* p[0].sbasic �x�s���o�t���X�ӧޯ� */
  {
    if (can_useskill(n))
    {
      /* ���|��A�C��K�ӡA�U�t�ޯ�̦h 32 �� */
      if (n <= 8)
	move(n + 7, 4);
      else if (n <= 16)
	move(n - 1, 20);
      else if (n <= 24)
	move(n - 9, 36);
      else
	move(n - 17, 52);

      prints("%2d.%s", n, p[n].name);
    }

    n++;
  }

  while (1)
  {
    if (vget(16, 0, "    �z�Q�ϥΨ��@�۩O�H[Q]���G", ans, 3, DOECHO))
    {
      if (ans[0] == 'q')
      {
	show_badman_pic(m.pic);
	return;
      }
      else
      {
	ch = atoi(ans);
	if (ch > 0 && ch < n && can_useskill(ch))
	{
	  break;
	}
      }
    }
  }

  d.hp -= p[ch].needhp;
  d.mp -= p[ch].needmp;
  d.vp -= p[ch].needvp;
  d.sp -= p[ch].needsp;
  d.tired += p[ch].addtired;

  /* itoc.010801: �u�ݭn�ˬd�O�_�z�W���A�����ˬd�O�_ < 0�A�]���e���ˬd�L�F */
  if (d.hp > d.maxhp)
    d.hp = d.maxhp;
  if (d.mp > d.maxmp)
    d.mp = d.maxmp;
  if (d.vp > d.maxvp)
    d.vp = d.maxvp;
  if (d.sp > d.maxsp)
    d.sp = d.maxsp;
  if (d.tired < 0)
    d.tired = 0;

  /* itoc.010801: ��� */
  /* �b�Ǫ��S�����m�U�A���q����������ˮ`�� d_dr */
  /* �b�Ǫ��S�����m�U�A���O�@��������ˮ`�� 120% * d_dr */
  /* �b�Ǫ��S�����m�U�A�ޯ����������ˮ`�� p[ch].effect + d_dr (�� d_mr d_sr d_hr)*/

  switch (p[ch].smode)
  {
    /* itoc.010729: random �ܤƶV�j���A�N�e�� miss */

  case 1:		/* �@��: �ΤF�H��[�j���m */
    /* �[�j���m�O d_resistmore + 40%�A�ҥH p[ch].effect ���ܤ� > 40 */
    d_resistmore = p[ch].effect * (125 - rand() % 50) / 100;	/* ��ĪG�� 75% ~ 125% */
    if (d_resistmore >= 100)
      d_resistmore = 99;	/* �Y d_resistmore > 100�A�Ϧ��ܥ[��F! */
    break;

  case 2:		/* ���\ */
    /* vp�b�e���w�g�[�L */
    break;

  case 3:		/* �ߪk */
    /* sp�b�e���w�g�[�L */
    break;

  case 4:		/* ���k */
    m.hp -= p[ch].effect * (120 - rand() % 40) / 100 + (dr + sr) / 2;
    break;

  case 5:		/* �C�k */
    m.hp -= p[ch].effect * (140 - rand() % 80) / 100 + (dr + hr) / 2;
    break;

  case 6:		/* �M�k */
    m.hp -= p[ch].effect * (160 - rand() % 120) / 100 + (sr + hr) / 2;
    break;

  case 7:		/* �t�� */
    /* �t���P��O�j�P�L���A�A�X�C��O�̨ϥ� */
    if (hr > d.level * 5)
      m.hp -= p[ch].effect * (120 - rand() % 40) / 100;		/* 100% effect */
    else if (hr > 0)
      m.hp -= p[ch].effect * (80 - rand() % 40) / 100;		/* 60% effect */
    else
      m.hp -= p[ch].effect * (50 - rand() % 40) / 100;		/* 30% effect */
    break;


  case -1:		/* �v���k�N */
    /* hp�b�e���w�g�[�L */
    break;

  case -2:		/* �p�t�k�N */
    m.hp -= p[ch].effect * (140 - rand() % 80) / 100 + mr;
    break;
    
  case -3:		/* �B�t�k�N */
    m.hp -= p[ch].effect * (120 - rand() % 40) / 100 + mr;
    break;

  case -4:		/* ���t�k�N */
    m.hp -= p[ch].effect * (200 - rand() % 200) / 100 + mr;	/* ��ĪG�� 1% ~ 200%�A�ܤƶW�j! */
    break;

  case -5:		/* �g�t�k�N */
    m.hp -= p[ch].effect * (110 - rand() % 20) / 100 + mr;
    break;

  case -6:		/* ���t�k�N */
    m.hp -= p[ch].effect * (130 - rand() % 60) / 100 + mr;
    break;

  case -7:		/* �s���k�N */
    /* itoc.010801: �M�@���]�k���P���O�A�s���k�N����� mr �]�� dr sr hr */
    m.hp -= p[ch].effect * (140 - rand() % 80) / 100 + (mr * 2 + dr + sr + hr) / 5;
    break;
  }

  show_fight_pic(p[ch].pic);
  vmsg(p[ch].msg);
  d_nodone = 0;		/* ��ʵ��� */
}


/* itoc.010729: �i�J�ϥΧޯ��� */
static void
pip_skill_menu(dr, sr, mr, hr)	/* �԰����ޯ઺���� */
  int dr;		/* �ˮ`�O damage rate */
  int sr;		/* ���O�j�� spirit rate */
  int mr;		/* �]�k�j�� magic rate */
  int hr;		/* �R���v hit rate */
{
  while (d_nodone)
  {   
    out_cmd(COLOR1 " �Z�\\��� " COLOR2 " [1]�@�� [2]���\\ [3]�ߪk [4]���k [5]�C�k [6]�M�k [7]�t�� [Q]���    \033[m", 
      COLOR1 " �k�N��� " COLOR2 " [A]�v�� [B]�p�t [C]�B�t [D]���t [E]�g�t [F]���t [G]�S�� [Q]���    \033[m");

    switch (vkey())
    {
    case 'q':
      return;

    case '1':
      pip_skill_doing_menu(skillAlist, dr, sr, mr, hr);
      break;

    case '2':
      pip_skill_doing_menu(skillBlist, dr, sr, mr, hr);
      break;

    case '3':
      pip_skill_doing_menu(skillClist, dr, sr, mr, hr);
      break;

    case '4':
      pip_skill_doing_menu(skillDlist, dr, sr, mr, hr);
      break;

    case '5':
      pip_skill_doing_menu(skillElist, dr, sr, mr, hr);
      break;

    case '6':
      pip_skill_doing_menu(skillFlist, dr, sr, mr, hr);
      break;

    case '7':
      pip_skill_doing_menu(skillGlist, dr, sr, mr, hr);
      break;

    case 'a':
      pip_skill_doing_menu(spellAlist, dr, sr, mr, hr);
      break;

    case 'b':
      pip_skill_doing_menu(spellBlist, dr, sr, mr, hr);
      break;

    case 'c':
      pip_skill_doing_menu(spellClist, dr, sr, mr, hr);
      break;

    case 'd':
      pip_skill_doing_menu(spellDlist, dr, sr, mr, hr);
      break;

    case 'e':
      pip_skill_doing_menu(spellElist, dr, sr, mr, hr);
      break;

    case 'f':
      pip_skill_doing_menu(spellFlist, dr, sr, mr, hr);
      break;

    case 'g':
      pip_skill_doing_menu(spellGlist, dr, sr, mr, hr);
      break;
    }
  }
}


  /*-----------------------------------------------------*/
  /* �԰��Ǫ��ޯ����                                    */
  /*-----------------------------------------------------*/


/* itoc.010801: �ѩ�Ǫ��ޯ�����A���ݭn���A�ҥH�n�M�p���ޯ�������}�g */

static void
pip_attack_skill(dr, sr, mr, hr)	/* �Ǫ��ޯ���� */
  int dr;		/* �ˮ`�O damage rate */
  int sr;		/* ���O�j�� spirit rate */
  int mr;		/* �]�k�j�� magic rate */
  int hr;		/* �R���v hit rate */
{
  int num, mankey;
  char buf[80];

  num = rand();	/* itoc.010801: �]���P�l���Ƥ��P�A�ҥH�ΦP�@�ӶüƨӬٸ귽 */

  /* itoc.010801: �Ǫ����ޯण�ĥΦ� mp/vp/sp �o�ب�סA�ӬO�Ѿ��v�ӬI�i */

  if (m.attribute && (num % 2))
  {
    mankey = m.attribute;	/* 50% �Ǫ��ϥΦۤv�Ҿժ����ޯ� */
  }
  else
  {
    mankey = rand() % 14 - 7;	/* �p�G�S�����w�ժ��ޯ�A����N�üƲ��ͤ@�t�ޯ� */
    if (!mankey)
      mankey = 7;
  }

  /* itoc.010801: ��� */
  /* �b�p���S�����m�U�A�Ǫ����q����������ˮ`�� m_dr */
  /* �b�p���S�����m�U�A�Ǫ����O�@��������ˮ`�� 120% * m_dr */
  /* �b�p���S�����m�U�A�Ǫ��ޯ����������ˮ`�� 150% * m_mr (�� m_dr�Bm_sr) */

  /* itoc.010814: �Ǫ����ޯ�j�P�W�M�p���@�ˡA�u�O�Ǫ��S���� vp/sp�A�ҥH���\/�ߪk�n�����O�� */

  switch (mankey)
  {
  case 1:		/* �@�� */
    /* �[�j���m�O m_resistmore + 40%�A�ҥH���\ effect ���ܤ� > 40�A�B�n < 100 */
    m_resistmore = 40 + num % 60;
    vmsg("���[�j���m�A�����ʤF�_��");
    break;

  case 2:		/* ���\ �� �{�qŧ�� */
    if (hr * (100 - num % 30) / 10 > d.level)
    {
      /* �{�q�_ŧ���S��N�O�M�Ǫ������ۤv����O�èS�����Y(�p�G�_ŧ���\����) */
      d.hp -= d.maxhp / 3;
      vmsg("�Т�����C�I���V�z�{�qŧ��");
    }
    else
    {
      m.hp -= m.maxhp / 5;
      if (m.hp <= 0)
	m.hp = 1;
      vmsg("�����ϵo�ʰ{�q�_ŧ�A���S�ন�\\�A�Ϧӳy���L�ۤv���l��");      
    }
    break;

  case 3:		/* �ߪk �� �l�� */
    /* itoc.010801: �Ǫ���V�h�A�l�o�V�h */
    m.hp += m.maxhp / 8;
    d.hp -= m.maxhp / 8;
    d.mp -= m.maxhp / 10;
    d.vp -= m.maxhp / 12;
    d.sp -= m.maxhp / 14;
    if (m.hp > m.maxhp)
      m.hp = m.maxhp;
    if (d.mp < 0)
      d.mp = 0;
    if (d.vp < 0)
      d.vp = 0;
    if (d.sp < 0)
      d.sp = 0;
    sprintf(buf, "%s�����a�r�F�z�@�f�A���D�o�N�O�ǻ������l��N�H", m.name);    
    vmsg(buf);
    break;

  case 4:		/* ���k */
    vmsg("�j�s�����A��襴�o�z���u���y");
    d.hp -= (dr + sr) * (125 + num % 50) / 200;
    break;

  case 5:		/* �C�k */
    if (num % 3 == 0)
    {
      /* itoc.010801: ���@�ǩǩǪ��ĪG */
      char name[3][9] = {"�@�_�g��", "�G�_�R��", "�T�_�дo"};

      for (num = 0; num < 3; num++)
      {
	sprintf(buf, "�ѹP���C %s", name[num]);		/* itoc: �f�}�����ѹP���C */
	vmsg(buf);
      }
    }
    else
    {
      vmsg("���Ѫ�B�A�z���F���@�C");
    }
    d.hp -= (dr + hr) * (125 + num % 50) / 200;
    break;

  case 6:		/* �M�k */
    vmsg("�z���F��⪺�ѴݤM�k�I");
    d.hp -= (sr + hr) * (125 + num % 50) / 200;
    break;

  case 7:		/* �t�� */
    vmsg("���������A�z���F��⪺�t���I");
    d.hp -= hr * (85 + num % 30) / 100;		/* �@��ޯ�����O 150% m_hr�A���t������t�A�u�� 100% */
    break;

  case -1:		/* �v���k�N */
    m.hp += m.maxhp * (40 + rand() % 30) / 100;	/* �^�_ 40%~70% ���� */
    if (m.hp > m.maxhp)
    {
      m.hp = m.maxhp;
      vmsg("�@�}�t����ģ���A�L�����p������_�F");
    }
    else
    {
      vmsg("���ϥ��]�k�v���F�ۤv");
    }
    break;

  case -2:		/* �p�t�k�N */
  case -3:		/* �B�t�k�N */
  case -4:		/* ���t�k�N */
  case -5:		/* �g�t�k�N */
  case -6:		/* ���t�k�N */
    /* itoc.010814: �Ҧ��������k�N�X�֦b�@�_�B�z */
    {
      /* itoc.010814: �C�t�k�N�U 2 ���H�t�X mankey �ӿ�l���~ */
      char name[10][9] = {"�q���", "�p���~", "�H�B��", "�����", "�����]", "���K�y", "��g��", "���Y��", "�ɭ���", "�g�"};

      num = mr * (125 + num % 50) / 100;	/* �ɥ� num ��ˮ`�I�� */
      d.hp -= num;
      sprintf(buf, "���۴��F%s�A�z���ˤF %d �I", name[-2 * (mankey + 2) + (num % 2)], num);
      vmsg(buf);
    }
    break;

  case -7:		/* �s���k�N */
    /* �ɺ���S����! */
    m.hp = m.maxhp;
    d.hp -= (dr + sr + mr + hr) * (75 + num % 50) / 800;
    sprintf(buf, "%s�ϥΤF���]����A�z�������O���", m.name);
    vmsg(buf);
    break;

  default:
    /* itoc.010814: ���Ӧ����A�W�C�d�򤤪��� :p */
    sprintf(buf, "�Чi�D�����A�i%s�j���ݩʡi%d�j�]�w���~", m.name, m.attribute);
    vmsg(buf);
    break;
  }
}


  /*-----------------------------------------------------*/
  /* �԰����z����                                        */
  /*-----------------------------------------------------*/


/* itoc.010731: �Ǫ��M�p�������z�����A�i�H�g���P�@��禡 */

static void
pip_attack_normal(who, dr, hr)		/* itoc.010731: ���q���� */
  int who;		/* �ڬO�� 1: �p���U�������O  0: �Ǫ��U�������O */
  int dr;		/* �ˮ`�O damage rate */
  int hr;		/* �R���v hit rate */
{
  int injure;
  char buf[80];

  injure = hr * (150 - rand() % 100) / 100;	/* �M�w�O�_�R�� */

  if (who)		/* �p������ */
  {
    d_resistmore = 0;
    d_nodone = 0;
    d.tired += 1 + rand() % 2;

    if (hr > d.level * 10)
    {
      /* �b���S�����m�U�A���q����������ˮ`�� dr */
      injure = dr * (110 - rand() % 20) * (100 - m_resistmore) / 10000;

      if (injure > 0)
      {
	m.hp -= injure;
	d.hexp += rand() % 2 + 2;
	d.hskill += rand() % 2 + 1;
	sprintf(buf, "���q�����A%s�ͩR�O��C %d", m.name, injure);
      }
      else
      {
	sprintf(buf, "�z������²���O��%s���o", m.name);
      }
    }
    else
    {
      strcpy(buf, "���M�S����");
    }
  }
  else			/* �Ǫ����� */
  {
    m_resistmore = 0;

    if (injure > d.level * 10)
    {
      injure = dr * (110 - rand() % 20) * (100 - d_resistmore) / 10000;

      if (injure > 0)
      {
	d.hp -= injure;
	sprintf(buf, "%s���q�����A�z�ͩR�O��C %d", m.name, injure);
      }
      else
      {
	strcpy(buf, "�z�����ݤ��_��誺����");
      }
    }
    else
    {
      strcpy(buf, "�z�{���L��誺����");
    }
  }

  vmsg(buf);
}


static void
pip_attack_aggressive(who, dr, hr)	/* itoc.010731: ���O�@�� */
  int who;		/* �ڬO�� 1: �p���U�������O  0: �Ǫ��U�������O */
  int dr;		/* �ˮ`�O damage rate */
  int hr;		/* �R���v hit rate */
{
  int injure;
  char buf[80];

  injure = hr * (200 - rand() % 200) / 100;	/* �M�w�O�_�R���A���O�@�����üƼv�T����j */

  if (who)		/* �p������ */
  {
    d_resistmore = 0;
    d_nodone = 0;
    d.hp -= 5;			/* ���O�@���n����B����e���� */
    d.tired += 1 + rand() % 3;

    if (hr > d.level * 10)
    {
      /* �b���S�����m�U�A���O����������ˮ`�� 120% * dr */
      injure = dr * (130 - rand() % 20) * (100 - m_resistmore) / 10000;

      if (injure > 0)
      {
	m.hp -= injure;
	d.hexp += rand() % 3 + 2;
	d.hskill += rand() % 3 + 1;
	sprintf(buf, "�z���O�@���A%s�ͩR�O��C %d", m.name, injure);
      }
      else
      {
	sprintf(buf, "�z������²���O��%s���o", m.name);
      }
    }
    else
    {
      strcpy(buf, "���M�S����");
    }
  }
  else			/* �Ǫ����� */
  {
    m_resistmore = 0;

    if (injure > d.level * 10)
    {
      injure = dr * (130 - rand() % 20) * (100 - d_resistmore) / 10000;

      if (injure > 0)
      {
	d.hp -= injure;
	sprintf(buf, "%s���O�@���A�z�ͩR�O��C %d", m.name, injure);
      }
      else
      {
	strcpy(buf, "�z�����ݤ��_��誺����");
      }
    }
    else
    {
      strcpy(buf, "�z�{���L��誺����");
    }
  }

  vmsg(buf);
}



/*-------------------------------------------------------*/
/* ��ԥD�禡                                            */
/*-------------------------------------------------------*/


static void
pip_vs_showing()
{
  int color;
  char inbuf1[20], inbuf2[20];

  clear();
  move(0, 0);

  prints("\033[1;41m  " BBSNAME PIPNAME " ��\033[32m%s\033[37m%-13s                                            \033[m\n", 
    d.sex == 1 ? "��" : (d.sex == 2 ? "��" : "�H"), d.name);

  /* itoc.010801: �ù��W��q�X�p������� */

  if (d.tired >= 80)
    color = 31;
  else if (d.tired >= 60 && d.tired < 80)
    color = 33;
  else
    color = 37;

  sprintf(inbuf1, "%d%s/%d%s", d.hp > 1000 ? d.hp / 1000 : d.hp,
    d.hp > 1000 ? "K" : "", d.maxhp > 1000 ? d.maxhp / 1000 : d.maxhp,
    d.maxhp > 1000 ? "K" : "");
  sprintf(inbuf2, "%d%s/%d%s", d.mp > 1000 ? d.mp / 1000 : d.mp,
    d.mp > 1000 ? "K" : "", d.maxmp > 1000 ? d.maxmp / 1000 : d.maxmp,
    d.maxmp > 1000 ? "K" : "");

  outs("\033[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m\n");
  prints("\033[1;31m�x\033[33m��  �R:\033[37m%-12s\033[33m�k  �O:\033[37m%-12s\033[33m�h  ��:\033[%dm%-12d\033[33m��  ��:\033[37m%-12d\033[31m�x\033[m\n", inbuf1, inbuf2, color, d.tired, d.money);
  prints("\033[1;31m�x\033[33m��  ��:\033[37m%-12d\033[33m��  �m:\033[37m%-12d\033[33m�t  ��:\033[37m%-12d\033[33m�g  ��:\033[37m%-12d\033[31m�x\033[m\n", d.attack, d.resist, d.speed, d.exp);
  prints("\033[1;31m�x\033[33m��  ��:\033[37m%-12d\033[33m�j�ɤY:\033[37m%-12d\033[33m�s  ��:\033[37m%-12d\033[33m�F  ��:\033[37m%-12d\033[31m�x\033[m\n", d.food, d.burger, d.cookie, d.medicine);
  outs("\033[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");

  /* itoc.010801: �ù����� 7~16 �C �q�X�Ǫ������� */
  show_badman_pic(m.pic);

  /* itoc.010801: �ù��U��q�X�Ǫ������ */

  sprintf(inbuf1, "%d%s/%d%s", m.hp > 1000 ? m.hp / 1000 : m.hp,
    m.hp > 1000 ? "K" : "", m.maxhp > 1000 ? m.maxhp / 1000 : m.maxhp,
    m.maxhp > 1000 ? "K" : "");

  move(18, 0);
  outs("\033[1;34m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m\n");
  prints("\033[1;34m�x\033[32m�m  �W:\033[37m%-12s\033[32m��  �R:\033[37m%-12s\033[32m��  �@:\033[37m%-12d\033[32m�{  ��:\033[37m%-12d\033[34m�x\033[m\n", m.name, inbuf1, m.armor, m.dodge);
  prints("\033[1;34m�x\033[32m��  ��:\033[37m%-12d\033[32m��  �O:\033[37m%-12d\033[32m�]  �k:\033[37m%-12d\033[32m��  ��:\033[37m%-12d\033[34m�x\033[m\n", m.attack, m.spirit, m.magic, m.money);
  outs("\033[1;34m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m\n");

  out_cmd("", COLOR1 " �԰��R�O " COLOR2 " [1]���q [2]���O [3]�ޯ� [4]���m [5]�ɥR [6]�ҧ� [A]�۰� [Q]�k�R    \033[m");
}


static void
pip_vs_ending(winorlost, area)
  int winorlost;		/* 2:K���Ǫ�  1:�Ǫ��k�]  -1:�p���k�]  -2:�p���QK�� */
  char area;			/* !=0: K�Ǫ�  0:��ì�u���� */
{
  int mode;

  mode = winorlost;
  if (!area)
    mode += 8;

  clrfromto(7, 16);
  move(8, 0);

  /* itoc.010731: ��ì�u���ɭY�O��F�A�h��_�@�Ǧ�F�Y�OĹ�F�A��_������ */
  /* itoc.010731: �Գӥ[�g���/���A�ԱѦ���/�ݩʡF����ì�u�ԱѤ�����/�ݩ� */

  switch (mode)
  {
  case 10:		/* ��ì�u KO */
    d.tired = 0;
    d.hp = d.maxhp;
    d.hexp += rand() % 3 + 2;
    d.mexp += rand() % 3 + 2;
    d.exp += m.exp;
    outs("           \033[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m\n");
    prints("           \033[1;31m�x \033[37m�Z�N�j�|���p��\033[33m%-13s                \033[31m�x\033[m\n", d.name);
    prints("           \033[1;31m�x \033[37m���ѤF�j�l�����\033[32m%-13s              \033[31m�x\033[m\n", m.name);
    outs("           \033[1;31m�x \033[37m�i���M�g�糣�W�ɤF����                     \033[31m�x\033[m\n");
    outs("           \033[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");
    vmsg("�z���ѤF�@�ӱj�w���å�");
    break;

  case 9:		/* ��ì�u win */
    d.tired = 0;
    d.hp = d.maxhp;
    d.hexp += rand() % 2 + 1;
    d.mexp += rand() % 2 + 1;
    d.exp += m.exp * (50 + rand() % 20) / 100;		/* �o 60% ���g��� */
    outs("           \033[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m\n");
    prints("           \033[1;31m�x \033[37m�Z�N�j�|���p��\033[33m%-13s                \033[31m�x\033[m\n", d.name);
    prints("           \033[1;31m�x \033[37m���ѤF�j�l�����\033[32m%-13s              \033[31m�x\033[m\n", m.name);
    outs("           \033[1;31m�x \033[37m�i���M�g�糣�W�ɤF�@��                     \033[31m�x\033[m\n");
    outs("           \033[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");
    vmsg("���t����a�A�z�ѨM�F�o�����񪺼Ĥ�");
    break;

  case 7:		/* ��ì�u lose */
    d.tired = 50;
    d.hp = d.maxhp / 3;
    d.hexp -= rand() % 2 + 1;
    d.mexp -= rand() % 2 + 1;
    if (d.hexp < 0)
      d.hexp = 0;
    if (d.mexp < 0)
      d.mexp = 0;
    outs("           \033[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m\n");
    prints("           \033[1;31m�x \033[37m�Z�N�j�|���p��\033[33m%-13s                \033[31m�x\033[m\n", d.name);
    prints("           \033[1;31m�x \033[37m�Q\033[32m%-13s\033[37m��⥴�o����y��            \033[31m�x\033[m\n", m.name);
    outs("           \033[1;31m�x \033[37m�M�w�^�a�n�n�A��m                         \033[31m�x\033[m\n");
    outs("           \033[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");
    vmsg("���]���z�ߤ��۷��O���D");
    break;

  case 6:		/* ��ì�u KO-ed */
    d.tired = 50;
    d.hp = d.maxhp / 3;
    d.hexp -= rand() % 3 + 2;
    d.mexp -= rand() % 3 + 2;
    if (d.hexp < 0)
      d.hexp = 0;
    if (d.mexp < 0)
      d.mexp = 0;
    outs("           \033[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m\n");
    prints("           \033[1;31m�x \033[37m�Z�N�j�|���p��\033[33m%-13s                \033[31m�x\033[m\n", d.name);
    prints("           \033[1;31m�x \033[37m�������O\033[32m%-13s\033[37m�����                \033[31m�x\033[m\n", m.name);
    outs("           \033[1;31m�x \033[37m�o�}���~�٭n���g����                       \033[31m�x\033[m\n");
    outs("           \033[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");
    vmsg("�z���b�a�W�a�a�@��");
    break;

  case 2:		/* KO �Ǫ� */
    d.money += m.money;
    d.exp += m.exp;
    d.exp += m.exp;
    d.brave += rand() % 4 + 3;
    outs("           \033[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m\n");
    prints("           \033[1;31m�x \033[37m�^�i���p��\033[33m%-13s                    \033[31m�x\033[m\n", d.name);
    prints("           \033[1;31m�x \033[37m���ѤF���c���Ǫ�\033[32m%-13s              \033[31m�x\033[m\n", m.name);
    outs("           \033[1;31m�x \033[37m�i���M�g�糣�W�ɤF����                     \033[31m�x\033[m\n");
    outs("           \033[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");
    vmsg("��覺���F�ڡA�ҥH�z��ӤF��");
    break;

  case 1:		/* �Ǫ��k�] */
    d.money += m.money * (30 + rand() % 20) / 100;	/* �o 40% ���� */
    d.exp += m.exp * (50 + rand() % 20) / 100;		/* �o 60% ���g��� */
    d.brave += rand() % 3 + 2;
    outs("           \033[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m\n");
    prints("           \033[1;31m�x \033[37m�^�i���p��\033[33m%-13s                    \033[31m�x\033[m\n", d.name);
    prints("           \033[1;31m�x \033[37m���ѤF���c���Ǫ�\033[32m%-13s              \033[31m�x\033[m\n", m.name);
    outs("           \033[1;31m�x \033[37m�i���M�g�糣�W�ɤF�@��                     \033[31m�x\033[m\n");
    outs("           \033[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");
    vmsg("�����{�F..�����F�@�ǿ����z...");
    break;

  case -1:		/* �p���k�] */
    d.money -= d.level * 5 + rand() % 100;
    d.brave -= rand() % 3 + 2;
    if (d.money < 0)
      d.money = 0;
    if (d.brave < 0)
      d.brave = 0;
    outs("           \033[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m\n");
    prints("           \033[1;31m�x \033[37m�i�����p��\033[33m%-13s                    \033[31m�x\033[m\n", d.name);
    prints("           \033[1;31m�x \033[37m�b�P\033[32m%-13s\033[37m���԰���                  \033[31m�x\033[m\n", m.name);
    outs("           \033[1;31m�x \033[37m�����a����F�A�b���S�O���L..........       \033[31m�x\033[m\n");
    outs("           \033[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");
    vmsg("�p������F....");
    break;

  case -2:		/* �p�� KO-ed */
    outs("           \033[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m\n");
    prints("           \033[1;31m�x \033[37m�i�����p��\033[33m%-13s                    \033[31m�x\033[m\n", d.name);
    prints("           \033[1;31m�x \033[37m�b�P\033[32m%-13s\033[37m���԰���                  \033[31m�x\033[m\n", m.name);
    outs("           \033[1;31m�x \033[37m�����a�}�`�F�A�b���S�O�q�s..........       \033[31m�x\033[m\n");
    outs("           \033[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");
    vmsg("�p���}�`�F....");
    pipdie("\033[1;31m�԰����Q�����F...\033[m  ", 1);
    break;
  }
}


/* static */			/* itoc.010731: ����ì�u��ԥ� */
int				/* 1: �p�� win  0: �p�� lose */
pip_vs_man(p, area)
  struct playrule p;		/* �ǤJ�Ǫ���� */
  int area;			/* !=0: K�Ǫ��ɪ��ϰ�  0: ��ì�u���� */
{
  int d_dr;			/* �p�����Ǫ� ���z�������� damage rate */
  int d_sr;			/* �p�����Ǫ� �Z�\�������� spirit rate */
  int d_mr;			/* �p�����Ǫ� �]�k�������� magic rate */
  int d_hr;			/* �p�����Ǫ� ���z/�Z�\/�]�k�����R���v hit rate */

  int m_dr;			/* �Ǫ����p�� ���z�������� damage rate */
  int m_sr;			/* �Ǫ����p�� �Z�\�������� spirit rate */
  int m_mr;			/* �Ǫ����p�� �]�k�������� magic rate */
  int m_hr;			/* �Ǫ����p�� ���z/�Z�\/�]�k�����R���v hit rate */

  int randnum;			/* random number */
  int pipkey;			/* �p���U�����O */
  int mankey;			/* �Ǫ��U�����O */
  char buf[80];

  /* ���ͩǪ���� */
  if (area)		/* K�Ǫ� */
  {
    badman_generate(area);
  }
  else			/* ��ì�u���� */
  { 
    strcpy(m.name, p.name);
    m.attribute = p.attribute;
    m.hp = p.hp;
    m.maxhp = p.maxhp;
    m.attack = p.attack;
    m.spirit = p.spirit;
    m.magic = p.magic;
    m.armor = p.armor;
    m.dodge =  p.dodge;
    m.money = p.money;
    m.pic = p.pic;
  }

  /* itoc.010731: �@�}�l�N��n�@�ǰѼơA�M�ᵥ�@�U�����ǤJ��ƨD�ˮ`�� */
  d_dr = d.attack + (d.hskill + d.hexp) / 10 - m.armor;
  d_sr = (d.attack + d.brave) / 2 + (d.hskill + d.hexp) / 10 - m.armor;
  d_mr = d.mskill + (d.immune + d.mexp) / 10 - m.armor;
  d_hr = d.speed + (d.hskill + d.hexp) / 10 - m.dodge;
  m_dr = m.attack - d.resist;
  m_sr = m.spirit - (d.speed + d.resist) / 2;
  m_mr = m.magic - d.immune;
  m_hr = m.attack - d.speed;

  /* itoc.020718: �n�ˬd�o�ǰѼơA�T�w�O���� */
  if (d_dr <= 0)
    d_dr = 1;
  if (d_sr <= 0)
    d_sr = 1;
  if (d_mr <= 0)
    d_mr = 1;
  if (d_hr <= 0)
    d_hr = 1;
  if (m_dr <= 0)
    m_dr = 1;
  if (m_sr <= 0)
    m_sr = 1;
  if (m_mr <= 0)
    m_mr = 1;
  if (m_hr <= 0)
    m_hr = 1;

  /* itoc.010820: �p�G���S��ޯ�A�i�H�A�[�� */
  if (d.skillXYZ & 0x0001)		/* ���k���i */
    d_dr += d.level << 3;
  else if (d.skillXYZ & 0x0002)		/* �O�ޤs�e */
    d_sr += d.level << 3;
  else if (d.skillXYZ & 0x0004)		/* �]����F */
    d_mr += d.level << 3;
  else if (d.skillXYZ & 0x0008)		/* �ְ��[�@ */
    d_hr += d.level << 3;

  /* itoc.010730: �M�w�֥����� */
  if (d.skillXYZ & 0x0040)		/* �u������ */
  {
    d_nodone = 1;
  }
  else
  {
    d_nodone = rand() % 10 > 2;		/* 30% �ѩǪ������� */
    if (!d_nodone)
    {
      sprintf(buf, "�z�Q%s��ŧ�F", m.name);
      vmsg(buf);
    }
  }

  d_resistmore = 0;
  m_resistmore = 0;
  pipkey = 0;

  for (;;)		/* �L�a�j�� */
  {
    /* �q�X�԰��D�e�� */
    pip_vs_showing();

    while (d_nodone)
    {
      if (pipkey != 'a')	/* itoc.010820: �۰ʧ��� */
        pipkey = vkey();

      switch (pipkey)		/* �p���U���O */
      {
      case 'a':		/* �۰ʧ����u���洶�q���� */
      case '1':		/* ���q���� */
	show_fight_pic(1);
	pip_attack_normal(1, d_dr, d_hr);
	break;

      case '2':		/* ���O�@�� */
	if (d.hp > 5)
	{
	  show_fight_pic(2);
	  pip_attack_aggressive(1, d_dr, d_hr);
	}
	else
	{
	  vmsg("�z��O�o��t�F�ٷQ���O�@��");
	}
	break;

      case '3':		/* �ޯ�: �Z�\/�]�k */
	pip_skill_menu(d_dr, d_sr, d_mr, d_hr);
	if (d_nodone)	/* �i�J skill_menu �X�ӭY�٬O d_nodone�A��ܩ��ϥΧޯ�A�n��ø���O */
	  out_cmd("", COLOR1 " �԰��R�O " COLOR2 " [1]���q [2]���O [3]�ޯ� [4]���m [5]�ɥR [6]�ҧ� [A]�۰� [Q]�k�R    \033[m");
	break;

      case '4':		/* ���m */
	d_resistmore = 40;		/* �Ǫ�������� 40% */
	d.tired += rand() % 2 + 1;
	d_nodone = 0;
	show_fight_pic(3);
	vmsg("�p���[�j���m��....");
	break;

      case '5':		/* �Y�ɫ~ */
	d_nodone = pip_basic_feed();	/* feed ���Ǧ^ 0 ��ܦY���F�A�Ǧ^ 1 ��ܩ��S�Y */
	pip_vs_showing();		/* itoc.010801: �ݭn���㭫ø */

	if (d.skillXYZ & 0x0020)	/* �o�ӳt */
	  d_nodone = 1;
	break;

      case '6':		/* �ҧ� */
	if (m.hp < m.maxhp * (d.level - rand() % 150) / 100)
	{
	  d.mexp += 8;
	  m.hp = 0;
	  vmsg("�������\\�A�W�n�W�[");
	}
	else
	{
	  vmsg("��������");
	}
	d_nodone = 0;
	break;

      case 'q':		/* �k�] */
  	pip_vs_ending(-1, area);
	return 0;
      }
    }

    /* �p��������A�ݭn�ˬd�p��/�Ǫ����A */
    if (m.hp <= 0)			/* �Ǫ������F */
    {
      pip_vs_ending(2, area);
      return 1;
    }
    else if (d.tired >= 100)
    {
      /* �p���֦��F�A���m�O�j�� */
      vmsg("�z�w�g�ӯh�֤F�A���m�O�j�T���C");
      d_resistmore = -100;	/* �Ǫ������[ 100% */
    }


    /* �즹�p���w�U�����O�A�өǪ��U���O */

    /* �M�w�Ǫ��U�����O */
    randnum = rand() % 100;
    if (m.attribute)		/* ���S���ޯ� */
    {
      if (randnum < 40)
	mankey = 1;		/* 40% ���q���� */
      else if (randnum < 50)
	mankey = 2;		/* 10% ���O���� */
      else if (randnum < 90)
	mankey = 3;		/* 40% �ޯ���� */
      else if (randnum < 98)
	mankey = 4;		/* 8% �[�j���m */
      else
	mankey = 5;		/* 2% �k���Ԥ� */
    }
    else
    {
      if (randnum < 40)
	mankey = 1;		/* 40% ���q���� */
      else if (randnum < 65)
	mankey = 2;		/* 25% ���O���� */
      else if (randnum < 90)
	mankey = 3;		/* 25% �ޯ���� */
      else if (randnum < 98)
	mankey = 4;		/* 8% �[�j���m */
      else
	mankey = 5;		/* 2% �k���Ԥ� */
    }

    switch (mankey)
    {
    case 2:		/* �Ǫ����O���� */
      if (m.hp > 5)		/* �Y�Ǫ��夣�줭�I�A�ܴ��q���� */
      {
	show_fight_pic(52);
	pip_attack_aggressive(0, m_dr, m_hr);
	break;
      }

    case 1:		/* �Ǫ����q���� */
      show_fight_pic(51);
      pip_attack_normal(0, m_dr, m_hr);
      break;

    case 3:		/* �Ǫ��ޯ���� */
      pip_attack_skill(m_dr, m_sr, m_mr, m_hr);
      break;

    case 4:		/* �Ǫ����m */
      m_resistmore = 40;	/* �p��������� 40% */
      break;

    case 5:		/* �Ǫ��k�] */
      pip_vs_ending(1, area);
      return 1;
    }

    /* �Ǫ�������A�u�ݭn�ˬd�p�����A */

    if (d.hp <= 0)	/* �p�������F */
    {
      pip_vs_ending(-2, area);
      return 0;
    }

    d_nodone = 1;	/* �S�Ӥp���F */

  }		/* ���� for �j��A�԰����� */
}


/*-------------------------------------------------------*/
/* �a�ϲ��;�                                            */
/*-------------------------------------------------------*/


/* itoc.041017: ���F�h�˩ʡA�g�@�Ӧa�ϲ��;� */

/* �b�o���ж��̦����Ǥ�V�i�H�e�i */
#define MAP_EAST	0x01
#define MAP_WEST	0x02
#define MAP_SOUTH	0x04
#define MAP_NORTH	0x08


static int		/* �Ǧ^���ж������Ǥ�V�i�H�� */
map_generate()
{
  int direction;
  int map[16] = 
  {
    MAP_EAST | MAP_WEST | MAP_SOUTH | MAP_NORTH,
    MAP_EAST, MAP_WEST, MAP_SOUTH, MAP_NORTH, 
    MAP_EAST | MAP_WEST, MAP_EAST | MAP_SOUTH, MAP_EAST | MAP_NORTH, MAP_WEST | MAP_SOUTH, MAP_WEST | MAP_NORTH, MAP_SOUTH | MAP_NORTH, 
    MAP_EAST | MAP_WEST | MAP_SOUTH, MAP_EAST | MAP_WEST | MAP_NORTH, MAP_EAST | MAP_SOUTH | MAP_NORTH, MAP_WEST | MAP_SOUTH | MAP_NORTH, 
    MAP_EAST | MAP_WEST | MAP_SOUTH | MAP_NORTH
  };

  /* �M�w�o���ж������ǥX�f */

  direction = map[rand() & 15];	/* �M�w�o���ж������ǥX�f */
  move(16, 0);
  prints("  �X�f�G\033[1m%s%s%s%s\033[m", 
    direction & MAP_EAST ? "\033[31m�F " : "",
    direction & MAP_WEST ? "\033[32m�� " : "",
    direction & MAP_SOUTH ? "\033[33m�n " : "",
    direction & MAP_NORTH ? "\033[34m�_" : "");

  return direction;
}


/*-------------------------------------------------------*/
/* �԰��D���                                            */
/*-------------------------------------------------------*/


int
pip_fight_menu()
{
  int ch, area, direction;

  show_badman_pic(0);
  out_cmd(COLOR1 " �Ұ� " COLOR2 " [1]�����}�] [2]�_��B�� [3]�j�N��� [Q]���}                            \033[m", 
    COLOR1 " �A�� " COLOR2 " [4]�H�u�q�� [5]�a������ [6]���e�s�L [Q]���}                            \033[m");

  while (1)
  {
    area = vkey();
    if (area == 'q')
      return 0;
    if (area >= '1' && area <= '6')
      break;
  }

  clrfromto(7, 17);

  while (d.hp > 0)
  {
    direction = map_generate();
    out_cmd("", COLOR1 " �ϰ� " COLOR2 " [F]���� [E/W/S/N]�F/��/�n/�_ [Q]�^�a                                   \033[m");

re_key:
    ch = vkey();

    if (ch == 'q')
      break;

    if (ch == 'f')	/* ���� */
    {
      pip_basic_feed();
      continue;
    }


    /* ��L���䳣�O��V */

    ch = ch == 'e' ? MAP_EAST : ch == 'w' ? MAP_WEST : ch == 's' ? MAP_SOUTH : ch == 'n' ? MAP_NORTH : 0;
    if (!(ch & direction))	/* �o�Ӥ�V����e�i */
      goto re_key;		/* ���ݭn���s���ͦa�� */

    if ((ch == 'e' && !(direction & MAP_EAST)) || (ch == 'e' && !(direction & MAP_EAST)) ||
      (ch == 'e' && !(direction & MAP_EAST)) || (ch == 'e' && !(direction & MAP_EAST)))

    if (d.vp < 10)	/* �԰��a�Ϥ����ʦ����ʤO�A�ܤ� 10 �~�ಾ�� */
    {
      vmsg("�z�w�ֱo�L�k�A���");
      continue;		/* �i�H�������~�� */
    }

    d.vp -= 2;

    if (rand() % 3)	/* �T�����G�����|�J��ĤH */
    {
      pip_vs_man(m, area);	/* �i�J�԰� */
    }
    else
    {
      if (d.skillXYZ & 0x0010)	/* �M�s�Z */
      {
	d.hp += (d.maxhp >> 2);
	if (d.hp > d.maxhp)
	  d.hp = d.maxhp;
	vmsg("�z�u�����H���t�Ѿ�");
      }
      else
      {
	vmsg("�S�o�ͥ���ơI");
      }
    }
  }

  /* itoc.010730: �b���}�԰����ɤ~�@���ˬd�O�_�ɯ� */
  pip_check_levelup();

  return 0;
}
#endif	/* HAVE_GAME */
