/*-------------------------------------------------------*/
/* util/transfavor.c                   			 */
/*-------------------------------------------------------*/
/* target : WD �� Maple 3.02 �ڪ��̷R�ഫ           	 */
/* create : 01/09/15                     		 */
/* update :   /  /                   			 */
/* author : itoc.bbs@bbs.ee.nctu.edu.tw          	 */
/*-------------------------------------------------------*/
/* syntax : transfavor                     		 */
/*-------------------------------------------------------*/


#if 0

   1. �ק� transmf()
   2. �ഫ�n�ͦW�椧�e�A�z�������ഫ���ݪO�ΨϥΪ̡C

   ps. �ϥΫe�Х���ƥ��Ause on ur own risk. �{����H�Х]�[ :p
   ps. �P�� lkchu �� Maple 3.02 for FreeBSD

#endif


#include "ats.h"

#ifdef MY_FAVORITE


/* ----------------------------------------------------- */
/* 3.02 functions                    			 */
/* ----------------------------------------------------- */


static void
_mf_fpath(fpath, userid, fname)
  char *fpath;
  char *userid;		/* lower ID */
  char *fname;  
{
  if (fname)
    sprintf(fpath, "usr/%c/%s/MF/%s", userid[0], userid, fname);
  else
    sprintf(fpath, "usr/%c/%s/MF", userid[0], userid);
}


/* ----------------------------------------------------- */
/* �ഫ�D�{��                                            */
/* ----------------------------------------------------- */


static void
transmf(userid)
  char *userid;
{
  ACCT acct;
  FILE *fp;
  int fd, num;
  char fpath[64], buf[64];
  char *str, brdname[IDLEN + 1];
  MF mf;

  /* �إߥؿ� */
  _mf_fpath(fpath, userid, NULL);
  mkdir(fpath, 0700);

  /* sob �� usr �ؿ������j�p�g�A�ҥH�n�����o�j�p�g */
  usr_fpath(buf, userid, FN_ACCT);
  if ((fd = open(buf, O_RDONLY)) >= 0)
  {
    read(fd, &acct, sizeof(ACCT));
    close(fd);
  }
  else
  {
    return;
  }

  sprintf(buf, OLD_BBSHOME"/home/%s/favor_boards", acct.userid);  /* �ª��ڪ��̷R */

  printf("�ഫ %s �G�ڪ��̷R\n",acct.userid);

  if (!(fp = fopen(buf, "r")))
    return;

  _mf_fpath(fpath, userid, FN_MF);
  num = 0;

  while (fgets(brdname, IDLEN + 1, fp))
  {
    for (str = brdname; *str; str++)
    {
      if (*str <= ' ')
      {
	*str = '\0';
	break;
      }
    }

    brd_fpath(buf, brdname, NULL);
    if (dashd(buf))			/* ���T���o�ӪO */
    {
      mf.chrono = ++num;      
      mf.mftype = MF_BOARD;
      str_ncpy(mf.xname, brdname, sizeof(mf.xname));
      mf.title[0] = '\0';		/* �ݪO���|�S�� mf.title */
      rec_add(fpath, &mf, sizeof(MF));
    }
  }

  fclose(fp);
}


int  
main(argc, argv)
  int argc;
  char *argv[];
{
  char c;
  char buf[64];
  struct dirent *de;
  DIR *dirp;

  /* argc == 1 ������ϥΪ� */
  /* argc == 2 ��Y�S�w�ϥΪ� */

  if (argc > 2)
  {
    printf("Usage: %s [target_user]\n", argv[0]);
    exit(-1);
  }

  chdir(BBSHOME);

  if (argc == 2)
  {
    transmf(argv[1]);
    exit(1);
  }

  /* �ഫ�ϥΪ̧ڪ��̷R */
  for (c = 'a'; c <= 'z'; c++)
  {
    sprintf(buf, "usr/%c", c);

    if (!(dirp = opendir(buf)))
      continue;

    while (de = readdir(dirp))
    {
      char *str;

      str = de->d_name;
      if (*str <= ' ' || *str == '.')
	continue;

      transmf(str);
    }

    closedir(dirp);    
  }
  return 0;
}

#else
int
main()
{
  printf("You should define MY_FAVORITE first.\n");
  return -1;
}
#endif		/* MY_FAVORITE */
