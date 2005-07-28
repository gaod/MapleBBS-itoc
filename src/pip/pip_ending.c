/*-------------------------------------------------------*/
/* pip_ending.c       ( NTHU CS MapleBBS Ver 3.10 )      */
/*-------------------------------------------------------*/
/* target : �����禡                                     */
/* create :   /  /                                       */
/* update : 01/08/14                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/* ------------------------------------------------------- */
/* �����ѼƳ]�w                                            */
/* ------------------------------------------------------- */


struct endingset
{
  char *girl;			/* �k�͵�����¾�~ */
  char *boy;			/* �k�͵�����¾�~ */
  int grade;			/* ���� */
};
typedef struct endingset endingset;


/* �U���� */
struct endingset endmodeallpurpose[] = 
{
  "�k��¾�~",		"�k��¾�~",		0, 
  "�����o�Ӱ�a�s�k��",	"�����o�Ӱ�a�s���",	500, 
  "������a���_��",	"������a���_��",	400, 
  "�����з|�����j�D��",	"�����з|�����Щv",	350, 
  "������a���j��",	"������a���j��",	320, 
  "�����@��դh",	"�����@��դh",		300, 
  "�����з|�����פk",	"�����з|��������",	150, 
  "�����k�x�W���k�x",	"�����k�x�W���k�x",	200, 
  "�������W���Ǫ�",	"�������W���Ǫ�",	120, 
  "�����@�W�k�x",	"�����@�W�k�x",		100, 
  "�b�|���|�u�@",	"�b�|���|�u�@",		100, 
  "�b���]�u�@",		"�b���]�u�@",		100, 
  "�b�A���u�@",		"�b�A���u�@",		100, 
  "�b�\\�U�u�@",	"�b�\\�U�u�@",		100, 
  "�b�а�u�@",		"�b�а�u�@",		100, 
  "�b�a�u�u�@",		"�b�a�u�u�@",		100, 
  "�b�����u�@",	"�b�����u�@",		100, 
  "�b���e�|�u�@",	"�b���e�|�u�@",		100, 
  "�b���y�Ϥu�@",	"�b���y�Ϥu�@",		100, 
  "�b�u�a�u�@",		"�b�u�a�u�@",		100, 
  "�b�Ӷ�u�@",		"�b�Ӷ�u�@",		100, 
  "����a�x�Юv�u�@",	"����a�x�Юv�u�@",	100, 
  "�b�s�a�u�@",		"�b�s�a�u�@",		100, 
  "�b�s���u�@",		"�b�s���u�@",		100, 
  "�b�j�]�`�|�u�@",	"�b�j�]�`�|�u�@",	100, 
  "�b�a������",		"�b�a������",		50, 
  "�b�|���|�ݮt",	"�b�|���|�ݮt",		50, 
  "�b���]�ݮt",		"�b���]�ݮt",		50, 
  "�b�A���ݮt",		"�b�A���ݮt",		50, 
  "�b�\\�U�ݮt",	"�b�\\�U�ݮt",		50, 
  "�b�а�ݮt",		"�b�а�ݮt",		50, 
  "�b�a�u�ݮt",		"�b�a�u�ݮt",		50, 
  "�b�����ݮt",	"�b�����ݮt",		50, 
  "�b���e�|�ݮt",	"�b���e�|�ݮt",		50, 
  "�b���y�ϭݮt",	"�b���y�ϭݮt",		50, 
  "�b�u�a�ݮt",		"�b�u�a�ݮt",		50, 
  "�b�Ӷ�ݮt",		"�b�Ӷ�ݮt",		50, 
  "����a�x�Юv�ݮt",	"����a�x�Юv�ݮt",	50, 
  "�b�s�a�ݮt",		"�b�s�a�ݮt",		50, 
  "�b�s���ݮt",		"�b�s���ݮt",		50, 
  "�b�j�]�`�|�ݮt",	"�b�j�]�`�|�ݮt",	50, 
  NULL,			NULL,			0
};


/* �԰��� */
struct endingset endmodecombat[] = 
{
  "�k��¾�~",		"�k��¾�~",		0, 
  "�Q�ʬ��i�� �Ԥh��",	"�Q�ʬ��i�� �Ԥh��",	420, 
  "�Q���¬��@�ꪺ�N�x",	"�Q���¬��@�ꪺ�N�x",	300, 
  "��W��a��ö�����",	"��W��a��ö�����",	200, 
  "��F�Z�N�Ѯv",	"��F�Z�N�Ѯv",		150, 
  "�ܦ��M�h���İ�a",	"�ܦ��M�h���İ�a",	160, 
  "�먭�x�Ȧ����h�L",	"�먭�x�Ȧ����h�L",	80, 
  "�ܦ������y�H",	"�ܦ������y�H",		10, 
  "�H�ħL�u�@����",	"�H�ħL�u�@����",	10, 
  NULL,			NULL,			0
};


/* �]�k�� */
struct endingset endmodemagic[] = 
{
  "�k��¾�~",		"�k��¾�~",		0, 
  "�Q�ʬ��i�� �]�k��",	"�Q�ʬ��i�� �]�k��",	420, 
  "�Q�u�����c�]�k�v",	"�Q�u�����x�]�k�v",	280, 
  "��F�]�k�Ѯv",	"��F�]�k�Ѯv",		160, 
  "�ܦ��@���]�ɤh",	"�ܦ��@���]�ɤh",	180, 
  "��F�]�k�v",		"��F�]�k�v",		120, 
  "�H�e�R���H��R����",	"�H�e�R���H��R����",	40, 
  "�����@���]�N�v",	"�����@���]�N�v",	20, 
  "�������Y���H",	"�������Y���H",		10, 
  NULL,			NULL,			0
};


/* ������ */
struct endingset endmodesocial[] = 
{
  "�k��¾�~",		"�k��¾�~",		0, 
  "����������d�m",	"�����k�����t����",	170, 
  "�Q�D�令�����m",	"�Q�襤��k�����ҴB",	260, 
  "�����B�諸�ҤH",	"�����F�k�B�諸�ҴB",	130, 
  "�����I�����d�l",	"�����k�I�����ҴB",	100, 
  "�����ӤH���d�l",	"�����k�ӤH���ҴB",	80, 
  "�����A�H���d�l",	"�����k�A�H���ҴB",	80, 
  "�����a�D������",	"�����k�a�D������",	-40, 
  NULL,			NULL,			0
};


/* ���N�� */
struct endingset endmodeart[] = 
{
  "�k��¾�~",		"�k��¾�~",		0, 
  "�����F�p��",		"�����F�p��",		100, 
  "�����F�@�a",		"�����F�@�a",		100, 
  "�����F�e�a",		"�����F�e�a",		100, 
  "�����F�R�Юa",	"�����F�R�Юa",		100, 
  NULL,			NULL,			0
};


/* �t���� */
struct endingset endmodeblack[] = 
{
  "�k��¾�~",		"�k��¾�~",		0,
  "�ܦ��F�k�]��",	"�ܦ��F�j�]��",		-1000,
  "�V���F�өf",		"�V���F�y�]",		-350,
  "���F��ۤk�����u�@",	"���F��۰�����u�@",	-150,
  "��F�µ󪺤j�j",	"��F�µ󪺦Ѥj",	-500,
  "�ܦ����ű@��",	"�ܦ����ű���",		-350,
  "�ܦ��B�ۮv�B�ۧO�H",	"�ܦ��������F�O�H��",	-350,
  "�H�y�a���u�@�ͬ�",	"�H�������u�@�ͬ�",	-350,
  NULL,			NULL,			0
};


/* �a���� */
struct endingset endmodefamily[] = 
{
  "�k��¾�~",		"�k��¾�~",		0,
  "���b�s�Q�צ�",	"���b�s���צ�",		50,
  "���b�a�̶���",	"���b�a�̶���",		10,
  NULL,			NULL,			0
};


/*-------------------------------------------------------*/
/* �����禡              				 */
/*-------------------------------------------------------*/


/* �u�@���ҧP�_ */
static int
pip_max_worktime()		/* workind: �b���̤u�@�̦h�� */
{
  int workind, times;		/* �X�� */

  times = 20;		/* �Y�S���W�L 20 �����u�@���ҡA�h�Ǧ^ workind = 0 */
  workind = 0;

  if (d.workA > times)
  {
    times = d.workA;
    workind = 1;
  }
  if (d.workB > times)
  {
    times = d.workB;
    workind = 2;
  }
  if (d.workC > times)
  {
    times = d.workC;
    workind = 3;
  }
  if (d.workD > times)
  {
    times = d.workD;
    workind = 4;
  }
  if (d.workE > times)
  {
    times = d.workE;
    workind = 5;
  }
  if (d.workF > times)
  {
    times = d.workF;
    workind = 6;
  }
  if (d.workG > times)
  {
    times = d.workG;
    workind = 7;
  }
  if (d.workH > times)
  {
    times = d.workH;
    workind = 8;
  }
  if (d.workI > times)
  {
    times = d.workI;
    workind = 9;
  }
  if (d.workJ > times)
  {
    times = d.workJ;
    workind = 10;
  }
  if (d.workK > times)
  {
    times = d.workK;
    workind = 11;
  }
  if (d.workL > times)
  {
    times = d.workL;
    workind = 12;
  }
  if (d.workM > times)
  {
    times = d.workM;
    workind = 13;
  }
  if (d.workN > times)
  {
    times = d.workN;
    workind = 14;
  }
  if (d.workO > times)
  {
    times = d.workO;
    workind = 16;
  }
  if (d.workP > times)
  {
    times = d.workP;
    workind = 16;
  }

  return workind;
}


/* �����P�_ */
static int		/* return 1:�t�� 2:���N 3:�U�� 4:�Ԥh 5:�]�k 6:���� 7:�a�� */
pip_future_decide(modeallpurpose)
  int *modeallpurpose;	/* �p�G�O�U�൲���A�����٭n return �O���@�����U�� */
{
  *modeallpurpose = 0;	/* �w�] 0 */

  /* �t�� */
  if ((d.etchics == 0 && d.sin >= 100) || (d.etchics > 0 && d.etchics < 50 && d.sin >= 250))
  {
    return 1;
  }

  /* ���N */
  if (d.art > d.hexp && d.art > d.mexp && d.art > d.hskill &&
    d.art > d.mskill && d.art > d.social && d.art > d.family &&
    d.art > d.homework && d.art > d.wisdom && d.art > d.charm &&
    d.art > d.belief && d.art > d.manners && d.art > d.speech &&
    d.art > d.cook && d.art > d.love)
  {
    return 2;
  }

  /* �԰� */
  if (d.hexp >= d.social && d.hexp >= d.mexp && d.hexp >= d.family)
  {
    *modeallpurpose = 1;
    if (d.hexp > d.social + 50 || d.hexp > d.mexp + 50 || d.hexp > d.family + 50)
      return 4;
    return 3;
  }

  /* �]�k */
  if (d.mexp >= d.hexp && d.mexp >= d.social && d.mexp >= d.family)
  {
    *modeallpurpose = 2;
    if (d.mexp > d.hexp || d.mexp > d.social || d.mexp > d.family)
      return 5;
    return 3;
  }

  /* ���� */
  if (d.social >= d.hexp && d.social >= d.mexp && d.social >= d.family)
  {
    *modeallpurpose = 3;
    if (d.social > d.hexp + 50 || d.social > d.mexp + 50 || d.social > d.family + 50)
      return 6;
    return 3;
  }

  /* �a�� */
  *modeallpurpose = 4;
  if (d.family > d.hexp + 50 || d.family > d.mexp + 50 || d.family > d.social + 50)
    return 7;
  return 3;
}


/* ���B���P�_ */
static int		/* return grade */
pip_marry_decide()
{
  if (d.lover)		/* �ӤH */
  {
    /* d.lover = 3 4 5 6 7:�ӤH */
    return 80;
  }

  if (d.royalJ >= d.relation)
  {
    if (d.royalJ >= 100)
    {
      d.lover = 1;	/* ���l */
      return 200;
    }
  }
  else
  {
    if (d.relation >= 100)
    {
      d.lover = 2;	/* ���˩Υ��� */
      return 0;
    }
  }

  /* d.lover = 0; */	/* �樭 */
  return 40;
}


static int
pip_endwith_black(buf)	/* �t�� */
  char *buf;
{
  int m;

  if (d.sin > 500 && d.mexp > 500)		/* �]�� */
    m = 1;
  else if (d.hexp > 600)			/* �y�] */
    m = 2;
  else if (d.speech > 100 && d.art >= 80)	/* SM */
    m = 3;
  else if (d.hexp > 320 && d.character > 200 && d.charm < 200)	/* �µ�Ѥj */
    m = 4;
  else if (d.character > 200 && d.charm > 200 && d.speech > 70 && d.toman > 70)	/* ���ű@�� */
    m = 5;
  else if (d.wisdom > 450)			/* �B�F�v */
    m = 6;
  else						/* �y�a */
    m = 7;

  if (d.sex == 1)
    strcpy(buf, endmodeblack[m].boy);
  else
    strcpy(buf, endmodeblack[m].girl);

  return endmodeblack[m].grade;
}


static int
pip_endwith_social(buf)	/* ���� */
  char *buf;
{
  int m;

  if (d.social > 600)
    m = (d.charm > 500) ? 1 : 2;
  else if (d.social > 450)
    m = 1;
  else if (d.social > 380)
    m = (d.character > d.charm) ?  3 : 4;
  else if (d.social > 250)
    m = (d.wisdom > d.affect) ? 5 : 6;
  else
    m = 7;

  d.lover = 10;

  if (d.sex == 1)
    strcpy(buf, endmodesocial[m].boy);
  else
    strcpy(buf, endmodesocial[m].girl);

  return endmodesocial[m].grade;
}


static int
pip_endwith_magic(buf)	/* �]�k */
  char *buf;
{
  int m;

  if (d.mexp > 800)
    m = (d.affect > d.wisdom && d.affect > d.belief && d.etchics > 100) ? 1 : 2;
  else if (d.mexp > 600)
    m = (d.speech >= 350) ? 3 : 4;
  else if (d.mexp > 500)
    m = 5;
  else if (d.mexp > 300)
    m = 6;
  else
    m = (d.character > 200) ? 7 : 8;

  if (d.sex == 1)
    strcpy(buf, endmodemagic[m].boy);
  else
    strcpy(buf, endmodemagic[m].girl);

  return endmodemagic[m].grade;
}


static int
pip_endwith_combat(buf)	/* �԰� */
  char *buf;
{
  int m;

  if (d.hexp > 1500)
    m = (d.affect > d.wisdom && d.affect > d.belief && d.etchics > 100) ? 1 : 2;
  else if (d.hexp > 1000)
    m = (d.character > 300 && d.etchics > 50) ? 3 : 4;
  else if (d.hexp > 800)
    m = (d.vp > 500) ? 5 : 6;
  else
    m = (d.attack > 200) ? 7 : 8;

  if (d.sex == 1)
    strcpy(buf, endmodecombat[m].boy);
  else
    strcpy(buf, endmodecombat[m].girl);

  return endmodecombat[m].grade;
}


static int
pip_endwith_family(buf)	/* �a�� */
  char *buf;
{
  int m;

  if (d.relation < 50)
    m = 1;
  else
    m = 2;

  if (d.sex == 1)
    strcpy(buf, endmodefamily[m].boy);
  else
    strcpy(buf, endmodefamily[m].girl);

  return endmodefamily[m].grade;
}


static int
pip_endwith_allpurpose(buf, mode)	/* �U�� */
  char *buf;
  int mode;
{
  int m;
  int point, workind;

  /* �̬O���@�����U��A�ӨM�w point �I�� */

  if (mode == 1)
    point = d.hexp;
  else if (mode == 2)
    point = d.mexp;
  else if (mode == 3)
    point = d.social;
  else if (mode == 4)
    point = d.family;

  if (point > 1000)
  {
    m = (d.character > 1000) ? 1 : 2;
  }
  else if (point > 800)
  {
    m = (d.belief > d.etchics && d.belief > d.wisdom) ? 3 :
      (d.etchics > d.belief && d.etchics > d.wisdom) ? 4 : 5;
  }
  else if (point > 500)
  {
    m = (d.belief > d.etchics && d.belief > d.wisdom) ? 6 :
      (d.etchics > d.belief && d.etchics > d.wisdom) ? 7 : 8;
  }
  else if (point > 300)
  {
    workind = pip_max_worktime();
    m = (workind < 2) ? 9 : 8 + workind;
  }
  else
  {
    workind = pip_max_worktime();
    m = (workind < 2) ? 25 : 24 + workind;
  }

  if (d.sex == 1)
    strcpy(buf, endmodeallpurpose[m].boy);
  else
    strcpy(buf, endmodeallpurpose[m].girl);

  return endmodeallpurpose[m].grade;
}


static int
pip_endwith_art(buf)		/* ���N */
  char *buf;
{
  int m;

  if (d.speech > 100)
    m = 1;
  else if (d.wisdom > 400)
    m = 2;
  else if (d.classI > d.classJ)
    m = 3;
  else
    m = 4;

  if (d.sex == 1)
    strcpy(buf, endmodeart[m].boy);
  else
    strcpy(buf, endmodeart[m].girl);

  return endmodeart[m].grade;
}


/* ------------------------------------------------------- */
/* �����M�w                                                */
/* ------------------------------------------------------- */

static void
pip_endwith_decide(endbuf1, endbuf2, endmode, endgrade)
  char *endbuf1, *endbuf2;
  int *endmode, *endgrade;
{
  char *name[8][2] = 
  {
    {"�k��", "�k��"},
    {"�������l", "���F���D"},
    {"�����z", "���F�z"},
    {"�����ӤH��", "���F�k�ӤH��"},
    {"�����ӤH��", "���F�k�ӤH��"},
    {"�����ӤH��", "���F�k�ӤH��"},
    {"�����ӤH��", "���F�k�ӤH��"},
    {"�����ӤH��", "���F�k�ӤH��"}
  };

  int modeallpurpose;

  /* �B�z endbuf1 */
  *endmode = pip_future_decide(&modeallpurpose);
  switch (*endmode)
  {
  /* 1:�t�� 2:���N 3:�U�� 4:�Ԥh 5:�]�k 6:���� 7:�a�� */
  case 1:
    *endgrade = pip_endwith_black(endbuf1);
    break;
  case 2:
    *endgrade = pip_endwith_art(endbuf1);
    break;
  case 3:
    *endgrade = pip_endwith_allpurpose(endbuf1, modeallpurpose);
    break;
  case 4:
    *endgrade = pip_endwith_combat(endbuf1);
    break;
  case 5:
    *endgrade = pip_endwith_magic(endbuf1);
    break;
  case 6:
    *endgrade = pip_endwith_social(endbuf1);
    break;
  case 7:
    *endgrade = pip_endwith_family(endbuf1);
    break;
  }

  *endgrade += pip_marry_decide();

  /* �B�z endbuf2 */
  if (d.lover >= 1 && d.lover <= 7)
  {
    if (d.sex == 1)
      strcpy(endbuf2, name[d.lover][1]);
    else
      strcpy(endbuf2, name[d.lover][0]);
  }
  else if (d.lover == 10)			/* ��������¾�~�P�ɤ]�O�B�ê��p */
  {
    strcpy(endbuf2, endbuf1);
  }
  else /* if (d.lover == 0) */
  {
    if (d.sex == 1)
      strcpy(endbuf2, "���F�P�檺�k��");
    else
      strcpy(endbuf2, "�����F�P�檺�k��");
  }
}


static void
pip_ending_grade(endgrade)
  int endgrade;
{
  clrfromto(1, 23);
  move(8, 17);
  outs("\033[1;36m�P�±z�������" BBSNAME "�p�����C��\033[m");
  move(10, 17);
  outs("\033[1;37m�g�L�t�έp�⪺���G�G\033[m");
  move(12, 17);
  prints("\033[1;36m�z���p��\033[37m%s\033[36m�`�o����\033[1;5;33m%d\033[m", d.name, endgrade);
}


int
pip_ending_screen()		/* �����e�� */
{
  char endbuf1[50], endbuf2[50];
  int endgrade = 0;
  int endmode = 0;

  pip_endwith_decide(endbuf1, endbuf2, &endmode, &endgrade);

  clear();
  move(1, 0);
  outs("        \033[1;33m�������������������������������������������������ߢ~��������\033[m\n");
  outs("        \033[1;37m��      ����    ������      ����      ����    ������      ��\033[m\n");
  outs("        \033[1;33m��    ������    ������  �~����������������    ������  ������\033[m\n");
  outs("        \033[1;32m��    ������  ��  ����  ������������������  ��  ����  ������\033[m\n");
  outs("        \033[1;37m��      ����  ��  ����      ����      ����  ��  ����      ��\033[m\n");
  outs("        \033[1;35m�������������������������������������������������墢��������\033[m\n");
  outs("        \033[1;31m�w�w�w�w�w�w�w�w�w�w\033[41;37m " BBSNAME PIPNAME "�������i\033[0;1;31m�w�w�w�w�w�w�w�w�w�w\033[m\n");
  outs("        \033[1;36m  �o�Ӯɶ�������ı�a�٬O���{�F..\033[m\n");
  prints("        \033[1;37m  \033[33m%s\033[37m �o���}�z���ŷx�h��A�ۤv�@�����b�~���D�ͦs�F..\033[m\n", d.name);
  outs("        \033[1;36m  �b�z���U�оɥL���o�q�ɥ��A���L��Ĳ�F�ܦh���A���i�F�ܦh����O..\033[m\n");
  prints("        \033[1;37m  �]���o�ǡA���p�� \033[33m%s\033[37m ���᪺�ͬ��A�ܱo��h���h���F..\033[m\n", d.name);
  outs("        \033[1;36m  ���z�����ߡA�z���I�X�A�z�Ҧ����R..\033[m\n");
  prints("        \033[1;37m  \033[33m%s\033[37m �|�û����ʰO�b�ߪ�..\033[m", d.name);
  vmsg("���U�Ӭݥ��ӵo�i");

  clrfromto(7, 19);
  outs("        \033[1;34m�w�w�w�w�w�w�w�w�w�w\033[44;37m " BBSNAME PIPNAME "���ӵo�i\033[0;1;34m�w�w�w�w�w�w�w�w�w�w\033[m\n");
  prints("        \033[1;36m  �z�L�����y�A���ڭ̤@�_�Ӭ� \033[33m%s\033[36m �����ӵo�i�a..\033[m\n", d.name);
  prints("        \033[1;37m�p�� \033[33m%s\033[37m ���%s..\033[m\n", d.name, endbuf1);
  prints("        \033[1;37m�ܩ�p�����B�ê��p�A�L���%s�A�B�ú�O�ܬ���..\033[m\n", endbuf2);
  outs("        \033[1;36m��..�o�O�@�Ӥ�����������..\033[m");
  vmsg("�ڷQ�z�@�w�ܷP�ʧa");

  show_ending_pic(0);
  vmsg("�ݤ@�ݤ����o");

  pip_ending_grade(endgrade);
  vmsg("�U�@���O�p����ơA���� copy �U�Ӱ�����");

  pip_query_self();
  vmsg("�w��A�ӬD��..");

  pipdie("\033[1;31m�C�������o..\033[m  ", 3);
  return 0;
}


/* ------------------------------------------------------- */
/* �H�����t                                                */
/* ------------------------------------------------------- */


int				/* 1:�����D�B  0:�ڵ��D�B */
pip_marriage_offer()		/* �D�B */
{
  char buf[128];
  int money, who;
  char *name[5][2] = {{"�k�ӤH��", "�ӤH��"}, {"�k�ӤH��", "�ӤH��"}, {"�k�ӤH��", "�ӤH��"}, {"�k�ӤH��", "�ӤH��"}, {"�k�ӤH��", "�ӤH��"}};

  do
  {
    who = rand() % 5;
  } while (d.lover == (who + 3));	/* �ӨD�B�̭n���O�{�b�����B�d */

  money = rand() % 5000 + 4000;
  sprintf(buf, " %s�a�ӤF���� %d�A�n�V�z���p���D�B�A�z�@�N��(Y/N)�H[N] ", name[who][d.sex - 1], money);
  if (ians(b_lines - 1, 0, buf) == 'y')
  {
    if (d.wantend != 1 && d.wantend != 4)
    {
      sprintf(buf, " ���㤧�e�w�g���B���F�A�z�T�w�n�Ѱ��±B���A��߱B����(Y/N)�H[N] ");
      if (ians(b_lines - 1, 0, buf) != 'y')
      {
	d.social += 10;			/* �����±B���[���� */
	vmsg("�٬O�����±B���n�F..");
	return 0;
      }
      d.social -= rand() % 50 + 100;	/* ��������B�������� */
    }
    d.charm -= rand() % 5 + 20;
    d.lover = who + 3;
    d.relation -= 20;
    if (d.relation < 0)
      d.relation = 0;
    if (d.wantend < 4)
      d.wantend = 2;
    else
      d.wantend = 5;
    vmsg("�ڷQ���O�@�ӫܦn����Q..");
    d.money += money;
    return 1;
  }
  else
  {
    d.charm += rand() % 5 + 20;
    d.relation += 20;
    if (d.wantend == 1 || d.wantend == 4)
      vmsg("���٦~��..�߱��٤��w..");
    else
      vmsg("�ڦ��w���B���F..�藍�_..");
    return 0;
  }
}


int			/* 1:�Q�e�R 0:���ΨS�� */
pip_meet_divine()	/* �e�R�v�ӳX */
{
  char buf[80];
  int money;

  clrfromto(6, 17);
  move(7, 14);
  outs("\033[1;33;5m�n�n�n..\033[0;1;37m��M�ǨӰ}�}���V���n..\033[m");
  move(9, 14);
  outs("\033[1;37;46m    ��ӬO���C�|�����e�R�v�ӳX�F.......   \033[m");
  vmsg("�}�����L�i�ӧa..");

  money = 300 * (d.bbtime / 60 / 30 + 1);	/* �~���V�j�A�n�᪺���V�h */
  if (d.money >= money)
  {
    sprintf(buf, "�z�n�� %d ���e�R��(Y/N)�H[N] ", money);
    if (ians(11, 14, buf) == 'y')
    {
      sprintf(buf, "�z���p��%s�H��i�઺�����O", d.name);
      switch (rand() % 4)	/* �H�K�ˤ@�� */
      {
        /* �H�U�� strcat �� end message �O�k�k�ۦP�� */
      case 0:
	strcat(buf, endmodemagic[2 + rand() % 5].girl);
	break;
      case 1:
	strcat(buf, endmodecombat[2 + rand() % 6].girl);
	break;
      case 2:
	strcat(buf, endmodeart[2 + rand() % 6].girl);
	break;
      case 3:
	strcat(buf, endmodeallpurpose[6 + rand() % 15].girl);	
	break;
      }
      d.money -= money;
      
      move(13, 14);
      outs("\033[1;33m�b�ڥe�R���G�ݨ�..\033[m");
      move(15, 14);
      outs(buf);
      vmsg("���´f�U�A���t�A�����F�A���Ǥ���ǧڳ�");
      return 1;
    }
    else
    {
      vmsg("�z���Q�e�R�ڡH..�u�i��..���u�����U���a..");
    }
  }
  else
  {
    vmsg("�z����������..�u�O�i��..���U���a..");
  }
  return 0;
}


int
pip_meet_sysop()	/* itoc.000416: �J�W�����j�j */
{
  char msg[5][40] =
  {
    "��M�����_�ʤF�_��..",   "�@�}�����H���Ӧ�..",
    "��..���H���z���ӻH..", "�P���M���x�F�_��..",
    "�����a�ݨ�@�ӤH�v.."
  };

  clrfromto(6, 17);
  move(7, 14);
  outs(msg[rand() % 5]);	/* �üƨM�w�X���ԭz */
  vmsg("��ӬO�������W��" SYSOPNICK "�X�{�F");
  move(9, 14);

  switch (rand() % 4)		/* �H���M�w�����ƥ� (�H���P�üƼW�[���) */
  {
  case 0:
    if (d.weight > 50)
    {
      d.weight -= 20;
      outs("�z�ӭD�F�A�ڬ��z�i���פ�N..");
    } 
    else
    {
      d.weight += 20;
      outs("�z�ӽG�F�A�ڬ��z�i��W�Τ�N..");
    }
    break;

  case 1:
    d.money += 1000;
    outs("�������Ѥ߱��n�A�e�z�@��������n�ƪ�..");
    break;

  case 2:
    d.hp = d.maxhp;
    d.mp = d.maxmp;
    d.vp = d.maxvp;
    d.sp = d.maxsp;
    outs(SYSOPNICK "�e�z�@���d�~�Q���Y�A�z����O������_..");
    break;

  case 3:
    if (d.money >= 100000)
    {
      if (ians(9, 14, "�z�n�� 100000 �������K�v�e���j�k��(Y/N)�H[N] ") == 'y')
      {
	/* �ݩʤW�� 5% */
	d.money -= 100000;
	d.maxhp = d.maxhp * 105 / 100;
	d.maxmp = d.maxmp * 105 / 100;
	d.maxsp = d.maxsp * 105 / 100;
	d.maxsp = d.maxsp * 105 / 100;
	d.attack = d.attack * 105 / 100;
	d.resist = d.resist * 105 / 100;
	d.speed = d.speed * 105 / 100;
	d.character = d.character * 105 / 100;
	d.love = d.love * 105 / 100;
	d.wisdom = d.wisdom * 105 / 100;
	d.art = d.art * 105 / 100;
	d.brave = d.brave * 105 / 100;
	d.homework = d.homework * 105 / 100;
        move(11, 14);
	outs("�g�L�j�����e������A�z�o�{�Ҧ���O���W�@�F..");
      }
      else
      {
        move(11, 14);
	outs("�z���Q�n�ڡH..�u�i���A���u�����U���a..");
      }
    }    
    else
    {
      outs("�j��ı�o�M�z�S���t��..");
    }
    break;
  }
  
  vmsg("�@�಴�A" SYSOPNICK "�N�����F..");
  return 0;
}


int
pip_meet_smith()	/* itoc.021101: �J�W�K�K */
{
  int randnum;
  char *equip;

  clrfromto(6, 17);
  move(7, 14);
  outs("�p��l�n�h����ڡH");
  vmsg("���H�b�I��s�F�z�@�n�A����K�K����");
  move(9, 14);
  outs("�z�Ϊ��O������˳ơA���ڰe�z�@��s���a");

  randnum = rand();

  switch (randnum % 5)
  {
  case 0:			/* �Y���Z�� */
    randnum = randnum % 10;
    d.weaponhead += randnum;
    pip_weapon_wear(0, randnum);
    equip = d.equiphead;
    if (!*equip)
      strcpy(equip, "�K�K���O�w");
    break;

  case 1:			/* �ⳡ�Z�� */
    randnum = randnum % 10;
    d.weaponhand += randnum;
    pip_weapon_wear(1, randnum);
    equip = d.equiphand;
    if (!*equip)
      strcpy(equip, "�K�K�����M");
    break;

  case 2:			/* �޵P�Z�� */
    randnum = randnum % 10;
    d.weaponshield += randnum;
    pip_weapon_wear(2, randnum);
    equip = d.equipshield;
    if (!*equip)
      strcpy(equip, "�K�K���ޥ�");
    break;

  case 3:			/* ����Z�� */
    randnum = randnum % 10;
    d.weaponbody += randnum;
    pip_weapon_wear(3, randnum);
    equip = d.equipbody;
    if (!*equip)
      strcpy(equip, "�K�K���D�T");
    break;

  case 4:			/* �}���Z�� */
    randnum = randnum % 10;
    d.weaponfoot += randnum;
    pip_weapon_wear(4, randnum);
    equip = d.equipfoot;
    if (!*equip)
      strcpy(equip, "�K�K���@�H");
    break;
  }

  while (!vget(b_lines, 0, "�Ь��˳ƨ��ӷs�W�r�G", equip, 11, GCARRY))
    ;

  vmsg("�K�K���z���ӻH�A�H�H���}");
  return 0;
}


int
pip_meet_angel()	/* itoc.010814: �J��Ѩ� */
{
  clear();
  show_system_pic(0);
  move(17, 10);
  prints("\033[1;36m�˷R��\033[1;33m%s ��\033[m", d.name);
  move(18, 10);
  outs("\033[1;37m�ݨ�z�o�˧V�O�����i�ۤv����O  ���ڤߤ��Q����������..\033[m");
  move(19, 10);
  outs("\033[1;36m�p�ѨϧڨM�w���z���๪�y���y  �����a���U�z�@�U..^_^\033[m");
  move(20, 10);

  switch (rand() % 8)
  {
  case 1:
    outs("\033[1;33m�ڱN���z���U����O�������ɦʤ�������..\033[m");
    d.maxhp = d.maxhp * 105 / 100;
    d.hp = d.maxhp;
    d.maxmp = d.maxmp * 105 / 100;
    d.mp = d.maxmp;
    d.maxvp = d.maxvp * 105 / 100;
    d.vp = d.maxvp;
    d.maxsp = d.maxsp * 105 / 100;
    d.sp = d.maxsp;
    d.attack = d.attack * 105 / 100;
    d.resist = d.resist * 105 / 100;
    d.speed = d.speed * 105 / 100;
    d.character = d.character * 105 / 100;
    d.love = d.love * 105 / 100;
    d.wisdom = d.wisdom * 105 / 100;
    d.art = d.art * 105 / 100;
    d.brave = d.brave * 105 / 100;
    d.homework = d.homework * 105 / 100;
    break;

  case 2:
  case 3:
    outs("\033[1;33m�ڱN���z���԰���O�������ɦʤ����Q��..\033[m");
    d.attack = d.attack * 110 / 100;
    d.resist = d.resist * 110 / 100;
    d.speed = d.speed * 110 / 100;
    d.brave = d.brave * 110 / 100;
    break;

  case 4:
  case 5:
    outs("\033[1;33m�ڱN���z���ͩR�B�k�O�B���ʡB���O�������ɦʤ����K��..\033[m");
    d.maxhp = d.maxhp * 108 / 100;
    d.hp = d.maxhp;
    d.maxmp = d.maxmp * 108 / 100;
    d.mp = d.maxmp;
    d.maxvp = d.maxvp * 108 / 100;
    d.vp = d.maxvp;
    d.maxsp = d.maxsp * 108 / 100;
    d.sp = d.maxsp;
    break;

  case 6:
  case 7:
    outs("\033[1;33m�ڱN���z���P����O�������ɦʤ����G�Q��..\033[m");
    d.character = d.character * 120 / 100;
    d.love = d.love * 120 / 100;
    d.wisdom = d.wisdom * 120 / 100;
    d.art = d.art * 120 / 100;
    d.homework = d.homework * 120 / 100;
    break;
  }

  vmsg("���~��[�o��..");
  return 0;
}
#endif	/* HAVE_GAME */
