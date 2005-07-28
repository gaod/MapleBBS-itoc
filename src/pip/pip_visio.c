/*-------------------------------------------------------*/
/* pipvisio.c	( NTHU CS MapleBBS Ver 3.10 )      	 */
/*-------------------------------------------------------*/
/* target : �i�p���C��                                   */
/* create :   /  /                                       */
/* update : 01/08/03                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/* picture: tiball.bbs@bbs.nhctc.edu.tw                  */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


#if 0	/* itoc.010803.����:�ù��t�m */

�̫e��      �@�Ǹ�����

����        ��

b_lines - 2 �߰ݦC
b_lines - 1 ���O�C
b_lines     ���O�C

#endif



/*-------------------------------------------------------*/
/* ���O���o                                              */
/*-------------------------------------------------------*/


int			/* itoc.010802: ���N vans("Y/N�H[Y] ") �o�����D�Ϊ��禡 */
ians(x, y, msg)
  int x, y;		/* itoc.010803: �@�뮳 (b_lines - 2, 0) �ӷ�߰ݦC */
  char *msg;
{
  move(x, 0);		/* �M����C */
  clrtoeol();
  move(x, y);
  outs(msg);
  outs("\033[47m  \033[m");
  move(x, y + strlen(msg));	/* ���в��ʮخؤ� */
  return vkey();		/* �u����A���ݭn�� ENTER */

#if 0
  if (ch >= 'A' && ch <= 'Z')
    ch |= 0x20;		/* ���p�g */
#endif
}


/*-------------------------------------------------------*/
/* �ù�����						 */
/*-------------------------------------------------------*/


void
clrfromto(from, to)	/* �M�� from~to �C�A�̫��а��d�b (from, 0) */
  int from, to;
{
  while (to >= from)
  {
    move(to, 0);
    clrtoeol();
    to--;
  }
}


static int			/* 1: �ɮצb  0: �ɮפ��b */
show_file(fpath, ln, lines)	/* �q�� ln �C�}�l�L�ɮ� lines �C */
  char *fpath;
  int ln, lines;
{
  FILE *fp;
  char buf[ANSILINELEN];
  int i;

  if ((fp = fopen(fpath, "r")))
  {
    clrfromto(ln, ln + lines);
    i = lines;
    while (fgets(buf, ANSILINELEN, fp) && i--)
      outs(buf);
    fclose(fp);
    return 1;
  }

  /* itoc.010802: �^���ɮ׿� */
  sprintf(buf, "%s �򥢡A�Чi������", fpath);
  zmsg(buf);
  return 0;
}


/*-------------------------------------------------------*/
/* �p���ϧΰ�                  				 */
/*-------------------------------------------------------*/


/* itoc.010801: �Ш̷ӦU show_xxx_pic() �� show_file(buf, ln, lines) �� lines �ӱ���ϮצC�� */
/* itoc.010801: ���`���p�U���O 10 �C (��n�]�O�ʺA�ݪO���C��)�C�Y ln=0 �B lines=b_lines+1 ��ܾ�ӿù����M */


/* -------------------- */
/* �@�몺�ϡA�b 7~16 �C */
/* -------------------- */

int
show_basic_pic(i)
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "basic/pic%d", i);
  return show_file(buf, 7, 10);
}


int
show_feed_pic(i)		/* �Y�F�� */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "feed/pic%d", i);
  return show_file(buf, 7, 10);
}


int
show_usual_pic(i)		/* ���`���A */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "usual/pic%d", i);
  return show_file(buf, 7, 10);
}


int
show_special_pic(i)		/* �S���� */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "special/pic%d", i);
  return show_file(buf, 7, 10);
}

int
show_practice_pic(i)		/* �צ�Ϊ��� */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "practice/pic%d", i);
  return show_file(buf, 7, 10);
}


int
show_job_pic(i)			/* ���u��show�� */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "job/pic%d", i);
  return show_file(buf, 7, 10);
}


int
show_play_pic(i)		/* �𶢪��� */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "play/pic%d", i);
  return show_file(buf, 7, 10);
}


int
show_guess_pic(i)		/* �q���� */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "guess/pic%d", i);
  return show_file(buf, 7, 10);
}


int
show_badman_pic(i)		/* �Ǫ� */
  int i;
{
  char buf[64];

/* itoc.010731:  picabc   abc �O�s��
   �ʦ�� a �O�����A�Q��ƭӦ�� bc �O�����Ϫ��s�� */

  sprintf(buf, PIP_PICHOME "badman/pic%03d", i);
  return show_file(buf, 7, 10);
}


int
show_fight_pic(i)		/* ���[ */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "fight/pic%d", i);
  return show_file(buf, 7, 10);
}


int
show_resultshow_pic(i)		/* ��ì�u */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "resultshow/pic%d", i);
  return show_file(buf, 7, 10);
}


int
show_quest_pic(i)		/* ���� */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "quest/pic%d", i);
  return show_file(buf, 7, 10);
}


/* -------------------------- */
/* �@�ǯS���ϡA�q�b 1~10 �C */
/* -------------------------- */

int
show_weapon_pic(i)		/* �Z���� */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "weapon/pic%d", i);
  return show_file(buf, 1, 10);
}


int
show_palace_pic(i)		/* �Ѩ����ڥ� */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "palace/pic%d", i);
  return show_file(buf, 1, 10);
}


/* ------------------------------------- */
/* �@�ǯS���ϡA�q�b 4~19(b_lines-4) �C */
/* ------------------------------------- */

int
show_system_pic(i)		/* �t�� */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "system/pic%d", i);
  return show_file(buf, 4, 16);
}


int
show_ending_pic(i)		/* ���� */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "ending/pic%d", i);
  return show_file(buf, 4, 16);
}


/* -------------------------- */
/* �@�ǯS���ϡA�q�b��ӿù� */
/* -------------------------- */

int
show_die_pic(i)			/* ���` */
  int i;
{
  char buf[64];
  sprintf(buf, PIP_PICHOME "die/pic%d", i);
  return show_file(buf, 0, b_lines + 1);
}
#endif	/* HAVE_GAME */
