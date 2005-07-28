/*-------------------------------------------------------*/
/* util/fix_uno.c	( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : ���ةҦ��ϥΪ̪� userno			 */
/* create : 04/10/16					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"


#undef	VERBOSE		/* �O�_��ܸԲӰT�� */

#ifdef VERBOSE
#define DEBUG(arg)	printf arg
#else
#define DEBUG(arg)	;
#endif


#define FN_OLDACCT	"olduserno"	/* �O�������ª� userno */


typedef struct
{
  int userno;
  char userid[IDLEN + 1];
}	UNO;


/*-------------------------------------------------------*/
/* ���� .ACCT �� .USR					 */
/*-------------------------------------------------------*/


static int
new_acct(userid)
  char *userid;
{
  static int userno = 1;	/* userno �q 1 �}�l */
  static time_t now = 100000;	/* �H�K���@�Ӯɶ� */

  char fpath[64];
  ACCT acct;
  SCHEMA slot;
  UNO uno;

  usr_fpath(fpath, userid, FN_ACCT);
  if (rec_get(fpath, &acct, sizeof(ACCT), 0) < 0)
  {
    /* �p�G�䤣�� .ACCT�A�n�R���Ҧ��W�� */
    usr_fpath(fpath, userid, FN_PAL);
    unlink(fpath);
#ifdef HAVE_LIST
    usr_fpath(fpath, userid, FN_LIST);
    unlink(fpath);
#endif
#ifdef HAVE_ALOHA
    usr_fpath(fpath, userid, FN_ALOHA);
    unlink(fpath);
#endif

    DEBUG(("�¤� %s �s�� userno ���� => �L�kŪ���ӨϥΪ̪����\n", userid));
    return;
  }

  /* �N�쥻�� userno �ƥ������y�C���ഫ�ϥ� */
  memset(&uno, 0, sizeof(UNO));
  uno.userno = acct.userno;
  str_ncpy(uno.userid, acct.userid, sizeof(uno.userid));
  rec_add(FN_OLDACCT, &uno, sizeof(UNO));

  acct.userno = userno++;
  unlink(fpath);
  rec_add(fpath, &acct, sizeof(ACCT));

  memset(&slot, 0, sizeof(SCHEMA));
  slot.uptime = now++;
  memcpy(slot.userid, acct.userid, IDLEN);
  rec_add(FN_SCHEMA, &slot, sizeof(SCHEMA));

  DEBUG(("�¤� %s �s�� userno ���\\\n", userid));
}


/*-------------------------------------------------------*/
/* �d userno						 */
/*-------------------------------------------------------*/


static int new_num;
static UNO *new_uno;


static int
uno_cmp_userid(a, b)
  UNO *a, *b;
{
  return strcmp(a->userid, b->userid);
}


static void
collect_new_uno()
{
  int fd, num;
  SCHEMA slot;

  if ((new_num = rec_num(FN_SCHEMA, sizeof(SCHEMA))) > 0)
  {
    new_uno = (UNO *) malloc(new_num * sizeof(UNO));

    if ((fd = open(FN_SCHEMA, O_RDONLY)) >= 0)
    {
      num = 0;
      while (read(fd, &slot, sizeof(SCHEMA)) == sizeof(SCHEMA) && num < new_num)
      {
	new_uno[num].userno = num + 1;
	str_ncpy(new_uno[num].userid, slot.userid, sizeof(new_uno[num].userid));	/* slot.userid ���t '\0' */
	num++;
      }
      close(fd);
    }

    if (new_num > 1)
      qsort(new_uno, new_num, sizeof(UNO), uno_cmp_userid);
  }
}


static int
acct_uno(userid)	/* �� ID ��s�� userno */
  char *userid;
{
  UNO uno, *find;

  str_ncpy(uno.userid, userid, sizeof(uno.userid));	/* ���� strcpy �Y�i�A���H���U�@ */
  if (find = bsearch(&uno, new_uno, new_num, sizeof(UNO), uno_cmp_userid))
    return find->userno;
  return 0;
}


static int old_num;
static UNO *old_uno;


static int
uno_cmp_userno(a, b)
  UNO *a, *b;
{
  return a->userno - b->userno;
}


static void
collect_old_uno()
{
  int fsize;

  if (old_uno = (UNO *) f_img(FN_OLDACCT, &fsize))
  {
    old_num = fsize / sizeof(UNO);
    if (old_num > 1)
      qsort(old_uno, old_num, sizeof(UNO), uno_cmp_userno);
  }
}


static int
acct_uno2(olduno)	/* ���ª� userno ��s�� userno */
  int olduno;
{
  UNO uno, *find;

  uno.userno = olduno;
  if (find = bsearch(&uno, old_uno, old_num, sizeof(UNO), uno_cmp_userno))
    return acct_uno(find->userid);
  return 0;
}


/*-------------------------------------------------------*/
/* ���� pal/list.?/aloha/benz/bpal			 */
/*-------------------------------------------------------*/


#define BENZ_MAX	512	/* ���]�C�ӤH���t�Ψ�M���W�L 512 �H */

static int rec_max;
static char *rec_pool;


static int
new_pal(userid)
  char *userid;
{
  int fd, num;
  char folder[64];
  PAL pal;

  usr_fpath(folder, userid, FN_PAL);
  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    num = 0;
    while (read(fd, &pal, sizeof(PAL)) == sizeof(PAL) && num < rec_max)
    {
      if ((pal.userno = acct_uno(pal.userid)) > 0)
      {
	memcpy(rec_pool + num * sizeof(PAL), &pal, sizeof(PAL));
	num++;
      }
    }
    close(fd);

    unlink(folder);
    rec_add(folder, rec_pool, num * sizeof(PAL));
    DEBUG(("���\\���� %s ���B�ͦW��A�@ %d �H\n", userid, num));
  }
}


#ifdef HAVE_LIST
static int
new_list(userid)
  char *userid;
{
  int fd, ch, num;
  char folder[64], fname[16];
  PAL pal;

  for (ch = '1'; ch <= '5'; ch++)
  {
    sprintf(fname, "%s.%c", FN_LIST, ch);
    usr_fpath(folder, userid, fname);
    if ((fd = open(folder, O_RDONLY)) >= 0)
    {
      num = 0;
      while (read(fd, &pal, sizeof(PAL)) == sizeof(PAL) && num < rec_max)
      {
	if ((pal.userno = acct_uno(pal.userid)) > 0)
	{
	  memcpy(rec_pool + num * sizeof(PAL), &pal, sizeof(PAL));
	  num++;
	}
      }
      close(fd);

      unlink(folder);
      rec_add(folder, rec_pool, num * sizeof(PAL));
      DEBUG(("���\\���� %s ���S��W��A�@ %d �H\n", userid, num));
    }
  }
}
#endif


#ifdef HAVE_ALOHA
static int
new_aloha(userid)
  char *userid;
{
  int fd, num;
  char folder[64];
  ALOHA aloha;

  FRIENZ frienz;
  char fpath[64];

  usr_fpath(folder, userid, FN_ALOHA);
  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    /* �ǳƦn�n�[�J��誺 frienz */
    memset(&frienz, 0, sizeof(FRIENZ));
    strcpy(frienz.userid, userid);
    if ((frienz.userno = acct_uno(userid)) > 0)
    {
      num = 0;
      while (read(fd, &aloha, sizeof(ALOHA)) == sizeof(ALOHA) && num < rec_max)
      {
	if ((aloha.userno = acct_uno(aloha.userid)) > 0)
	{
	  memcpy(rec_pool + num * sizeof(ALOHA), &aloha, sizeof(ALOHA));
	  num++;

	  /* ��ۤv�[�J��誺 frienz �� */
	  usr_fpath(fpath, aloha.userid, FN_FRIENZ);
	  rec_add(fpath, &frienz, sizeof(FRIENZ));
	  DEBUG(("���\\���� %s ���W���q���W��A�@ %d �H\n", userid, num));
	}
      }
    }
    close(fd);

    unlink(folder);
    rec_add(folder, rec_pool, num * sizeof(ALOHA));
  }
}
#endif


#ifdef LOGIN_NOTIFY
static int
new_benz(userid)
  char *userid;
{
  int fd, num;
  char folder[64];
  BENZ benz;

  usr_fpath(folder, userid, FN_BENZ);
  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    num = 0;
    while (read(fd, &benz, sizeof(BENZ)) == sizeof(BENZ) && num < rec_max)
    {
      if ((benz.userno = acct_uno(benz.userid)) > 0)
      {
	memcpy(rec_pool + num * sizeof(BENZ), &benz, sizeof(BENZ));
	num++;
      }
    }
    close(fd);

    unlink(folder);
    rec_add(folder, rec_pool, num * sizeof(BENZ));
    DEBUG(("���\\���� %s ���t�Ψ�M�W��A�@ %d �H\n", userid, num));
  }
}
#endif


static int
new_bmw(userid)
  char *userid;
{
  int fsize;
  char folder[64];
  BMW *data, *head, *tail;

  usr_fpath(folder, userid, FN_BMW);
  if (data = (BMW *) f_img(folder, &fsize))
  {
    head = data;
    tail = data + (fsize / sizeof(BMW));
    do
    {
      head->sender = acct_uno2(head->sender);
      head->recver = acct_uno2(head->recver);
    } while (++head < tail);

    unlink(folder);
    rec_add(folder, data, fsize);
    free(data);

    DEBUG(("���\\���� %s �����y�C��A�@ %d ��\n", userid, fsize / sizeof(BMW)));
  }
}


#ifdef HAVE_MODERATED_BOARD
static int
new_bpal(brdname)
  char *brdname;
{
  int fd, num;
  char folder[64];
  PAL pal;

  brd_fpath(folder, brdname, FN_PAL);
  if ((fd = open(folder, O_RDONLY)) >= 0)
  {
    num = 0;
    while (read(fd, &pal, sizeof(PAL)) == sizeof(PAL) && num < rec_max)
    {
      if ((pal.userno = acct_uno(pal.userid)) > 0)
      {
	memcpy(rec_pool + num * sizeof(PAL), &pal, sizeof(PAL));
	num++;
      }
    }
    close(fd);

    unlink(folder);
    rec_add(folder, rec_pool, num * sizeof(PAL));
    DEBUG(("���\\���� %s ���O�ͦW��A�@ %d �H\n", brdname, num));
  }
}
#endif


/*-------------------------------------------------------*/
/* �D�禡						 */
/*-------------------------------------------------------*/


int
main()
{
  char c;
  char *userid, fpath[64];
  struct dirent *de;
  DIR *dirp;
#ifdef HAVE_MODERATED_BOARD
  FILE *fp;
#endif

  chdir(BBSHOME);

  /* itoc.050113: ���e�@���O����Ӧs�A�̫�A�@���g�^�A�`�� I/O */
  rec_max = PAL_MAX;
#ifdef HAVE_ALOHA
  if (rec_max < ALOHA_MAX)
    rec_max = ALOHA_MAX;
#endif
#ifdef LOGIN_NOTIFY
  if (rec_max < BENZ_MAX)
    rec_max = BENZ_MAX;
#endif
  rec_pool = (char *) malloc(REC_SIZ * rec_max);

  /*-----------------------------------------------------*/
  /* �Ĥ@��: �¤��s�� userno�A�çR�� frienz		 */
  /*-----------------------------------------------------*/

  unlink(FN_SCHEMA);

  for (c = 'a'; c <= 'z'; c++)
  {
    printf("�¤��s�� userno: �}�l�B�z %c �}�Y�� ID\n", c);
    sprintf(fpath, "usr/%c", c);

    if (!(dirp = opendir(fpath)))
      continue;

    while (de = readdir(dirp))
    {
      userid = de->d_name;
      if (*userid <= ' ' || *userid == '.')
	continue;

      /* �N .ACCT ���s�� userno�A�üg�^ .USR */
      new_acct(userid);

#ifdef HAVE_ALOHA
      /* �R�� frienz */
      usr_fpath(fpath, userid, FN_FRIENZ);
      unlink(fpath);
#endif
    }

    closedir(dirp);
  }

  collect_new_uno();
  collect_old_uno();
  printf("�¤��Ҧ��H�s�� userno �����A�����@ %d �H\n", new_num);


  /*-----------------------------------------------------*/
  /* �ĤG��: ���ةҦ��H�� pal/list.?/aloha/benz/bmw	 */
  /*-----------------------------------------------------*/

  for (c = 'a'; c <= 'z'; c++)
  {
    printf("���طs�� pal/list/aloha/benz: �}�l�B�z %c �}�Y�� ID\n", c);
    sprintf(fpath, "usr/%c", c);

    if (!(dirp = opendir(fpath)))
      continue;

    while (de = readdir(dirp))
    {
      userid = de->d_name;
      if (*userid <= ' ' || *userid == '.')
	continue;

      new_pal(userid);
#ifdef HAVE_LIST
      new_list(userid);
#endif
#ifdef HAVE_ALOHA
      new_aloha(userid);
#endif
#ifdef LOGIN_NOTIFY
      new_benz(userid);
#endif
      new_bmw(userid);
    }

    closedir(dirp);
  }


#ifdef HAVE_MODERATED_BOARD
  /*-----------------------------------------------------*/
  /* �ĤT��: ���ةҦ��ݪO�� bpal			 */
  /*-----------------------------------------------------*/

  printf("���طs�� bpal\n");
  if (fp = fopen(FN_BRD, "r"))
  {
    BRD brd;
    while (fread(&brd, sizeof(BRD), 1, fp) == 1)
    {
      if (*brd.brdname)
	new_bpal(brd.brdname);
    }
  }
#endif

  free(rec_pool);
  free(new_uno);
  free(old_uno);
  unlink(FN_OLDACCT);

  return 0;
}
