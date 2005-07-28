/*-------------------------------------------------------*/
/* util/redir.c		( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : �۰ʭ���.DIR�{��				 */
/* create : 99/10/07					 */
/* update : 04/11/29					 */
/* author : Thor.bbs@bbs.cs.nthu.edu.tw			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/
/* input  : scan current directory			 */
/* output : generate .DIR.re				 */
/*-------------------------------------------------------*/


#if 0

  ���{���i�H���جݪO�B��ذϡB�H�c�� .DIR
  �| scan current directory �Ӳ��� .DIR.re
  �H chrono �ӱƧ�

#endif


#include "bbs.h"


#define FNAME_DB_SIZE   2048


typedef char FNAME[9];
static FNAME *n_pool;
static int n_size, n_head;


static int
pool_add(fname)
  FNAME fname;
{
  char *p;

  /* initial pool */
  if (!n_pool)
  {
    n_pool = (FNAME *) malloc(FNAME_DB_SIZE * sizeof(FNAME));
    n_size = FNAME_DB_SIZE;
    n_head = 0;
  }

  if (n_head >= n_size)
  {
    n_size += (n_size >> 1);
    n_pool = (FNAME *) realloc(n_pool, n_size * sizeof(FNAME));
  }

  p = n_pool[n_head];

  if (strlen(fname) != 8)
    return -1;

  strcpy(p, fname);

  n_head++;

  return 0;
}


static int type;		/* 'b':�ݪO 'g':��ذ� 'm':�H�c */


static HDR *
article_parse(fname)
  char *fname;
{
  char buf[ANSILINELEN], *ptr1, *ptr2, *ptr3;
  FILE *fp;
  static HDR hdr;

  memset(&hdr, 0, sizeof(HDR));

  /* fill in chrono/date/xmode/xid/xname */
  hdr.chrono = chrono32(fname);
  str_stamp(hdr.date, &hdr.chrono);
  strcpy(hdr.xname, fname);
  if (type == 'm')
    hdr.xmode = MAIL_READ;

  if (*fname == 'F')	/* �p�G�O���v�A�ѩ�S����L��T�F�A�ҥH�u���H�K�� */
  {
    hdr.xmode = GEM_FOLDER;
    strcpy(hdr.owner, STR_SYSOP);
    strcpy(hdr.nick, SYSOPNICK);
    strcpy(hdr.title, "�ɮ��B��");
    return &hdr;
  }

  sprintf(buf, "%c/%s", (type == 'm') ? '@' : fname[7], fname);
  if (fp = fopen(buf, "r"))
  {
    if (fgets(buf, sizeof(buf), fp))
    {
      if (ptr1 = strchr(buf, '\n'))
	ptr1 = '\0';

      if (!strncmp(buf, STR_AUTHOR1 " ", LEN_AUTHOR1 + 1))
	ptr1 = buf + LEN_AUTHOR1 + 1;
      else if (!strncmp(buf, STR_AUTHOR2 " ", LEN_AUTHOR2 + 1))
	ptr1 = buf + LEN_AUTHOR2 + 1;
      else
	ptr1 = NULL;

      if (ptr1 && *ptr1)
      {
	ptr2 = strchr(ptr1 + 1, '@');
	ptr3 = strchr(ptr1 + 1, '(');

	if (ptr2 && (!ptr3 || ptr2 < ptr3))	/* �b�ʺٸ̭��� @ ����O email */
	{
	  str_from(ptr1, hdr.owner, hdr.nick);
	  hdr.xmode |= POST_INCOME;	/* also MAIL_INCOME */
	}
	else if (ptr3)
	{
	  ptr3[-1] = '\0';
	  str_ncpy(hdr.owner, ptr1, sizeof(hdr.owner));
	  if (ptr2 = strchr(ptr3 + 1, ')'))
	  {
	    *ptr2 = '\0';
	    str_ncpy(hdr.nick, ptr3 + 1, sizeof(hdr.nick));
	  }
	}
      }

      if (fgets(buf, sizeof(buf), fp))
      {
	if (ptr1 = strchr(buf, '\n'))
	  *ptr1 = '\0';

	if (!strncmp(buf, "���D: ", LEN_AUTHOR1 + 1))
	  ptr1 = buf + LEN_AUTHOR1 + 1;
	else if (!strncmp(buf, "��  �D: ", LEN_AUTHOR2 + 1))
	  ptr1 = buf + LEN_AUTHOR2 + 1;
	else
	  ptr1 = NULL;

	if (ptr1 && *ptr1)	/* �����D��� */
	  str_ncpy(hdr.title, ptr1, sizeof(hdr.title));
      }
    }

    fclose(fp);
  }

  return &hdr;
}


static char *allindex = ".DIR.tmp";


static void
allindex_collect()
{
  int i;
  char *fname, fpath[64];
  FILE *fp;

  /* �N�Ҧ��� F* ���g�J�@�ӼȦs�� */
  if (fp = fopen(allindex, "w"))
  {
    for (i = 0; i < n_head; i++)
    {
      fname = n_pool[i];

      if (*fname != 'F')
	continue;

      sprintf(fpath, "%c/%s", fname[7], fname);
      f_suck(fp, fpath);
    }
    fclose(fp);
  }
}


static int
allindex_search(fname)
  char *fname;
{
  HDR old;
  int fd;
  int rc = 0;

  if ((fd = open(allindex, O_RDONLY)) >= 0)
  {
    while (read(fd, &old, sizeof(HDR)) == sizeof(HDR))
    {
      if (!strcmp(fname, old.xname))
      {
	rc = 1;
	break;
      }
    }
    close(fd);
  }
  return rc;
}


static int
fname_cmp(s1, s2)
  char *s1, *s2;
{
  return strcmp(s1 + 1, s2 + 1);
}


static void
usage(argv)
  char *argv[];
{
  char *str = argv[0];

  printf("Usage: �Ы��w�Ѽ�\n");
  printf("  ���� �ݪO�峹 ���޽а��� %s -b\n", str);
  printf("  ���غ�ذϤ峹���޽а��� %s -g\n", str);
  printf("  ���� �H�c�H�� ���޽а��� %s -m\n", str);
  printf("���浲���H��A�A�N .DIR.re �л\\ .DIR �Y�i\n");

  exit(0);
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int i;
  char *fname, buf[10];
  FILE *fp;
  HDR *hdr;
  struct dirent *de;
  DIR *dirp;

  if (argc != 2)
    usage(argv);

  type = argv[1][1];
  if (type != 'b' && type != 'g' && type != 'm')
    usage(argv);

  /* readdir 0-9A-V(�ݪO�B��ذ�) �� @(�H�c) */

  buf[1] = '\0';

  for (i = 0; i < 32; i++)
  {
    *buf = (type == 'm') ? '@' : radix32[i];

    if (dirp = opendir(buf))
    {
      while (de = readdir(dirp))
      {
	fname = de->d_name;
	if (type == 'b' && *fname != 'A')			/* �ݪO���O A1234567 */
	  continue;
	if (type == 'g' && *fname != 'A' && *fname != 'F')	/* ��ذϥ��O A1234567 �� F1234567 */
	  continue;
	if (type == 'm' && *fname != '@')			/* �H�c���O @1234567 */
	  continue;

	if (pool_add(fname) < 0)
	  printf("Bad article/folder name %c/%s\n", *buf, fname);
      }
      closedir(dirp);
    }

    if (type == 'm')	/* �H�c�u�ݭn�� @ �o�ӥؿ� */
      break;
  }

  if (n_head)
  {
    /* �ݪO/�H�c �N������ n_pool �̭����Ҧ��ɮץ[�i .DIR �Y�i */
    /* ��ذϪ��ܡA�h�O�u�ݭn��S���b��L F* �̭����[�J .DIR */

    /* qsort chrono */
    if (n_head > 1)
      qsort(n_pool, n_head, sizeof(FNAME), fname_cmp);

    if (type == 'g')
      allindex_collect();

    /* generate .DIR.re */
    if (fp = fopen(".DIR.re", "w"))
    {
      /* for each file/folder */
      for (i = 0; i < n_head; i++)
      {
	fname = n_pool[i];

	if (type == 'g' && allindex_search(fname))	/* �w�g�b��L���v�̭��F�A���N���|�O�b .DIR �̭� */
	  continue;

	/* parse header */
	if (hdr = article_parse(fname))
	  fwrite(hdr, sizeof(HDR), 1, fp);
      }
      fclose(fp);
    }

    if (type == 'g')
      unlink(allindex);
  }

  return 0;
}
