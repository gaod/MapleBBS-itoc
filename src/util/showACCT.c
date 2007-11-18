/*-------------------------------------------------------*/
/* util/showACCT.c      ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : ��ܨϥΪ̸��				 */
/* create : 01/07/16					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"


#undef	SHOW_PASSWORD		/* ��ܱK�X (�ܯӮɶ�) */


#ifdef SHOW_PASSWORD

#if 0
#  define GUESS_LEN	3	/* ���T�X(�t)�H�U���Ҧ��K�X�զX (�̦h�O PSWDLEN) */
#  define GUESS_START	' '
#  define GUESS_END	0x7f
#else
#  define GUESS_LEN	6	/* �����X(�t)�H�U���Ʀr�K�X�զX (�̦h�O PSWDLEN) */
#  define GUESS_START	'0'
#  define GUESS_END	'9'
#endif


static inline void
showpasswd(passwd)
  char *passwd;
{

  int i, index;
  char guess[PSWDLEN + 1];

  /* �L�׬O���� encrypt ����k�A���w�q GUESS_START �}�l try �� GUESS_END */

  memset(guess, 0, sizeof(guess));
  index = 0;
  guess[index] = GUESS_START;

  while (index < GUESS_LEN)
  {
    if (!chkpasswd(passwd, guess))
    {
      printf("�K�X: %s \n", guess);
      return;
    }

    for (i = index; i >= 0; i--)
    {
      if (guess[i] < GUESS_END)
      {
	guess[i]++;
	break;
      }
      else     /* �i�� */
      {
	guess[i] = GUESS_START;
      }
    }

    if (i < 0)	/* �Ҧ� index �쳣�է��F */
    {
      index++;
      guess[index] = GUESS_START;
      printf("���y�K�X���A�еy��...�]�w���� %d ��K�X�����y�^\n", index);
    }
  }
  printf("�K�X���׶W�L %d ��\n", GUESS_LEN);
}
#endif


static char *
_bitmsg(str, level)
  char *str;
  int level;
{
  int cc;
  int num;
  static char msg[33];

  num = 0;
  while (cc = *str)
  {
    msg[num] = (level & 1) ? cc : '-';
    level >>= 1;
    str++;
    num++;
  }
  msg[num] = 0;

  return msg;
}


static inline void
showACCT(acct)
  ACCT *acct;
{
  char msg1[40], msg2[40], msg3[40], msg4[40], msg5[40], msg6[40];

  strcpy(msg1, _bitmsg(STR_PERM, acct->userlevel));
  strcpy(msg2, _bitmsg(STR_UFO, acct->ufo));
  strcpy(msg3, Btime(&(acct->firstlogin)));
  strcpy(msg4, Btime(&(acct->lastlogin)));
  strcpy(msg5, Btime(&(acct->tcheck)));
  strcpy(msg6, Btime(&(acct->tvalid)));

  printf("> ------------------------------------------------------------------------------------------ \n"
    "�s��: %-15d [ID]: %-15s �m�W: %-15s �ʺ�: %-15s \n" 
    "�v��: %-37s �]�w: %-37s \n" 
    "ñ�W: %-37d �ʧO: %-15.2s \n" 
    "�ȹ�: %-15d ����: %-15d �ͤ�: %02d/%02d/%02d \n" 
    "�W��: %-15d �峹: %-15d �o�H: %-15d \n" 
    "����: %-37s �W��: %-30s \n" 
    "�ˬd: %-37s �q�L: %-30s \n" 
    "�n�J: %-30s \n" 
    "�H�c: %-60s \n", 
    acct->userno, acct->userid, acct->realname, acct->username, 
    msg1, msg2,
    acct->signature, "�H���" + (acct->sex << 1),
    acct->money, acct->gold, acct->year, acct->month, acct->day, 
    acct->numlogins, acct->numposts, acct->numemails, 
    msg3, msg4, 
    msg5, msg6, 
    acct->lasthost, 
    acct->email);

#ifdef SHOW_PASSWORD
  showpasswd(acct->passwd);
#endif
}


int
main(argc, argv)
  int argc;
  char **argv;
{
  int i;

  if (argc < 2)
  {
    printf("Usage: %s UserID1 [UserID2] ...\n", argv[0]);
    return -1;
  }

  chdir(BBSHOME);

  for (i = 1; i < argc; i++)
  {
    ACCT acct;
    char fpath[64];

    usr_fpath(fpath, argv[i], FN_ACCT);
    if (rec_get(fpath, &acct, sizeof(ACCT), 0) < 0)
    {
      printf("%s: read error (maybe no such id?)\n", argv[i]);
      continue;
    }

    showACCT(&acct);
  }
  printf("> ------------------------------------------------------------------------------------------ \n");

  return 0;
}
