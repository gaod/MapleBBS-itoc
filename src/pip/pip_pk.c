/* ----------------------------------------------------- */
/* pip_pk.c	( NTHU CS MapleBBS Ver 3.10 )      	 */
/* ----------------------------------------------------- */
/* target : PK ��Կ��                                  */
/* create : 02/02/17                                     */
/* update :   /  /		  			 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/* ----------------------------------------------------- */


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


#if 0

  0. ���k�O�i�J�q�l���C���H��� PK�A�M��@�H�� 1)�W�իݵo�A
     �t�@�H�� 2)�U�D�Ԯѿ�J�e�̪� ID �Y�i�}�l��ԡC

  1. PK ���ɭԥΪ��ȬO ptmp-> �̭����A�ҥH��ԧ� d. �ä��|���ܡC
  2. ���W�[����A��Ԫ������覡�� pip_fight.c ���h�ˡC
  3. ��Ԯɤ��i�Y�ɫ~�C (�i��)
  4. ��Ԯ� skillXYZ �S���S�O�ĥΡC (�i��)

  5. pip_pk_skill() �]�O���i���k�A�Q�諸�H�A�ۤv�q pip_fight.c �۹L�ӡC

#endif

/*-------------------------------------------------------*/
/* pip PK cache                                          */
/*-------------------------------------------------------*/


static PCACHE *pshm;
static PTMP *cp;		/* �ڪ��p�� */
static PTMP *ep;		/* ��⪺�p�� */


static void
pip_pkshm_init()
{
  pshm = shm_new(PIPSHM_KEY, sizeof(PCACHE));
}


static int
pip_ptmp_new(pp)
  PTMP *pp;
{
  PTMP *pentp, *ptail;

  /* --------------------------------------------------- */
  /* semaphore : critical section                        */
  /* --------------------------------------------------- */

#ifdef	HAVE_SEM
  sem_lock(BSEM_ENTER);
#endif

  pentp = pshm->pslot;
  ptail = pentp + MAX_PIPPK_USER;

  do
  {
    if (!pentp->inuse)		/* ���@�ӪŦ�l�Acp-> ���V */
    {
      memcpy(pentp, pp, sizeof(PTMP));
      cp = pentp;

#ifdef	HAVE_SEM
      sem_lock(BSEM_LEAVE);
#endif

      return 1;
    }
  } while (++pentp < ptail);

  /* Thor:�i�Duser���H�n���@�B�F */

#ifdef	HAVE_SEM
  sem_lock(BSEM_LEAVE);
#endif

  return 0;
}


static int
pip_ptmp_setup()
{
  PTMP ptmp;

  memset(&ptmp, 0, sizeof(PTMP));

  /* ���ݩ� */
  strcpy(ptmp.name, d.name);
  strcpy(ptmp.userid, cuser.userid);

  ptmp.sex = d.sex;
  ptmp.level = d.level;

  /* ��ɺ� */
  ptmp.hp = d.maxhp;
  ptmp.mp = d.maxmp;
  ptmp.vp = d.maxvp;
  ptmp.sp = d.maxsp;
  ptmp.maxhp = d.maxhp;
  ptmp.maxmp = d.maxmp;
  ptmp.maxvp = d.maxvp;
  ptmp.maxsp = d.maxsp;

  /* �U�ا�����O */
  ptmp.combat = d.attack + (d.resist >> 2);	/* ���z���q: �M�w�u�׷i�B���m�v���j�� */
  ptmp.magic = d.immune + (d.mskill >> 2);	/* �]�k�y��: �M�w�u�k�N-�U�t�v���j�� */
  ptmp.speed = d.speed + (d.hskill >> 2);	/* �ӱ��ޥ�: �M�w�u�ޯ�-�@���B�ޯ�-���\�B�ޯ�-�C�k�v���j�� */
  ptmp.spirit = d.brave + (d.etchics >> 2);	/* ���O�j��: �M�w�u�ޯ�-�ߪk�B�ޯ�-���k�B�ޯ�-�M�k�v���j�� */
  ptmp.charm = d.charm + (d.art >> 2);		/* �ʷP�y�O: �M�w�u�y�b�B�l��v���j�� */
  ptmp.oral = d.speech + (d.manners >> 2);	/* �f�Y�a�e: �M�w�u���A�B���ʡv���j�� */
  ptmp.cook = d.cook + (d.homework >> 2);	/* �����i��: �M�w�u�ޯ�-�t���B�k�N-�v���v */

  if (!pip_ptmp_new(&ptmp))
  {
    vmsg("��p�APK ���Ⱥ��F��");
    return 0;
  }

  return 1;
}


static void
pip_ptmp_free()
{
  if (!cp || !cp->inuse)
    return;

#ifdef  HAVE_SEM
  sem_lock(BSEM_ENTER);
#endif

  cp->inuse = 0;

#ifdef  HAVE_SEM
  sem_lock(BSEM_LEAVE);
#endif
}


static PTMP *
pip_ptmp_get(userid, inuse)
  char *userid;
  int inuse;		/* 1:��u�W�իݵo�v���H�ӬD��  2:��u�U�D�Ԯѡv���D�Ԫ̦^�� */
{
  PTMP *pentp, *ptail;

  pentp = pshm->pslot;
  ptail = pentp + MAX_PIPPK_USER;
  do
  {
    if (pentp->inuse == inuse && !str_cmp(pentp->userid, userid))
      return pentp;
  } while (++pentp < ptail);

  return NULL;
}


static void
pip_ptmp_show()
{
  int max;
  PTMP *pentp, *ptail;

  clrfromto(7, 16);
  move(10, 5);

  max = 0;
  pentp = pshm->pslot;
  ptail = pentp + MAX_PIPPK_USER;
  do
  {
    if (pentp->inuse)
    {
      max++;
      prints("\033[1;3%dm%-20s\033[m", 2 + pentp->inuse, pentp->userid);

      if (max % 4 == 0)
        move(10 + max % 4, 5);
    }
  } while (++pentp < ptail);

  move(8, 5);
  prints("\033[1;31m�԰���  \033[33m�W�իݵo  \033[34m�D�Ԫ̵��ݦ^��\033[m"
    "  �ثe���l�̦� \033[1;36m%d/%d\033[m ����", max, MAX_PIPPK_USER);
}


/*-------------------------------------------------------*/
/* ��ԥD�禡                                            */
/*-------------------------------------------------------*/


  /*-----------------------------------------------------*/
  /* ���y����                                            */
  /*-----------------------------------------------------*/


static void
pip_pk_turn()	/* ����� */
{
  cp->done = 1;
  ep->done = 0;
}


  /*-----------------------------------------------------*/
  /* �e�����                                            */
  /*-----------------------------------------------------*/


static void
pip_pk_showfoot()
{
  out_cmd("", COLOR1 " �԰��R�O " COLOR2 " [1]�׷i [2]�ޯ� [3]�y�b [4]�l�� [5]���A [6]���� [Q]�{��            \033[m");
}


static void
pip_pk_showing()
{
  int pic;
  char inbuf1[20], inbuf2[20], inbuf3[20], inbuf4[20];
  
  clear();
  move(0, 0);

  prints("\033[1;41m  " BBSNAME PIPNAME " ��\033[32m%s\033[37m%-13s                                            \033[m\n", 
    cp->sex == 1 ? "��" : (cp->sex == 2 ? "��" : "�H"), cp->name);

  /* �ù��W��q�X�ڪ��p����� */

  sprintf(inbuf1, "%d%s/%d%s", cp->hp > 1000 ? cp->hp / 1000 : cp->hp,
    cp->hp > 1000 ? "K" : "", cp->maxhp > 1000 ? cp->maxhp / 1000 : cp->maxhp,
    cp->maxhp > 1000 ? "K" : "");
  sprintf(inbuf2, "%d%s/%d%s", cp->mp > 1000 ? cp->mp / 1000 : cp->mp,
    cp->mp > 1000 ? "K" : "", cp->maxmp > 1000 ? cp->maxmp / 1000 : cp->maxmp,
    cp->maxmp > 1000 ? "K" : "");
  sprintf(inbuf3, "%d%s/%d%s", cp->vp > 1000 ? cp->vp / 1000 : cp->vp,
    cp->vp > 1000 ? "K" : "", cp->maxvp > 1000 ? cp->maxvp / 1000 : cp->maxvp,
    cp->maxvp > 1000 ? "K" : "");
  sprintf(inbuf4, "%d%s/%d%s", cp->sp > 1000 ? cp->sp / 1000 : cp->sp,
    cp->sp > 1000 ? "K" : "", cp->maxsp > 1000 ? cp->maxsp / 1000 : cp->maxsp,
    cp->maxsp > 1000 ? "K" : "");

  outs("\033[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m\n");
  prints("\033[1;31m�x\033[33m��  �R:\033[37m%-12s\033[33m�k  �O:\033[37m%-12s\033[33m���ʤO:\033[37m%-12s\033[33m��  �O:\033[37m%-12s\033[31m�x\033[m\n", inbuf1, inbuf2, inbuf3, inbuf4);
  prints("\033[1;31m�x\033[33m��  ��:\033[37m%-12d\033[33m�]  �k:\033[37m%-12d\033[33m��  ��:\033[37m%-12d\033[33m�Z  �N:\033[37m%-12d\033[31m�x\033[m\n", cp->combat, cp->magic, cp->speed, cp->spirit);
  prints("\033[1;31m�x\033[33m�y  �O:\033[37m%-12d\033[33m�f  �~:\033[37m%-12d\033[33m�i  ��:\033[37m%-12d\033[33m��  ��:\033[37m%-12d\033[31m�x\033[m\n", cp->charm, cp->oral, cp->cook, cp->level);
  outs("\033[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");

  /* �ù����� 7~16 �C �q�X�Ǫ������� */
  pic = 101 + 100 * (rand() % 5) + rand() % 3;    /* 101~501 102~502 103~503 �Q����@ */
  show_badman_pic(pic);

  /* �ù��U��q�X��誺�p����� */

  sprintf(inbuf1, "%d%s/%d%s", ep->hp > 1000 ? ep->hp / 1000 : ep->hp,
    ep->hp > 1000 ? "K" : "", ep->maxhp > 1000 ? ep->maxhp / 1000 : ep->maxhp,
    ep->maxhp > 1000 ? "K" : "");
  sprintf(inbuf2, "%d%s/%d%s", ep->mp > 1000 ? ep->mp / 1000 : ep->mp,
    ep->mp > 1000 ? "K" : "", ep->maxmp > 1000 ? ep->maxmp / 1000 : ep->maxmp,
    ep->maxmp > 1000 ? "K" : "");
  sprintf(inbuf3, "%d%s/%d%s", ep->vp > 1000 ? ep->vp / 1000 : ep->vp,
    ep->vp > 1000 ? "K" : "", ep->maxvp > 1000 ? ep->maxvp / 1000 : ep->maxvp,
    ep->maxvp > 1000 ? "K" : "");
  sprintf(inbuf4, "%d%s/%d%s", ep->sp > 1000 ? ep->sp / 1000 : ep->sp,
    ep->sp > 1000 ? "K" : "", ep->maxsp > 1000 ? ep->maxsp / 1000 : ep->maxsp,
    ep->maxsp > 1000 ? "K" : "");

  move(18, 0);
  outs("\033[1;34m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m\n");
  prints("\033[1;34m�x\033[32m�m  �W:\033[37m%-12s\033[32m��  ��:\033[37m%-12s\033[32m��  �O:\033[37m%-12s\033[32m��  ��:\033[37m%-12d\033[34m�x\033[m\n", ep->name, ep->userid, ep->sex == 1 ? "��" : (ep->sex == 2 ? "��" : "�H"), ep->level);
  prints("\033[1;34m�x\033[32m��  �R:\033[37m%-12s\033[32m�k  �O:\033[37m%-12s\033[32m���ʤO:\033[37m%-12s\033[32m��  �O:\033[37m%-12s\033[34m�x\033[m\n", inbuf1, inbuf2, inbuf3, inbuf4);
  outs("\033[1;34m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m\n");

  pip_pk_showfoot(); 
}


static void
pip_pk_ending()
{
  clrfromto(7, 16);
  move(8, 0);

  if (cp->hp > 0)
  {
    outs("           \033[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m\n");
    prints("           \033[1;31m�x \033[37m�Z�N�j�|���p��\033[33m%-13s                \033[31m�x\033[m\n", cp->name);
    prints("           \033[1;31m�x \033[37m���ѤF�j�l�����\033[32m%-13s              \033[31m�x\033[m\n", ep->name);
    outs("           \033[1;31m�x \033[37m�i���M�g�糣�W�ɤF����                     \033[31m�x\033[m\n");
    outs("           \033[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");
    vmsg("�z���ѤF�@�ӱj�w���å�");
    d.exp += ep->level;
  }
  else
  {
    outs("           \033[1;31m�z�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�{\033[m\n");
    prints("           \033[1;31m�x \033[37m�Z�N�j�|���p��\033[33m%-13s                \033[31m�x\033[m\n", cp->name);
    prints("           \033[1;31m�x \033[37m�Q\033[32m%-13s\033[37m��⥴�o����y��            \033[31m�x\033[m\n", ep->name);
    outs("           \033[1;31m�x \033[37m�M�w�^�a�n�n�A��m                         \033[31m�x\033[m\n");
    outs("           \033[1;31m�|�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�}\033[m");
    vmsg("�Q���Ѫ��z�ߤ��۷��O���D");
    d.exp -= cp->level;
  }
}


  /*-----------------------------------------------------*/
  /* ��ʨ禡                                            */
  /*-----------------------------------------------------*/


static void
pip_pk_combat()		/* �׷i */
{
  int injure;

  injure = cp->combat - (ep->combat >> 2);
  if (injure > 0)
  {
    ep->hp -= injure;
    show_fight_pic(1);
    vmsg("�z�y���F��誺�ˮ`");
  }
  else
  {
    show_fight_pic(2);
    vmsg("�z�������b��貴��²���O�k�o");
  }
  pip_pk_turn();  
}


static void 
pip_pk_skill()		/* �ޯ�: �Z�\/�]�k */
{
  /* itoc.020217: �i�o�g�� pip_fight.c �̭����ت��F�A�����쪺�H�ۤv�۵ۧ� :p */

  /* �ޯण���I�A���ĪG����t(�u����L����b) */

  int ch, class;
  int injure[5] = {125, 200, 300, 450, 750};

  out_cmd(COLOR1 " �Z�\\��� " COLOR2 " [1]�@�� [2]���\\ [3]�ߪk [4]���k [5]�C�k [6]�M�k [7]�t�� [Q]���    \033[m",
    COLOR1 " �k�N��� " COLOR2 " [A]�v�� [B]�p�t [C]�B�t [D]���t [E]�g�t [F]���t [G]�S�� [Q]���    \033[m");

  for (;;)
  {
    ch = vkey();

    if (ch == 'q')
    {
      pip_pk_showfoot();
      return;
    }

    else if (ch == '1')		/* �@�� */
    {
      class = rand() % 10;
      if (class == 0)		/* 10% �����v�ϼu�����A�i�A�ק��� */
      {
        ep->hp -= cp->speed >> 3;
        vmsg("��⪺�����ϼu�^�L�����W");
      }
      else if (class <= 2)	/* 20% �����v�Ϲ�誫�z�����û����C�A�i�A�ק��� */
      {
        ep->combat = ep->combat * 4 / 5;
        vmsg("��誺����F�A�ݨӵu�ɶ������|��_");
      }
      else			/* 70% �����v���򳣨S�� */
      {
        vmsg("�z�Ĩ����m���I");
        break;
      }
      pip_pk_showfoot();
      return;
    }

    else if (ch == '2')		/* ���\ */
    {
      class = cp->speed >> 9;
      if (class > 4)
        class = 4;

      cp->vp += injure[class];	/* �� vp �]�ɥ� injure[] */      
      if (cp->vp > cp->maxvp)
        cp->vp = cp->maxvp;

      vmsg("�믫�����A�ǳƦA��");
      break;
    }

    else if (ch == '3')		/* �ߪk */
    {
      class = cp->spirit >> 9;
      if (class > 4)
        class = 4;

      cp->sp += injure[class];	/* �� sp �]�ɥ� injure[] */      
      if (cp->sp > cp->maxsp)
        cp->sp = cp->maxsp;

      vmsg("���O�R�K�A�ǳƦA��");
      break;
    }

    else if (ch == '4')		/* ���k */
    {
      class = cp->spirit >> 9;
      if (class > 4)
        class = 4;

      ep->hp -= injure[class] * (75 + rand() % 50) / 100;
      vmsg("������O������x�W�A�ĤO�@��");
      break;
    }

    else if (ch == '5')		/* �C�k */
    {
      class = cp->speed >> 9;
      if (class > 4)
        class = 4;

      ep->hp -= injure[class] * (50 + rand() % 100) / 100;
      vmsg("�ּC�ٶó¡A���C������");
      break;
    }

    else if (ch == '6')		/* �M�k */
    {
      class = cp->spirit >> 9;
      if (class > 4)
        class = 4;

      ep->hp -= injure[class] * (30 + rand() % 140) / 100;
      vmsg("������O������x�W�A�ĤO�@��");
      break;
    }

    else if (ch == '7')		/* �t�� */
    {
      class = cp->cook >> 9;
      if (class > 4)
        class = 4;

      ep->hp -= injure[class] * (80 + rand() % 40) / 100;
      vmsg("�z�����a���U�F�r�ġA��⤣�`�N�N�Y�F�U�h");
      break;
    }

    else if (ch == 'a')		/* �v�� */
    {
      class = cp->cook >> 10;	/* �� hp �����e����� */
      if (class > 4)
        class = 4;

      cp->hp += injure[class];	/* �� hp �]�ɥ� injure[] */      
      if (cp->hp > cp->maxhp)
        cp->hp = cp->maxhp;

      vmsg("�R�q�H��A�A�ץX�o");
      break;
    }

    else if (ch >= 'b' && ch <= 'f')		/* �U�t�k�N */
    {
      char buf[64];
      char name[5][3] = {"�p", "�B", "��", "�g", "��"};

      class = cp->magic >> 9;
      if (class > 4)
        class = 4;

      ep->hp -= injure[class] * (50 + rand() % 100) / 100;
      sprintf(buf, "�z�I�i�F%s�t�k�N�A�¤O�Q��", name[ch - 'b']);
      vmsg(buf);
      break;
    }

    else if (ch == 'g')		/* �S�� */
    {
      class = cp->magic >> 9;
      if (class > 4)
        class = 4;

      cp->mp += injure[class];	/* �� mp �]�ɥ� injure[] */      
      if (cp->mp > cp->maxmp)
        cp->mp = cp->maxmp;

      vmsg("��q�R��A�]�O�A�{");
      break;
    }
  }

  pip_pk_turn();
}


static void
pip_pk_charm()		/* �y�b: �� hp */
{
  int class;
  char buf[80];
  char name[5][9] = {"�Z�ҫU�l", "�ⱡ�g", "�p��", "�s��", "����"};
  int injure[5] = {250, 400, 600, 900, 1500};
  int needhp[5] = {350, 600, 900, 1500, 3200};

  class = cp->charm >> 8;	/* �۹��H�U�T�̡A��һݪ��e����C�N�i�H�I�i�j�j�y�b�N�A���Ӫ��O hp */
  if (class > 4)
    class = 4;

  if (cp->hp >= needhp[class])
  {
    cp->hp -= needhp[class];
    ep->hp -= injure[class] * (75 + rand() % 50) / 100;
    sprintf(buf, "�@�s%s�b�z�����Ϥ��U�A���R�������", name[class]);
    vmsg(buf);
    pip_pk_turn();
  }
  else
  {
    vmsg("�z�������O�ˤf�A�ٷQ�y�b��");
    pip_pk_showfoot();  
  }
}


static void
pip_pk_summon()		/* �l��: �� mp */
{
  int class;
  char buf[80];
  char name[5][9] = {"�v�ܩi", "���Y��", "�r��", "���j���s", "��������"};
  int injure[5] = {250, 400, 600, 900, 1500};
  int needmp[5] = {350, 600, 900, 1500, 3200};

  class = cp->charm >> 9;
  if (class > 4)
    class = 4;

  if (cp->mp >= needmp[class])
  {
    cp->mp -= needmp[class];
    ep->hp -= injure[class] * (75 + rand() % 50) / 100;
    sprintf(buf, "�z�l��X%s�A�����a���F���@��", name[class]);
    vmsg(buf);
    pip_pk_turn();
  }
  else
  {
    vmsg("�z�P������h�ΡA����]�l�ꤣ�X��");
    pip_pk_showfoot();  
  }
}


static void
pip_pk_convince()	/* ���A: �� vp */
{
  int class;
  char buf[80];
  char name[5][9] = {"����ǤH", "�u�\\�Y��", "���J�W�H", "�F���s��", "���Ѥj�t"};
  int injure[5] = {250, 400, 600, 900, 1500};
  int needvp[5] = {350, 600, 900, 1500, 3200};

  class = cp->oral >> 9;
  if (class > 4)
    class = 4;

  if (cp->vp >= needvp[class])
  {
    cp->vp -= needvp[class];
    ep->hp -= injure[class] * (75 + rand() % 50) / 100;
    sprintf(buf, "�z���\\�a���A%s�ӧU�z�@�u���O", name[class]);
    vmsg(buf);
    pip_pk_turn();
  }
  else
  {
    vmsg("����ǨӤ@�}�n���G�Q���A�ڡA�A���@�ʦ~�a");
    pip_pk_showfoot();  
  }
}


static void
pip_pk_incite()		/* ����: �� sp */
{
  int class;
  char buf[80];
  char name[5][9] = {"���ǩ�", "���C�|", "�a����", "�Q���s", "�K�Ѩ�"};
  int injure[5] = {250, 400, 600, 900, 1500};
  int needsp[5] = {350, 600, 900, 1500, 3200};

  class = cp->oral >> 9;
  if (class > 4)
    class = 4;

  if (cp->sp >= needsp[class])
  {
    cp->sp -= needsp[class];
    ep->hp -= injure[class] * (40 + rand() % 120) / 100;	/* �ܲ��ʸ��e���T�̬��� */
    sprintf(buf, "�z�i���a����%s��ӱڸs�ӹ�I�ĤH", name[class]);
    vmsg(buf);
    pip_pk_turn();
  }
  else
  {
    vmsg("���H�����ҰʡA�z���p�e���ѤF");
    pip_pk_showfoot();  
  }
}


static void
pip_pk_man()		/* ����ڤU���O */
{
  /* �q�X�԰��D�e�� */
  pip_pk_showing();

  while (!cp->done)
  {
    switch (vkey())
    {
    case '1':		/* �׷i */
      pip_pk_combat();
      break;

    case '2':		/* �ޯ�: �Z�\/�]�k */
      pip_pk_skill();
      break;

    case '3':		/* �y�b */
      pip_pk_charm();
      break;

    case '4':		/* �l�� */
      pip_pk_summon();
      break;

    case '5':		/* ���A */
      pip_pk_convince();
      break;

    case '6':		/* ���� */
      pip_pk_incite(); 
      break;

    case 'q':		/* �{�� */
      cp->hp = 0;
      pip_pk_turn();
      break;
    }
  }
}


static void
pip_pk_wait()		/* ���ݹ��U���O */
{
  int fd;
  struct timeval tv = {1, 100};

  /* �q�X�԰��D�e�� */
  pip_pk_showing();

  outz("���ݹ�誺���� Q)�{��");
  refresh();

  while (!ep->done)
  {
    fd = 1;
    if (select(1, (fd_set *) &fd, NULL, NULL, &tv) > 0)
    {
      if (vkey() == 'q')
      {
	cp->done = 1;	/* �G�H���j��������� */
	ep->done = 1;      
	cp->hp = 0;	/* �������}��� */
	break;
      }
    }
  }
}


/*-------------------------------------------------------*/
/* ��ԥD���                                            */
/*-------------------------------------------------------*/


int 
pip_pk_menu()
{
  int ch;
  char userid[IDLEN + 1];
  struct timeval tv = {1, 100};

  /* itoc.020327: �����Z�j�����D�O�A�p�G��Ԩ�@�b�A�䤤�@�H�_�u�F�A
     �t�~�@�H�|�i�J�j��A�u��� Q ���} */

  if (d.hp <= 0)
    return XEASY;

  pip_pkshm_init();

  pip_ptmp_show();

  ch = ians(b_lines, 0, "�� �p����� 1)�W�իݵo 2)�U�D�Ԯ� [Q]���} ");
  if (ch == '1')
  {
    /* �]�w cp-> */
    if (!pip_ptmp_setup())
      return XEASY;

    cp->inuse = 1;

    outz("���ԬD�Ԥ� Q)���}");
    refresh();
    do
    {
      ch = 1;
      if (select(1, (fd_set *) &ch, NULL, NULL, &tv) > 0)
      {
	if (vkey() == 'q')
	{
	  pip_ptmp_free();
	  return XEASY;
	}
      }
    } while (!*cp->mateid);

    if (!(ep = pip_ptmp_get(cp->mateid, 2)))
    {
      pip_ptmp_free();
      return XEASY;
    }
    cp->done = 0;	/* �Q�D�Ԫ̥���ʡA�D�Ԫ̴N�|�q���L */
    cp->inuse = -1;	/* ��賣�i�J�԰� */
    ep->inuse = -1;
  }
  else if (ch == '2')
  {
    /* �M�w PK ��Թ�H ep-> */
    if (!vget(b_lines, 0, msg_uid, userid, IDLEN + 1, DOECHO) || 
      !str_cmp(cuser.userid, userid) ||
      !(ep = pip_ptmp_get(userid, 1)))
    {
      return XEASY;
    }

    /* �]�w cp-> */
    if (!pip_ptmp_setup())
      return XEASY;

    strcpy(cp->mateid, userid);
    strcpy(ep->mateid, cuser.userid);
    cp->done = 1;	/* �D�Ԫ̫��ʡA�N�|�D�ʳq���Q�D�Ԫ� */
    cp->inuse = 2;
  }
  else
  {
    return XEASY;
  }

  ch = d.tired;

  for (;;)		/* ����� */
  {  
    pip_pk_man();
    if (ep->hp <= 0 || cp->hp <= 0)
      break;          

    pip_pk_wait();
    if (cp->hp <= 0 || ep->hp <= 0)
      break;
  }

  /* ���G�P�_ */
  pip_pk_ending();

  pip_ptmp_free();

  d.tired = ch;		/* �H�K PK ���Ӥ[�APK ������Ӧ] tired �L�������F */
  return 0;
}
#endif	/* HAVE_GAME */
