/*-------------------------------------------------------*/
/* pip_prac.c         ( NTHU CS MapleBBS Ver 3.10 )      */
/*-------------------------------------------------------*/
/* target : �צ���                                     */
/* create :   /  /                                       */
/* update : 01/08/14                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* �צ���:���� �m�Z �צ�     				 */
/*-------------------------------------------------------*/

/*-------------------------------------------------------*/
/* ��Ʈw                      			 	 */
/*-------------------------------------------------------*/

static char *classrank[6] = {"�S��", "���", "����", "����", "�i��", "�M�~"};

static int classmoney[11][2] = 
{
  {0, 0}, {60, 110}, {70, 120}, {70, 120}, {80, 130}, {70, 120},
  {60, 110}, {90, 140}, {70, 120}, {70, 120}, {80, 130}
};

static int classvariable[11][4] = 
{
  {0, 0, 0, 0},
  {5, 5, 4, 4}, {5, 7, 6, 4}, {5, 7, 6, 4}, {5, 6, 5, 4}, {7, 5, 4, 6},
  {7, 5, 4, 6}, {6, 5, 4, 6}, {6, 6, 5, 4}, {5, 5, 4, 7}, {7, 5, 4, 7}
};

static char classword[11][5][41] = 	/* ���G�Q�Ӥ���r */
{
  {"�ҦW", "���\\�@", "���\\�G", "���Ѥ@", "���ѤG"},

  {"�۵M���", "���b�Υ\\Ū�Ѥ�..", "�ڬO�o��������",
  "�o�D���ݤ�����..�ǤF", "�ᤣ���F :~~~~~~"},

  {"��֧���", "�ɫe�����..�ìO�a�W��..", "�����ͫn��..�K�ӵo�X�K..",
  "��..�W�Ҥ��n�y�f��", "�ٲV��..�֭I��֤T�ʭ�"},

  {"���ǱШ|", "���p����  ���p����", "���ڭ̪ﱵ�Ѱ󤧪�",
  "��..�b�F�����H�٤��n�n��", "���ǫ��Y�ª�..�Цn�n��..:("},

  {"�x�ǱШ|", "�]�l�L�k�O����L�k��..", "�q�x����A�ڭn�a�L�h���M",
  "����}�Σ��H�V�ð}�ΡH @_@", "�s�T��ӳ������n�A�ٷQ���M�H"},

  {"�C�D�޳N", "�ݧڪ��F�`..", "�ڨ� �ڨ� �ڨ���..",
  "�C�n��í�@�I��..", "�b��a�����H�C�����@�I"},

  {"�氫�ԧ�", "�٦׬O�٦�  �I�I..", "�Q�K�ɤH���..",
  "�}�A�𰪤@�I��..", "���Y���o��S�O��.."},

  {"�]�k�Ш|", "���� ���� ��������..", "�D�x�����i���Ϲ���������סH�H",
  "�p�߱������n�ô�..", "����f�����n�y������y�W.."},

  {"§���Ш|", "�n����§������..", "�ڶ٭�..��������..",
  "���Ǥ��|���H�ѧr..", "���_���ӨS����..�ѣ�.."},

  {"ø�e�ޥ�", "�ܤ�����..�����N�ѥ�..", "�o�T�e���C��f�t���ܦn..",
  "���n���e�Ű�..�n�[�o..", "���n�r�e����..�a�a�p����.."},

  {"�R�Чޥ�", "���o�N���@�����Z��..", "�R�вӭM�ܦn��..",
  "����A�X�n�@�I..", "���U���n�o��ʾ|.."}
};


/*-------------------------------------------------------*/
/* �禡�w                                                */
/*-------------------------------------------------------*/


static int
pip_practice_gradeup(classnum, classgrade, newgrade)	/* �צ浥�Ŵ��� */
  int classnum;		/* �Ҹ� */
  int classgrade;	/* �~�� */
  int newgrade;		/* �s�~�� */
{
  /* itoc.0108802: ���٭p��Anewgrade �q 0 �}�l��Aclassgrade �q 1 �}�l�� */
  if (newgrade >= classgrade && newgrade < 5)
  {
    char buf[80];
    sprintf(buf, "�U�����W [%8s%4s�ҵ{]", classword[classnum][0], classrank[newgrade + 1]);
    vmsg(buf);
  }
  return 0;
}


/* �ǤJ:�Ҹ� ���� �ͩR �ּ� ���� żż �Ǧ^:�ܼ�12345   �Ǧ^: -1:��� 0:���� 1:���\ */
static int
pip_practice_function(classnum, classgrade, pic1, pic2, change1, change2, change3, change4, change5)
  int classnum;			/* �צ���� */
  int classgrade;		/* �צ浥�� */
  int pic1, pic2;		/* ���� */
  int *change1;			/* �D�n�ݩʼW�[ */
  int *change2;			/* ���n�ݩʼW�[ */
  int *change3;			/* ���[�ݩʼW�[ */
  int *change4;			/* �۫g�ݩʴ�� */
  int *change5;			/* �ۥ��ݩʴ�� */
{
  int grade, success;
  char buf[80];

  /* itoc.010803: �ˬd classgrade�A�קK�N�~ */
  /* �]���٨S update�Alearn_skill �i�� < 0 */
  if (LEARN_LEVEL < 0)
  {
    vmsg("�z�w�g�֨��z�F");
    return -1;
  }

  /* itoc.010803: classgrade ���ӥu�q 1~5 �� */
  if (classgrade < 0)
    grade = 1;
  else if (classgrade > 5)
    grade = 5;
  else
    grade = classgrade;

  /* ������k */
  success = grade * classmoney[classnum][0] + classmoney[classnum][1];	/* �ɥ� success */
  sprintf(buf, "  [%8s%4s�ҵ{]�n�� %d���A�T�w�n��(Y/N)�H[Y] ", classword[classnum][0], classrank[grade], success);

  if (ians(b_lines - 2, 0, buf) == 'n')
    return -1;
  if (d.money < success)
  {
    vmsg("�ܩ�p�A�z����������");
    return -1;
  }
  count_tired(4, 5, 1, 100, 1);
  d.money -= success;

  /* ���\�P�_���P�_ */
  success = (d.hp / 2 + rand() % 20 > d.tired);		/* 1: ���\   0: ���� */

  d.hp -= rand() % 5 + classvariable[classnum][0];
  d.happy -= rand() % 5 + classvariable[classnum][1];
  d.satisfy -= rand() % 5 + classvariable[classnum][2];
  d.shit += rand() % 5 + classvariable[classnum][3];

  /* �[���I�Ʀ��\�O���Ѫ� 1.5 ���A�����I�ƥ��ѬO���\�� 1.5 �� */
  /* learn_skill �i�q 2%~100% */
  *change1 = (7 + 6 * (rand() % grade)) * 2 * LEARN_LEVEL / (3 - success);	/* �D�n�ݩʥ[����� 3*classgrade+4 (�Y�צ榨�\�B���] learn_level = 100%) */
  *change2 = (5 + 4 * (rand() % grade)) * 2 * LEARN_LEVEL / (3 - success);	/* ���n�ݩʥ[����� 2*classgrade+3 (�Y�צ榨�\�B���] learn_level = 100%) */
  *change3 = (3 + 2 * (rand() % grade)) * 2 * LEARN_LEVEL / (3 - success);	/* ���[�ݩʥ[����� classgrade+2   (�Y�צ榨�\�B���] learn_level = 100%) */

  *change4 = (5 + rand() % grade) * 2 / (1 + success);			/* �۫g�ݩʦ������ classgrade/2+4.5 (�Y�צ榨�\) */
  *change5 = (5 + rand() % grade) * 2 / (2 + success);			/* �ۥ��ݩʦ������ classgrade/3+3 (�Y�צ榨�\) */

  /* �üƿ�@�ӹϨӨq */
  if (rand() % 2)
    show_practice_pic(pic1);
  else
    show_practice_pic(pic2);

  vmsg(classword[classnum][3 - 2 * success + rand() % 2]);	/* �Ĥ@�G�ӬO���\�T���A�T�|�ӬO���ѰT�� */
  return success;
}


/*-------------------------------------------------------*/
/* �צ���:���� �m�Z �צ�     				 */
/*-------------------------------------------------------*/


/* itoc.010802: �U�� classgrage ���ɩw�O�p�����Y���ݩ� / 200�A��M�]�i�H�[�v�B�z */

int
pip_practice_classA()
{
  /* �z�w�w�w�w�s�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{ */
  /* �x�۵M��Ǣx���ݩʡG���O�B���]�B�q��          �x */
  /* �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x        �x�t�ݩʡG�H���B�q��                �x */
  /* �|�w�w�w�w�r�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�} */

  int class;
  int change1, change2, change3, change4, change5;

  class = (d.wisdom * 3 + d.immune * 2) / 1000 + 1;	/* ��� */

  if (pip_practice_function(1, class, 11, 12, &change1, &change2, &change3, &change4, &change5) < 0)
    return 0;

  d.wisdom += change1;
  d.immune += change2;
  d.belief -= change4;
  d.classA++;

  if (d.belief < 0)
    d.belief = 0;

  /* itoc.010802: �üƾǷ|�s���k�N */
  if (rand() % 30 == 0)
    pip_learn_skill(-7);

  pip_practice_gradeup(1, class, (d.wisdom * 3 + d.immune * 2) / 1000);
  return 0;
}


int
pip_practice_classB()
{
  /* �z�w�w�w�w�s�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{ */
  /* �x  �ֵ�  �x���ݩʡG�P���B���B���N          �x */
  /* �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x        �x�t�ݩʡG�q�ʡB���]                �x */
  /* �|�w�w�w�w�r�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�} */

  int class;
  int change1, change2, change3, change4, change5;

  class = (d.affect * 3 + d.character * 2 + d.art) / 1200 + 1;	/* �ֵ� */

  if (pip_practice_function(2, class, 21, 22, &change1, &change2, &change3, &change4, &change5) < 0)
    return 0;

  d.affect += change1;
  d.character += change2;
  d.art += change3;
  d.immune -= change5;
  d.classB++;

  if (d.immune < 0)
    d.immune = 0;

  /* itoc.010814: �üƾǷ|�ߪk */
  if (rand() % 10 == 0)
    pip_learn_skill(3);

  pip_practice_gradeup(2, class, (d.affect * 3 + d.character * 2 + d.art) / 1200);
  return 0;
}


int
pip_practice_classC()
{
  /* �z�w�w�w�w�s�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{ */
  /* �x  ����  �x���ݩʡG�H���B���]�B���O          �x */
  /* �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x        �x�t�ݩʡG�����B�q��                �x */
  /* �|�w�w�w�w�r�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�} */
  
  int class;
  int change1, change2, change3, change4, change5;

  class = (d.belief * 3 + d.immune * 2 + d.wisdom) / 1200 + 1;	/* ���� */

  if (pip_practice_function(3, class, 31, 32, &change1, &change2, &change3, &change4, &change5) < 0)
    return 0;

  d.belief += change1;
  d.immune += change2;
  d.wisdom += change3;
  d.attack -= change4;
  d.classC++;

  if (d.attack < 0)
    d.attack = 0;

  /* itoc.010802: �üƾǷ|�v���k�N */
  if (rand() % 10 == 0)
    pip_learn_skill(-1);

  pip_practice_gradeup(3, class, (d.belief * 3 + d.immune * 2 + d.wisdom) / 1200);
  return 0;
}


int
pip_practice_classD()
{
  /* �z�w�w�w�w�s�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{ */
  /* �x  �x��  �x���ݩʡG�԰��޳N�B���O�B�q��      �x */
  /* �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x        �x�t�ݩʡG�P���B�q��                �x */
  /* �|�w�w�w�w�r�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�} */

  int class;
  int change1, change2, change3, change4, change5;

  class = (d.hskill * 3 + d.wisdom * 2) / 1000 + 1;

  if (pip_practice_function(4, class, 41, 42, &change1, &change2, &change3, &change4, &change5) < 0)
    return 0;

  d.hskill += change1;
  d.wisdom += change2;
  d.affect -= change4;
  d.classD++;

  if (d.affect < 0)
    d.affect = 0;

  /* itoc.010814: �üƾǷ|�@�� */
  if (rand() % 10 == 0)
    pip_learn_skill(1);

  pip_practice_gradeup(4, class, (d.hskill * 3 + d.wisdom * 2) / 1000);
  return 0;
}


int
pip_practice_classE()
{
  /* �z�w�w�w�w�s�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{ */
  /* �x  �C�N  �x���ݩʡG�����B�԰��޳N�B���m      �x */
  /* �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x        �x�t�ݩʡG�P���B�q��                �x */
  /* �|�w�w�w�w�r�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�} */

  int class;
  int change1, change2, change3, change4, change5;

  class = (d.attack * 3 + d.hskill * 2 + d.resist) / 1200 + 1;

  if (pip_practice_function(5, class, 51, 52, &change1, &change2, &change3, &change4, &change5) < 0)
    return 0;

  d.attack += change1;
  d.hskill += change2;
  d.resist += change3;
  d.affect -= change4;
  d.classE++;

  if (d.affect < 0)
    d.affect = 0;

  /* itoc.010802: �üƾǷ|�C�k */
  if (rand() % 10 == 0)
    pip_learn_skill(5);

  pip_practice_gradeup(5, class, (d.attack * 3 + d.hskill * 2 + d.resist) / 1200);
  return 0;
}


int
pip_practice_classF()
{
  /* �z�w�w�w�w�s�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{ */
  /* �x  �氫  �x���ݩʡG���m�B�t�סB����          �x */
  /* �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x        �x�t�ݩʡG�P���B�q��                �x */
  /* �|�w�w�w�w�r�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�} */

  int class;
  int change1, change2, change3, change4, change5;

  class = (d.resist * 3 + d.speed * 2 + d.attack) / 1200 + 1;

  if (pip_practice_function(6, class, 61, 62, &change1, &change2, &change3, &change4, &change5) < 0)
    return 0;

  d.resist += change1;
  d.speed += change2;
  d.attack += change3;
  d.affect -= change4;
  d.classF++;

  if (d.affect < 0)
    d.affect = 0;

  /* itoc.010802: �üƾǷ|���k */
  if (rand() % 10 == 0)
    pip_learn_skill(4);

  pip_practice_gradeup(6, class, (d.resist * 3 + d.speed * 2 + d.attack) / 1200);
  return 0;
}


int
pip_practice_classG()
{
  /* �z�w�w�w�w�s�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{ */
  /* �x  �]�k  �x���ݩʡG�]�k�޳N�B���]�B�q��      �x */
  /* �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x        �x�t�ݩʡG�����B�t��                �x */
  /* �|�w�w�w�w�r�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�} */

  int class;
  int change1, change2, change3, change4, change5;

  class = (d.mskill * 3 + d.immune * 2) / 1000 + 1;

  if (pip_practice_function(7, class, 71, 72, &change1, &change2, &change3, &change4, &change5) < 0)
    return 0;

  d.mskill += change1;
  d.immune += change2;
  d.attack -= change4;
  d.speed -= change5;
  d.classG++;

  if (d.attack < 0)
    d.attack = 0;
  if (d.speed < 0)
    d.speed = 0;

  /* itoc.010802: �üƾǷ|���t�]�k���@ */
  if (rand() % 7 == 0)
    pip_learn_skill(- 2 - rand() % 5);

  pip_practice_gradeup(7, class, (d.mskill * 3 + d.immune * 2) / 1000);
  return 0;
}


int
pip_practice_classH()
{
  /* �z�w�w�w�w�s�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{ */
  /* �x  §��  �x���ݩʡG§���B���B�ͦR          �x */
  /* �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x        �x�t�ݩʡG�t�סB�q��                �x */
  /* �|�w�w�w�w�r�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�} */

  int class;
  int change1, change2, change3, change4, change5;

  class = (d.manners * 3 + d.character * 2 + d.speech) / 1200 + 1;

  if (pip_practice_function(8, class, 81, 82, &change1, &change2, &change3, &change4, &change5) < 0)
    return 0;

  d.manners += change1;
  d.character += change2;
  d.speech += change3;
  d.speed -= change4;
  d.classH++;

  if (d.speed < 0)
    d.speed = 0;

  pip_practice_gradeup(8, class, (d.manners * 3 + d.character * 2 + d.speech) / 1200);
  return 0;
}


int
pip_practice_classI()
{
  /* �z�w�w�w�w�s�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{ */
  /* �x  ø�e  �x���ݩʡG���N�B�P���B�q��          �x */
  /* �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x        �x�t�ݩʡG�q�ʡB�q��                �x */
  /* �|�w�w�w�w�r�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�} */

  int class;
  int change1, change2, change3, change4, change5;

  class = (d.art * 3 + d.character * 2) / 1000 + 1;

  if (pip_practice_function(9, class, 91, 92, &change1, &change2, &change3, &change4, &change5) < 0)
    return 0;

  d.art += change1;
  d.character += change2;
  d.classI++;

  /* itoc.010814: �üƾǷ|�M�k */
  if (rand() % 10 == 0)
    pip_learn_skill(6);

  pip_practice_gradeup(9, class, (d.art * 3 + d.character * 2) / 1000);
  return 0;
}


int
pip_practice_classJ()
{
  /* �z�w�w�w�w�s�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{ */
  /* �x  �R��  �x���ݩʡG���N�B�y�O�B���          �x */
  /* �x        �u�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�t */
  /* �x        �x�t�ݩʡG�����B�]�k�޳N            �x */
  /* �|�w�w�w�w�r�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�} */

  int class;
  int change1, change2, change3, change4, change5;

  class = (d.art * 3 + d.charm * 2 + d.character) / 1200 + 1;

  if (pip_practice_function(10, class, 101, 102, &change1, &change2, &change3, &change4, &change5) < 0)
    return 0;

  d.art += change1;
  d.charm += change2;
  d.character += change3;
  d.attack -= change4;
  d.mskill -= change5;
  d.classJ++;

  if (d.attack < 0)
    d.attack = 0;
  if (d.mskill < 0)
    d.mskill = 0;

  /* itoc.010802: �üƾǷ|���\ */
  if (rand() % 10 == 0)
    pip_learn_skill(2);

  pip_practice_gradeup(10, class, (d.art * 3 + d.charm * 2 + d.character) / 1200);
  return 0;
}
#endif		/* HAVE_GAME */
