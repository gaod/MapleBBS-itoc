/*-------------------------------------------------------*/
/* util/tran/ats.h                                       */
/*-------------------------------------------------------*/
/* target : ATS �� Maple 3.02 �ഫ                       */
/* create : 02/10/26                                     */
/* author : ernie@micro8.ee.nthu.edu.tw                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#if 0

  1. �]�w OLD_BBSHOME�BFN_PASSWD�BFN_BOARD
  2. �ק�Ҧ��� old struct

  3. �����b brd �৹�~�i�H�ഫ gem
  4. �����b usr �� brd ���৹�~�i�H�ഫ mf
  5. �����b usr �� brd ���৹�~�i�H�ഫ pal
  6. �����b usr �৹�~�i�H�ഫ bmw
  7. ��ĳ�ഫ���Ǭ� usr -> brd -> gem -> mf -> pal -> bmw
          
#endif


#include "bbs.h"

/* --------------------- �Ъ`�N �o���̥u��ܤ@�w�q ---------------------- */
#define  NEW_STATION                  /* �w�q�� �s��/�з� ���x            */
#undef   OLD_ATSVERSION               /* �w�q���ª��ȯ����� (1.20a �H��)  */
                                      /* �����ª����ϥγ̤p�ϥΪ̸�Ƶ��c */
                                      /* ���ݰ����ഫ�{�� single_multi_st */
#undef   TRANS_FROM_SOB               /* �w�q���F�y�ഫ                   */
#undef   TRANS_FROM_FB3               /* �w�q�������ഫ                   */
#undef   TRANS_FROM_COLA              /* �w�q���i���ഫ                   */
/* ---------------------------------------------------------------------- */

#ifdef   TRANS_FROM_FB3
  #undef ENCPASSLEN                   /* �w�q�Y�����ഫ ���L�]�w ENCPASSLEN */
#endif
#ifndef  OLD_ATSVERSION
#undef   MIN_USEREC_STRUCT            /* �w�q�ϥγ̤p�ϥΪ̸�Ƶ��c ���\�� */
#endif                                /* ����ĳ�ϥ� �|���h�\�h�\�� ���D��  */
                                      /* �x���Y���w�魭�� �B���Q�]�Ӧh�\�� */


#define OLD_BBSHOME     "/home/bbs/bbsrs"           /* SOB */
#define FN_PASSWD       "/home/bbs/bbsrs/.PASSWDS"  /* SOB */
#define FN_BOARD        "/home/bbs/bbsrs/.BOARDS"   /* SOB */


#undef HAVE_PERSONAL_GEM                       /* SOB �O�S���ӤH��ذϪ� */


#define ASTRLEN   80             /* Length of most string data */
#define ABTLEN    48             /* Length of board title */
#define ATTLEN    72             /* Length of title */
#define ANAMELEN  40             /* Length of username/realname */
#define AFNLEN    33             /* Length of filename  */
#define AIDLEN    12             /* Length of bid/uid */

  #define APASSLEN  14           /* Length of encrypted passwd field for ATS */

#define AREGLEN   38             /* Length of registration data */

/* ----------------------------------------------------- */
/* .PASSWDS struct : 1024 bytes                          */
/* ----------------------------------------------------- */

struct userec {
  char userid[AIDLEN + 1];
  char realname[20];
  char username[24];
  char passwd[APASSLEN];
  uschar uflag;
  usint userlevel;
  unsigned long int numlogins;
  unsigned long int numposts;
  time_t firstlogin;
  time_t lastlogin;
  char lasthost[80];
  char remoteuser[8];
  char email[50];
  char address[50];
  char justify[AREGLEN + 1];
  uschar month;
  uschar day;
  uschar year;
  uschar sex;
  uschar state;

  int havemoney;

#ifndef MIN_USEREC_STRUCT
  int havetoolsnumber;
  int havetools[20];
  int addexp;
  usint nowlevel;
  char working[20];

  uschar hp;
  uschar str;
  uschar mgc;
  uschar skl;
  uschar spd;
  uschar luk;
  uschar def;
  uschar mdf;

  uschar spcskl[6];
  uschar wepnlv[2][10];
  uschar curwepnlv[2][1];
  uschar curwepnatt;
  uschar curwepnhit;
  uschar curwepnweg;
  uschar curwepnspc[4];

  char lover[AIDLEN+1];
  char commander;
  char belong[21];
  char curwepclass[10];
  char class[7];
#endif

#ifndef NO_USE_MULTI_STATION
  char station[AIDLEN+1];
#endif

#ifndef MIN_USEREC_STRUCT
  char classsex;
  char wephavespc[5];

  char cmdrname[AIDLEN+1];
  char class_spc_skll[6];
#endif

  usint welcomeflag;

#ifndef MIN_USEREC_STRUCT
  int win;
  int lost;
  int test_exp;
#endif

  char tty_name[20];
  char extra_mode[10];

#ifndef MIN_USEREC_STRUCT
  char class_spc_test[32];
  char mov;
  char toki_level;

  int will_value;
  int effect_value;
  int belive_value;
  int leader_value;

  char action_value;
#endif

#ifndef NO_USE_MULTI_STATION
  char station_member[40];
#endif

  uschar now_stno;
  usint good_posts;

#ifdef CHANGE_USER_MAIL_LIMIT
  int max_mail_number;
  int max_mail_kbytes;
#endif

#ifdef TRANS_FROM_COLA
  usint staytime;
  #ifdef CHANGE_USER_MAIL_LIMIT
    int backup_int[41];
  #else
    int backup_int[43];
  #endif
#else
  #ifdef CHANGE_USER_MAIL_LIMIT
    int backup_int[42];
  #else
    int backup_int[44];
  #endif
#endif

#ifndef MIN_USEREC_STRUCT
  int ara_money;
#endif

#ifdef TRANS_FROM_COLA
  char blood;
  char normal_post;
  char backup_char[118];
#else
  char normal_post;
  char backup_char[119];
#endif

#ifndef MIN_USEREC_STRUCT
  int turn;
#endif
};
typedef struct userec userec;

/* ----------------------------------------------------- */
/* DIR of board struct                                   */
/* ----------------------------------------------------- */

#ifndef TRANS_FROM_FB3          /* struct size = 256 bytes */
  #ifndef TRANS_FROM_COLA
struct fileheader {
  char filename[AFNLEN-1];       /* M.109876543210.A */
  char report;                  /* Dopin : �s��� */
  char savemode;                /* file save mode */
  char owner[AIDLEN + 2];        /* uid[.] */
  char date[6];                 /* [02/02] or space(5) */
  char title[ATTLEN];
  uschar goodpost;              /* ���ˤ峹 */
  uschar filemode;              /* must be last field @ boards.c */
};
  #else
struct fileheader {             /* For Cola BBS */
  char filename[ASTRLEN];
  char owner[ASTRLEN];
  char title[ASTRLEN];
  char date[6];                 /* �ɤW for ATS/SOB */
  char savemode;
  uschar filemode;
  char report;
  uschar goodpost;
  char backup_char[6];          /* ��o�� */
};
  #endif
#else
struct fileheader {             /* This structure is used to hold data in */
  char filename[ASTRLEN-2];      /* the DIR files */
  char report;                  /* �ȯ����� */
  char savemode;                /* file save mode */
  char owner[ASTRLEN-6];
  char date[6];
  char title[ASTRLEN];
  unsigned level;
  unsigned char accessed[10];
  uschar goodpost;              /* ���ˤ峹 */
  uschar filemode;              /* must be last field @ boards.c */
};
#endif

typedef struct fileheader fileheader;


/* ----------------------------------------------------- */
/* BOARDS struct : Standard 656 bytes                    */
/* ----------------------------------------------------- */

struct boardheader {
  char brdname[(AIDLEN+1)*2];      /* bid */
  char title[ABTLEN + 1];
  char BM[AIDLEN * 3 + 3];       /* BMs' uid, token '/' */
  char pad[11];
  time_t bupdate;               /* note update time */
  char pad2[3];
  uschar bvote;                 /* Vote flags */
  time_t vtime;                 /* Vote close time */
  usint level;
  char document[128 * 3];       /* add extra document */
  char station[16];
  char sysop[16];
  char pastbrdname[16];
  char yankflags[16];
  char backup[64];
};
typedef struct boardheader boardheader;
