/*-------------------------------------------------------*/
/* util/transpip.c                   			 */
/*-------------------------------------------------------*/
/* target : WD �� Maple 3.02 �p������ഫ           	 */
/* create : 02/01/26                     		 */
/* update :   /  /                   			 */
/* author : itoc.bbs@bbs.ee.nctu.edu.tw          	 */
/*-------------------------------------------------------*/
/* syntax : transpip                     		 */
/*-------------------------------------------------------*/


#if 0

   1. �ק� transpip()
   2. �u�� chicken�Achicken.bak* �N����F

   ps. �ϥΫe�Х���ƥ��Ause on ur own risk. �{����H�Х]�[ :p
   ps. �P�� lkchu �� Maple 3.02 for FreeBSD

#endif


#include "windtop.h"

#ifdef HAVE_GAME

#include "../../pip/pipstruct.h"	/* �ޤJ�s�p�����ѼƳ]�w */


/* ----------------------------------------------------- */
/* �¤p���ѼƳ]�w					 */
/* ----------------------------------------------------- */


struct chicken 
{
 /* ---�򥻪����--- */
 char name[20];		/* �m    �W */
 char birth[21];	/* ��    �� */
 int year;		/* �ͤ�  �~ */
 int month;		/* �ͤ�  �� */
 int day; 		/* �ͤ�  �� */
 int sex;		/* ��    �O 1:��   2:��   */
 int death;             /* 1:  ���` 2:�߱� 3:���� */
 int nodone;		/* 1:  ���� */
 int relation;		/* ��H���Y */
 int liveagain;		/* �_������ */
 int dataB;
 int dataC;
 int dataD;
 int dataE;
  
 /* ---���骺�Ѽ�--- */
 int hp;		/* ��    �O */
 int maxhp;             /* �̤j��O */
 int weight;            /* ��    �� */
 int tired;		/* �h �� �� */
 int sick;		/* �f    �� */
 int shit;		/* �M �� �� */ 
 int wrist;		/* ��    �O */
 int bodyA;
 int bodyB;
 int bodyC;
 int bodyD;
 int bodyE;
 
 /* ---�������Ѽ�--- */
 int social;		/* ������� */
 int family;		/* �a�Ƶ��� */
 int hexp;		/* �԰����� */
 int mexp;		/* �]�k���� */
 int tmpA;
 int tmpB;
 int tmpC;
 int tmpD;
 int tmpE;
 
 /* ---�԰��ΰѼ�--- */
 int mp;		/* �k    �O */
 int maxmp;             /* �̤j�k�O */
 int attack;		/* �� �� �O */
 int resist;		/* �� �m �O */
 int speed;		/* �t    �� */
 int hskill;		/* �԰��޳N */
 int mskill;		/* �]�k�޳N */
 int mresist;		/* ���]��O */
 int magicmode;		/* �]�k���A */
 int fightB;
 int fightC;
 int fightD;
 int fightE;
 

 /* ---�Z�����Ѽ�--- */
 int weaponhead;	/* �Y���Z�� */
 int weaponrhand;	/* �k��Z�� */
 int weaponlhand;	/* ����Z�� */
 int weaponbody;	/* ����Z�� */
 int weaponfoot;	/* �}���Z�� */ 
 int weaponA;
 int weaponB;
 int weaponC;
 int weaponD;
 int weaponE;
 
 /* ---�U��O�Ѽ�--- */
 int toman;		/* �ݤH���� */ 
 int character;		/* �� �� �� */ 
 int love;		/* �R    �� */ 
 int wisdom;		/* ��    �z */
 int art;		/* ���N��O */
 int etchics;		/* �D    �w */
 int brave;		/* �i    �� */
 int homework;		/* ���a�~�� */
 int charm;		/* �y	�O */
 int manners;		/* §    �� */
 int speech;		/* ��	�R */
 int cookskill;		/* �i    �� */
 int learnA;
 int learnB;
 int learnC;
 int learnD;
 int learnE;
 
 
 /* ---�U���A�ƭ�--- */
 int happy;		/* �� �� �� */
 int satisfy;		/* �� �N �� */
 int fallinlove;	/* �ʷR���� */
 int belief;		/* �H    �� */
 int offense;		/* �o    �^ */
 int affect;		/* �P    �� */
 int stateA;
 int stateB;
 int stateC;
 int stateD;
 int stateE;
 
 /* ---�Y���F���--- */
 int food;		/* ��    �� */
 int medicine;          /* �F    �� */
 int bighp;             /* �j �� �Y */
 int cookie;		/* �s    �� */
 int ginseng;		/* �d�~�H�x */
 int snowgrass;		/* �Ѥs���� */
 int eatC;
 int eatD;
 int eatE;
 
 /* ---�֦����F��--- */
 int book;		/* ��    �� */
 int playtool; 		/* ��    �� */
 int money;		/* ��    �� */
 int thingA;		
 int thingB;		
 int thingC;
 int thingD;
 int thingE; 
 
 /* ---�q�����Ѽ�--- */
 int winn;		
 int losee;
 
 /* ---�Ѩ�����-- */
 int royalA;		/* from�u�� */
 int royalB;		/* from��� */
 int royalC;		/* from�N�x */
 int royalD;		/* from�j�� */
 int royalE;		/* from���q */ 		
 int royalF;		/* from�d�m */
 int royalG;		/* from���m */
 int royalH;  		/* from��� */
 int royalI;		/* from�p�� */
 int royalJ;		/* from���l */
 int seeroyalJ;		/* �O�_�w�g�ݹL���l�F */
 int seeA;
 int seeB;
 int seeC;
 int seeD;
 int seeE;

 /* ---����---- */
 int wantend;		/* 20������ 1:���n�B���B 2:���n�B�w�B  3:���n�B��ĤT�� 4:�n�B���B  5:�n�B�w�B 6:�n�B��ĤT�� */
 int lover;		/* �R�H 0:�S�� 1:�]�� 2:�s�� 3:A 4:B 5:C 6:D 7:E  */ 
 
 /* -------�u�@����-------- */
 int workA;		/* �a�� */
 int workB;		/* �O�i */
 int workC;		/* �ȩ� */
 int workD;		/* �A�� */
 int workE;		/* �\�U */
 int workF;		/* �а� */
 int workG;		/* �a�u */
 int workH;		/* ��� */
 int workI;		/* ���v */
 int workJ;		/* �y�H */
 int workK;		/* �u�a */
 int workL;		/* �u�� */
 int workM;		/* �a�� */
 int workN;		/* �s�a */
 int workO;		/* �s�� */
 int workP;		/* �]�`�| */
 int workQ;
 int workR;
 int workS;
 int workT;
 int workU;
 int workV;
 int workW;
 int workX;
 int workY;
 int workZ;
 
 /* -------�W�Ҧ���-------- */
 int classA;
 int classB;
 int classC;
 int classD;
 int classE;
 int classF;
 int classG;
 int classH; 
 int classI;
 int classJ;
 int classK;
 int classL;
 int classM;
 int classN;
 int classO;
 
 /* ---�p�����ɶ�--- */
 time_t bbtime;
};
typedef struct chicken chicken;


/* ----------------------------------------------------- */
/* �ഫ�D�{��                                            */
/* ----------------------------------------------------- */


static void
transpip(userid)
  char *userid;
{
  int fd;
  char fpath[64], buf[20];
  FILE *fp;
  struct chicken d;	/* �¤p�� */
  struct CHICKEN p;	/* �s�p�� */
  

  usr_fpath(fpath, userid, "chicken");

  if (fp = fopen(fpath, "r"))
  {
    /* Ū�X�¤p����� */

    fgets(buf, 20, fp);
    d.bbtime = (time_t) atoi(buf);

    fscanf(fp,
      "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
      "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
      "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
      "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
      "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
      "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
      "%d %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
      "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
      "%d %d %d",
      &(d.year), &(d.month), &(d.day), &(d.sex), &(d.death), &(d.nodone), &(d.relation), &(d.liveagain), &(d.dataB), &(d.dataC), &(d.dataD), &(d.dataE),
      &(d.hp), &(d.maxhp), &(d.weight), &(d.tired), &(d.sick), &(d.shit), &(d.wrist), &(d.bodyA), &(d.bodyB), &(d.bodyC), &(d.bodyD), &(d.bodyE),
      &(d.social), &(d.family), &(d.hexp), &(d.mexp), &(d.tmpA), &(d.tmpB), &(d.tmpC), &(d.tmpD), &(d.tmpE),
      &(d.mp), &(d.maxmp), &(d.attack), &(d.resist), &(d.speed), &(d.hskill), &(d.mskill), &(d.mresist), &(d.magicmode), &(d.fightB), &(d.fightC), &(d.fightD), &(d.fightE),
      &(d.weaponhead), &(d.weaponrhand), &(d.weaponlhand), &(d.weaponbody), &(d.weaponfoot), &(d.weaponA), &(d.weaponB), &(d.weaponC), &(d.weaponD), &(d.weaponE),
      &(d.toman), &(d.character), &(d.love), &(d.wisdom), &(d.art), &(d.etchics), &(d.brave), &(d.homework), &(d.charm), &(d.manners), &(d.speech), &(d.cookskill), &(d.learnA), &(d.learnB), &(d.learnC), &(d.learnD), &(d.learnE),
      &(d.happy), &(d.satisfy), &(d.fallinlove), &(d.belief), &(d.offense), &(d.affect), &(d.stateA), &(d.stateB), &(d.stateC), &(d.stateD), &(d.stateE),
      &(d.food), &(d.medicine), &(d.bighp), &(d.cookie), &(d.ginseng), &(d.snowgrass), &(d.eatC), &(d.eatD), &(d.eatE),
      &(d.book), &(d.playtool), &(d.money), &(d.thingA), &(d.thingB), &(d.thingC), &(d.thingD), &(d.thingE),
      &(d.winn), &(d.losee),
      &(d.royalA), &(d.royalB), &(d.royalC), &(d.royalD), &(d.royalE), &(d.royalF), &(d.royalG), &(d.royalH), &(d.royalI), &(d.royalJ), &(d.seeroyalJ), &(d.seeA), &(d.seeB), &(d.seeC), &(d.seeD), &(d.seeE),
      &(d.wantend), &(d.lover), d.name,
      &(d.classA), &(d.classB), &(d.classC), &(d.classD), &(d.classE),
      &(d.classF), &(d.classG), &(d.classH), &(d.classI), &(d.classJ),
      &(d.classK), &(d.classL), &(d.classM), &(d.classN), &(d.classO),
      &(d.workA), &(d.workB), &(d.workC), &(d.workD), &(d.workE),
      &(d.workF), &(d.workG), &(d.workH), &(d.workI), &(d.workJ),
      &(d.workK), &(d.workL), &(d.workM), &(d.workN), &(d.workO),
      &(d.workP), &(d.workQ), &(d.workR), &(d.workS), &(d.workT),
      &(d.workU), &(d.workV), &(d.workW), &(d.workX), &(d.workY), &(d.workZ));

    fclose(fp);

    /* �ഫ�p����� */

    memset(&p, 0, sizeof(p));

    str_ncpy(p.name, d.name, sizeof(p.name));
    sprintf(p.birth, "%02d/%02d/%02d", d.year % 100, d.month, d.day);

    p.bbtime = d.bbtime;

    p.year = d.year;
    p.month = d.month;
    p.day = d.day;
    p.sex = d.sex;
    p.death = d.death;
    p.liveagain = d.liveagain;
    p.wantend = d.wantend;
    p.lover = d.lover;
    p.seeroyalJ = d.seeroyalJ;
    p.quest = 0;		/* �¹q�l���S������ */

    p.relation = d.relation;
    p.happy = d.happy;
    p.satisfy = p.satisfy;
    p.fallinlove = d.fallinlove;
    p.belief = d.belief;
    p.sin = d.offense;
    p.affect = d.affect;

    p.weight = d.weight;
    p.tired = d.tired;
    p.sick = d.sick;
    p.shit = d.shit;

    p.social = d.social;
    p.family = d.family;
    p.hexp = d.hexp;
    p.mexp = d.mexp;

    p.toman = d.toman;
    p.character = d.character;
    p.love = d.love;
    p.wisdom = d.wisdom;
    p.art = d.art;
    p.etchics = d.etchics;
    p.brave = d.brave;
    p.homework = d.homework;
    p.charm = d.charm;
    p.manners = d.manners;
    p.speech = d.speech;
    p.cook = d.cookskill;
    p.attack = d.attack;
    p.resist = d.resist;
    p.speed = d.speed;
    p.hskill = d.hskill;
    p.mskill = d.mskill;
    p.immune = d.mresist;

    p.level = 1;		/* �q 1 �Ŷ}�l */
    p.exp = 0;
    p.hp = d.hp;
    p.maxhp = d.maxhp;
    p.mp = d.mp;
    p.maxmp = d.maxmp;
    p.vp = d.hp;		/* �¹q�l���S�� vp/sp �� hp/mp �ӮM */
    p.maxvp = d.maxhp;
    p.sp = d.mp;
    p.maxsp = d.maxmp;

    /* �¹q�l���S���ޯ�A�w�]�� 0 */

    /* �Z���q�q���m�� 0�A�H�K�Z���C���P */

    p.food = d.food;
    p.cookie = d.cookie;
    p.pill = 0;
    p.medicine = d.medicine;
    p.burger = d.bighp;
    p.ginseng = d.ginseng;
    p.paste = 0;
    p.snowgrass = d.snowgrass;

    p.money = d.money;
    p.book = d.book;
    p.toy = d.playtool;
    p.playboy = 0;

    p.royalA = d.royalA;
    p.royalB = d.royalB;
    p.royalC = d.royalC;
    p.royalD = d.royalD;
    p.royalE = d.royalE;
    p.royalF = d.royalF;
    p.royalG = d.royalG;
    p.royalH = d.royalH;
    p.royalI = d.royalI;
    p.royalJ = d.royalJ;

    p.workA = d.workA;
    p.workB = d.workB;
    p.workC = d.workC;
    p.workD = d.workD;
    p.workE = d.workE;
    p.workF = d.workF;
    p.workG = d.workG;
    p.workH = d.workH;
    p.workI = d.workI;
    p.workJ = d.workJ;
    p.workK = d.workK;
    p.workL = d.workL;
    p.workM = d.workM;
    p.workN = d.workN;
    p.workO = d.workO;
    p.workP = d.workP;

    p.winn = d.winn;
    p.losee = d.losee;
    p.classA = d.classA;
    p.classB = d.classB;
    p.classC = d.classC;
    p.classD = d.classD;
    p.classE = d.classE;
    p.classF = d.classF;
    p.classG = d.classG;
    p.classH = d.classH;
    p.classI = d.classI;
    p.classJ = d.classJ;    
    
    /* �g�J�s�p����� */

    unlink(fpath);		/* ���حӷs�� */
    fd = open(fpath, O_WRONLY | O_CREAT, 0600);
    write(fd, &p, sizeof(CHICKEN));
    close(fd);    
  }
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  char c;
  char buf[64];
  struct dirent *de;
  DIR *dirp;

  /* argc == 1 ������ϥΪ� */
  /* argc == 2 ��Y�S�w�ϥΪ� */

  if (argc > 2)
  {
    printf("Usage: %s [target_user]\n", argv[0]);
    exit(-1);
  }

  chdir(BBSHOME);

  if (argc == 2)
  {
    transpip(argv[1]);
    exit(1);
  }

  /* �ഫ�ϥΪ̤p����� */
  for (c = 'a'; c <= 'z'; c++)
  {
    sprintf(buf, "usr/%c", c);

    if (!(dirp = opendir(buf)))
      continue;

    while (de = readdir(dirp))
    {
      char *str;

      str = de->d_name;
      if (*str <= ' ' || *str == '.')
	continue;

      transpip(str);
    }

    closedir(dirp);
  }
  return 0;
}
#else
int
main()
{
  printf("You should define HAVE_GAME first.\n");
  return -1;
}
#endif	/* HAVE_GAME */
