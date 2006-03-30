/*-------------------------------------------------------*/
/* util/transbmw.c                   			 */
/*-------------------------------------------------------*/
/* target : WD �� Maple 3.02 ���y�O���ഫ           	 */
/* create : 02/01/22                     		 */
/* update :   /  /                   			 */
/* author : itoc.bbs@bbs.ee.nctu.edu.tw          	 */
/*-------------------------------------------------------*/
/* syntax : transbmw                     		 */
/*-------------------------------------------------------*/


#if 0

   1. �ק� transbmw()

   ps. �ϥΫe�Х���ƥ��Ause on ur own risk. �{����H�Х]�[ :p
   ps. �P�� lkchu �� Maple 3.02 for FreeBSD

#endif


#include "ats.h"


/* ----------------------------------------------------- */
/* 3.02 functions                    			 */
/* ----------------------------------------------------- */


static void
_mail_self(userid, fpath, owner, title)		/* itoc.011115: �H�ɮ׵��ۤv */
  char *userid;		/* ����� */
  char *fpath;		/* �ɮ׸��| */
  char *owner;		/* �H��H */
  char *title;		/* �l����D */
{
  HDR fhdr;
  char folder[64];

  usr_fpath(folder, userid, FN_DIR);
  close(hdr_stamp(folder, HDR_LINK, &fhdr, fpath));
  str_ncpy(fhdr.owner, owner, sizeof(fhdr.owner));
  str_ncpy(fhdr.title, title, sizeof(fhdr.title));
  fhdr.xmode = 0;
  rec_add(folder, &fhdr, sizeof(fhdr));
}


/* ----------------------------------------------------- */
/* �ഫ�D�{��                                            */
/* ----------------------------------------------------- */


static void
transbmw(userid)
  char *userid;
{
  ACCT acct;
  int fd;
  char buf[64],buf2[64],buf3[64];


  printf("�}�l�ƥ� %-15s ���y�O�����n�ͦW�桮�ڪ��̷R\n",acct.userid);
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

  sprintf(buf, OLD_BBSHOME"/home/%s/writelog", acct.userid);	/* �ª����y�O�� */

  if (dashf(buf))
    _mail_self(acct.userid, buf, "[�ƥ�]", "���y�O��");

  sprintf(buf2, OLD_BBSHOME"/home/%s/overrides", acct.userid);    /* �ª��n�ͦW�� */

  if (dashf(buf2))
    _mail_self(acct.userid, buf2, "[�ƥ�]", "�n�ͦW��");

  sprintf(buf3, OLD_BBSHOME"/home/%s/favor_boards", acct.userid);  /* �ª��ڪ��̷R */

  if (dashf(buf3))
    _mail_self(acct.userid, buf3, "[�ƥ�]", "�ڪ��̷R");
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
    transbmw(argv[1]);
    exit(1);
  }

  /* �ഫ�ϥΪ̤��y�O�� */
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

      transbmw(str);
    }

    closedir(dirp);    
  }
  return 0;
}
