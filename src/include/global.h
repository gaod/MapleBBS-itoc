/*-------------------------------------------------------*/
/* global.h	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : global definitions & variables		 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#ifndef	_GLOBAL_H_
#define	_GLOBAL_H_


#ifdef	_MAIN_C_
# define VAR
# define INI(x)		= x
#else
# define VAR		extern
# define INI(x)
#endif


/* ----------------------------------------------------- */
/* GLOBAL DEFINITION					 */
/* ----------------------------------------------------- */


/* itoc.010821: �R�W��h: �ӤH�֦���(�p�ӤH/�ݪO/��ذϩҿW�۾֦���)�פ� FN_XXXX
   �t�ξ֦��̡A�Y�O���ܰʪ����A�פ� FN_ETC_XXXX�F�Y�O�ܰ��ɮסA�פ� FN_RUN_XXXX */


/* ----------------------------------------------------- */
/* �ӤH�ؿ��ɦW�]�w                                      */
/* ----------------------------------------------------- */


#define	FN_ACCT		".ACCT"		/* User account */
#define FN_BRH		".BRH"		/* board reading history */
#define FN_CZH		".CZH"		/* class zap history */

#define	FN_PLANS	"plans"		/* �p�e�� */
#define	FN_SIGN		"sign"		/* ñ�W��.? */

#define FN_LOG		"log"		/* �W���ӷ��O�� */
#define FN_JUSTIFY	"justify"	/* �����{�Ҹ�� */
#define FN_EMAIL	"email"		/* �{�ҧ���^�H�O�� */
#define FN_ACL		"acl"		/* �W���a�I�]�w */

#define FN_BMW		"bmw"		/* ���y�O�� Binary Message Write */
#define FN_AMW		"amw"		/* ���y�O�� ASCII Message Write */
#define	FN_PAL		"friend"	/* �B�ͦW�� (�x�s�b�ۤv�ؿ��A�O�������ǤH) */

#ifdef HAVE_LIST
#define FN_LIST		"list"		/* �S��W��.? */
#endif

#ifdef LOGIN_NOTIFY
#define FN_BENZ		"benz"		/* �t�Ψ�M�W���q�� */
#endif

#ifdef  HAVE_ALOHA
#define FN_ALOHA	"aloha"		/* �W���q�� (�x�s�b�ۤv�ؿ��A�O�������ǤH) */
#define FN_FRIENZ	"frienz"	/* �W���q�� (�x�s�b���ؿ��A���W����Ĳ�o) */
#endif

#define FN_PAYCHECK	"paycheck"	/* �䲼 */

#ifdef LOG_TALK
#define FN_TALK_LOG	"talk.log"	/* ��ѰO���� */
#endif

#ifdef MY_FAVORITE
#define FN_MF		"@MyFavorite"	/* �ڪ��̷R */
#endif

#ifdef HAVE_CLASSTABLE
#define FN_CLASSTBL	"classtable"	/* �\�Ҫ� */
#define FN_CLASSTBL_LOG	"classtable.log"/* �\�Ҫ� */
#endif

#ifdef HAVE_CREDIT
#define FN_CREDIT	"credit"	/* �O�b�� */
#endif

#ifdef HAVE_CALENDAR
#define FN_TODO		"todo"		/* ��ƾ� */
#endif


/* ----------------------------------------------------- */
/* �ݪO/��ذ�/�H�c�ɦW�]�w                              */
/* ----------------------------------------------------- */


#define FN_DIR		".DIR"		/* index */
#define FN_VCH		".VCH"		/* vote control header */
#define FN_NOTE		"note"		/* �i�O�e�� */


/* ----------------------------------------------------- */
/* �t���ɦW�]�w                                          */
/* ----------------------------------------------------- */

  /* --------------------------------------------------- */
  /* �ڥؿ��U�t���ɮ�                                    */
  /* --------------------------------------------------- */


#define FN_BRD		".BRD"		/* board list */
#define FN_SCHEMA	".USR"		/* userid schema */


  /* --------------------------------------------------- */
  /* run/ �ؿ��U�t���ɮ�                                 */
  /* --------------------------------------------------- */


#define	FN_RUN_USIES	"run/usies"	/* BBS log */
#define FN_RUN_NOTE_ALL	"run/note.all"	/* �d���O */
#define FN_RUN_PAL	"run/pal.log"	/* �B�ͶW�L�W���O�� */

#define FN_RUN_ADMIN	"run/admin.log"	/* �����欰�O�� */

#ifdef LOG_SONG_USIES
#define FN_RUN_SONGUSIES "run/song_usies" /* �I�q�O�� */
#endif

#ifdef HAVE_REGISTER_FORM
#define FN_RUN_RFORM	"run/rform"	/* ���U��� */
#define FN_RUN_RFORM_LOG "run/rform.log" /* ���U���f�ְO���� */
#endif

#ifdef MODE_STAT
#define FN_RUN_MODE_LOG	"run/mode.log"	/* �ϥΪ̰ʺA�έp - record per hour */
#define FN_RUN_MODE_CUR	"run/mode.cur"
#define FN_RUN_MODE_TMP	"run/mode.tmp"
#endif

#ifdef HAVE_ANONYMOUS
#define FN_RUN_ANONYMOUS "run/anonymous" /* �ΦW�o��峹�O�� */
#endif

#ifdef HAVE_BUY
#define FN_RUN_BANK_LOG	"run/bank.log"	/* �׿��O�� */
#endif

#ifdef HAVE_SIGNED_MAIL
#define FN_RUN_PRIVATE	"run/prikey"	/* �q�lñ�� */
#endif

#define FN_RUN_EMAILREG	"run/emailreg"	/* �O���Ψӻ{�Ҫ��H�c */
#define FN_RUN_MAIL_LOG	"run/mail.log"	/* �H�H���O�� */

#define FN_RUN_POST	"run/post"	/* �峹�g�Ʋέp */
#define	FN_RUN_POST_LOG	"run/post.log"	/* �峹�g�Ʋέp */

/* reaper �Ҳ��ͪ� log */
#define FN_RUN_LAZYBM	"run/lazybm"	/* ���i�O�D�έp */
#define FN_RUN_MANAGER	"run/manager"	/* �S���v���ϥΪ̦C�� */
#define FN_RUN_REAPER	"run/reaper"	/* �������W���Q�M�����ϥΪ̦C�� */
#define FN_RUN_EMAILADDR "run/emailaddr" /* �P�@ email �{�Ҧh���C�� */

#define BMTA_LOGFILE	"run/bmta.log"	/* ���~���H���O�� */


  /* --------------------------------------------------- */
  /* etc/ �ؿ��U�t���ɮ�				 */
  /* --------------------------------------------------- */


#define FN_ETC_EXPIRE	"etc/expire.conf" 	/* �ݪO�峹�g�ƤW���]�w */

#define FN_ETC_VALID	"etc/valid"		/* �����{�ҫH�� (Email �{�ҮɡA�H�h�����~�H�c) */
#define FN_ETC_JUSTIFIED "etc/justified"	/* �{�ҳq�L�q�� (�{�ҳq�L�ɡA�H�쯸���H�c) */
#define FN_ETC_REREG	"etc/re-reg"		/* ���s�{�ҳq�� (�{�ҹL���ɡA�H�쯸���H�c) */

#define FN_ETC_CROSSPOST "etc/cross-post"	/* ��K���v�q�� (Cross-Post �ɡA�H�쯸���H�c) */

#define FN_ETC_BADID	"etc/badid"		/* �����W�� (�ڵ����U ID) */
#define FN_ETC_SYSOP	"etc/sysop"		/* ���ȦW�� */

#define FN_ETC_FEAST	"etc/feast"		/* �`�� */

#define FN_ETC_HOST	"etc/host"		/* �G�m IP */
#define FN_ETC_FQDN	"etc/fqdn"		/* �G�m FQDN */

#define FN_ETC_TIP	"etc/tip"		/* �C��p���Z */

#define FN_ETC_LOVELETTER "etc/loveletter"	/* ���Ѳ��;���w */


  /* --------------------------------------------------- */
  /* etc/ �ؿ��U access crontrol list (ACL)		 */
  /* --------------------------------------------------- */

#define TRUST_ACLFILE	"etc/trust.acl"		/* �{�ҥզW�� */
#define UNTRUST_ACLFILE	"etc/untrust.acl"	/* �{�Ҷ¦W�� */

#define MAIL_ACLFILE	"etc/mail.acl"		/* ���H�զW�� */
#define UNMAIL_ACLFILE	"etc/unmail.acl"	/* ���H�¦W�� */

#define BBS_ACLFILE	"etc/bbs.acl"		/* �ڵ� telnet �s�u�W�� */


/* ----------------------------------------------------- */
/* �U�ӪO���ɦW�]�w                                      */
/* ----------------------------------------------------- */


#if 0	/* itoc.000512: �t�άݪO�A�w�]�p�g�A�j�g�L�� */
�ݪO        ��   ��   ��   �z       �\Ū�v��      �o���v��
sysop       ���� ���i�����E���ڳ��� 0             0
0announce   ���� �^�ѩӹB�E�����@�� 0             PERM_ALLADMIN
test        ���� ���ձM�ϡE�s����z 0             PERM_BASIC
note        ���� �ʺA�ݪO�E�]���ܻy 0             PERM_POST
newboard    ���� �}�P�M��E�s�p���a 0             PERM_POST
ktv         ���� �I�q�O���E�u����� 0             PERM_SYSOP
record      ���� �Ĳ��W���E�t�ΰO�� 0             PERM_SYSOP
deleted     ���� �峹�@�ϡE�귽�^�� PERM_BM       PERM_SYSOP
bm          ���� �M�~�Q�סE�O�D��� PERM_BM       0
admin       ���� �c�d�Ѧa�E�����ۺN PERM_ALLADMIN 0
log         ���� �t�ΫO�I�E�w���O�� PERM_ALLADMIN PERM_SYSOP
junk        ���� �峹�M�z�E�U�����I PERM_ALLBOARD PERM_SYSOP
UnAnonymous ���� �¨纡�ѡE�ΦW�{�� PERM_ALLBOARD PERM_SYSOP

�䤤����Ū�����ݪO�b Class �������n�]�w ��ƫO�K
#endif

/* �H�U�O�@�w�n�����t�άݪO�A���O�@�ӪO�i�H���ШϥΦh���A�Ҧp��Z�q�����M�ʺA�ݪO�@�� note �O */

#define BN_CAMERA	"note"		/* �ʺA�ݪO��b�o�O����ذ� */
#define BN_ANNOUNCE	"0announce"	/* ���i�ݪO�A�j���\Ū */
#define BN_JUNK		"junk"		/* �ۤv�R�����峹��b�� */
#define BN_DELETED	"deleted"	/* �O�D�R�����峹��b�� */
#define BN_SECURITY	"log"		/* �t�Φw���O�� */
#define BN_RECORD	"record"	/* �t�Ϊ��@��O�� */
#define BN_UNANONYMOUS	"UnAnonymous"	/* �ΦW�O���峹�|�ƻs�@���b�o�� */
#define BN_KTV		"ktv"		/* �I�q�O����b�o�O�A�q����b�o�O����ذ� */
#define BN_REQUEST	BN_CAMERA	/* �q����Z�B */

#define BN_NULL		"�|����w"	/* �i�����٨S�i�J����ݪO�A����ܪ��ݪO�W�� */


/* ----------------------------------------------------- */
/* ��L�]�w                                              */
/* ----------------------------------------------------- */


#define KEY_BKSP	8	/* �M Ctrl('H') �ۦP */
#define KEY_TAB		9	/* �M Ctrl('I') �ۦP */
#define KEY_ENTER	10	/* �M Ctrl('J') �ۦP */
#define KEY_ESC		27
#define KEY_UP		-1
#define KEY_DOWN	-2
#define KEY_RIGHT	-3
#define KEY_LEFT	-4
#define KEY_HOME	-21
#define KEY_INS		-22
#define KEY_DEL		-23
#define KEY_END		-24
#define KEY_PGUP	-25
#define KEY_PGDN	-26


#define I_TIMEOUT	-31
#define I_OTHERDATA	-32


#define Ctrl(c)		(c & '\037')
#define Esc(c)		(c)		/* itoc.030824: �� TRAP_ESC */
#define isprint2(c)	(c >= ' ')


#if 0	/* itoc.020108: ��������� */

  int HEX = vkey('KEY');
  �o��O�� vkey() ��J��L�A�ǥX����ƭȹ�����C


/* �ȬO�t�� */

KEY_INS   ffffffea	KEY_DEL   ffffffe9
KEY_HOME  ffffffeb	KEY_END   ffffffe8
KEY_PGUP  ffffffe7	KEY_PGDN  ffffffe6

KEY_UP    ffffffff	KEY_DOWN  fffffffe
KEY_RIGHT fffffffd	KEY_LEFT  fffffffc

/* �ȬO���� */

�z�w�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�s�w�{
�x HEX�x00�x01�x02�x03�x04�x05�x06�x07�x08�x09�x0a�x0b�x0c�x0d�x0e�x0f�x
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x KEY�x  �x^A�x^B�x^C�x^D�x^E�x^F�x^G�x^H�x^I�x^J�x^K�x^L�x^M�x^N�x^O�x
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x HEX�x10�x11�x12�x13�x14�x15�x16�x17�x18�x19�x1a�x1b�x1c�x1d�x1e�x1f�x
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x KEY�x^P�x^Q�x^R�x^S�x^T�x^U�x^V�x^W�x^X�x^Y�x^Z�xEs�x  �x  �x  �x  �x
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x HEX�x20�x21�x22�x23�x24�x25�x26�x27�x28�x29�x2a�x2b�x2c�x2d�x2e�x2f�x	/* 0x22 �O���޸��A�קK compile ���~ */
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x KEY�x  �x !�xXX�x #�x $�x %�x &�xXX�x (�x )�x *�x  �x ,�x -�x .�x /�x	/* 0x27 �O��޸��A�קK compile ���~ */
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x HEX�x30�x31�x32�x33�x34�x35�x36�x37�x38�x39�x3a�x3b�x3c�x3d�x3e�x3f�x
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x KEY�x 0�x 1�x 2�x 3�x 4�x 5�x 6�x 7�x 8�x 9�x :�x ;�x <�x  �x >�x ?�x
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x HEX�x40�x41�x42�x43�x44�x45�x46�x47�x48�x49�x4a�x4b�x4c�x4d�x4e�x4f�x
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x KEY�x @�x A�x B�x C�x D�x E�x F�x G�x H�x I�x J�x K�x L�x M�x N�x O�x
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x HEX�x50�x51�x52�x53�x54�x55�x56�x57�x58�x59�x5a�x5b�x5c�x5d�x5e�x5f�x
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x KEY�x P�x Q�x R�x S�x T�x U�x V�x W�x X�x Y�x Z�x [�x \�x ]�x ^�x _�x
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x HEX�x60�x61�x62�x63�x64�x65�x66�x67�x68�x69�x6a�x6b�x6c�x6d�x6e�x6f�x
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x KEY�x `�x a�x b�x c�x d�x e�x f�x g�x h�x i�x j�x k�x l�x m�x n�x o�x
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x HEX�x70�x71�x72�x73�x74�x75�x76�x77�x78�x79�x7a�x7b�x7c�x7d�x7e�x7f�x
�u�w�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�q�w�t
�x KEY�x p�x q�x r�x s�x t�x u�x v�x w�x x�x y�x z�x {�x |�x }�x  �x  �x
�|�w�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�r�w�}

/* �ȭ��� */

KEY_BKSP == Ctrl('H') == 0x08
'\t' == Ctrl('I') == 0x09
'\n' == Ctrl('J') == 0x0a
'\r' == Ctrl('M') == 0x0d

#endif


/* ----------------------------------------------------- */
/* �T���r��G�W�ߥX�ӡA�H�Q�䴩�U�ػy��			 */
/* ----------------------------------------------------- */


#define QUOTE_CHAR1	'>'
#define QUOTE_CHAR2	':'

#define	STR_SPACE	" \t\n\r"

#define	STR_AUTHOR1	"�@��:"
#define	STR_AUTHOR2	"�o�H�H:"
#define	STR_POST1	"�ݪO:"
#define	STR_POST2	"����:"
#define	STR_REPLY	"Re: "
#define	STR_FORWARD	"Fw: "

#define STR_LINE	"\n\
> -------------------------------------------------------------------------- <\n\n"

#define LEN_AUTHOR1	(sizeof(STR_AUTHOR1) - 1)
#define LEN_AUTHOR2	(sizeof(STR_AUTHOR2) - 1)

#define STR_SYSOP	"sysop"
#define STR_GUEST	"guest"
#define STR_NEW		"new"

#define STR_ANONYMOUS	"�������H��"		/* �n�u�� IDLEN �Ӧr */

#define MSG_SEPERATOR	"\
�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w"

#define MSG_CANCEL	"����"
#define MSG_USR_LEFT	"���w�g���h"
#define MSG_XY_NONE	"�ŵL�@��"

#define MSG_USERPERM	"�v�����šG"
#define MSG_READPERM	"�\\Ū�v���G"
#define MSG_POSTPERM	"�o���v���G"
#define MSG_BRDATTR	"�ݪO�ݩʡG"
#define MSG_USERUFO	"�ߺD�X�СG             �� / ��                                 �� / ��"

#define MSG_XYPOST1	"���D����r�G"
#define MSG_XYPOST2	"�@������r�G"

#define MSG_DEL_OK	"�R������"
#define MSG_DEL_CANCEL	"�����R��"
#define MSG_DEL_ERROR	"�R�����~"
#define MSG_DEL_NY	"�нT�w�R��(Y/N)�H[N] "

#define MSG_SURE_NY	"�бz�T�w(Y/N)�H[N] "
#define MSG_SURE_YN	"�бz�T�w(Y/N)�H[Y] "

#define MSG_MULTIREPLY	"�z�T�w�n�s�զ^�H(Y/N)�H[N] "

#define MSG_BID		"�п�J�ݪO�W�١G"
#define MSG_UID		"�п�J�N���G"
#define MSG_PASSWD	"�п�J�K�X�G"

#define ERR_BID		"���~���ݪO�W��"
#define ERR_UID		"���~���ϥΪ̥N��"
#define ERR_PASSWD	"�K�X��J���~"
#define ERR_EMAIL	"���X�檺 E-mail address"

#define MSG_SENT_OK	"�H�w�H�X"

#define MSG_LIST_OVER	"�z���W��Ӧh�A�е��[��z"

#define MSG_COINLOCK	"�z����H�����i��o�˪��A��"
#define MSG_REG_VALID	"�z�w�g�q�L�����{�ҡA�Э��s�W��"

#define	MSG_LL		"\033[32m[�s�զW��]\033[m\n"
#define MSG_DATA_CLOAK	"<��ƫO�K>\n"

#define MSG_CHKDATA	"�� ��ƾ�z�]�֤��A�еy�� \033[5m...\033[m"
#define MSG_QUITGAME	"�����F�ڡH�U���A�Ӯ@�I ^_^"

#define MSG_CHAT_ULIST	\
"\033[7m �ϥΪ̥N��    �ثe���A  �x �ϥΪ̥N��    �ثe���A  �x �ϥΪ̥N��    �ثe���A \033[m"


/* ----------------------------------------------------- */
/* GLOBAL VARIABLE					 */
/* ----------------------------------------------------- */


VAR int bbsmode;		/* bbs operating mode, see modes.h */
VAR usint bbstate;		/* bbs operatine state */

VAR ACCT cuser;			/* current user structure */
VAR UTMP *cutmp;		/* current user temporary */

VAR time_t ap_start;		/* �i���ɨ� */
VAR int total_user;		/* �ϥΪ̦ۥH�������W�H�� */

VAR int b_lines;		/* bottom line */
VAR int b_cols;			/* bottom columns */
VAR int d_cols;			/* difference columns from standard */

VAR char fromhost[48];		/* from FQDN */

VAR char ve_title[80];		/* edited title */
VAR char quote_file[80];
VAR char quote_user[80];
VAR char quote_nick[80];

VAR char hunt[32];		/* hunt keyword */

VAR int curredit;		/* current edit mode */
VAR time_t currchrono;		/* current file timestamp */
VAR char currtitle[80];		/* currently selected article title */

VAR int currbno;		/* currently selected board bno */
VAR usint currbattr;		/* currently selected board battr */
VAR char currboard[BNLEN + 1];	/* currently selected board brdname */
VAR char currBM[BMLEN + 7];	/* currently selected board BM */	/* BMLEN + 1 + strlen("�O�D�G") */

/* filename */

VAR char *fn_acct	INI(FN_ACCT);
VAR char *fn_dir	INI(FN_DIR);
VAR char *fn_bmw	INI(FN_BMW);
VAR char *fn_amw	INI(FN_AMW);
VAR char *fn_pal	INI(FN_PAL);
VAR char *fn_plans	INI(FN_PLANS);
VAR char *fn_note	INI(FN_NOTE);


/* message */

VAR char *msg_seperator	INI(MSG_SEPERATOR);

VAR char *msg_cancel	INI(MSG_CANCEL);

VAR char *msg_sure_ny	INI(MSG_SURE_NY);

VAR char *msg_uid	INI(MSG_UID);

VAR char *msg_del_ny	INI(MSG_DEL_NY);

VAR char *err_bid	INI(ERR_BID);
VAR char *err_uid	INI(ERR_UID);
VAR char *err_email	INI(ERR_EMAIL);

VAR char *msg_sent_ok	INI(MSG_SENT_OK);

VAR char *msg_list_over	INI(MSG_LIST_OVER);

VAR char *msg_reg_valid	INI(MSG_REG_VALID);
VAR char *msg_coinlock	INI(MSG_COINLOCK);

VAR char *str_sysop	INI(STR_SYSOP);
VAR char *str_author1	INI(STR_AUTHOR1);
VAR char *str_author2	INI(STR_AUTHOR2);
VAR char *str_post1	INI(STR_POST1);
VAR char *str_post2	INI(STR_POST2);
VAR char *str_host	INI(MYHOSTNAME);
VAR char *str_site	INI(BBSNAME);
VAR char *str_line	INI(STR_LINE);

VAR char *str_ransi	INI("\033[m");

#undef	VAR
#undef	INI

#endif				/* _GLOBAL_H_ */
