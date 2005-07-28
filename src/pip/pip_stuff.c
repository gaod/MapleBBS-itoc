/*-------------------------------------------------------*/
/* pip_stuff.c         ( NTHU CS MapleBBS Ver 3.10 )     */
/*-------------------------------------------------------*/
/* target : �ɯšB�t�ΡB�S���浥���C���K�禡           */
/* create :   /  /                                       */
/* update : 01/08/14                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* ��Ʀs��						 */
/*-------------------------------------------------------*/


void
pip_write_file()		/* �C���g��ƤJ�ɮ� */
{
  int fd;
  char fpath[64];

  usr_fpath(fpath, cuser.userid, fn_pip);
  fd = open(fpath, O_WRONLY | O_CREAT, 0600);	/* fpath �����w�g�s�b */
  write(fd, &d, sizeof(CHICKEN));
  close(fd);
}


int		/* >=0:���\  <0:���� */
pip_read_file(userid, p)	/* �C��Ū��ƥX�ɮ� */
  char *userid;
  struct CHICKEN *p;
{
  int fd;
  char fpath[64];

  usr_fpath(fpath, userid, fn_pip);
  fd = open(fpath, O_RDONLY);		/* fpath �����w�g�s�b */
  if (fd >= 0)
  {
    read(fd, p, sizeof(CHICKEN));
    close(fd);
  }
  return fd;
}


int			/* 1: ���\�g�J  0: ��� */
pip_write_backup()	/* �p���i�׳ƥ� */
{
  char *files[4] = {"�S��", "�i�פ@", "�i�פG", "�i�פT"};
  char buf[80], fpath[64];
  int ch;

  show_basic_pic(101);

  ch = ians(b_lines - 2, 0, "�x�s [1] �i�פ@ [2] �i�פG [3] �i�פT [Q] ���G[Q] ") - '0';

  if (ch < 1 || ch > 3)
  {
    vmsg("����x�s�C���ƥ�");
    return 0;
  }

  sprintf(buf, "�T�w�n�x�s�� [%s] �ɮ׶�(Y/N)�H[N] ", files[ch]);
  if (ians(b_lines - 2, 0, buf) != 'y')
  {
    vmsg("����x�s�ɮ�");
    return 0;
  }

  sprintf(buf, "�x�s [%s] �ɮק����F", files[ch]);
  vmsg(buf);

  sprintf(buf, "%s.bak%d", fn_pip, ch);
  usr_fpath(fpath, cuser.userid, buf);
  ch = open(fpath, O_WRONLY | O_CREAT, 0600);	/* fpath �����w�g�s�b */
  write(ch, &d, sizeof(CHICKEN));
  close(ch);

  return 1;
}


int			/* 1: ���\Ū�X  0: ��� */
pip_read_backup()	/* �p���ƥ�Ū�� */
{
  char *files[4] = {"�S��", "�i�פ@", "�i�פG", "�i�פT"};
  char buf[80], fpath[64];
  int ch, fd;

  show_basic_pic(102);

  ch = ians(b_lines - 2, 0, "Ū�� [1] �i�פ@ [2] �i�פG [3] �i�פT [Q] ���G[Q] ") - '0';

  if (ch < 1 || ch > 3)
  {
    vmsg("���Ū���C���ƥ�");
    return 0;
  }

  sprintf(buf, "%s.bak%d", fn_pip, ch);
  usr_fpath(fpath, cuser.userid, buf);

  fd = open(fpath, O_RDONLY);		/* fpath �����w�g�s�b */
  if (fd >= 0)
  {
    sprintf(buf, "�T�w�nŪ���� [%s] �ɮ׶�(Y/N)�H[N] ", files[ch]);
    if (ians(b_lines - 2, 0, buf) == 'y')
    {
      read(fd, &d, sizeof(CHICKEN));
      close(fd);
      sprintf(buf, "Ū�� [%s] �ɮק����F", files[ch]);
      vmsg(buf);
      return 1;
    }
    vmsg("���Ū���ɮ�");
    close(fd);
    return 0;
  }
  else
  {
    sprintf(buf, "�ɮ� [%s] ���s�b", files[ch]);
    vmsg(buf);
    return 0;
  }
}


/*-------------------------------------------------------*/
/* �p�����A�禡					 	 */
/*-------------------------------------------------------*/


void
pipdie(msg, diemode)	/* �p�����` */
  char *msg;
  int diemode;
{
  vs_head("�q�l�i�p��", str_site);

  if (diemode == 1)
  {
    show_die_pic(1);
    vmsg("�����ӱa���p���F");
    vs_head("�q�l�i�p��", str_site);
    show_die_pic(2);
    move(14, 20);
    prints("�i�����p��\033[1;31m%s\033[m", msg);
    vmsg(BBSNAME "�s����....");
  }
  else if (diemode == 2)
  {
    show_die_pic(3);
    vmsg("����..�ڳQ���F.....");
  }
  else if (diemode == 3)
  {
    show_die_pic(0);
    vmsg("�C�������o..");
  }

  d.death = diemode;
  pip_write_file();
}


int
count_tired(prob, base, mode, mul, cal)		/* itoc.010803: �̷ӶǤJ���޼ƨӼW��h�ҫ� */
  int prob;				/* ���v */
  int base;				/* ���� */
  int mode;				/* ���� 1:�M�~�֦���  0:�M�~�ֵL��  */
  int mul;				/* �[�v (�H % �ӭp 100->1) */
  int cal;				/* 1:�[�h��  0:��h�� */
{
  int tiredvary;       /* ���ܭ� */

  /* ������ܶq */  
  tiredvary = rand() % prob + base;  

  if (mode)            /* �M�~�֦��� */
  {
    int tm;            /* �~�� */
    tm = d.bbtime / 60 / 30;

    /* itoc.010803: �~���V�p�A�[�h�Ҥ���֡A��_�h�Ҥ]����� */
    /* �`�N����g�� tiredvary *= 6 / 5; �� :p */

    if (tm <= 3)       /* 0~3 �� */
    {
      tiredvary = cal ? tiredvary * 16 / 15 : tiredvary * 6 / 5;
    }
    else if (tm <= 7)  /* 4~7 �� */
    {
      tiredvary = cal ? tiredvary * 11 / 10 : tiredvary * 8 / 7;
    }
    else if (tm <= 10) /* 8~10 �� */
    {
      tiredvary = cal ? tiredvary * 8 / 7 : tiredvary * 11 / 10;
    }
    else               /* 11 ���H�W */
    {
      tiredvary = cal ? tiredvary * 6 / 5 : tiredvary * 16 / 15;
    }
  }

  /* �A��[�v */
  if (cal)
  {
    d.tired += tiredvary * mul / 100;
    if (d.tired > 100)
      d.tired = 100;
  }
  else
  {
    d.tired -= tiredvary;      /* ���Ȥ��A�[�v�F */
    if (d.tired < 0)
      d.tired = 0;
  }
}


/*-------------------------------------------------------*/
/* �S����:�ݯf ��� 				         */
/*-------------------------------------------------------*/


int				/* 1:�ݧ����  0:�S�f����|�c�d */
pip_see_doctor()		/* ����� */
{
  char buf[256];
  long savemoney;
  savemoney = d.sick * 25;
  if (d.sick <= 0)
  {
    vmsg("�z��..�S�f����|�F��..�Q�|�F..��~~");
    d.character -= rand() % 3 + 1;
    if (d.character < 0)
      d.character = 0;
    d.happy -= (rand() % 3 + 3);
    d.satisfy -= rand() % 3 + 2;
  }
  else if (d.money < savemoney)
  {
    sprintf(buf, "�z���f�n�� %d ����....�z��������...", savemoney);
    vmsg(buf);
  }
  else
  {
    d.tired -= rand() % 10 + 20;
    if (d.tired < 0)
      d.tired = 0;
    d.sick = 0;
    d.money = d.money - savemoney;
    move(4, 0);
    show_special_pic(1);
    vmsg("�Ĩ�f��..�S���Ƨ@��!!");
    return 1;
  }
  return 0;
}


int				/* 1:��e  0:�S����e */
pip_change_weight()		/* �W�D/��� */
{
  char buf[80];
  int weightmp;

  show_special_pic(2);

  out_cmd("", COLOR1 " ���e " COLOR2 " [1]�ǲμW�D [2]�ֳt�W�D [3]�ǲδ�� [4]�ֳt��� [Q]���X                \033[m");

  switch (vkey())
  {
  case '1':
    if (d.money < 80)
    {
      vmsg("�ǲμW�D�n80����....�z��������...");
    }
    else
    {
      if (ians(b_lines - 1, 0, " �ݪ�O 80 ���]3��5����^�A�T�w��(Y/N)�H[N] ") == 'Y')
      {
	weightmp = 3 + rand() % 3;
	d.weight += weightmp;
	d.money -= 80;
	d.hp -= rand() % 2 + 3;
	show_special_pic(3);
	sprintf(buf, "�`�@�W�[�F %d ����", weightmp);
	vmsg(buf);
	return 1;
      }
      else
      {
	vmsg("�^����N�o.....");
      }
    }
    break;

  case '2':
    vget(b_lines - 1, 0, " �W�@����n 30 ���A�z�n�W�h�֤���O�H[�ж�Ʀr]�G", buf, 4, DOECHO);
    weightmp = atoi(buf);
    if (weightmp <= 0)
    {
      vmsg("��J���~..����o...");
    }
    else if (d.money > (weightmp * 30))
    {
      sprintf(buf, " �W�[ %d ����A�`�@�ݪ�O�F %d ���A�T�w��(Y/N)�H[N] ", weightmp, weightmp * 30);      
      if (ians(b_lines - 1, 0, buf) == 'y')
      {
	d.money -= weightmp * 30;
	d.weight += weightmp;
	count_tired(5, 8, 0, 100, 1);
	d.hp -= (rand() % 2 + 3);
	d.sick += rand() % 10 + 5;
	show_special_pic(3);
	sprintf(buf, "�`�@�W�[�F %d ����", weightmp);
	vmsg(buf);
	return 1;
      }
      else
      {
	vmsg("�^����N�o.....");
      }
    }
    else
    {
      vmsg("�z���S����h��.......");
    }
    break;

  case '3':
    if (d.money < 80)
    {
      vmsg("�ǲδ�έn80����....�z��������...");
    }
    else
    {
      if (ians(b_lines - 1, 0, "�ݪ�O 80��(3��5����)�A�T�w��(Y/N)�H[N] ") == 'y')
      {
	weightmp = 3 + rand() % 3;
	d.weight -= weightmp;
	if (d.weight <= 0)
	  d.weight = 1;
	d.money -= 100;
	d.hp -= rand() % 2 + 3;
	show_special_pic(4);
	sprintf(buf, "�`�@��֤F %d ����", weightmp);
	vmsg(buf);
	return 1;
      }
      else
      {
	vmsg("�^����N�o.....");
      }
    }
    break;

  case '4':
    vget(b_lines - 1, 0, " ��@����n 30 ���A�z�n��h�֤���O�H[�ж�Ʀr]�G", buf, 4, DOECHO);
    weightmp = atoi(buf);
    if (weightmp <= 0)
    {
      vmsg("��J���~..����o...");
    }
    else if (d.weight <= weightmp)
    {
      vmsg("�z�S���򭫳�.....");
    }
    else if (d.money > (weightmp * 30))
    {
      sprintf(buf, " ��� %d ����A�`�@�ݪ�O�F %d ���A�T�w��(Y/N)�H[N] ", weightmp, weightmp * 30);
      if (ians(b_lines - 1, 0, buf) == 'y')
      {
	d.money -= weightmp * 30;
	d.weight -= weightmp;
	count_tired(5, 8, 0, 100, 1);
	d.hp -= (rand() % 2 + 3);
	d.sick += rand() % 10 + 5;
	show_special_pic(4);
	sprintf(buf, "�`�@��֤F %d ����", weightmp);
	vmsg(buf);
	return 1;
      }
      else
      {
	vmsg("�^����N�o.....");
      }
    }
    else
    {
      vmsg("�z���S����h��.......");
    }
    break;
  }
  return 0;
}


/*-------------------------------------------------------*/
/* �t�ο��: �ӤH��� ���X �p�����  �S�O�A��		 */
/*-------------------------------------------------------*/


static int			/* 0: �S���i�p��  1: ���i�p�� */
pip_data_list(userid)		/* �ݬY�H�p���ԲӸ�� */
  char *userid;
{
  char buf1[20], buf2[20], buf3[20], buf4[20];
  int ch, page;
  struct CHICKEN chicken;

  if (!strcmp(cuser.userid, userid))	/* itoc.021031: �p�G�d�ߦۤv�A�q�O����s�{�b�� */
  {
    memcpy(&chicken, &d, sizeof(CHICKEN));
  }
  else if (pip_read_file(userid, &chicken) < 0)
  {
    vmsg("�L�S���i�p����");
    return 0;
  }

  page = 1;

  do
  {
    clear();
    move(1, 0);

    /* itoc,010802: ���F�ݲM���@�I�A�ҥH prints() �̭����޼ƴN���_��g�b�ӦC�̫� */

    if (page == 1)
    {
      outs("\033[1;31m �~�t\033[41;37m �򥻸�� \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��\033[m\n");
      prints("\033[1;31m �x\033[33m�̩m    �W : \033[37m%-10s\033[33m�̥�    �� : \033[37m%-10s\033[33m�̩�    �O : \033[37m%-10s\033[31m �x\033[m\n", chicken.name, chicken.birth, chicken.sex == 1 ? "��" : "��");
      prints("\033[1;31m �x\033[33m�̪�    �A : \033[37m%-10s\033[33m�̴_������ : \033[37m%-10d\033[33m�̦~    �� : \033[37m%-10d\033[31m �x\033[m\n", chicken.death == 1 ? "���`" : chicken.death == 2 ? "�߱�" : chicken.death == 3 ? "����" : "���`", chicken.liveagain, chicken.bbtime / 60 / 30);

      outs("\033[1;31m �u�t\033[41;37m ���A���� \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\033[m\n");
      prints("\033[1;31m �x\033[33m�̿ˤl���Y : \033[37m%-10d\033[33m�̧� �� �� : \033[37m%-10d\033[33m�̺� �N �� : \033[37m%-10d\033[31m �x\033[m\n", chicken.relation, chicken.happy, chicken.satisfy);
      prints("\033[1;31m �x\033[33m���ʷR���� : \033[37m%-10d\033[33m�̫H    �� : \033[37m%-10d\033[33m�̸o    �^ : \033[37m%-10d\033[31m �x\033[m\n", chicken.fallinlove, chicken.belief, chicken.sin);
      prints("\033[1;31m �x\033[33m�̷P    �� : \033[37m%-10d\033[33m��         : \033[37m%-10s\033[33m��         : \033[37m%-10s\033[31m �x\033[m\n", chicken.affect, "", "");

      outs("\033[1;31m �u�t\033[41;37m ���d���� \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\033[m\n");
      prints("\033[1;31m �x\033[33m����    �� : \033[37m%-10d\033[33m�̯h �� �� : \033[37m%-10d\033[33m�̯f    �� : \033[37m%-10d\033[31m �x\033[m\n", chicken.weight, chicken.tired, chicken.sick);
      prints("\033[1;31m �x\033[33m�̲M �� �� : \033[37m%-10d\033[33m��         : \033[37m%-10s\033[33m��         : \033[37m%-10s\033[31m �x\033[m\n", chicken.shit, "", "");

      outs("\033[1;31m �u�t\033[41;37m �C���I�� \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\033[m\n");
      prints("\033[1;31m �x\033[33m�̲q �� Ĺ : \033[37m%-10d\033[33m�̲q �� �� : \033[37m%-10d\033[33m�̲q������ : \033[37m%-10d\033[31m �x\033[m\n", chicken.winn, chicken.losee, chicken.tiee);

      outs("\033[1;31m �u�t\033[41;37m �����Ѽ� \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\033[m\n");
      prints("\033[1;31m �x\033[33m�̪������ : \033[37m%-10d\033[33m�̮a�Ƶ��� : \033[37m%-10d\033[33m�̾԰����� : \033[37m%-10d\033[31m �x\033[m\n", chicken.social, chicken.family, chicken.hexp);
      prints("\033[1;31m �x\033[33m���]�k���� : \033[37m%-10d\033[33m��         : \033[37m%-10s\033[33m��         : \033[37m%-10s\033[31m �x\033[m\n", chicken.hexp, "", "");

      outs("\033[1;31m �u�t\033[41;37m �����Ѽ� \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\033[m\n");
      prints("\033[1;31m �x\033[33m�̪������ : \033[37m%-10d\033[33m�̮a�Ƶ��� : \033[37m%-10d\033[33m�̾԰����� : \033[37m%-10d\033[31m �x\033[m\n", chicken.social, chicken.family, chicken.hexp);
      prints("\033[1;31m �x\033[33m���]�k���� : \033[37m%-10d\033[33m��         : \033[37m%-10s\033[33m��         : \033[37m%-10s\033[31m �x\033[m\n", chicken.hexp, "", "");

      outs("\033[1;31m ���w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��\033[m\n");
      move(b_lines - 2, 0);
      outs("                                                             \033[1;36m�Ĥ@��\033[37m/\033[36m�@�T��\033[m\n");
    }
    else if (page == 2)
    {
      outs("\033[1;31m �~�t\033[41;37m ��O�Ѽ� \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��\033[m\n");
      prints("\033[1;31m �x\033[33m�̫ݤH���� : \033[37m%-10d\033[33m�̮� �� �� : \033[37m%-10d\033[33m�̷R    �� : \033[37m%-10d\033[31m �x\033[m\n", chicken.toman, chicken.character, chicken.love);
      prints("\033[1;31m �x\033[33m�̴�    �O : \033[37m%-10d\033[33m�����N��O : \033[37m%-10d\033[33m�̹D    �w : \033[37m%-10d\033[31m �x\033[m\n", chicken.wisdom, chicken.art, chicken.etchics);
      prints("\033[1;31m �x\033[33m�̫i    �� : \033[37m%-10d\033[33m�̱��a�~�� : \033[37m%-10d\033[33m�̾y    �O : \033[37m%-10d\033[31m �x\033[m\n", chicken.brave, chicken.homework, chicken.charm);
      prints("\033[1;31m �x\033[33m��§    �� : \033[37m%-10d\033[33m�̽�    �R : \033[37m%-10d\033[33m�̲i    �� : \033[37m%-10d\033[31m �x\033[m\n", chicken.manners, chicken.speech, chicken.cook);
      prints("\033[1;31m �x\033[33m�̧� �� �O : \033[37m%-10d\033[33m�̨� �m �O : \033[37m%-10d\033[33m�̳t    �� : \033[37m%-10d\033[31m �x\033[m\n", chicken.attack, chicken.resist, chicken.speed);
      prints("\033[1;31m �x\033[33m�̾԰��޳N : \033[37m%-10d\033[33m���]�k�޳N : \033[37m%-10d\033[33m�̧��]��O : \033[37m%-10d\033[31m �x\033[m\n", chicken.hskill, chicken.mskill, chicken.immune);

      sprintf(buf1, "%d%s/%d%s", chicken.hp > 1000 ? chicken.hp / 1000 : chicken.hp, chicken.hp > 1000 ? "K" : "",	/* HP */
	chicken.maxhp > 1000 ? chicken.maxhp / 1000 : chicken.maxhp, chicken.maxhp > 1000 ? "K" : "");
      sprintf(buf2, "%d%s/%d%s", chicken.mp > 1000 ? chicken.mp / 1000 : chicken.mp, chicken.mp > 1000 ? "K" : "",	/* MP */      
	chicken.maxmp > 1000 ? chicken.maxmp / 1000 : chicken.maxmp, chicken.maxmp > 1000 ? "K" : "");
      sprintf(buf3, "%d%s/%d%s", chicken.vp > 1000 ? chicken.vp / 1000 : chicken.vp, chicken.vp > 1000 ? "K" : "",	/* VP */
	chicken.maxvp > 1000 ? chicken.maxvp / 1000 : chicken.maxvp, chicken.maxvp > 1000 ? "K" : "");
      sprintf(buf4, "%d%s/%d%s", chicken.sp > 1000 ? chicken.sp / 1000 : chicken.sp, chicken.sp > 1000 ? "K" : "",	/* SP */      
	chicken.maxsp > 1000 ? chicken.maxsp / 1000 : chicken.maxsp, chicken.maxsp > 1000 ? "K" : "");            

      outs("\033[1;31m �u�t\033[41;37m �԰����� \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\033[m\n");
      prints("\033[1;31m �x\033[33m�̵�    �� : \033[37m%-10d\033[33m�̸g �� �� : \033[37m%-10d\033[33m��   ��    : \033[37m%-10s\033[31m �x\033[m\n", chicken.level, chicken.exp, buf1);
      prints("\033[1;31m �x\033[33m�̪k    �O : \033[37m%-10s\033[33m�̲� �� �O : \033[37m%-10s\033[33m�̤�    �O : \033[37m%-10s\033[31m �x\033[m\n", buf2, buf3, buf4);

      outs("\033[1;31m �u�t\033[41;37m �����w�s \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\033[m\n");
      prints("\033[1;31m �x\033[33m�̭�    �� : \033[37m%-10d\033[33m�̹s    �� : \033[37m%-10d\033[33m��         : \033[37m%-10s\033[31m �x\033[m\n", chicken.food, chicken.cookie, "");
      prints("\033[1;31m �x\033[33m�̤j �� �� : \033[37m%-10d\033[33m���F    �� : \033[37m%-10d\033[33m�̤j �� �Y : \033[37m%-10d\033[31m �x\033[m\n", chicken.pill, chicken.medicine, chicken.burger);
      prints("\033[1;31m �x\033[33m�̤H    �x : \033[37m%-10d\033[33m���_ �� �I : \033[37m%-10d\033[33m�̳�    �� : \033[37m%-10d\033[31m �x\033[m\n", chicken.ginseng, chicken.paste, chicken.snowgrass);

      outs("\033[1;31m �u�t\033[41;37m ���~�w�s \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\033[m\n");
      prints("\033[1;31m �x\033[33m�̪�    �� : \033[37m%-10d\033[33m��         : \033[37m%-10s\033[33m��         : \033[37m%-10s\033[31m �x\033[m\n", chicken.money, "", "");
      prints("\033[1;31m �x\033[33m�̮�    �� : \033[37m%-10d\033[33m�̪�    �� : \033[37m%-10d\033[33m�̽ҥ~Ū�� : \033[37m%-10d\033[31m �x\033[m\n", chicken.book, chicken.toy, chicken.playboy);

 
      outs("\033[1;31m ���w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��\033[m\n");
      move(b_lines - 2, 0);
      outs("                                                             \033[1;36m�ĤG��\033[37m/\033[36m�@�T��\033[m\n");
    }
    else /* if (page == 3) */
    {
      outs("\033[1;31m �~�t\033[41;37m �Ѩ����� \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��\033[m\n");
      prints("\033[1;31m �x\033[33m�̦u�æn�P : \033[37m%-10d\033[33m�̪�æn�P : \033[37m%-10d\033[33m�̱N�x�n�P : \033[37m%-10d\033[31m �x\033[m\n", chicken.royalA, chicken.royalB, chicken.royalC);
      prints("\033[1;31m �x\033[33m�̤j�ڦn�P : \033[37m%-10d\033[33m�̲��q�n�P : \033[37m%-10d\033[33m���d�m�n�P : \033[37m%-10d\033[31m �x\033[m\n", chicken.royalD, chicken.royalE, chicken.royalF);
      prints("\033[1;31m �x\033[33m�̤��m�n�P : \033[37m%-10d\033[33m�̰���n�P : \033[37m%-10d\033[33m�̤p���n�P : \033[37m%-10d\033[31m �x\033[m\n", chicken.royalG, chicken.royalH, chicken.royalI);

      outs("\033[1;31m �u�t\033[41;37m �u�@���� \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\033[m\n");
      prints("\033[1;31m �x\033[33m�̮a�Ʀ��� : \033[37m%-10d\033[33m�̫O�i���� : \033[37m%-10d\033[33m�̮ȩ����� : \033[37m%-10d\033[31m �x\033[m\n", chicken.workA, chicken.workB, chicken.workC);
      prints("\033[1;31m �x\033[33m�̹A������ : \033[37m%-10d\033[33m���\\�U���� : \033[37m%-10d\033[33m�̱а󦸼� : \033[37m%-10d\033[31m �x\033[m\n", chicken.workD, chicken.workE, chicken.workF);
      prints("\033[1;31m �x\033[33m�̦a�u���� : \033[37m%-10d\033[33m�̥�즸�� : \033[37m%-10d\033[33m�̬��v���� : \033[37m%-10d\033[31m �x\033[m\n", chicken.workG, chicken.workH, chicken.workI);
      prints("\033[1;31m �x\033[33m���y�H���� : \033[37m%-10d\033[33m�̤u�a���� : \033[37m%-10d\033[33m�̦u�Ӧ��� : \033[37m%-10d\033[31m �x\033[m\n", chicken.workJ, chicken.workK, chicken.workL);
      prints("\033[1;31m �x\033[33m�̮a�Ц��� : \033[37m%-10d\033[33m�̰s�a���� : \033[37m%-10d\033[33m�̰s������ : \033[37m%-10d\033[31m �x\033[m\n", chicken.workM, chicken.workN, chicken.workO);
      prints("\033[1;31m �x\033[33m�̩] �` �| : \033[37m%-10d\033[33m��         : \033[37m%-10s\033[33m��         : \033[37m%-10s\033[31m �x\033[m\n", chicken.workP, "", "");

      outs("\033[1;31m �u�t\033[41;37m �W�Ҧ��� \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\033[m\n");
      prints("\033[1;31m �x\033[33m�̦۵M��� : \033[37m%-10d\033[33m�̭�֧��� : \033[37m%-10d\033[33m�̯��ǱШ| : \033[37m%-10d\033[31m �x\033[m\n", chicken.classA, chicken.classB, chicken.classC);
      prints("\033[1;31m �x\033[33m�̭x�ǱШ| : \033[37m%-10d\033[33m�̼C�D�޳N : \033[37m%-10d\033[33m�̮氫�ԧ� : \033[37m%-10d\033[31m �x\033[m\n", chicken.classD, chicken.classE, chicken.classF);
      prints("\033[1;31m �x\033[33m���]�k�Ш| : \033[37m%-10d\033[33m��§���Ш| : \033[37m%-10d\033[33m��ø�e�ޥ� : \033[37m%-10d\033[31m �x\033[m\n", chicken.classG, chicken.classH, chicken.classI);
      prints("\033[1;31m �x\033[33m�̻R�Чޥ� : \033[37m%-10d\033[33m��         : \033[37m%-10s\033[33m��         : \033[37m%-10s\033[31m �x\033[m\n", chicken.classJ, "", "");

      outs("\033[1;31m �u�t\033[41;37m �˳ƦC�� \033[0;1;31m�u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t\033[m\n");
      prints("\033[1;31m �x\033[33m���Y���˳� : \033[37m%-10s\033[33m�̤ⳡ�˳� : \033[37m%-10s\033[33m�̬޵P�˳� : \033[37m%-10s\033[31m �x\033[m\n", chicken.equiphead, chicken.equiphand, chicken.equipshield);
      prints("\033[1;31m �x\033[33m�̨���˳� : \033[37m%-10s\033[33m�̸}���˳� : \033[37m%-10s\033[33m��           \033[37m%-10s\033[31m �x\033[m\n", chicken.equipbody, chicken.equipfoot, "");

      outs("\033[1;31m ���w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��\033[m\n");
      move(b_lines - 2, 0);
      outs("                                                             \033[1;36m�ĤT��\033[37m/\033[36m�@�T��\033[m\n");
    }

    out_cmd("", COLOR1 " �d�� " COLOR2 " [��/PgUP]���W�@�� [��/PgDN]���U�@�� [Q]���}                            \033[m");

    switch (ch = vkey())
    {
    case KEY_UP:
    case KEY_PGUP:
      if (page > 1)
        page--;
      break;

    default:
      if (page < 3)
	page++;
      break;
    }
  } while (ch != 'q' && ch != KEY_LEFT);

  return 1;
}


int
pip_query_self()		/* �d�ߦۤv */
{
  pip_data_list(cuser.userid);
  return 0;
}


int				/* 1:���X���\  0:�S�o���� */
pip_query()			/* ���X�p�� */
{
  int uno;
  char uid[IDLEN + 1];

  vs_bar("���X�P��");
  if (vget(1, 0, msg_uid, uid, IDLEN + 1, GET_USER))
  {
    move(2, 0);
    if (uno = acct_userno(uid))
    {
      pip_data_list(uid);
      return 1;
    }
    else
    {
      outs(err_uid);
      clrtoeol();
    }
  }
  return 0;
}


int				/* 1:���  0:��i */
pip_system_freepip()
{
  char buf[80];

  if (ians(b_lines - 2, 0, "�u���n��Ͷ�(Y/N)�H[N] ") == 'y')
  {
    sprintf(buf, "\033[1;31m%s �Q���ߪ� %s �ᱼ�F~\033[m", d.name, cuser.userid);
    pipdie(buf, 2);
    return 1;
  }
  return 0;
}


int
pip_system_service()
{
  int choice;
  char buf[128];

  out_cmd("", COLOR1 " �A�� " COLOR2 " [1]�R�W�j�v [2]�ܩʤ�N [3]�����]�� [Q]���}                            \033[m");

  switch (vkey())
  {
  case '1':
    vget(b_lines - 2, 0, "���p�����s���Ӧn�W�r�G", buf, 11, DOECHO);
    if (!buf[0])
    {
      vmsg("���@�U�Q�n�A�Ӧn�F  :)");
    }
    else
    {
      strcpy(d.name, buf);
      vmsg("���  ���@�ӷs���W�r��...");
    }
    break;

  case '2':			/* �ܩ� */
    if (d.sex == 1)	/* 1:�� 2:�� */
    {
      choice = 2;		/* ��-->�� */
      sprintf(buf, "�N�p���ѡ��ܩʦ��𪺶�(Y/N)�H[N] ");
    }
    else
    {
      choice = 1;		/* ��-->�� */
      sprintf(buf, "�N�p���ѡ��ܩʦ��񪺶�(Y/N)�H[N] ");
    }
    if (ians(b_lines - 2, 0, buf) == 'y')
    {
      d.sex = choice;
      vmsg("�ܩʤ�N����...");
    }
    break;

  case '3':
    /* 1:���n�B���B 4:�n�B���B */
    if (d.wantend == 1 || d.wantend == 2 || d.wantend == 3)
    {
      choice = 3;		/* �S��-->�� */
      sprintf(buf, "�N�p���C���令�i��20�������j(Y/N)�H[N] ");
    }
    else
    {
      choice = -3;		/* ��-->�S�� */
      sprintf(buf, "�N�p���C���令�i�S��20�������j(Y/N)�H[N] ");
    }
    if (ians(b_lines - 2, 0, buf) == 'y')
    {
      d.wantend += choice;
      vmsg("�C�������]�w����...");
    }
    break;
  }
  return 0;
}
#endif	/* HAVE_GAME */
