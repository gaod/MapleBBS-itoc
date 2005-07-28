/*-------------------------------------------------------*/
/* util/cola.h						 */
/*-------------------------------------------------------*/
/* target : Cola �� Maple 3.02 �ഫ			 */
/* create : 03/02/11                                     */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

  0. �Х��� Cola ��Ҧ��ݪO�^��W�r��b 12 �r�H���C

  1. �����b brd �৹�~�i�H�ഫ gem
  2. ��ĳ�ഫ���Ǭ� usr -> brd -> gem -> post

  3. �]�w COLABBS_HOME�BCOLABBS_BOARDS�BCOLABBS_MAN�BFN_BOARD

#endif


#include "bbs.h"


#define	COLABBS_HOME	"/tmp/home"		/* �ª� Cola BBS ���ϥΪ̥ؿ� */
#define COLABBS_BOARDS	"/tmp/boards"		/* �ª� Cola BBS ���ݪO�ؿ� */
#define COLABBS_MAN	"/tmp/man"		/* �ª� Cola BBS ����ذϥؿ� */
#define FN_BOARD	"/tmp/.boards"		/* �ª� Cola BBS �� .boards */


/* ----------------------------------------------------- */
/* old .PASSWDS struct : 512 bytes                       */
/* ----------------------------------------------------- */

/* itoc.030211: �u�ഫ userid �M passwd */
typedef struct
{
  char userid[13];
  char blank1;
  char passwd[14];
  char username[40];
  char realname[80];
  char blank2[364];
} userec;


/* ----------------------------------------------------- */
/* old DIR of board struct : 128 bytes                   */
/* ----------------------------------------------------- */

typedef struct
{
  char filename[34];		/* M.9876543210.A */
  char blank1[46];
  char owner[14];		/* userid[.] */
  char blank2[57];
  char date[6];			/* [08/27] or space(5) */
  char title[74];
  char blank3[25];
} fileheader;


/* ----------------------------------------------------- */
/* old BOARDS struct : 512 bytes                         */
/* ----------------------------------------------------- */

/* itoc.030211: �u�ഫ brdname �M title */
typedef struct
{
  char brdname[13];
  char blank1[147];
  char title[96];
} boardheader;
