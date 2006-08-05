/*-------------------------------------------------------*/
/* util/windtop.h					 */
/*-------------------------------------------------------*/
/* target : WindTop �� Maple �ഫ			 */
/* create : 03/06/30					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

  0. �O�d��� brd/ gem/ usr/ .USR�A��l�����s����

  1. �]�w FN_BOARD

  2. �Q�� windtop2brd �ഫ .BRD

  3. �Q�� windtop2usr �ഫ .ACCT

  4. �Q�� windtop2mf �ഫ MF

  5. �Q�� windtop2pip �ഫ chicken

  6. �N�s���� gem/@/ �U���o���ɮ׽ƻs�L��
     @apply @e-mail @goodbye @index @justify @newuser @opening.0
     @opening.1 @opening.2 @post @re-reg @tryout @welcome
            
  7. �W BBS ���A�b (A)nnounce �̭��A�إH�U�G�Ө��v���Ҧ����
     {���D} �����Q��
     {�Ʀ�} �έp���

#endif


#include "bbs.h"


#define FN_BOARD	"/tmp/.BRD"	/* WindTop BBS �� .BRD */


/* ----------------------------------------------------- */
/* old ACCT struct					 */
/* ----------------------------------------------------- */


typedef struct
{
  int userno;			/* unique positive code */
  char userid[13];		/* userid */
  char passwd[14];;		/* user password crypt by DES */
  uschar signature;		/* user signature number */
  char realname[20];		/* user realname */
  char username[24];		/* user nickname */
  usint userlevel;		/* user perm */
  int numlogins;		/* user login times */
  int numposts;			/* user post times */
  usint ufo;			/* user basic flags */
  time_t firstlogin;		/* user first login time */
  time_t lastlogin;		/* user last login time */
  time_t staytime;		/* user total stay time */
  time_t tcheck;		/* time to check mbox/pal */
  char lasthost[32];		/* user last login remote host */
  int numemail;			/* �H�o Inetrnet E-mail ���� */
  time_t tvalid;		/* �q�L�{�ҡB��� mail address ���ɶ� */
  char email[60];		/* user email */
  char address[60];		/* user address */
  char justify[60];		/* FROM of replied justify mail */
  char vmail[60];		/* �q�L�{�Ҥ� email */
  time_t deny;			/* user violatelaw time */
  int request;			/* �I�q�t�� */
  usint ufo2;			/* �������ӤH�]�w */
  char ident[108];		/* user remote host ident */
  time_t vtime;			/* validate time */
}	userec;


/* ----------------------------------------------------- */
/* old BRD struct					 */
/* ----------------------------------------------------- */


typedef struct
{
  char brdname[13];		/* board ID */
  char title[43];
  char color;
  char class[5];
  char BM[37];			/* BMs' uid, token '/' */

  uschar bvote;			/* �@���X���벼�|�椤 */

  time_t bstamp;		/* �إ߬ݪO���ɶ�, unique */
  usint readlevel;		/* �\Ū�峹���v�� */
  usint postlevel;		/* �o��峹���v�� */
  usint battr;			/* �ݪO�ݩ� */
  time_t btime;			/* .DIR �� st_mtime */
  int bpost;			/* �@���X�g post */
  time_t blast;			/* �̫�@�g post ���ɶ� */
  usint expiremax;		/* Expire Max Post */
  usint expiremin;		/* Expire Min Post */
  usint expireday;		/* Expire old Post */
  usint n_reads;		/* �ݪO�\Ū�֭p times/hour */
  usint n_posts;		/* �ݪO�o��֭p times/hour */
  usint n_news;			/* �ݪO��H�֭p times/hour */
  usint n_bans;			/* �ݪO�ɫH�֭p times/hour */
  char  reserve[100];		/* �O�d���� */
}	boardheader;


/* ----------------------------------------------------- */
/* old BRD battr					 */
/* ----------------------------------------------------- */


#define BATTR_NOZAP       0x0001  /* ���i zap */
#define BATTR_NOTRAN      0x0002  /* ����H */
#define BATTR_NOCOUNT     0x0004  /* ���p�峹�o��g�� */
#define BATTR_NOSTAT      0x0008  /* ���ǤJ�������D�έp */
#define BATTR_NOVOTE      0x0010  /* �����G�벼���G�� [sysop] �O */
#define BATTR_ANONYMOUS   0x0020  /* �ΦW�ݪO */
#define BATTR_NOFORWARD   0x0040  /* lkchu.981201: ���i��H */
#define BATTR_LOGEMAIL    0x0080  /* �۰ʪ��[e-mail */
#define BATTR_NOBAN       0x0100  /* ���׫H */
#define BATTR_NOLOG       0x0200  /* �����������H�k */
#define BATTR_NOCNTCROSS  0x0400  /* ������ cross post */
#define BATTR_NOREPLY     0x0800  /* ����^�峹 */
#define BATTR_NOLOGREAD   0x1000  /* �������ݪ��\Ū�v */
#define BATTR_CHECKWATER  0x2000  /* ����������� */
#define BATTR_CHANGETITLE 0x4000  /* ���D�ק睊�W */
#define BATTR_MODIFY      0x8000  /* �ϥΪ̭ק�峹 */
#define BATTR_PRH         0x10000 /* ���ˤ峹 */
#define BATTR_NOTOTAL     0x20000 /* ���έp�ݪO�ϥά��� */


/* ----------------------------------------------------- */
/* old MF struct					 */
/* ----------------------------------------------------- */

typedef struct
{
  time_t chrono;                /* timestamp */
  int xmode;

  int xid;                      /* reserved �O�d*/

  char xname[31];               /* �ɮצW�� */
  signed char pushscore;
  char owner[80];               /* �@�� (E-mail address) */
  char nick[50];                /* �ʺ� */

  char date[9];                 /* [96/12/01] */
  char title[73];               /* �D�D (TTLEN + 1) */
}   mfheader;


/* ----------------------------------------------------- */
/* old MF file						 */
/* ----------------------------------------------------- */

#define FN_OLD_FAVORITE		"favorite"
#define FN_OLDIMG_FAVORITE	"favorite.img"
