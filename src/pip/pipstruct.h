/* ----------------------------------------------------- */
/* pip_struct.h     ( NTHU CS MapleBBS Ver 3.10 )        */
/* ----------------------------------------------------- */
/* target : �p�� data structure                          */
/* create :   /  /                                       */
/* update : 01/08/14                                     */
/* author : dsyan.bbs@@forever.twbbs.org                 */  
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/* ----------------------------------------------------- */


#ifndef	_PIP_STRUCT_H_
#define	_PIP_STRUCT_H_


/* ------------------------------------------------------- */
/* �p���ѼƳ]�w                   			   */
/* ------------------------------------------------------- */


struct CHICKEN
{
  /* ---�m�W�Υͤ�--- */
  char name[IDLEN + 1];		/* �m    �W */
  char birth[9];		/* ��    �� */

  /* ---�p�����ɶ�--- */        
  time_t bbtime;		/* ���p�����`�ɶ�(��) */
  				/* itoc.010804: �ثe���]�w�O 30 ��(�Y30*60��) ���@�� */

  /* �H�U���O integer */
  /* �C�@���������O�d�ܤQ��H�ǳ��X�R */
  /* �S�����Ѫ��o�ǳ��O�O�d�A�S���ϥΪ���� */

  /* ---�򥻸��--- */
  int year;			/* �ͤ�  �~ */
  int month;			/* �ͤ�  �� */
  int day;			/* �ͤ�  �� */
  int sex;			/* ��    �O 1:�� 2:��  */
  int death;			/* ��    �A 1:���` 2:�߱� 3:���� */
  int liveagain;		/* �_������ */
  int wantend;			/* 20������ 1:���n�B���B 2:���n�B�w�B  3:���n�B��ĤT�� 4:�n�B���B  5:�n�B�w�B 6:�n�B��ĤT�� */
  int lover;			/* �R�H     0:�S�� 1:�]�� 2:�s�� 3:A 4:B 5:C 6:D 7:E */
  int seeroyalJ;		/* �O�_�i�H�J�W���l/���D  1:�i�H(���l�w�g�^��F) 0:����(���l�٦b��æ) */
  int quest;			/* ��    �� 0:�L���� !=0:���Ƚs�� */

  /* ---���A����--- */
  /* itoc.010730: �o�ǫ��Ʀb�u�@/�ǲ�/�C�������� */
  int relation;			/* �ˤl���Y (�H�M�d�����������Y) */
  int happy;			/* �� �� �� */
  int satisfy;			/* �� �N �� */
  int fallinlove;		/* �ʷR���� */
  int belief;			/* �H    �� */
  int sin;			/* �o    �^ */
  int affect;			/* �P    �� */
  int state7;
  int state8;
  int state9;
  
  /* ---���d����--- */
  /* itoc.010730: �o�ǫ��Ʀb�u�@/�ǲ�/�C�������� */
  int weight;			/* ��    �� */
  int tired;			/* �h �� �� */
  int sick;			/* �f    �� */
  int shit;			/* �M �� �� */
  int body4;
  int body5;
  int body6;
  int winn;			/* �q��Ĺ������ */
  int losee;			/* �q���骺���� */
  int tiee;			/* �q�����⪺���� */

  /* ---�����Ѽ�--- */
  int social;			/* ������� */
  int family;			/* �a�Ƶ��� */
  int hexp;			/* �԰����� */
  int mexp;			/* �]�k���� */
  int value4;
  int value5;
  int value6;
  int value7;
  int value8;
  int value9;

  /* ---��O�Ѽ�--- */
  /* itoc.010730: �o�ǫ��Ʀb�ǲߤ��j�q���ܡA�b�u�@���L�q�վ� */
  int toman;			/* �ݤH���� */
  int character;		/* �� �� �� */
  int love;			/* �R    �� */
  int wisdom;			/* ��    �O */
  int art;			/* ���N��O */
  int etchics;			/* �D    �w */
  int brave;			/* �i    �� */
  int homework;			/* ���a�~�� */
  int charm;			/* �y    �O */
  int manners;			/* §    �� */
  int speech;			/* ��    �R */
  int cook;			/* �i    �� */
  int attack;			/* �� �� �O */
  int resist;			/* �� �m �O */
  int speed;			/* �t    �� */
  int hskill;			/* �԰��޳N */
  int mskill;			/* �]�k�޳N */
  int immune;			/* ���]��O */
  int learn18;
  int learn19;

  /* ---�԰�����--- (�H�ɯŦӼW�[) */
  /* itoc.010730: ���F�W�[�԰������n�ʡAmaxhp maxmp maxvp maxsp
     �o���ݩ����ӥu�b exp �W�[�ɯū�A�~��j�q�W�[ */
  /* itoc.010804: �ثe�e�\�b�Y�Ǫ��p�U maxhp �H 0~3 �I���t�׼W�[ */

  int level;			/* ��    �� */
  int exp;			/* �g �� �� */
  int hp;			/* Health Point �� */
  int maxhp;			/* �̤j�� */
  int mp;			/* Mana Point �k�O */
  int maxmp;			/* �̤j�k�O */
  int vp;			/* moVe Point ���ʤO */
  int maxvp;			/* �̤j���ʤO */
  int sp;			/* Spirit Point ���O */
  int maxsp;			/* �̤j���O */

  /* ---�ҾǷ|�ޯ�--- */	/* bitwise operation */
  usint skillA;			/* �ޯ�: �@�� */
  usint skillB;			/* �ޯ�: ���\ */
  usint skillC;			/* �ޯ�: �ߪk */
  usint skillD;			/* �ޯ�: ���k */
  usint skillE;			/* �ޯ�: �C�k */
  usint skillF;			/* �ޯ�: �M�k */
  usint skillG;			/* �ޯ�: �t���B�r */
  usint skill7;
  usint skill8;
  usint skillXYZ;		/* �S��ޯ� */
  usint spellA;			/* �v���k�N */
  usint spellB;			/* �p�t�k�N */
  usint spellC;			/* �B�t�k�N */
  usint spellD;			/* ���t�k�N */
  usint spellE;			/* �g�t�k�N */
  usint spellF;			/* ���t�k�N */
  usint spellG;			/* �s���k�N */
  usint spell7;
  usint spell8;
  usint spell9;

  /* ---�Z�����Ѽ�--- */
  int weaponhead;		/* �Y���Z�� */
  int weaponhand;		/* �ⳡ�Z�� */
  int weaponshield;		/* �޵P�Z�� */
  int weaponbody;		/* ����Z�� */
  int weaponfoot;		/* �}���Z�� */
  int weapon5;
  int weapon6;
  int weapon7;
  int weapon8;
  int weapon9;

  /* ---�Y���F��--- */
  int food;			/* ��    �� */
  int cookie;			/* �s    �� */  
  int eat2;
  int pill;			/* �j �� �� : �ɦ� */
  int medicine;			/* �F    �� : �ɪk�O */
  int burger;			/* �j �� �Y : �ɲ��ʤO */
  int ginseng;			/* �d�~�H�x : �ɤ��O */
  int paste;			/* �¥��_��I : ����� */
  int snowgrass;		/* �Ѥs���� : �q�q���� */
  int eat9;

  /* ---�֦����F��--- */
  int money;			/* ��    �� */  
  int book;			/* ��    �� */
  int toy;			/* ��    �� */  
  int playboy;			/* �ҥ~Ū�� */
  int thing4;
  int thing5;
  int thing6;
  int thing7;
  int thing8;
  int thing9;  

  /* ---�Ѩ�����-- */
  int royalA;			/* �M �u�� ���n�P */
  int royalB;			/* �M ��� ���n�P */
  int royalC;			/* �M �N�x ���n�P */
  int royalD;			/* �M �j�� ���n�P */
  int royalE;			/* �M ���q ���n�P */
  int royalF;			/* �M �d�m ���n�P */
  int royalG;			/* �M ���m ���n�P */
  int royalH;			/* �M ��� ���n�P */
  int royalI;			/* �M �p�� ���n�P */
  int royalJ;			/* �M ���l/���D ���n�P */

  /* -------�u�@����-------- */
  int workA;			/* �a�� */
  int workB;			/* �O�i */
  int workC;			/* �ȩ� */
  int workD;			/* �A�� */
  int workE;			/* �\�U */
  int workF;			/* �а� */
  int workG;			/* �a�u */
  int workH;			/* ��� */
  int workI;			/* ���v */
  int workJ;			/* �y�H */
  int workK;			/* �u�a */
  int workL;			/* �u�� */
  int workM;			/* �a�� */
  int workN;			/* �s�a */
  int workO;			/* �s�� */
  int workP;			/* �]�`�| */
  int work16;
  int work17;
  int work18;
  int work19;

  /* -------�W�Ҧ���-------- */
  int classA;			/* �۵M��� */
  int classB;			/* ��֧��� */
  int classC;			/* ���ǱШ| */
  int classD;			/* �x�ǱШ| */
  int classE;			/* �C�D�޳N */
  int classF;			/* �氫�ԧ� */
  int classG;			/* �]�k�Ш| */
  int classH;			/* §���Ш| */
  int classI;			/* ø�e�ޥ� */
  int classJ;			/* �R�Чޥ� */
  int class10;
  int class11;
  int class12;
  int class13;
  int class14;
  int class15;
  int class16;
  int class17;
  int class18;
  int class19;

  /* ---�Z�����W��--- */
  char equiphead[11];		/* �Y���Z���W�� */
  char equiphand[11];		/* �ⳡ�Z���W�� */
  char equipshield[11];		/* �޵P�Z���W�� */
  char equipbody[11];		/* ����Z���W�� */
  char equipfoot[11];		/* �}���Z���W�� */
  char equip5[11];
  char equip6[11];
  char equip7[11];
  char equip8[11];
  char equip9[11];
};
typedef struct CHICKEN CHICKEN;


/* ------------------------------------------------------- */
/* ���~�ѼƳ]�w                                            */
/* ------------------------------------------------------- */

struct itemset
{
  int num;			/* �s�� */
  char *name;			/* �W�r */
  char *msgbuy;			/* �\�� */
  char *msguse;			/* ���� */
  int price;			/* ���� */
};
typedef struct itemset itemset;


/* ------------------------------------------------------- */
/* �Ѩ����ڰѼƳ]�w                                        */
/* ------------------------------------------------------- */

struct royalset
{
  char *num;			/* �N�X */
  char *name;			/* ���ڪ��W�r */
  int needmode;			/* �ݭn��mode *//* 0:���ݭn 1:§�� 2:�ͦR */
  int needvalue;		/* �ݭn��value */
  int addtoman;			/* �̤j���W�[�q */
  int maxtoman;			/* �w�s�q */
  char *words1;
  char *words2;
};
typedef struct royalset royalset;


/* ------------------------------------------------------- */
/* �ޯ�ѼƳ]�w                                            */
/* ------------------------------------------------------- */


#if 0		/* itoc.010729.���� */

  smode �n���� struce CHICKEN ���� skill/spell
  smode = +1 �� skillA�Asmode = +2 �� skillB
  smode = -1 �� spellA�Asmode = -2 �� spellB

  sno �O���ޯ઺�s���A�Ҧp 0x01 �O�C�k�ҡA0x02 �O�C�k�A�A0x04 �O�C�k�� (�`�N�O bit operation)
  sbasic �h�O���ޯ઺�򥻧ޯ�A�Y�n���ǲ߼C�k�Ҥ��H��~��ǲ߼C�k�B�A����C�k�B�� sbasic = 0x01 | 0x04 = 0x05

  needhp/nedmp/addtired �Y�O�����A�N�O����/���k�O/�[�h��

#endif


struct skillset
{
  int smode;			/* skill mode  0:�S��  >0:�Z�\  <0:�]�k */
  usint sno;			/* skill number */
  usint sbasic;			/* basic skill */
  char name[13];		/* �ޯ઺�W�r�A����Ӥ���r */
  int needhp;			/* �ͩR�O������ */
  int needmp;			/* �k�O������ */
  int needvp;			/* ���ʤO������ */
  int needsp;			/* ���O������ */
  int addtired;			/* �h�ҭȪ����� */
  int effect;			/* �ĪG/�j�z */
  int pic;			/* ���� */
  char msg[41];			/* �ϥΧޯ઺�����A����20�Ӥ���r */  
};
typedef struct skillset skillset;


/* ------------------------------------------------------- */
/* ��檺�]�w                                              */
/* ------------------------------------------------------- */

struct pipcommands
{
  int (*fptr) ();
  int key;
};
typedef struct pipcommands pipcommands;


/* ------------------------------------------------------- */
/* �Ǫ��ѼƳ]�w                                            */
/* ------------------------------------------------------- */

struct playrule
{
  /* itoc.010731: �Ǫ��u�� hp �o�ئ�A���ݭn mp/vp/sp/tired�A
    �����ޯ�����L���A���O�Ǫ��������Ҧ��O�ѶüƨM�w�A�p���i²��� */

  /* itoc.010731: ���Ŭ� n �Ū��Ǫ��A�����
     maxhp = 0.75*(n^2)   (�M���a�@��)
     attack/spirit/magic/armor/dodge = 10*n
     money = 10*n
     exp = 5*n (��h�W��20���Ǫ��ɤ@��) */
               
  char name[13];		/* �W�r�A����Ӥ���r */
  int attribute;		/* �R�Ϊ������ޯ�  0:�L  >0:�Z�\  <0:�]�k */
  int hp;			/* �� */
  int maxhp;			/* �̤j�� */

  /* ���O�P�Ǫ� ���z/�Z�\/�]�k �����p�������� */
  int attack;			/* ���z������O */
  int spirit;			/* ���O���ơA�ޯ������O */
  int magic;			/* �]�k���ơA�k�N������O */

  /* ���O�P�p�����Ǫ��� �ˮ`/�R���v �t���� */
  int armor;			/* ���@���ơA�Ө���������O */
  int dodge;			/* �{�׫��ơA�{�ק�������O */

  /* �����Ǫ������y */
  int money;			/* �����Ǫ��o�쪺�]�_ */
  int exp;			/* �����Ǫ��o�쪺�g��� */

  int pic;			/* ���� */
};
typedef struct playrule playrule;


/* ------------------------------------------------------- */
/* �Z���ѼƳ]�w                                            */
/* ------------------------------------------------------- */

struct weapon
{
  char name[11];		/* �W�١A����Ӥ���r */
  int quality;			/* �~�� */
  int cost;			/* ���� */
};
typedef struct weapon weapon;


/* ------------------------------------------------------- */
/* PK ��԰ѼƳ]�w                                         */
/* ------------------------------------------------------- */


#define	MAX_PIPPK_USER	10	/* �̦h�P�ɦ� 10 �H�b PK ���� */

struct PTMP
{
  char inuse;			/* 0:���ϥ� 1:�W�իݵo 2:�U�D�Ԯ� -1:�԰��� */
  char done;			/* 0:����� 1:�w��� */ 
  char name[IDLEN + 1];		/* �m�W */
  char userid[IDLEN + 1];	/* �ۤv�� ID */
  char mateid[IDLEN + 1];	/* ��⪺ ID */

  int sex;			/* �ʧO */
  int level;			/* ���� */

  int hp;			/* Health Point �� */
  int maxhp;			/* �̤j�� */
  int mp;			/* Mana Point �k�O */
  int maxmp;			/* �̤j�k�O */
  int vp;			/* moVe Point ���ʤO */
  int maxvp;			/* �̤j���ʤO */
  int sp;			/* Spirit Point ���O */
  int maxsp;			/* �̤j���O */

  int combat;			/* ���z���q: �M�w�u�׷i�B���m�v���j�� */
  int magic;			/* �]�k�y��: �M�w�u�k�N-�U�t�v���j�� */
  int speed;			/* �ӱ��ޥ�: �M�w�u�ޯ�-�@���B�ޯ�-���\�B�ޯ�-�C�k�v���j�� */
  int spirit;			/* ���O�j��: �M�w�u�ޯ�-�ߪk�B�ޯ�-���k�B�ޯ�-�M�k�v���j�� */
  int charm;			/* �ʷP�y�O: �M�w�u�y�b�B�l��v���j�� */
  int oral;			/* �f�Y�a�e: �M�w�u���A�B���ʡv���j�� */
  int cook;			/* �����i��: �M�w�u�ޯ�-�t���B�k�N-�v���v */
};
typedef struct PTMP PTMP;


struct PCACHE
{
  PTMP pslot[MAX_PIPPK_USER];	/* PTMP slots */
};
typedef struct PCACHE PCACHE;

#endif		/* _PIP_STRUCT_H_ */
