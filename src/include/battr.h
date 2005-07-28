/*-------------------------------------------------------*/
/* battr.h	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : Board Attribution				 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#ifndef	_BATTR_H_
#define	_BATTR_H_


/* ----------------------------------------------------- */
/* Board Attribution : flags in BRD.battr		 */
/* ----------------------------------------------------- */


#define BRD_NOZAP	0x01	/* ���i zap */
#define BRD_NOTRAN	0x02	/* ����H */
#define BRD_NOCOUNT	0x04	/* ���p�峹�o��g�� */
#define BRD_NOSTAT	0x08	/* ���ǤJ�������D�έp */
#define BRD_NOVOTE	0x10	/* �����G�벼���G�� [record] �O */
#define BRD_ANONYMOUS	0x20	/* �ΦW�ݪO */
#define BRD_NOSCORE	0x40	/* �������ݪO */


/* ----------------------------------------------------- */
/* �U�غX�Ъ�����N�q					 */
/* ----------------------------------------------------- */


#define NUMBATTRS	7

#define STR_BATTR	"zTcsvA%"			/* itoc: �s�W�X�Ъ��ɭԧO�ѤF��o�̰� */


#ifdef _ADMIN_C_
static char *battr_tbl[NUMBATTRS] =
{
  "���i Zap",			/* BRD_NOZAP */
  "����H�X�h",			/* BRD_NOTRAN */
  "���O���g��",			/* BRD_NOCOUNT */
  "�����������D�έp",		/* BRD_NOSTAT */
  "�����}�벼���G",		/* BRD_NOVOTE */
  "�ΦW�ݪO",			/* BRD_ANONYMOUS */
  "�������ݪO",			/* BRD_NOSCORE */
};

#endif

#endif				/* _BATTR_H_ */
