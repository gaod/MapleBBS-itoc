/*-------------------------------------------------------*/
/* pip_job.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : ���u                                         */
/* create :   /  /                                       */
/* update : 01/08/15                                     */
/* author : dsyan.bbs@forever.twbbs.org		 	 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* �禡�w                                                */
/*-------------------------------------------------------*/


/* itoc.010815: �g�@�����Ϊ� function */

static int		/* �Ǧ^: -1:���  0~100:���\���p */
pip_job_function(classgrade, tired_prob, tired_base, pic)
  int classgrade;	/* ���ߪ��p: 0% ~ 100 % */
  int tired_prob;	/* �p��h�Ҫ����v */
  int tired_base;	/* �p��h�Ҫ����� */
  int pic;		/* �n�q���� */
{
  int grade;

  /* �]���٨S update�Alearn_skill �i�� < 0 */
  if (LEARN_LEVEL < 0)
  {
    vmsg("�z�w�g�֨��z�F");
    return -1;
  }

  grade = classgrade * LEARN_LEVEL;

  /* grade ���ӥu�q 0~100% */
  if (grade < 0)
    grade = 0;
  else if (grade > 100)
    grade = 100;

  /* �Ҧ��u�@���|���ܪ��ݩ� */
  count_tired(tired_prob, tired_base, 1, 100, 1);	/* �W�[�h�һP�~�֦��� */
  d.shit += rand() % 3 + 5;
  d.hp -= rand() % 2 + 4;
  d.happy -= rand() % 3 + 4;
  d.satisfy -= rand() % 3 + 4;

  show_job_pic(pic);
  return grade;		/* �^�Ǥu�@���G: 0:���ѳz�F  ~   100:�������\ */
}


/*-------------------------------------------------------*/
/* ���u���:�a�� �W�u �a�� �a�u				 */
/*-------------------------------------------------------*/


int
pip_job_workA()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x�a�x�޲z�x+ �ݤH���� ���a�~�� �i�� �ˤl���Y �a�Ƶ���  �x */
  /* �x        �x- �P��                                      �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 2, 5, 11)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 80 + (d.homework + d.cook) / 50;
    vmsg("�a�ƫܦ��\\��..�h�@�I�����z..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 60 + (d.homework + d.cook) / 55;
    vmsg("�a�����Z���Q����..���..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 40 + (d.homework + d.cook) / 60;
    vmsg("�a�ƴ����q�q��..�i�H��n��..�[�o..");
  }
  else
  {
    class = 1;
    d.money += 20 + (d.homework + d.cook) / 65;
    vmsg("�a�ƫ��V�|��..�o�ˤ����..");
  }

  d.toman += rand() % 2;
  d.homework += rand() % 2 + class;
  d.cook += rand() % 2 + class;
  d.relation += rand() % 2;
  d.family += class;

  d.affect -= rand() % 5;
  if (d.affect < 0)
    d.affect = 0;

  d.workA++;
  return 0;
}


int
pip_job_workB()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x�|���|  �x+ �ݤH���� �R�� �P��                        �x */
  /* �x        �x- �y�O                                      �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 3, 7, 21)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 150 + (d.toman + d.love) / 50;
    vmsg("��O�i�ܦ��\\��..�U���A�ӳ�..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 120 + (d.toman + d.love) / 55;
    vmsg("�O�i�ٷ�������..���..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 100 + (d.toman + d.love) / 60;
    vmsg("�p�B�ͫܥֳ�..�[�o..");
  }
  else
  {
    class = 1;
    d.money += 80 + (d.toman + d.love) / 65;
    vmsg("���V�|��..�s�p�B�ͳ��n����..");
  }

  d.toman += rand() % 3;
  d.love += rand() % 3 + class;
  d.affect += class;

  d.charm -= rand() % 5;
  if (d.charm < 0)
    d.charm = 0;

  /* itoc.010824: �üƾǷ|�t�� */
  if (rand() % 30 == 0)
    pip_learn_skill(7);

  d.workB++;
  return 0;
}


int
pip_job_workC()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x���]    �x+ ���a�~�� �i�� �a�Ƶ���                    �x */
  /* �x        �x- �L                                        �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 4, 8, 31)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 250 + (d.homework + d.cook) / 50;
    vmsg("���]�Ʒ~�]�]��W..�Ʊ�z�A�L������..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 200 + (d.homework + d.cook) / 55;
    vmsg("���]���Z���Q����..���..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 150 + (d.homework + d.cook) / 60;
    vmsg("�����q�q��..�i�H��n��..�[�o..");
  }
  else
  {
    class = 1;
    d.money += 100 + (d.homework + d.cook) / 65;
    vmsg("�o�ӫ��V�|��..�o�ˤ����..");
  }

  d.homework += rand() % 2 + class;
  d.cook += rand() % 2 + class;
  d.family += class;

  d.workC++;
  return 0;
}


int
pip_job_workD()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x�A��    �x+ �L                                        �x */
  /* �x        �x- ���                                      �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 6, 12, 41)) < 0)
    return 0;

  if (class >= 75)
  {
    d.money += 250 + (d.attack + d.resist) / 50;
    vmsg("���Ϫ����n�n��..�Ʊ�z�A������..");
  }
  else if (class >= 50)
  {
    d.money += 210 + (d.attack + d.resist) / 55;
    vmsg("����..�٤�����..");
  }
  else if (class >= 25)
  {
    d.money += 160 + (d.attack + d.resist) / 60;
    vmsg("�����q�q��..�i�H��n��..");
  }
  else
  {
    d.money += 120 + (d.attack + d.resist) / 65;
    vmsg("�z���ӾA�X�A�����u�@..");
  }

  d.character -= rand() % 5;
  if (d.character < 0)
    d.character = 0;

  d.workD++;
  return 0;
}


int
pip_job_workE()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x�\�U    �x+ ���a�~�� �i��                             �x */
  /* �x        �x- �L                                        �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  class = d.cook / 6 - d.tired;
  if ((class = pip_job_function(class, 4, 9, 51)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 250 + (d.homework + d.cook) / 50;
    vmsg("�ȤH�����Ӧn�Y�F..�A�Ӥ@�L�a..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 200 + (d.homework + d.cook) / 55;
    vmsg("�N���٤����Y��..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 150 + (d.homework + d.cook) / 60;
    vmsg("�����q�q��..�i�H��n��..");
  }
  else
  {
    class = 1;
    d.money += 100 + (d.homework + d.cook) / 65;
    vmsg("�p���ݥ[�j��..");
  }

  d.homework += rand() % 2 + class;
  d.cook += rand() % 5 + class;

  d.workE++;
  return 0;
}


int
pip_job_workF()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x�а�    �x+ �R�� �D�w �H��                            �x */
  /* �x        �x- �o�^                                      �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 3, 6, 61)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 100 + (d.etchics + d.belief) / 50;
    vmsg("�D�`���±z��..�u�O�o�O���U��");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 75 + (d.etchics + d.belief) / 55;
    vmsg("���±z����������..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 50 + (d.etchics + d.belief) / 60;
    vmsg("�u���ܦ��R�߰�..���L���I�p�֪��ˤl..");
  }
  else
  {
    class = 1;
    d.money += 25 + (d.etchics + d.belief) / 65;
    vmsg("�ө^�m����..���]���ॴ�V��..");
  }

  d.love += rand() % 2 + class;
  d.etchics += rand() % 4 + class;
  d.belief += rand() % 4 + class;

  d.sin -= rand() % 9;
  if (d.sin < 0)
    d.sin = 0;

  d.workF++;
  return 0;
}


int
pip_job_workG()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x�a�u    �x+ �ݤH���� �y�O �ͦR �t��                   �x */
  /* �x        �x- �L                                        �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 5, 10, 71)) < 0)
    return 0;

  d.money += 200 + (d.charm + d.speech) * class / 5000;
  vmsg("�\\�a�u�n��ĵ���..:p");

  d.toman += rand() % 2;
  d.charm += rand() % 2;
  d.speed += rand() % 2 + 1;
  d.speech += rand() % 2 + 1;

  d.workG++;
  return 0;
}


int
pip_job_workH()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x����  �x+ �����O                                    �x */
  /* �x        �x- ���                                      �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  if (d.bbtime < 1800 * 1)
  {
    vmsg("�p���Ӥp�F�A�@���H��A�ӧa..");
    return 0;
  }

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 7, 14, 81)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 350 + (d.maxhp + d.attack) / 50;
    vmsg("�z�äO�ܦn��..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 300 + (d.maxhp + d.attack) / 55;
    vmsg("��F���־��..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 250 + (d.maxhp + d.attack) / 60;
    vmsg("�����q�q��..�i�H��n��..");
  }
  else
  {
    class = 1;
    d.money += 200 + (d.maxhp + d.attack) / 65;
    vmsg("�ݥ[�j��..����A�ӧa..");
  }

  d.attack += rand() % 2 + class;

  d.character -= rand() % 5;
  if (d.character < 0)
    d.character = 0;

  d.workH++;
  return 0;
}


int
pip_job_workI()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x���e�|  �x+ ���N �P��                                 �x */
  /* �x        �x- �L                                        �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  if (d.bbtime < 1800 * 1)
  {
    vmsg("�p���Ӥp�F�A�@���H��A�ӧa..");
    return 0;
  }

  class = d.art / 6 - d.tired;
  if ((class = pip_job_function(class, 5, 10, 91)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 400 + (d.art + d.affect) / 50;
    vmsg("�ȤH���ܳ��w���z���y����..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 360 + (d.art + d.affect) / 55;
    vmsg("����������..�ᦳ�ѥ�..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 320 + (d.art + d.affect) / 60;
    vmsg("��������..�A�[�o�@�I..");
  }
  else
  {
    class = 1;
    d.money += 250 + (d.art + d.affect) / 65;
    vmsg("�ݥ[�j��..�H��A�ӧa..");
  }

  d.art += rand() % 3 + class;
  d.affect += rand() % 2 + class;

  d.workI++;
  return 0;
}


int
pip_job_workJ()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x���y��  �x+ �����O �t��                               �x */
  /* �x        �x- ��� �R��                                 �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  if (d.bbtime < 1800 * 2)
  {
    vmsg("�p���Ӥp�F�A�G���H��A�ӧa..");
    return 0;
  }

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 6, 13, 101)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 300 + (d.attack + d.speed) / 50;
    vmsg("�z�O�������y�H..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 270 + (d.attack + d.speed) / 55;
    vmsg("�����٤�����..�i�H���\\�@�y�F..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 240 + (d.attack + d.speed) / 60;
    vmsg("���y�O��O�P���O�����X..");
  }
  else
  {
    class = 1;
    d.money += 210 + (d.attack + d.speed) / 65;
    vmsg("�޳N�t�j�H�N..�A�[�o��..");
  }

  d.attack += rand() % 2 + class;
  d.speed += rand() % 2 + class;

  d.character -= rand() % 5;
  if (d.character < 0)
    d.character = 0;
  d.love -= rand() % 5;
  if (d.love < 0)
    d.love = 0;

  d.workJ++;
  return 0;
}


int
pip_job_workK()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x�u�a    �x+ ���m�O                                    �x */
  /* �x        �x- �y�O                                      �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  if (d.bbtime < 1800 * 2)
  {
    vmsg("�p���Ӥp�F�A�G���H��A�ӧa..");
    return 0;
  }

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 7, 15, 111)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 250 + (d.maxhp + d.resist) / 50;
    vmsg("�u�{�ܧ���..���¤F..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 220 + (d.maxhp + d.resist) / 55;
    vmsg("�u�{�|�ٶ��Q..���W�F..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 200 + (d.maxhp + d.resist) / 60;
    vmsg("�u�{�t�j�H�N..�A�[�o��..");
  }
  else
  {
    class = 1;
    d.money += 160 + (d.maxhp + d.resist) / 65;
    vmsg("��..�ݥ[�j�ݥ[�j..");
  }

  d.resist += rand() % 2 + class;

  d.charm -= rand() % 5;
  if (d.charm < 0)
    d.charm = 0;

  d.workK++;
  return 0;
}


int
pip_job_workL()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x�Ӷ�    �x+ �i�� ���]��O �P��                        �x */
  /* �x        �x- �y�O                                      �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  if (d.bbtime < 1800 * 3)
  {
    vmsg("�p���Ӥp�F�A�T���H��A�ӧa..");
    return 0;
  }

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 4, 8, 121)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 200 + (d.brave + d.affect) / 50;
    vmsg("�u�Ӧ��\\��..�h�¤F");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 150 + (d.brave + d.affect) / 55;
    vmsg("�u���ٺ⦨�\\��..�°�..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 120 + (d.brave + d.affect) / 60;
    vmsg("�u���ٺ�t�j�H�N��..�[�o..");
  }
  else
  {
    class = 1;
    d.money += 80 + (d.brave + d.affect) / 65;
    vmsg("�ڤ]����K��ԣ�F..�ЦA�[�o..");
  }

  d.brave += rand() % 4 + class;
  d.immune += rand() % 3 + class;
  d.affect += class;

  d.charm -= rand() % 5;
  if (d.charm < 0)
    d.charm = 0;

  d.workL++;
  return 0;
}


int
pip_job_workM()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x�a�x�Юv�x+ ���O �ͦR                                 �x */
  /* �x        �x- �L                                        �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  if (d.bbtime < 1800 * 4)
  {
    vmsg("�p���Ӥp�F�A�|���H��A�ӧa..");
    return 0;
  }

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 3, 7, 131)) < 0)
    return 0;

  d.money += 50 + (d.wisdom + d.character) * class / 5000;
  vmsg("�a�л��P..��M���N�֤@�I�o");

  d.wisdom += rand() % 2 + 3;
  d.speech += rand() % 2 + 1;

  d.workM++;
  return 0;
}


int
pip_job_workN()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x�s��    �x+ �y�O �ͦR �i��                            �x */
  /* �x        �x- ���O �������                             �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  if (d.bbtime < 1800 * 5)
  {
    vmsg("�p���Ӥp�F�A�����H��A�ӧa..");
    return 0;
  }

  class = d.charm / 6 - d.tired;
  if ((class = pip_job_function(class, 5, 11, 141)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 500 + (d.charm + d.speech) / 50;
    vmsg("�ܬ���..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 400 + (d.charm + d.speech) / 55;
    vmsg("�Z���w�諸�C..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 300 + (d.charm + d.speech) / 60;
    vmsg("�ܥ��Z��..���������..");
  }
  else
  {
    class = 1;
    d.money += 200 + (d.charm + d.speech) / 65;
    vmsg("�A�O������..�Х[�o..");
  }

  d.charm += rand() % 3 + class;
  d.speech += rand() % 2 + class;
  d.cook += class;

  d.wisdom -= rand() % 5;
  if (d.wisdom < 0)
    d.wisdom = 0;
  d.social -= rand() % 5;
  if (d.social < 0)
    d.social = 0;

  d.workN++;
  return 0;
}


int
pip_job_workO()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x�s�a    �x+ �y�O �o�^                                 �x */
  /* �x        �x- �ݤH���� �D�w �ˤl���Y �H�� �������      �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  if (d.bbtime < 1800 * 5)
  {
    vmsg("�p���Ӥp�F�A�����H��A�ӧa..");
    return 0;
  }

  class = d.charm / 6 - d.tired;
  if ((class = pip_job_function(class, 6, 12, 151)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 600 + (d.charm + d.speech) / 50;
    vmsg("�z�O���������P��..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 500 + (d.charm + d.speech) / 55;
    vmsg("�z�Z���w�諸�C..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 400 + (d.charm + d.speech) / 60;
    vmsg("�ܥ��Z..����������..");
  }
  else
  {
    class = 1;
    d.money += 300 + (d.charm + d.speech) / 65;
    vmsg("��..�A�O������..");
  }

  d.charm += rand() % 4 + class;
  d.sin += rand() % 4 + class;

  d.toman -= rand() % 5;
  if (d.toman < 0)
    d.toman = 0;
  d.etchics -= rand() % 5;
  if (d.etchics < 0)
    d.etchics = 0;
  d.relation -= rand() % 5;
  if (d.relation < 0)
    d.relation = 0;
  d.belief -= rand() % 5;
  if (d.belief < 0)
    d.belief = 0;
  d.social -= rand() % 5;
  if (d.social < 0)
    d.social = 0;

  d.workO++;
  return 0;
}


int
pip_job_workP()
{
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x�]�`�|  �x+ �y�O �ͦR �o�^                            �x */
  /* �x        �x- �ݤH���� ��� �D�w �ˤl���Y �H�� ������� �x */
  /* �u�w�w�w�w�q�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */

  int class;

  if (d.bbtime < 1800 * 6)
  {
    vmsg("�p���Ӥp�F�A�����H��A�ӧa..");
    return 0;
  }

  class = (d.charm + d.art - d.belief) / 6 - d.tired;
  if ((class = pip_job_function(class, 6, 12, 161)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 1000 + (d.charm + d.speech) / 50;
    vmsg("�z�O���]�`�|�̰{�G���P�P��..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 800 + (d.charm + d.speech) / 55;
    vmsg("���..�z�Z���w�諸�C..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 600 + (d.charm + d.speech) / 60;
    vmsg("�n�[�o�F��..��������..");
  }
  else
  {
    class = 1;
    d.money += 400 + (d.charm + d.speech) / 65;
    vmsg("��..�����..");
  }

  d.charm += rand() % 5 + class;
  d.speech += rand() % 2 + class;
  d.sin += rand() % 6 + class;

  d.toman -= rand() % 5;
  if (d.toman < 0)
    d.toman = 0;
  d.character -= rand() % 5;
  if (d.character < 0)
    d.character = 0;
  d.etchics -= rand() % 5;
  if (d.etchics < 0)
    d.etchics = 0;
  d.relation -= rand() % 5;
  if (d.relation < 0)
    d.relation = 0;
  d.belief -= rand() % 5;
  if (d.belief < 0)
    d.belief = 0;
  d.social -= rand() % 5;
  if (d.social < 0)
    d.social = 0;

  d.workP++;
  return 0;
}
#endif		/* HAVE_GAME */
