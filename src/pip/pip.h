/* ----------------------------------------------------- */
/* pip.h     ( NTHU CS MapleBBS Ver 3.10 )               */
/* ----------------------------------------------------- */
/* target : �p�� data structure                          */
/* create : 01/08/16                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/* ----------------------------------------------------- */


#ifndef	_PIP_H_
#define	_PIP_H_


#if 0	/* ���v�ŧi */

  �ھڧڪ����ҡA�q�l�����̫e���O�� [�Ѫ��a�[ dsyan] �Ҽ��g�A
  ���ۦb [�����Ѱ� fennet] �⤤���F�@���ܰʡA
  ��Ӧb [�P�ŤU���]�� chiyuan] �j�睊�A�Φ��F�P�ž԰����C

  ��� [������ visor] �N�o�{�� port �� WindTopBBS �ӨϥΡA
  �ثe [�P�n�@�R itoc] �b�H�o���{������¦�U�A���F�j�T�ת��睊�C

  �U�ɮפw�g�Q�ڧ���a��L�F�A�b�Y�Ǥ譱���F�@�ǳ̨ΤơA
  �]�A�{�������s���g�B�s idea ���[�J�����A������n���ܰʻ�����U�G

  1) �N�@�Ӫ�U�檺�{���ҲդơA���U *.c ���A�H��b���ק�ɯ���K�C
  2) �N�U�{���H indent �ƪ��A�O�D�t�κ��@�̾\Ū�{�����K�Q�C
  3) struct ���ܰʡA�[�J�@�Ƿs�����C
  4) �԰�/�צ�[���ݩʤ������ܰʡC
  5) ���g���~���;����{���C
  6) ���g�a�ϲ��;����{���C
  7) ���s�гy�s���ޯ�[�c�C
  8) �s�W�@�Ƕüƨƥ�A���O�Ǩ�ޯ�άO�J�쯸�����C
  9) �s�W���Ȭ[�c�A�ɯŭn�ѥ��ȡC
 10) �ﵽ�Z���t�ΡA���C�ӤH���Z���h�ˤơC
 11) �Τ@�Ҧ����Φr�εe���B�z�C
 12) �[�J�j�q�����ѡC
 13) �j�T�״�֤����n����ø�C
 14) ��L..
  
  �Ʊ�o�ǧV�O�A�൹�z�a�ӳ\�h�K�Q�A�p�G������N���A�]�w��ӫH���СC

        �x�n�@�� �P�n�@�R  itoc.bbs@bbs.tnfsh.tn.edu.tw  2001.08.16

#endif

/* include �ɧ��R�W�� pipxxx.h   C �ɧ��R�W�� pip_xxx.c */
#include "pipglobal.h"
#include "pipstruct.h"


#define PIP_PICHOME	"etc/game/pip/"


#define LEARN_LEVEL	((d.happy+d.satisfy)/100)	/* �ǲ߮ĪG�P�ּ֤κ��������� */


/* itoc.010801: �q�X�̫�G�C�����O�C */
#define	out_cmd(cmd_1,cmd_2)	{ move(b_lines - 1, 0); clrtoeol(); outs(cmd_1); \
				move(b_lines, 0); clrtoeol(); outs(cmd_2); }

#endif		/* _PIP_H_ */
