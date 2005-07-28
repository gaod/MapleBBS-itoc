/*-------------------------------------------------------*/
/* modes.h	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : user operating mode & status		 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#ifndef _MODES_H_
#define _MODES_H_


/* ----------------------------------------------------- */
/* user �ާ@���A�P�Ҧ�					 */
/* ----------------------------------------------------- */

/* itoc.010329.����: �� 0 �O�d�A���Ӱ��S��γ~ */

#define M_0MENU		1	/*  main MENU */
#define M_AMENU		2	/* admin MENU */
#define M_MMENU		3	/*  mail MENU */
#define M_TMENU		4	/*  talk MENU */
#define M_UMENU		5	/*  user MENU */
#define M_XMENU		6	/*  tool MENU */
				/* M_XMENU �O menu ���̫�@�� */
				/* M_XMENU ���e���ʺA�ݪO */

#define M_LOGIN		7	/* login */

#define M_GEM		8	/* announce */
#define M_BOARD		9	/* board list */
#define M_MF		10	/* my favorite */
#define M_READA		11	/* read article */
#define M_RMAIL		12	/* read mail */

#define M_PAL		13	/* set pal/aloha list */
#define M_LUSERS	14	/* user list */
#define M_VOTE		15
#define M_BMW           16
#define M_SONG		17
#define M_COSIGN	18

#define M_SYSTEM	19	/* �� M_SYSTEM(�t) �P M_CHAT(�t) �������� talk request */

#define M_XFILES        20	/* admin set system files */
#define M_UFILES        21	/* user set user files  */

#define M_BMW_REPLY     22
#define M_GAME		23
#define M_POST		24
#define M_SMAIL		25
#define M_TRQST		26

#define M_TALK		27	/* �� M_TALK(�t) �P M_IDLE(�t) ���� mateid */
#define M_CHAT		28	/* �� M_BBTP(�t) �P M_CHAT(�t) �������� talk request */
#define M_PAGE		29
#define M_QUERY		30
#define M_IDLE		31	/* �� M_TALK(�t) �P M_IDLE(�t) ���� mateid */

#define M_XMODE		32
#define M_MAX           M_XMODE


#ifdef	_MODES_C_
static char *ModeTypeTable[] =
{
  "�O�d",

  "�D���",			/* M_0MENU */
  "�t�κ��@",			/* M_AMENU */
  "�l����",			/* M_MMENU */
  "��Ϳ��",			/* M_TMENU */
  "�ϥΪ̿��",			/* M_UMENU */
  "Xyz ���",			/* M_XMENU */

  "�W���~��",			/* M_LOGIN */

  "���G��",			/* M_GEM */
  "�ݪO�C��",			/* M_BOARD */
  "�ڪ��̷R",			/* M_MF */
  "�\\Ū�峹",			/* M_READA */
  "Ū�H",			/* M_RMAIL */

  "����B��",			/* M_PAL */
  "�ϥΪ̦W��",			/* M_LUSERS */
  "�벼��",			/* M_VOTE */
  "��ݤ��y",                   /* M_BMW */
  "�I�q",			/* M_SONG */
  "�ݪO�s�p",			/* M_COSIGN */

  "�t�κ޲z",			/* M_SYSTEM */

  "�s�t���ɮ�",                 /* M_XFILES */
  "�s�ӤH�ɮ�",                 /* M_UFILES */

  "���y�ǳƤ�",			/* M_BMW_REPLY */
  "���C��",			/* M_GAME */
  "�o��峹",			/* M_POST */
  "�g�H",			/* M_SMAIL */
  "�ݾ�",			/* M_TRQST */

  "���",			/* M_TALK */	/* �� mateid ���ʺA����r���i�Ӫ� */
  "���",			/* M_CHAT */
  "�I�s",			/* M_PAGE */
  "�d��",			/* M_QUERY */
  "�o�b",			/* M_IDLE */

  "��L"			/* M_XMODE */
};
#endif				/* _MODES_C_ */


/* ----------------------------------------------------- */
/* menu.c �����Ҧ�					 */
/* ----------------------------------------------------- */


#define XEASY	0x333		/* Return value to un-redraw screen */


/* ----------------------------------------------------- */
/* pal.c �����Ҧ�					 */
/* ----------------------------------------------------- */


#define PALTYPE_PAL	0		/* �B�ͦW�� */
#define PALTYPE_LIST	1		/* �s�զW�� */
#define PALTYPE_BPAL	2		/* �O�ͦW�� */
#define PALTYPE_VOTE	3		/* ����벼�W�� */


/* ----------------------------------------------------- */
/* visio.c �����Ҧ�					 */
/* ----------------------------------------------------- */


/* Flags to getdata input function */

#define NOECHO		0x0000		/* ����ܡA�Ω�K�X���o */
#define DOECHO		0x0100		/* �@����� */
#define LCECHO		0x0200		/* low case echo�A�����p�g */
#define GCARRY		0x0400		/* �|��ܤW�@��/�ثe���� */

#define GET_LIST	0x1000		/* ���o Link List */
#define GET_USER	0x2000		/* ���o user id */
#define GET_BRD		0x4000		/* ���o board id */


/* ----------------------------------------------------- */
/* read.c �����Ҧ�					 */
/* ----------------------------------------------------- */

/* for tag */

#define	TAG_NIN		0		/* ���ݩ� TagList */
#define	TAG_TOGGLE	1		/* ���� Taglist */
#define	TAG_INSERT	2		/* �[�J TagList */


/* for bbstate : bbs operating state */

#define	STAT_POST	0x0010000	/* �O�_�i�H�b currboard �o��峹 */
#define STAT_BOARD	0x0020000	/* �O�_�i�H�b currboard �R���Bmark�峹 (�O�D�B�ݪO�`��) */
#define STAT_BM		0x0040000	/* �O�_�� currboard ���O�D */
#define	STAT_LOCK	0x0100000	/* �O�_����w�ù� */
#define	STAT_STARTED	0x8000000	/* �O�_�w�g�i�J�t�� */


/* for user's board permission level & state record */

#define BRD_L_BIT	0x0001		/* �i�C�Alist */
#define BRD_R_BIT	0x0002		/* �iŪ�Aread */
#define BRD_W_BIT	0x0004		/* �i�g�Awrite */
#define BRD_X_BIT	0x0008		/* �i�ޡAexecute�A�O�D�B�ݪO�`�� */
#define BRD_M_BIT	0x0010		/* �i�z�Amanage�A�O�D */

#define BRD_V_BIT	0x0020		/* �w�g�}�L�F�Avisit ==> �ݹL�u�i�O�e���v */
#define BRD_H_BIT	0x0040		/* .BRH �����\Ū�O�� (history) */
#define BRD_Z_BIT	0x0080		/* .BRH zap ���F */


/* for user's gem permission level record */

#define GEM_W_BIT	0x0001		/* �i�g�Awrite�A�O�D�A�ݪO�`�� */
#define GEM_X_BIT	0x0002		/* �i�ޡAexecute�A���� */
#define GEM_M_BIT	0x0004		/* �i�z�Amanage�A�O�D */


/* for curredit */

#define EDIT_MAIL	0x0001		/* �ثe�O mail/board ? */
#define EDIT_LIST	0x0002		/* �O�_�� mail list ? */
#define EDIT_BOTH	0x0004		/* both reply to author/board ? */
#define EDIT_OUTGO	0x0008		/* ����H�X�h */
#define EDIT_ANONYMOUS	0x0010		/* �ΦW�Ҧ� */
#define EDIT_RESTRICT	0x0020		/* �[�K�s�� */


/* ----------------------------------------------------- */
/* xover.c �����Ҧ�					 */
/* ----------------------------------------------------- */


#define XO_DL		0x80000000

#define	XO_MODE		0x10000000


#define XO_NONE		(XO_MODE + 0)
#define XO_INIT		(XO_MODE + 1)
#define XO_LOAD		(XO_MODE + 2)
#define XO_HEAD		(XO_MODE + 3)
#define XO_NECK		(XO_MODE + 4)
#define XO_BODY		(XO_MODE + 5)
#define XO_FOOT		(XO_MODE + 6)	/* itoc.����: �M�� b_lines */
#define XO_LAST		(XO_MODE + 7)
#define	XO_QUIT		(XO_MODE + 8)


#define	XO_RSIZ		256		/* max record length */
#define XO_TALL		(b_lines - 3)	/* page size = b_lines - 3 (���h head/neck/foot �@�T��) */


#define	XO_MOVE		0x20000000	/* cursor movement */
#define	XO_WRAP		0x08000000	/* cursor wrap in movement */
#define	XO_TAIL		(XO_MOVE - 999)	/* init cursor to tail */


#define	XO_ZONE		0x40000000	/* �i�J�Y�@�� zone */
#define	XZ_BACK		0x100


#define	XZ_CLASS	(XO_ZONE + 0)	/* �ݪO�C�� */
#define	XZ_ULIST	(XO_ZONE + 1)	/* �u�W�ϥΪ̦W�� */
#define	XZ_PAL		(XO_ZONE + 2)	/* �B�ͦW�� */
#define XZ_ALOHA	(XO_ZONE + 3)	/* �W���q���W�� */ 
#define	XZ_VOTE		(XO_ZONE + 4)	/* �벼 */
#define XZ_BMW		(XO_ZONE + 5)	/* ���y */
#define	XZ_MF		(XO_ZONE + 6)	/* �ڪ��̷R */
#define XZ_COSIGN	(XO_ZONE + 7)	/* �s�p */
#define	XZ_SONG		(XO_ZONE + 8)	/* �I�q */
#define XZ_NEWS		(XO_ZONE + 9)	/* �s�D�\Ū�Ҧ� */

/* �H�U���� thread �D�D���\Ū���\�� */
/* �H�U���� tag �\�� */

#define XZ_XPOST        (XO_ZONE + 10)	/* �j�M�峹�Ҧ� */
#define	XZ_MBOX		(XO_ZONE + 11)	/* �H�c */
#define	XZ_POST		(XO_ZONE + 12)	/* �ݪO */
#define XZ_GEM		(XO_ZONE + 13)	/* ��ذ� */

#endif				/* _MODES_H_ */
