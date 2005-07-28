/*-------------------------------------------------------*/
/* util/setusr.c	( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : �]�w�ϥΪ̸��                               */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/* create : 01/07/16                                     */
/* update :                                              */
/*-------------------------------------------------------*/


#if 0
  �]�w itoc �� �m�W�����j�� �ʺ٬��j�� �ȹ���1000 ������500
  setusr -r ���j�� -n �j�� -m 1000 -g 500 itoc

  �]�w itoc ���v��
  setusr -p 11100...1101 itoc   (�v���� 32 = NUMPERMS ��)
            ^^^^^ 32 �� 0 �M 1

  �]�w itoc ���ߺD
  setusr -f 11100...1101 itoc   (�ߺD�� 27 = NUMUFOS ��)
            ^^^^^ 27 �� 0 �M 1

  �ӨϥΪ̥������b�u�W�~����
#endif


#include "bbs.h"


#define MAXUSIES	9	/* �@�� 9 �إi�H�諸 */

static void
usage(msg)
  char *msg;
{
  int i, len;
  char buf[80];
  char *usies[MAXUSIES] =
  {
    "r realname", "n username", "m money", "g gold", "# userno", 
    "e email", "j 1/0(justify)", "p userlevel", "f ufo"
  };


  printf("Usage: %s [-%s] [-%s] [-%s] ... [-%s] UserID\n", 
    msg, usies[0], usies[1], usies[2], usies[MAXUSIES - 1]);
  len = strlen(msg);
  sprintf(buf, "%%%ds-%%s\n", len + MAXUSIES);
  for (i = 0; i < MAXUSIES; i++)
    printf(buf, "", usies[i]);
}


static usint
bitcfg(len, str)	/* config bits */
  int len;		/* ����쪺���� */
  char *str;		/* optarg */
{
  int i;
  char c;
  usint bits;

  if (strlen(str) != len)	/* �������ӼƦr */
  {
    bits = (usint) atoi(str);
  }
  else
  {
    bits = 0;
    for (i = 0; i < len; i++)
    {
      c = str[i];

      if (c != '0' && c != '1')
      {
	printf("bit �@�w�n�O 0 �� 1\n");
	exit(1);
      }

      bits <<= 1;
      bits |= c - '0';
    }
  }

  return bits;
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int c;
  char *userid, fpath[64];
  ACCT acct;

  if (argc < 4 || argc % 2)	/* argc �n�O���� */
  {
    usage(argv[0]);
    exit(1);
  }

  chdir(BBSHOME);

  userid = argv[argc - 1];
  usr_fpath(fpath, userid, FN_ACCT);
  if (rec_get(fpath, &acct, sizeof(ACCT), 0) < 0)
  {
    printf("%s: read error (maybe no such id?)\n", userid);
    exit(1);
  }

  while ((c = getopt(argc, argv, "r:n:m:g:#:e:j:p:f:")) != -1)
  {
    switch (c)
    {
    case 'r':		/* realname */
      strcpy(acct.realname, optarg);
      break;

    case 'n':		/* username */
      strcpy(acct.username, optarg);
      break;

    case 'm':		/* money */
      acct.money = atoi(optarg);
      break;

    case 'g':		/* gold */
      acct.gold = atoi(optarg);
      break;

    case '#':		/* userno */ 
      acct.userno = atoi(optarg);
      break;

    case 'e':		/* email */
      strcpy(acct.email, optarg);
      break;

    case 'j':		/* justify */
      if (atoi(optarg)) /* �{�ҳq�L */
	acct.userlevel |= PERM_VALID;
      else
	acct.userlevel &= ~PERM_VALID;
      time(&acct.tvalid);
      break;

    case 'p':		/* userlevel */
      acct.userlevel = bitcfg(NUMPERMS, optarg);
      break;

    case 'f':		/* ufo */
      acct.ufo = bitcfg(NUMUFOS, optarg);
      break;

    default:
      usage(argv[0]);
      exit(0);
    }
  }

  if (rec_put(fpath, &acct, sizeof(ACCT), 0, NULL) < 0)
    printf("%s: write error\n", userid);
}
