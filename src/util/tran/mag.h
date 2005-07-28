/*-------------------------------------------------------*/
/* util/mag.h						 */
/*-------------------------------------------------------*/
/* target : Magic �� Maple 3.02 �ϥΪ��ഫ		 */
/* create : 02/01/03					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

  1. �]�w FN_PASSWD�BFN_BOARD�BOLD_MAILPATH�BOLD_BOARDPATH�BOLD_MANPATH
  2. �ק�Ҧ��� old struct

  3. �� �¯��� mail ���� OLD_MAILPATH
  4. �� �¯��� boards/brdname ���� OLD_BOARDPATH/brdname
  5. �� �¯��� 0Announce/groups/ooxx.faq/brdname ���� OLD_MANPATH/brdname

  6. �����b brd �৹�~�i�H�ഫ gem
  7. ��ĳ�ഫ���Ǭ� usr -> brd -> gem

#endif

      
#include "bbs.h"


#define FN_PASSWDS      "/home/oldbbs/.PASSWDS"
#define FN_BOARDS       "/home/oldbbs/.BOARDS"
#define OLD_MAILPATH    "/home/oldbbs/mail"
#define OLD_BOARDPATH   "/home/oldbbs/boards"
#define OLD_MANPATH     "/home/oldbbs/groups"


/* ----------------------------------------------------- */
/* old .PASSWDS struct : 512 bytes                       */
/* ----------------------------------------------------- */


typedef struct
{
  char userid[14];
  time_t firstlogin;
  char termtype[16];
  unsigned int numlogins;
  unsigned int numposts;
  char flags[2];
  char passwd[14];
  char username[40];
  char ident[40];
  char lasthost[40];
  char realemail[40];
  unsigned userlevel;
  time_t lastlogin;
  time_t stay;
  char realname[40];
  char address[80];
  char email[80];
  int signature;
  unsigned int userdefine;
  int editor;
  unsigned int showfile;
  int magic;
  int addmagic;
  uschar bmonth;
  uschar bday;
  uschar byear;
  uschar sex;
  int money;
  int bank;
  int lent;

  int card;
  uschar mind;
  int unused1;
  usint unsign_0000;
  usint unsign_ffff;
}	userec;


/* ----------------------------------------------------- */
/* old DIR of board struct : 256 bytes			 */
/* ----------------------------------------------------- */


typedef struct		/* the DIR files */
{
  char filename[80];
  char owner[80];
  char title[80];
  unsigned level;
  unsigned char accessed[12];
}	fileheader;


/* ----------------------------------------------------- */
/* old BOARDS struct : 256 bytes			 */
/* ----------------------------------------------------- */


typedef struct		/* the BOARDS files */
{
  char filename[80];
  char owner[60];
  char BM[19];
  char flag;
  char title[80];
  unsigned level;
  unsigned char accessed[12];
}	boardheader;
