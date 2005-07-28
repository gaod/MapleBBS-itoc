/*-------------------------------------------------------*/
/* util/wd.h 	                                         */
/*-------------------------------------------------------*/
/* target : WD �� Maple 3.02 �ഫ			 */
/* create : 02/01/03                                     */
/* author : ernie@micro8.ee.nthu.edu.tw                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

  1. �]�w OLD_BBSHOME�BFN_PASSWD�BFN_BOARD
  2. �ק�Ҧ��� old struct

  3. �����b brd �৹�~�i�H�ഫ gem
  4. �����b usr �� brd ���৹�~�i�H�ഫ mf
  5. �����b usr �� brd ���৹�~�i�H�ഫ pal
  6. �����b usr �৹�~�i�H�ഫ bmw pip list
  7. ��ĳ�ഫ���Ǭ� usr -> brd -> gem -> mf -> pal -> bmw -> pip -> list

#endif


#include "bbs.h"


#define OLD_BBSHOME     "/home/oldbbs"		/* WD */
#define FN_PASSWD	"/home/oldbbs/.PASSWDS"	/* WD */
#define FN_BOARD        "/home/oldbbs/.BOARDS"	/* WD */


#define	HAVE_PERSONAL_GEM			/* WD �O���ӤH��ذϪ� */


/* ----------------------------------------------------- */
/* old .PASSWDS struct : 512 bytes                       */
/* ----------------------------------------------------- */

struct userec
{
  char userid[13];                /* �ϥΪ̦W��  13 bytes */
  char realname[20];              /* �u��m�W    20 bytes */
  char username[24];              /* �ʺ�        24 bytes */
  char passwd[14];                /* �K�X        14 bytes */
  uschar uflag;                   /* �ϥΪ̿ﶵ   1 byte  */
  usint userlevel;                /* �ϥΪ��v��   4 bytes */
  ushort numlogins;               /* �W������     2 bytes */
  ushort numposts;                /* POST����     2 bytes */
  time_t firstlogin;              /* ���U�ɶ�     4 bytes */
  time_t lastlogin;               /* �e���W��     4 bytes */
  char lasthost[24];              /* �W���a�I    24 bytes */
  char vhost[24];                 /* �������}    24 bytes */
  char email[50];                 /* E-MAIL      50 bytes */
  char address[50];               /* �a�}        50 bytes */
  char justify[39];               /* ���U���    39 bytes */
  uschar month;                   /* �X�ͤ��     1 byte  */
  uschar day;                     /* �X�ͤ��     1 byte  */
  uschar year;                    /* �X�ͦ~��     1 byte  */
  uschar sex;                     /* �ʧO         1 byte  */
  uschar state;                   /* ���A??       1 byte  */
  usint habit;                    /* �ߦn�]�w     4 bytes */
  uschar pager;                   /* �߱��C��     1 bytes */
  uschar invisible;               /* �����Ҧ�     1 bytes */
  usint exmailbox;                /* �H�c�ʼ�     4 bytes */
  usint exmailboxk;               /* �H�cK��      4 bytes */
  usint toquery;                  /* �n�_��       4 bytes */
  usint bequery;                  /* �H���       4 bytes */
  char toqid[13];	          /* �e���d��    13 bytes */
  char beqid[13];                 /* �e���Q�֬d  13 bytes */
  unsigned long int totaltime;    /* �W�u�`�ɼ�   8 bytes */
  usint sendmsg;                  /* �o�T������   4 bytes */
  usint receivemsg;               /* ���T������   4 bytes */
  unsigned long int goldmoney;    /* ���Ъ���     8 bytes */
  unsigned long int silvermoney;  /* �ȹ�         8 bytes */
  unsigned long int exp;          /* �g���       8 bytes */
  time_t dtime;                   /* �s�ڮɶ�     4 bytes */
  int limitmoney;                 /* �����U��     4 bytes */
};
typedef struct userec userec;


/* ----------------------------------------------------- */
/* old DIR of board struct : 128 bytes                   */
/* ----------------------------------------------------- */

struct fileheader
{
  char filename[33];		/* M.9876543210.A */
  char savemode;		/* file save mode */
  char owner[14];		/* uid[.] */
  char date[6];			/* [02/02] or space(5) */
  char title[73];
  uschar filemode;		/* must be last field @ boards.c */
};
typedef struct fileheader fileheader;


/* ----------------------------------------------------- */
/* old BOARDS struct : 512 bytes                         */
/* ----------------------------------------------------- */

struct boardheader
{
  char brdname[13];             /* �ݪO�^��W��    13 bytes */
  char title[49];               /* �ݪO����W��    49 bytes */
  char BM[39];                  /* �O�DID�M"/"     39 bytes */
  usint brdattr;                /* �ݪO���ݩ�       4 bytes */
  time_t bupdate;               /* note update time 4 bytes */
  uschar bvote;                 /* Vote flags       1 bytes */
  time_t vtime;                 /* Vote close time  4 bytes */
  usint level;                  /* �i�H�ݦ��O���v�� 4 bytes */
  unsigned long int totalvisit; /* �`���X�H��       8 bytes */
  unsigned long int totaltime;  /* �`���d�ɶ�       8 bytes */
  char lastvisit[13];           /* �̫�ݸӪO���H  13 bytes */
  time_t opentime;              /* �}�O�ɶ�         4 bytes */
  time_t lastime;               /* �̫���X�ɶ�     4 bytes */
  char passwd[14];              /* �K�X            14 bytes */
  unsigned long int postotal;   /* �`���q :p        8 bytes */
  usint maxpost;                /* �峹�W��         4 bytes */
  usint maxtime;                /* �峹�O�d�ɶ�     4 bytes */
  char desc[3][80];             /* ����y�z      240 bytes */
  char pad[87];
};
typedef struct boardheader boardheader;


/* ----------------------------------------------------- */
/* old FRIEND struct : 128 bytes                         */
/* ----------------------------------------------------- */

struct FRIEND
{
  char userid[33];              /* list name/userid */
  char savemode;
  char owner[14];               /* bbcall */
  char date[6];                 /* birthday */
  char desc[73];                /* list/user description */
  uschar ftype;                 /* mode:  PAL, BAD */
};
typedef struct FRIEND FRIEND;
