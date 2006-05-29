/*-------------------------------------------------------*/
/* theme.h	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : custom theme				 */
/* create : 02/08/17				 	 */
/* update :   /  /  				 	 */
/*-------------------------------------------------------*/


#ifndef	_THEME_H_
#define	_THEME_H_


/* ----------------------------------------------------- */
/* ���C��w�q�A�H�Q�����ק�				 */
/* ----------------------------------------------------- */

#define COLOR1		"\033[1;37;44m"	/* footer/feeter ���e�q�C�� */
#define COLOR2		"\033[0;30;47m"	/* footer/feeter ����q�C�� */
#define COLOR3		"\033[30;47m"	/* neck ���C�� */
#define COLOR4		"\033[1;44m"	/* ���� ���C�� */
#define COLOR5		"\033[34;47m"	/* more ���Y�����D�C�� */
#define COLOR6		"\033[37;44m"	/* more ���Y�����e�C�� */
#define COLOR7		"\033[1m"	/* �@�̦b�u�W���C�� */


/* ----------------------------------------------------- */
/* �ϥΪ̦W���C��					 */
/* ----------------------------------------------------- */

#define COLOR_NORMAL	""		/* �@��ϥΪ� */
#define COLOR_MYBAD	"\033[1;31m"	/* �a�H */
#define COLOR_MYGOOD	"\033[1;32m"	/* �ڪ��n�� */
#define COLOR_OGOOD	"\033[1;33m"	/* �P�ڬ��� */
#define COLOR_CLOAK	"\033[1;35m"	/* ���� */	/* itoc.����: �S�Ψ�A�n���H�Цۦ�[�J ulist_body() */
#define COLOR_SELF	"\033[1;36m"	/* �ۤv */
#define COLOR_BOTHGOOD	"\033[1;37m"	/* ���]�n�� */
#define COLOR_BRDMATE	"\033[36m"	/* �O�� */


/* ----------------------------------------------------- */
/* ����m						 */
/* ----------------------------------------------------- */

/* itoc.����: �`�N MENU_XPOS �n >= MENU_XNOTE + MOVIE_LINES */

#define MENU_XNOTE	2		/* �ʺA�ݪO�� (2, 0) �}�l */
#define MOVIE_LINES	10		/* �ʵe�̦h�� 10 �C */

#define MENU_XPOS	13		/* ���}�l�� (x, y) �y�� */
#define MENU_YPOS	((d_cols >> 1) + 18)


/* ----------------------------------------------------- */
/* �T���r��G*_neck() �ɪ� necker ����X�өw�q�b�o	 */
/* ----------------------------------------------------- */

/* necker ����Ƴ��O�G��A�q (1, 0) �� (2, 80) */

/* �Ҧ��� XZ_* ���� necker�A�u�O���Ǧb *_neck()�A�����æb *_head() */

/* ulist_neck() �� xpost_head() ���Ĥ@�����S�O�A���b���w�q */

#define NECKER_CLASS	"\033[40m��\033[30;47m�ݪO\033[37;46m���峹�@���\033[36;40m��\033[37m          S)�Ƨ�  c)�s�峹�Ҧ�  v|V)�аO�wŪ|��Ū  h)����\033[m\n" \
			COLOR3 "  %s   ��  �O       ���O��H��   ��   ��   �z%*s              �H�� �O    �D%*s    \033[m"

#define NECKER_ULIST	"\n" \
			COLOR3 "  �s��  �N��         �ʺ�%*s                 %-*s               �ʺA        ���m \033[m"

#define NECKER_PAL	"\033[30;46m��\033[37m���y��\033[30;47m�n��\033[37;46m���W��\033[36;40m��\033[37m   a)�s�W  c)�ק�  d)�R��  s)��z  m)�H�H  w)���y  h)����\033[m\n" \
			COLOR3 "  �s��    �N ��         ��       ��%*s                                           \033[m"

#define NECKER_ALOHA	"\033[30;46m��\033[37m���y�@�n�͢�\033[30;47m�W��\033[37;40m�� a)�s�W  c)�ק�  d|D)�R��  f)�ޤJ  m)�H�H  w)���y  h)����\033[m\n" \
			COLOR3 "  �s��   �W �� �q �� �W ��%*s                                                    \033[m"

#define NECKER_VOTE	"\033[40m��\033[30;47m�벼\033[37;46m�����ߢ@�s�p\033[36;40m��\033[37m        R)���G  ^P|^Q)�|��|���  V)�w��  E)�s��  h)����\033[m\n" \
			COLOR3 "  �s��      �}����   �D��H       ��  ��  �v  ��%*s                              \033[m"

#define NECKER_BMW	"\033[40m��\033[30;47m���y\033[37;46m���n�͢@�W��\033[36;40m��\033[37m       s)��s  w)���y   m)�H�H  d|D)�R��  ��)�d��  h)����\033[m\n" \
			COLOR3 "  �s�� �N  ��       ��       �e%*s                                          �ɶ� \033[m"

#define NECKER_MF	"\033[40m��\033[30;47m�̷R\033[37;46m���s�D�@�I�q\033[36;40m��\033[37m  ^P)�s�W  d)�R��  c)����  C|^V)�ƻs|�K�W  m)����  h)����\033[m\n" \
			COLOR3 "  %s   ��  �O       ���O��H��   ��   ��   �z%*s              �H�� �O    �D%*s    \033[m"

#define NECKER_COSIGN	"\033[30;46m��\033[37m�벼�@���ߢ�\033[30;47m�s�p\033[37;40m��                           ^P)�o��  d)�R�� y)�[�J  h)����\033[m\n" \
			COLOR3 "  �s��   �� ��  �|��H       ��  �O  ��  �D%*s                                   \033[m"

#define NECKER_SONG	"\033[30;46m��\033[37m�̷R�@�s�D��\033[30;47m�I�q\033[37;40m��                      o|m)�I�q|�ݪO|�H�c  ��)�s��  h)����\033[m\n" \
			COLOR3 "  �s��     �D              �D%*s                            [�s      ��] [��  ��]\033[m"

#define NECKER_NEWS	"\033[30;46m��\033[37m�̷R��\033[30;47m�s�D\033[37;46m���I�q\033[36;40m��\033[37m                                          ��)�s��  h)����\033[m\n" \
			COLOR3 "  �s��    �� �� �@  ��       �s  �D  ��  �D%*s                                   \033[m"

#define NECKER_XPOST	"\n" \
			COLOR3 "  �s��    �� �� �@  ��       ��  ��  ��  �D%*s                            ��:%s  \033[m"

#define NECKER_MBOX	"\033[40m��\033[30;47m�l��\033[37;46m���Ȧ�@�b��\033[36;40m��\033[37m      d)�R��  R|y)�s��|�^�H  s)�H�H x|X)���|��F  h)����\033[m\n" \
			COLOR3 "  �s��   �� �� �@  ��       �H  ��  ��  �D%*s                                    \033[m"

#define NECKER_POST	"\033[30;46m��\033[37m�ݪO��\033[30;47m�峹\033[37;46m�����\033[36;40m��\033[37m          S|a|/)�j�M|�@��|���D  ^P)�o��  z)��ذ�  h)����\033[m\n" \
			COLOR3 "  �s��    �� �� �@  ��       ��  ��  ��  �D%*s                 ��:%s  �H��:%-4d  \033[m"

#define NECKER_GEM	"\033[30;46m��\033[37m�ݪO�@�峹��\033[30;47m���\033[37;40m��   %21.21s   B)�Ҧ�  C)�Ȧs  F)��H  h)����\033[m\n" \
			COLOR3 "  �s��     �D              �D%*s                            [�s      ��] [��  ��]\033[m"

/* �H�U�o�ǫh�O�@���� XZ_* ���c�� necker */

#define NECKER_VOTEALL	"\033[30;46m��\033[37m�벼��\033[30;47m����\033[37;46m���s�p\033[36;40m��\033[37m                                          ��)�벼  h)����\033[m\n" \
			COLOR3 "  �s��   ��  �O       ���O��H��   ��   ��   �z%*s                  �O    �D%*s     \033[m"

#define NECKER_CREDIT	"\033[30;46m��\033[37m�l���@�Ȧ梨\033[30;47m�b��\033[37;40m��\033[m\n" \
			COLOR3 "  �s��   ��  ��   ����  ��  �B  ����     ��  ��%*s                               \033[m"

#define NECKER_HELP	"\033[30;46m��\033[37m������\033[30;47m�A��\033[37;46m�����U\033[36;40m��\033[37m                  T)���D  E)�s��  m)����  ^P)�s�W  d)�R��\033[m\n" \
			COLOR3 "  �s��    �� ��         ��       �D%*s                                           \033[m"

#define NECKER_INNBBS	"\033[30;46m��\033[37m��H��\033[30;47m��H\033[37;46m����H\033[36;40m��\033[37m               ^P)�s�W  d)�R��  E�s��  /)�j�M  Enter)�Բ�\033[m\n" \
			COLOR3 "  �s��            ��         �e%*s                                               \033[m"


/* ----------------------------------------------------- */
/* �T���r��Gmore() �ɪ� footer ����X�өw�q�b�o	 */
/* ----------------------------------------------------- */

/* itoc.010914.����: ��@�g�A�ҥH�s FOOTER�A���O 78 char */

/* itoc.010821: �`�N \\ �O \�A�̫�O�|�F�@�Ӫť��� :p */

#define FOOTER_POST	\
COLOR1 "  �峹��Ū  " COLOR2 " (ry)�^�� (=\\[]<>-+;'`)�D�D (|?QA)�j�M���D�@�� (kj)�W�U�g (C)�Ȧs "

#define FOOTER_MAILER	\
COLOR1 "  ��������  " COLOR2 " (ry)�s��/�^�H (X)��F (d)�R�� (m)�аO (C)�Ȧs (=\\[]<>-+;'`|?QAkj)"

#define FOOTER_GEM	\
COLOR1 "  ��ؿ�Ū  " COLOR2 " (=\\[]<>-+;'`)�D�D (|?QA)�j�M���D�@�� (kj)�W�U�g (������)�W�U���} "

#ifdef HAVE_GAME
#define FOOTER_TALK	\
COLOR1 "  ��ͼҦ�  " COLOR2 " (^O)�﫳 (^C,^D)������� (^T)�����I�s�� (^Z)�ֱ��C�� (^G)�͹�    "
#else
#define FOOTER_TALK	\
COLOR1 "  ��ͼҦ�  " COLOR2 " (^C,^D)������� (^T)�����I�s�� (^Z)�ֱ��C�� (^G)�͹� (^Y)�M��    "
#endif

#define FOOTER_COSIGN	\
COLOR1 "  �s�p����  " COLOR2 " (ry)�[�J�s�p (kj)�W�U�g (������)�W�U���} (h)����                 " 

#define FOOTER_MORE	\
COLOR1 " �s�� P.%d (%d%%) " COLOR2 " (h)���� [PgUp][PgDn][0][$]���� (/n)�j�M (C)�Ȧs (��q)���� "

#define FOOTER_VEDIT	\
COLOR1 "  %s  " COLOR2 " (^Z)���� (^W)�Ÿ� (^L)��ø (^X)�ɮ׳B�z ��%s�x%s��%5d:%3d  \033[m"


/* ----------------------------------------------------- */
/* �T���r��Gxo_foot() �ɪ� feeter ����X�өw�q�b�o      */
/* ----------------------------------------------------- */


/* itoc.010914.����: �C��h�g�A�ҥH�s FEETER�A���O 78 char */

#define FEETER_CLASS	\
COLOR1 "  �ݪO���  " COLOR2 " (c)�s�峹 (vV)�wŪ��Ū (y)�����C�X (z)��q (A)����j�M (S)�Ƨ�   "

#define FEETER_ULIST	\
COLOR1 "  ���ͦC��  " COLOR2 " (f)�n�� (t)��� (q)�d�� (ad)��� (w)���y (s)��s (TAB)����       "

#define FEETER_PAL	\
COLOR1 "  �I�B�ަ�  " COLOR2 " (a)�s�W (d)�R�� (c)�ͽ� (m)�H�H (f)�ޤJ�n�� (r^Q)�d�� (s)��s    "

#define FEETER_ALOHA	\
COLOR1 "  �W���q��  " COLOR2 " (a)�s�W (d)�R�� (D)�Ϭq�R�� (f)�ޤJ�n�� (r^Q)�d�� (s)��s        "

#define FEETER_VOTE	\
COLOR1 " �ݪO�벼 " COLOR2 " (��/r/v)�벼 (R)���G (^P)�s�W�벼 (E)�ק� (V)�w�� (b)�}�� (o)�W��  "

#define FEETER_BMW	\
COLOR1 "  ���y�^�U  " COLOR2 " (d)�R�� (D)�Ϭq�R�� (m)�H�H (w)���y (^R)�^�T (^Q)�d�� (s)��s    "

#define FEETER_MF	\
COLOR1 "  �̷R�ݪO  " COLOR2 " (^P)�s�W (Cg)�ƻs (p^V)�K�W (d)�R�� (c)�s�峹 (vV)�аO�wŪ/��Ū  "

#define FEETER_COSIGN	\
COLOR1 " �s�p�p�� " COLOR2 " (r)Ū�� (y)�^�� (^P)�o�� (d)�R�� (o)�}�O (c)���� (E)�s�� (B)�]�w   "

#define FEETER_SONG	\
COLOR1 "  �I�q�t��  " COLOR2 " (r)Ū�� (o)�I�q��ݪO (m)�I�q��H�c (E)�s���ɮ� (T)�s����D      "

#define FEETER_NEWS	\
COLOR1 "  �s�D�I��  " COLOR2 " (��/��)�W�U (PgUp/PgDn)�W�U�� (Home/End)���� (��r)��� (��)���}  "

#define FEETER_XPOST	\
COLOR1 "  ��C�j�M  " COLOR2 " (y)�^�� (x)��� (m)�аO (d)�R�� (^P)�o�� (^Q)�d�ߧ@�� (t)����    "

#define FEETER_MBOX	\
COLOR1 "  �H�H�۱�  " COLOR2 " (y)�^�H (F/X/x)��H/��F/��� (d)�R�� (D)�Ϭq�R�� (m)�аO        "

#define FEETER_POST	\
COLOR1 "  �峹�C��  " COLOR2 " (ry)�^�H (S/a)�j�M/���D/�@�� (~G)��C (x)��� (V)�벼 (u)�s�D    "

#define FEETER_GEM	\
COLOR1 "  �ݪO���  " COLOR2 " (^P/a/f)�s�W/�峹/�ؿ� (E)�s�� (T)���D (m)���� (c)�ƻs (p^V)�K�W "

#define FEETER_VOTEALL	\
COLOR1 "  �벼����  " COLOR2 " (��/��)�W�U (PgUp/PgDn)�W�U�� (Home/End)���� (��)�벼 (��)���}   "

#define FEETER_HELP	\
COLOR1 "  �������  " COLOR2 " (��/��)�W�U (PgUp/PgDn)�W�U�� (Home/End)���� (��r)�s�� (��)���}  "

#define FEETER_INNBBS	\
COLOR1 "  ��H�]�w  " COLOR2 " (��/��)�W�U (PgUp/PgDn)�W�U�� (Home/End)���� (��)(q)���}         "


/* ----------------------------------------------------- */
/* ���x�ӷ�ñ�W						 */
/* ----------------------------------------------------- */

/* itoc: ��ĳ banner ���n�W�L�T��A�L������ñ�i��|�y���Y�ǨϥΪ̪��ϷP */

#define EDIT_BANNER	"\n--\n" \
			" \033[1;41m �� �x \033[40;33m "SCHOOLNAME"��"BBSNAME" \033[35m�m"MYHOSTNAME"�n\033[m\n" \
			" \033[1;42m �� �� \033[40;36m %.0s%s\033[m\n"

#define MODIFY_BANNER	" \033[1;44m �s �� \033[40;37m %.0s%s\033[m\n"


/* ----------------------------------------------------- */
/* ��L�T���r��						 */
/* ----------------------------------------------------- */

#define VMSG_NULL	"                                           \033[1;36m �j�k�l�m�n�o�p �Ы����N���~�� �p\033[m"

#define ICON_UNREAD_BRD		"\033[33m��"		/* ��Ū�ݪO */
#define ICON_READ_BRD		"  "			/* �wŪ�ݪO */

#define ICON_GAMBLED_BRD	"\033[1;31m��\033[m"	/* �|���L�����ݪO */
#define ICON_VOTED_BRD		"\033[1;33m��\033[m"	/* �|��벼�����ݪO */
#define ICON_NOTRAN_BRD		"��"			/* ����H�O */
#define ICON_TRAN_BRD		"��"			/* ��H�O */

#define TOKEN_ZAP_BRD		'-'			/* zap �O */
#define TOKEN_FRIEND_BRD	'.'			/* �n�ͪO */
#define TOKEN_SECRET_BRD	')'			/* ���K�O */

#endif				/* _THEME_H_ */
