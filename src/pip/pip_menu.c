/*-------------------------------------------------------*/
/* pip_menu.c         ( NTHU CS MapleBBS Ver 3.10 )      */
/*-------------------------------------------------------*/
/* target : ���禡                                     */
/* create :   /  /                                       */
/* update : 01/08/15                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* ���						 	 */
/*-------------------------------------------------------*/


/* �D��� [1]�� [2]�}�� [3]�צ� [4]���� [5]���u [6]�S�� [7]�t�� */

int pip_basic_menu(), pip_store_menu(), pip_practice_menu(), pip_play_menu();
int pip_job_menu(), pip_special_menu(), pip_system_menu();

static struct pipcommands pipmainlist[] = 
{
  pip_basic_menu,	'1', 
  pip_store_menu,	'2', 
  pip_practice_menu,	'3', 
  pip_play_menu,	'4', 
  pip_job_menu,		'5', 
  pip_special_menu,	'6', 
  pip_system_menu,	'7',
  NULL,			'\0'
};


/* �򥻿�� [1]���� [2]�M�� [3]�� [4]�˿� [5]���� */

int pip_basic_feed(), pip_basic_takeshower(), pip_basic_takerest(), pip_basic_kiss(), pip_money();

static struct pipcommands pipbasiclist[] = 
{
  pip_basic_feed,	'1', 
  pip_basic_takeshower,	'2', 
  pip_basic_takerest,	'3', 
  pip_basic_kiss,	'4', 
  pip_money,		'5', 
  NULL,			'\0'
};


/* �}�� �i��`�Ϋ~�j[1]�K�Q�ө� [2]�ʯ��ľQ [3]�]�̮ѧ� */
/* ��� �i�Z���ʳf�j[A]�Y���˳� [B]�ⳡ�˳� [C]�޵P�˳� [D]����˳� [E]�}���˳� */

int pip_store_food(), pip_store_medicine(), pip_store_other();
int pip_store_weapon_head(), pip_store_weapon_hand(), pip_store_weapon_shield();
int pip_store_weapon_body(), pip_store_weapon_foot();

static struct pipcommands pipstorelist[] = 
{
  pip_store_food,	  '1', 
  pip_store_medicine,	  '2', 
  pip_store_other,	  '3', 
  pip_store_weapon_head,  'a', 
  pip_store_weapon_hand,  'b', 
  pip_store_weapon_shield,'c', 
  pip_store_weapon_body,  'd', 
  pip_store_weapon_foot,  'e', 
  NULL,			  '\0'
};


/* �צ� [A]���(1) [B]�ֵ�(1) [C]����(1) [D]�x��(1) [E]�C�N(1) */
/* ��� [F]�氫(1) [G]�]�k(1) [H]§��(1) [I]ø�e(1) [J]�R��(1) */

int pip_practice_classA(), pip_practice_classB(), pip_practice_classC(), pip_practice_classD(), pip_practice_classE();
int pip_practice_classF(), pip_practice_classG(), pip_practice_classH(), pip_practice_classI(), pip_practice_classJ();
  
static struct pipcommands pippracticelist[] = 
{
  pip_practice_classA,	'a', 
  pip_practice_classB,	'b', 
  pip_practice_classC,	'c', 
  pip_practice_classD,	'd', 
  pip_practice_classE,	'e', 
  pip_practice_classF,	'f', 
  pip_practice_classG,	'g', 
  pip_practice_classH,	'h', 
  pip_practice_classI,	'i', 
  pip_practice_classJ,	'j', 
  NULL, 		'\0'
};


/* ���ֿ�� [1]���B [2]�B�� [3]���| [4]�q�� [5]�ȹC [6]���~ [7]�ۺq */

int pip_play_stroll(), pip_play_sport(), pip_play_date(), pip_play_guess();
int pip_play_outing(), pip_play_kite(), pip_play_KTV();

static struct pipcommands pipplaylist[] = 
{
  pip_play_stroll,	'1', 
  pip_play_sport,	'2', 
  pip_play_date,	'3', 
  pip_play_guess,	'4', 
  pip_play_outing,	'5', 
  pip_play_kite,	'6', 
  pip_play_KTV,		'7', 
  NULL,			'\0'
};


/* ���u [A]�a�� [B]�O�i [C]���] [D]�A�� [E]�\�U [F]�а� [G]�a�u [H]��� */
/* ��� [I]���v [J]�y�H [K]�u�a [L]�u�� [M]�a�� [N]�s�a [O]�s�� [P]�]�`�| */

int pip_job_workA(), pip_job_workB(), pip_job_workC(), pip_job_workD();
int pip_job_workE(), pip_job_workF(), pip_job_workG(), pip_job_workH();
int pip_job_workI(), pip_job_workJ(), pip_job_workK(), pip_job_workL();
int pip_job_workM(), pip_job_workN(), pip_job_workO(), pip_job_workP();

static struct pipcommands pipjoblist[] = 
{
  pip_job_workA,	'a', 
  pip_job_workB,	'b', 
  pip_job_workC,	'c', 
  pip_job_workD,	'd', 
  pip_job_workE,	'e', 
  pip_job_workF,	'f', 
  pip_job_workG,	'g', 
  pip_job_workH,	'h', 
  pip_job_workI,	'i', 
  pip_job_workJ,	'j', 
  pip_job_workK,	'k', 
  pip_job_workL,	'l', 
  pip_job_workM,	'm', 
  pip_job_workN,	'n', 
  pip_job_workO,	'o', 
  pip_job_workP,	'p', 
  NULL,			'\0'
};


/* �S���� [1]��| [2]��e [3]�԰� [4]�Ӯc [5]���� [6]��� */

int pip_see_doctor(), pip_change_weight(), pip_fight_menu(), pip_go_palace(), pip_quest_menu(), pip_pk_menu();

static struct pipcommands pipspeciallist[] = 
{
  pip_see_doctor,	'1', 
  pip_change_weight,	'2', 
  pip_fight_menu,	'3', 
  pip_go_palace,	'4', 
  pip_quest_menu,	'5', 
  pip_pk_menu,		'6', 
  NULL,			'\0'
};


/* �t�ο��  [1]�ԲӸ�� [2]���X�L�H [3]�p����� [4]�S�O�A�� [S]�x�s�i�� [L]Ū���i�� */

int pip_query_self(), pip_query(), pip_system_freepip(), pip_system_service();
int pip_write_backup(), pip_read_backup();

static struct pipcommands pipsystemlist[] = 
{
  pip_query_self,	'1', 
  pip_query,		'2', 
  pip_system_freepip,	'3',
  pip_system_service,	'4',
  pip_write_backup,	's', 
  pip_read_backup,	'l', 
  NULL,			'\0'
};


/*-------------------------------------------------------*/
/* ���O���						 */
/*-------------------------------------------------------*/


/* �b b_lines �M b_lines - 1  �o�G�泣�O���O��� */
/* �Y���O�ĤG��A�h�Ĥ@��ۥ� */
/* �Ѧ� global.h FEETER */

/* �Ȩ� pip_do_menu() �ϥ� */

static char *menuname[8][2] = 
{
  {"",
   COLOR1 " ��� " COLOR2 " [1]�� [2]�}�� [3]�צ� [4]���� [5]���u [6]�S�� [7]�t�� [Q]���}        \033[m"},
   
  {"",
   COLOR1 " �� " COLOR2 " [1]���� [2]�M�� [3]�� [4]�˿� [5]���� [Q]���X                        \033[m"},

  {COLOR1 " �}�� " COLOR2 " ��`�Ϋ~ [1]�K�Q�ө� [2]�ʯ��ľQ [3]�]�̮ѧ� [Q]���X                   \033[m",
   COLOR1 " ���� " COLOR2 " �Z���ʳf [A]�Y���˳� [B]�ⳡ�˳� [C]�޵P�˳� [D]����˳� [E]�}���˳�   \033[m"},

  {COLOR1 " �צ� " COLOR2 " [A]��� [B]�ֵ� [C]���� [D]�x�� [E]�C�N                                \033[m",
   COLOR1 " �W�m " COLOR2 " [F]�氫 [G]�]�k [H]§�� [I]ø�e [J]�R�� [Q]���X                        \033[m"},

  {"",
   COLOR1 " ���� " COLOR2 " [1]���B [2]�B�� [3]���| [4]�q�� [5]�ȹC [6]���~ [7]�ۺq [Q]���X        \033[m"},

  {COLOR1 " ���u " COLOR2 " [A]�a�� [B]�O�i [C]���] [D]�A�� [E]�\\�U [F]�а� [G]�a�u [H]��� [Q]���X\033[m",
   COLOR1 " �ȿ� " COLOR2 " [I]���v [J]�y�H [K]�u�a [L]�u�� [M]�a�� [N]�s�a [O]�s�� [P]�]�`�|      \033[m"},

  {"",
   COLOR1 " �S�� " COLOR2 " [1]��| [2]��e [3]�԰� [4]�Ӯc [5]���� [6]��� [Q]���X                \033[m"},

  {COLOR1 " �A�� " COLOR2 " [1]�ԲӸ�� [2]���X�L�H [3]�p����� [4]�S�O�A��                        \033[m",
   COLOR1 " �t�� " COLOR2 " [S]�x�s�i�� [L]Ū���i�� [Q]���X                                        \033[m"}
};


/*-------------------------------------------------------*/
/* ���禡						 */
/*-------------------------------------------------------*/


  /*-----------------------------------------------------*/
  /* �`�n�禡						 */
  /*-----------------------------------------------------*/


#define PIP_CHECK_PERIOD	60		/* �C 60 ���ˬd�@�� */

static int			/* �^�� ���� */
pip_time_update()
{
  int oldtm, tm;

  /* �T�w�ɶ������� */

  if ((time(0) - last_time) >= PIP_CHECK_PERIOD)
  {
    do
    {
      d.shit += rand() % 3 + 3;		/* �����ơA�٬O�|��ż�� */
      d.tired -= 2;			/* �����ơA�h�ҷ�M��C�� */
      d.sick += rand() % 4 - 2;		/* �����ơA�f��|�H���v�W�[��֩μW�[�ֳ\ */
      d.happy += rand() % 4 - 2;	/* �����ơA�ַּ|�H���v�W�[��֩μW�[�ֳ\ */
      d.satisfy += rand() % 4 - 2;	/* �����ơA�����|�H���v�W�[��֩μW�[�ֳ\ */
      d.hp -= rand() % 3 + d.sick / 10;	/* �����ơA�{�l�]�|�j���A�]�|�]�ͯf���C�@�I */

      last_time += PIP_CHECK_PERIOD;	/* �U����s�ɶ� */
    } while ((time(0) - last_time) >= PIP_CHECK_PERIOD);

    /* �ˬd�~�� */

    oldtm = d.bbtime / 60 / 30;			/* ��s�e�X���F */
    d.bbtime += time(0) - start_time;		/* ��s�p�����ɶ�(�~��) */
    start_time = time(0);
    tm = d.bbtime / 60 / 30;			/* ��s��X���F */

    /* itoc.010815.����: �p�G�p���@���b����椤(�Ҧp�԰��צ�)�A
       ����|�]���ܤ[(�W�L30�����Y�@��)�S������ pip_time_update() �Ӥ@���[�n�h�� */

    /* itoc.010815: �@���L�n�h���|�֥[�n�h���L�ͤ骺�n�B�A�����ץ��A�@���h�ݤp�����B�@ :p */

    if (tm != oldtm)		/* ���Ƨ�s�e��p�G���P�A��ܪ��j�F */
    {
      /* ���j�ɪ��W�[���ܭ� */
      count_tired(1, 7, 0, 100, 0);	/* ��_�h�� */
      d.happy += rand() % 5 + 5;
      d.satisfy += rand() % 5;
      d.wisdom += 10;
      d.character += rand() % 5;
      d.money += 500;
      d.seeroyalJ = 1;			/* �@�~�i�H�����l�@�� */
      pip_write_file();			/* �۰��x�s */

      vs_head("�q�l�i�p��", str_site);
      show_basic_pic(20);		/* �ͤ�ּ� */
      vmsg("�p���L�ͤ�F");

      /* ��ì�u */
      if (tm % 2 == 0)		/* �G�~�@����ì�u */
        pip_race_main();

      /* ���� */
      if (tm >= 21 && (d.wantend == 4 || d.wantend == 5 || d.wantend == 6))	/* ���� 20 �� */
        pip_ending_screen();
    }
  }
  else
  {
    tm = d.bbtime / 60 / 30;	/* �p�G�S�� update�A�]�n�^�� tm(����) */
  }

  /* ���o�ƥ� */

  oldtm = rand() % 2000;	/* �ɥ� oldtm ���ü� */
  if (oldtm == 0 && tm >= 15 && d.charm >= 300 && d.character >= 300)
    pip_marriage_offer();	/* �ӤH�ӨD�B */
  else if (oldtm > 1998)
    pip_meet_divine();		/* �H���J��e�R�v */
  else if (oldtm > 1996)
    pip_meet_sysop();		/* �H���J�� SYSOP */
  else if (oldtm > 1994)
    pip_meet_smith();		/* �H���J���K�K */

  /* �ˬd�@�Ǳ`�ܰʪ��ȬO�_�z�� */
  /* itoc.010815: �b pip_time_update() �������ˬd shit/tired/sick >100 �� happy/satisfy/hp < 0
     �Ӧb pip_refresh_screen() �ˬd�ö��D�ŧi���` */

  if (d.shit < 0)
    d.shit = 0;
  if (d.tired < 0)
    d.tired = 0;
  if (d.sick < 0)
    d.sick = 0;

  if (d.happy > 100)
    d.happy = 100;
  if (d.satisfy > 100)
    d.satisfy = 100;
  if (d.hp > d.maxhp)
    d.hp = d.maxhp;

  return tm;
}


static int				/* -1:���`  0:�S�� */
pip_refresh_screen(menunum, mode)	/* ��ø��ӵe�� */
  int menunum;		/* ���s�� */
  int mode;		/* �e������ 0:�@�� 1:���� 2:���u 3:�צ� */
{
  int tm, age, pic;
  char inbuf2[20], inbuf3[20], inbuf4[20], inbuf5[20], inbuf6[20], inbuf7[20];

  char yo[12][5] = 
  {
    "�ϥ�", "����", "����", "�ൣ", "�֦~", "�C�~",
    "���~", "���~", "��~", "�Ѧ~", "�j�}", "���P"
  };

  tm = pip_time_update();	/* �T�w�ɶ��n������ */

  if (tm == 0)		/* �ϥ� */
    age = 0;
  else if (tm == 1)	/* ���� */
    age = 1;
  else if (tm <= 5)	/* ���� */
    age = 2;
  else if (tm <= 12)	/* �ൣ */
    age = 3;
  else if (tm <= 15)	/* �֦~ */
    age = 4;
  else if (tm <= 18)	/* �C�~ */
    age = 5;
  else if (tm <= 35)	/* ���~ */
    age = 6;
  else if (tm <= 45)	/* ���~ */
    age = 7;
  else if (tm <= 60)	/* ��~ */
    age = 8;
  else if (tm <= 70)	/* �Ѧ~ */
    age = 9;
  else if (tm <= 100)	/* �j�} */
    age = 10;
  else 			/* ���P */
    age = 11;

  sprintf(inbuf2, "%-4d/%4d", d.hp, d.maxhp);
  sprintf(inbuf3, "%-4d/%4d", d.mp, d.maxmp);
  sprintf(inbuf4, "%-4d/%4d", d.vp, d.maxvp);
  sprintf(inbuf5, "%-4d/%4d", d.sp, d.maxsp);
  sprintf(inbuf6, "%-4d/%4d", d.shit, d.sick);
  sprintf(inbuf7, "%-4d/%4d", d.happy, d.satisfy);

  if (menunum)
  {
    clear();
  }
  else	/* itoc.010816: �p�G�O�i�J�D���A���n�M���� 5~18 �C */
  {
    clrfromto(0, 4);
    clrfromto(19, b_lines);
  }

  /* �ù��W�� (0~4�C) ����I�� */

  move(0, 0);
  prints(COLOR1 " ��� " COLOR2 " %s            %-15s                                          \033[m\n", d.sex == 1 ? "��" : "��", d.name);

  /* itoc,010802: ���F�ݲM���@�I�A�ҥH prints() �̭����޼ƴN���_��g�b�ӦC�̫� */
  prints("\033[1;32m[��  �A] \033[37m%-9s\033[32m [��  ��] \033[37m%-9s\033[32m [�~  ��] \033[37m%-9d\033[32m [��  ��] \033[37m%-9d\033[m\n", yo[age], d.birth, tm, d.money);
  prints("\033[1;32m[��  �R] \033[35m%-9s\033[32m [�k  �O] \033[34m%-9s\033[32m [���ʤO] \033[36m%-9s\033[32m [��  �O] \033[31m%-9s\033[m\n", inbuf2, inbuf3, inbuf4, inbuf5);
  prints("\033[1;32m[��  ��] \033[37m%-9d\033[32m [�h  ��] \033[37m%-9d\033[32m [ż���f] \033[37m%-9s\033[32m [�֡���] \033[37m%-9s\033[m\n", d.weight, d.tired, inbuf6, inbuf7);

  if (mode == 0)		/* �@��e�� */
  {
    char *hint[3] = 
    {
      "\033[1;35m[������]:\033[37m�n�h�h�`�N�p�����h�ҫשM�f��A�H�K�֦��f��\033[m\n", 
      "\033[1;35m[������]:\033[37m�H�ɪ`�N�p�����ͩR�ƭȭ�I\033[m\n", 
      "\033[1;35m[������]:\033[37m�ּּ֧֪��p���~�O���֪��p��.....\033[m\n"
    };
    outs(hint[rand() % 3]);
  }
  else if (mode == 1)		/* ���� */
  {
    sprintf(inbuf2, "%-4d/%4d", d.food, d.cookie);
    sprintf(inbuf3, "%-4d/%4d", d.pill, d.medicine);
    sprintf(inbuf4, "%-4d/%4d", d.burger, d.ginseng);
    sprintf(inbuf5, "%-4d/%4d", d.paste, d.snowgrass);
    prints("\033[1;32m[�����s] \033[37m%-9s\033[32m [�١��F] \033[37m%-9s\033[32m [�ɡ��x] \033[37m%-9s\033[32m [�I����] \033[37m%-9s\033[m\n", inbuf2, inbuf3, inbuf4, inbuf5);
  }
  else if (mode == 2)		/* ���u */
  {
    prints("\033[1;36m[�R��]\033[37m%-5d\033[36m[���z]\033[37m%-5d\033[36m[���]\033[37m%-5d\033[36m[���N]\033[37m%-5d\033[36m[�D�w]\033[37m%-5d\033[36m[�i��]\033[37m%-5d\033[36m[�a��]\033[37m%-5d\n\033[m",
      d.love, d.wisdom, d.character, d.art, d.etchics, d.brave, d.homework);
  }
  else if (mode == 3)		/* �צ� */
  {
    prints("\033[1;36m[���z]\033[37m%-5d\033[36m[���]\033[37m%-5d\033[36m[���N]\033[37m%-5d\033[36m[�i��]\033[37m%-5d\033[36m[����]\033[37m%-5d\033[36m[���m]\033[37m%-5d\033[36m[�t��]\033[37m%-5d\n\033[m",
      d.wisdom, d.character, d.art, d.brave, d.attack, d.resist, d.speed);
  }

  /* �ù����� (5~18�C) ��ܹ� */

  tm *= 10;

  if (menunum)		/* itoc.010816: �p�G�O�i�X�D���A���ݭn��ø�������� */
  {
    /* �i�X�D��椣��ø�������Ϸ|���@�Ӥp bug�A�N�O�Ĥ@���i�ӥD���S���ϡA���L�i�H�٤U�j�q��ø */

    move(5, 0);
    outs("\033[34m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m");

    /* �Ѧ~�֤��魫�ӨM�w�p����`�ͬ��_�~���� */

    /* �魫���M�w�i�H�ѦҤU���@�I���{������ */
    if (d.weight < tm + 30)
      pic = 1;		/* �G */
    else if (d.weight < tm + 90)
      pic = 2;		/* ���� */
    else
      pic = 3;		/* �D */

    switch (age)
    {
    case 0:
    case 1:
    case 2:
      show_basic_pic(pic);	/* pic1~3 */
      break;

    case 3:
    case 4:
      show_basic_pic(pic + 3);	/* pic4~6 */
      break;

    case 5:
    case 6:
      show_basic_pic(pic + 6);	/* pic7~9 */
      break;

    case 7:
    case 8:
      show_basic_pic(pic + 9);	/* pic10~12 */
      break;

    case 9:
    case 10:
      show_basic_pic(pic + 12);	/* pic13~15 */
      break;

    case 11:
      show_basic_pic(pic + 15);	/* pic16~18 */      
      break;
    }

    move(18, 0);
    outs("\033[34m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");
  }

  /* �ù��U�� (19~b_lines�C) ��ܥثe���A�A�ö��K�ˬd�O�_���` */

  move(19, 0);
  outs("\033[1;34m�w\033[37;44m  �� �A \033[0;1;34m�w\033[m\n");

  /* itoc.010801: �ɥ� age �ˬd���A�A�Ѱ��@���ˬd�줤�B�ѧC�@���ˬd�줤 */

  age = d.shit;			/* �T�K�V�ֶV�n�A�z�Q age = 0 */
  if (age >= 100)
  {
    pipdie("\033[1;31m�z��䦺�F\033[m  ", 1);
    return -1;
  }
  else if (age >= 80)
  {
    outs("\033[1;35m�֯䦺�F\033[m  ");
    d.sick += 4;
  }
  else if (age >= 60)
  {
    outs("\033[1;33m�ܯ�F��\033[m  ");
  }
  else if (age >= 40)
  {
    outs("���I���  ");
  }
  else if (age == 0)
  {
    outs("���b�p��  ");
  }

  age = d.hp * 100 / d.maxhp;		/* age = �庡����� % */
  if (age >= 90)
  {
    outs("\033[1;33m��������\033[m  ");
  }
  else if (age >= 80)
  {
    outs("�{�l����  ");
  }
  else if (age <= 40)
  {
    outs("\033[1;33m�Q�Y�F��\033[m  ");
  }
  else if (age <= 0)
  {
    pipdie("\033[1;31m���j���F\033[m  ", 1);
    return -1;
  }
  else if (age <= 20)
  {
    outs("\033[1;35m�־j���F\033[m  ");
    d.sick += 3;
    d.happy -= 5;
    d.satisfy -= 3;
  }  

  age = d.tired;			/* �h�ҶV�C�V�n�A�z�Q age = 0 */
  if (age >= 100)
  {
    pipdie("\033[1;31m����֦��F\033[m  ", 1);
    return -1;
  }
  else if (age >= 80)
  {
    outs("\033[1;35m�u���ܲ�\033[m  ");
    d.sick += 5;
  }
  else if (age >= 60)
  {
    outs("\033[1;33m���I�p��\033[m  ");
  }
  else if (age <= 10)
  {
    outs("�믫�۷��  ");
  }
  else if (age <= 20)
  {
    outs("�믫�ܦn  ");
  }

  age = d.weight - tm;			/* �z�Q�魫 age = 60 �Y d.wight = 60 + 10 * tm */
					/* �`�N���� tm �w�g * 10 �L�F */
  if (age >= 130)
  {
    pipdie("\033[1;31m���Φ��F\033[m  ", 1);
    return -1;
  }
  else if (age >= 110)
  {
    outs("\033[1;35m�ӭD�F��\033[m  ");
    d.sick += 3;
    if (d.speed > 2)
      d.speed -= 2;
    else
      d.speed = 0;
  }
  else if (age >= 90)
  {
    outs("\033[1;33m���I�p�D\033[m  ");    
  }
  else if (age <= -10)
  {
    pipdie("\033[1;31m:~~ �G���F\033[m  ", 1);
    return -1;
  }
  else if (age <= 10)
  {
    outs("\033[1;33m���I�p�G\033[m  ");
  }
  else if (age <= 30)
  {
    outs("\033[1;35m�ӽG�F��\033[m ");
  }

  age = d.sick;				/* �e�f�V�C�V�n�A�z�Q age = 0 */
  if (age >= 100)
  {
    pipdie("\033[1;31m�f���F�� :~~\033[m  ", 1);
    return -1;
  }
  else if (age >= 75)
  {
    outs("\033[1;35m���f����\033[m  ");
    d.sick += 5;
    count_tired(1, 15, 1, 100, 1);
  }
  else if (age >= 50)
  {
    outs("\033[1;33m�ͯf�F��\033[m  ");
    count_tired(1, 8, 1, 100, 1);
  }

  age = d.happy;			/* �ֶּV���V�n�A�z�Q age = 100 */
  if (age >= 90)
  {
    outs("�ְּ�..  ");
  }
  else if (age >= 80)
  {
    outs("�ְּ�..  ");
  }
  else if (age <= 10)
  {
    outs("\033[1;35m�ܤ��ּ�\033[m  ");
  }
  else if (age <= 20)
  {
    outs("\033[1;33m���ӧּ�\033[m  ");
  }

  age = d.satisfy;			/* �����V���V�n�A�z�Q age = 100 */
  if (age >= 90)
  {
    outs("������..  ");
  }
  else if (age >= 80)
  {
    outs("������..  ");
  }
  else if (age <= 10)
  {
    outs("\033[1;35m�ܤ�����\033[m  ");
  }
  else if (age <= 20)
  {
    outs("\033[1;33m���Ӻ���\033[m  ");
  }

  return 0;
}


  /*-----------------------------------------------------*/
  /* ���D�禡					 	 */
  /*-----------------------------------------------------*/

/* ���� menu.c ���\�� */

static int
pip_do_menu(menunum, menumode, cmdtable)
  int menunum;		/* ���@����� */
  int menumode;		/* ���@���e�� */
  struct pipcommands cmdtable[];	/* ���O�� */
{
  int ch, key;
  struct pipcommands *cmd;

  while (1)
  {
    /* �P�_�O�_���`�A�����Y���^�W�@�h */
    if (d.death)
      return 0;

    /* �e����ø�A�çP�w��O�_���`�A�����Y���^�W�@�h */
    if (pip_refresh_screen(menunum, menumode))
      return 0;

    /* �L�X�̫�G�C���O�C */
    out_cmd(menuname[menunum][0], menuname[menunum][1]);

    switch (ch = vkey())
    {
    case KEY_LEFT:
    case 'q':
      return 0;

    default:
#if 0
      /* itoc.010815: ���p�g */
      if (ch >= 'A' && ch <= 'Z')
	ch |= 0x20;
#endif

      cmd = cmdtable;
      for (; key = cmd->key; cmd++)
      {
	if (ch == key)
	{
	  cmd->fptr();
	  break;	/* itoc.010815: ���槹�{���n���s���� */
	}
      }
      break;
    }
  }

  return 0;
}


  /*-----------------------------------------------------*/
  /* �D���: �� �}�� �צ� ���� ���u �S��		 */
  /*-----------------------------------------------------*/

int
pip_main_menu()
{
  pip_do_menu(0, 0, pipmainlist);
  return 0;
}


  /*-----------------------------------------------------*/
  /* �򥻿��: ���� �M�� �˿� ��			 */
  /*-----------------------------------------------------*/

int
pip_basic_menu()
{
  pip_do_menu(1, 0, pipbasiclist);
  return 0;
}


  /*-----------------------------------------------------*/
  /* �ө����: ���� �s�� �j�ɤY ���� �ѥ�		 */
  /*-----------------------------------------------------*/

int
pip_store_menu()
{
  pip_do_menu(2, 1, pipstorelist);
  return 0;
}


  /*-----------------------------------------------------*/
  /* �צ���: ���� �m�Z �צ�     			 */
  /*-----------------------------------------------------*/

int
pip_practice_menu()
{
  pip_do_menu(3, 3, pippracticelist);
  return 0;
}


  /*-----------------------------------------------------*/
  /* ���ֿ��: ���B �ȹC �B�� ���| �q��			 */
  /*-----------------------------------------------------*/

int
pip_play_menu()
{
  pip_do_menu(4, 0, pipplaylist);
  return 0;
}


  /*-----------------------------------------------------*/
  /* ���u���: �a�� �W�u �a�� �a�u			 */
  /*-----------------------------------------------------*/

int
pip_job_menu()
{
  pip_do_menu(5, 2, pipjoblist);
  return 0;
}


  /*-----------------------------------------------------*/
  /* �S����: �ݯf ��� �԰� ���X �¨�			 */
  /*-----------------------------------------------------*/

int
pip_special_menu()
{
  pip_do_menu(6, 0, pipspeciallist);
  return 0;
}


  /*-----------------------------------------------------*/
  /* �t�ο��: �ӤH��� �p����� �S�O�A��		 */
  /*-----------------------------------------------------*/

int
pip_system_menu()
{
  pip_do_menu(7, 0, pipsystemlist);
  return;
}
#endif		/* HAVE_GAME */
