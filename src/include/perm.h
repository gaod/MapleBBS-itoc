/*-------------------------------------------------------*/
/* perm.h	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : permission levels of user & board		 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#ifndef	_PERM_H_
#define	_PERM_H_


/* ----------------------------------------------------- */
/* These are the 32 basic permission bits.		 */
/* ----------------------------------------------------- */

#define	PERM_BASIC	0x00000001	/* 1-8 : ���v�� */
#define PERM_CHAT	0x00000002
#define	PERM_PAGE	0x00000004
#define PERM_POST	0x00000008
#define	PERM_VALID 	0x00000010	/* LOGINOK */
#define PERM_MBOX	0x00000020
#define PERM_CLOAK	0x00000040
#define PERM_XEMPT 	0x00000080

#define	PERM_9		0x00000100
#define	PERM_10		0x00000200
#define	PERM_11		0x00000400
#define	PERM_12		0x00000800
#define	PERM_13		0x00001000
#define	PERM_14		0x00002000
#define	PERM_15		0x00004000
#define	PERM_16		0x00008000

#define PERM_DENYPOST	0x00010000	/* 17-24 : �T���v�� */
#define	PERM_DENYTALK	0x00020000
#define	PERM_DENYCHAT	0x00040000
#define	PERM_DENYMAIL	0x00080000
#define	PERM_DENY5	0x00100000
#define	PERM_DENY6	0x00200000
#define	PERM_DENYLOGIN	0x00400000
#define	PERM_PURGE	0x00800000

#define PERM_BM		0x01000000	/* 25-32 : �޲z�v�� */
#define PERM_SEECLOAK	0x02000000
#define PERM_ADMIN3	0x04000000
#define PERM_REGISTRAR	0x08000000
#define PERM_ACCOUNTS	0x10000000
#define PERM_CHATROOM	0x20000000
#define	PERM_BOARD	0x40000000
#define PERM_SYSOP	0x80000000


/* ----------------------------------------------------- */
/* These permissions are bitwise ORs of the basic bits.	 */
/* ----------------------------------------------------- */


/* This is the default permission granted to all new accounts. */
#define PERM_DEFAULT 	PERM_BASIC

/* �� PERM_VALID �~�i�H�O�H�i�ӥ������U */
#ifdef HAVE_GUARANTOR
#define PERM_GUARANTOR	PERM_VALID
#endif

#if 0   /* itoc.����: ���󯸰��v�� */

  �Ҧ��{�������󯸰Ȫ��v�����令 PERM_ALLXXXX
  �b���e�@�ӹϨӴy�z�����v�����]�w�C

                  �z ���U�`�� PERM_REGISTRAR : �i�H�f���U��C
                  �x
                  �u �b���`�� PERM_ACCOUNTS : �i�H�ק��v���B��u�W�ϥΪ̡B�f���U��C
                  �x
  ���� PERM_SYSOP �q ��ѫ��`�� PERM_CHATROOM : �b��ѫǬO roomop�C
                  �x
                  �u �ݪO�`�� PERM_BOARD : �i�H�ק�ݪO�]�w�B�i�J���K�Φn�ͬݪO�C
                  �x
                  �| ���鯸�� PERM_ALLADMIN : �H�W�|���`�ޡA�����H�U�\��G
                     �W���ӷ��]�w�B�����N�B�����B�����w���{�ҡB�L���s�⨣�ߤT�ѡBmulti-login�B
                     �קﯸ�W���B��s�t�ΡB�ި��i�H�L�h�B�H�H�������ϥΪ̡B�H�c�L�W���C

  ���� PERM_SYSOP ���F�H�W�Ҧ��\��A�پ֦��H�U�\��G
  ��ذϫظm��ơB��ذϬݨ�[�K�ؿ������D�B�\Ū�Ҧ��H���H��B�o���Ҧ��H�b�ݭ��ӪO�B�}�ү����v���C
  
#endif

#define PERM_ALLADMIN	(PERM_REGISTRAR | PERM_BOARD | PERM_ACCOUNTS | PERM_SYSOP)	/* ���� */
#define	PERM_ALLREG	(PERM_SYSOP | PERM_ACCOUNTS | PERM_REGISTRAR)	/* �f���U�� */
#define	PERM_ALLACCT	(PERM_SYSOP | PERM_ACCOUNTS)			/* �b���޲z */
#define PERM_ALLCHAT	(PERM_SYSOP | PERM_CHATROOM)			/* ��Ѻ޲z */
#define PERM_ALLBOARD	(PERM_SYSOP | PERM_BOARD)			/* �ݪO�޲z */

#define PERM_ALLVALID	(PERM_VALID | PERM_POST | PERM_PAGE | PERM_CHAT)	/* �{�ҳq�L�������������v�� */
#define PERM_ALLDENY	(PERM_DENYPOST | PERM_DENYTALK | PERM_DENYCHAT | PERM_DENYMAIL)	/* �Ҧ����v */

#define PERM_LOCAL	PERM_BASIC	/* ���O guest �N��H�H�쯸����L�ϥΪ� */
#define PERM_INTERNET	PERM_VALID	/* �����{�ҹL�����~��H�H�� Internet */

/* #define HAS_PERM(x)	((x)?cuser.userlevel&(x):1) */
/* #define HAVE_PERM(x)	(cuser.userlevel&(x)) */
/* itoc.001217: �{�������|�� HAS_PERM(0) ���g�k�AHAVE_PERM ���|�Ψ� */
#define HAS_PERM(x)	(cuser.userlevel&(x))


/* ----------------------------------------------------- */
/* �U���v��������N�q					 */
/* ----------------------------------------------------- */

#define	NUMPERMS	32

#define STR_PERM	"bctpjm#x-------@PTCM--L*B#-RACBS"	/* itoc: �s�W�v�����ɭԧO�ѤF��o�̰� */

#ifdef _ADMIN_C_

static char *perm_tbl[NUMPERMS] = 
{
  "���v�O",			/* PERM_BASIC */
  "�i�J��ѫ�",			/* PERM_CHAT */
  "��H���",			/* PERM_PAGE */
  "�o��峹",			/* PERM_POST */
  "�����{��",			/* PERM_VALID */
  "�H��L�W��",			/* PERM_MBOX */
  "�����N",			/* PERM_CLOAK */
  "�ä[�O�d�b��",		/* PERM_XEMPT */

  "�O�d",			/* PERM_9 */
  "�O�d",			/* PERM_10 */
  "�O�d",			/* PERM_11 */
  "�O�d",			/* PERM_12 */
  "�O�d",			/* PERM_13 */
  "�O�d",			/* PERM_14 */
  "�O�d",			/* PERM_15 */
  "�O�d",			/* PERM_16 */

  "�T��o��峹",		/* PERM_DENYPOST */
  "�T�� talk",			/* PERM_DENYTALK */
  "�T�� chat",			/* PERM_DENYCHAT */
  "�T�� mail",			/* PERM_DENYMAIL */
  "�O�d",			/* PERM_DENY5 */
  "�O�d",			/* PERM_DENY6 */
  "�T�� login",			/* PERM_DENYLOGIN */
  "�M���b��",			/* PERM_PURGE */

  "�O�D",			/* PERM_BM */
  "�ݨ��Ԫ�",			/* PERM_SEECLOAK */
  "�O�d",			/* PERM_ADMIN3 */
  "���U�`��",			/* PERM_REGISTRAR */
  "�b���`��",			/* PERM_ACCOUNTS */
  "��ѫ��`��",			/* PERM_CHATCLOAK */
  "�ݪO�`��",			/* PERM_BOARD */
  "����"			/* PERM_SYSOP */
};

#endif
#endif				/* _PERM_H_ */
