/*-------------------------------------------------------*/
/* util/expire.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : �۰ʬ�H�u��{��				 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/
/* syntax : expire [day max min board]		 	 */
/* notice : �[�W board �ɡA�i expire+sync �Y�@ board	 */
/*-------------------------------------------------------*/


#include "bbs.h"


#if 0	/* itoc.030325: ²������ */

  expire.c �]�t�G�ذʧ@�Gexpire �M sync

  1) �ҿסu�L���v�A�N�O �峹����L�[/�ݪO�g�ƹL�h

  2) �ҿסuexpire�v�A�N�O�b .DIR ���ޤ���X�L�����峹�A��o hdr �q���ޤ������A
     �ñN���ɮקR��

  3) �ҿסusync�v�A���F�קK�w�Ц��h�l���U���A�t�ΨC 32 �ѷ|��ݪO�����ɮפ@�@
     �˵��A�Y�䤣�b .DIR ���A��ܳo���ɮפw�g�q���ޤ��򥢤F�A���ɨt�η|�����N
     �o�ɮקR��

#endif


#define	DEF_DAYS	365		/* �w�]�M���W�L 365 �Ѫ��峹 */
#define	DEF_MAXP	5000		/* �w�]�M���W�L 5000 �g���峹 */
#define	DEF_MINP	500		/* �w�]�C�� 500 �g���ݪO����峹 */


#define	EXPIRE_LOG	"run/expire.log"


typedef struct
{
  char bname[BNLEN + 1];	/* board ID */
  int days;			/* expired days */
  int maxp;			/* max post */
  int minp;			/* min post */
}      Life;


static int
life_cmp(a, b)
  Life *a, *b;
{
  return str_cmp(a->bname, b->bname);
}


/* ----------------------------------------------------- */
/* board�Gshm �������P cache.c �ۮe                      */
/* ----------------------------------------------------- */


static BCACHE *bshm;


static void
init_bshm()
{
  /* itoc.030727: �b�}�� bbsd ���e�A���ӴN�n����L account�A
     �ҥH bshm ���Ӥw�]�w�n */

  bshm = shm_new(BRDSHM_KEY, sizeof(BCACHE));

  if (bshm->uptime <= 0)	/* bshm ���]�w���� */
    exit(0);
}


/* ----------------------------------------------------- */
/* synchronize folder & files				 */
/* ----------------------------------------------------- */


static time_t synctime;


typedef struct
{
  time_t chrono;
  char prefix;		/* �ɮת� family */
  char exotic;		/* 1:���b.DIR��  0:�b.DIR�� */
} SyncData;


static SyncData *sync_pool;
static int sync_size, sync_head;


#define	SYNC_DB_SIZE	2048


static int
sync_cmp(a, b)
  SyncData *a, *b;
{
  return a->chrono - b->chrono;
}


static void
sync_init(fname)
  char *fname;
{
  int ch, prefix;
  time_t chrono;
  char *str, fpath[80];
  struct dirent *de;
  DIR *dirp;

  SyncData *xpool;
  int xsize, xhead;

  if (xpool = sync_pool)
  {
    xsize = sync_size;
  }
  else
  {
    xpool = (SyncData *) malloc(SYNC_DB_SIZE * sizeof(SyncData));
    xsize = SYNC_DB_SIZE;
  }

  xhead = 0;

  ch = strlen(fname);
  memcpy(fpath, fname, ch);
  fname = fpath + ch;
  *fname++ = '/';
  fname[1] = '\0';

  /* itoc.030325.����: ���� brd/brdname/?/* ���Ҧ��ɮ׳���i�h xpool[]
     �o���[��W�椤�A�M��^�� expire() ���ˬd���ɮ׬O�_�b .DIR ��
     �Y�b .DIR ���A�N�� exotic �]�^ 0
     �̫�b sync_check() �����[��W�椤 exotic �٬O 1 ���ɮ׳��R�� */

  ch = '0';
  for (;;)
  {
    *fname = ch++;

    if (dirp = opendir(fpath))
    {
      while (de = readdir(dirp))
      {
	str = de->d_name;
	prefix = *str;
	if (prefix == '.')
	  continue;

	chrono = chrono32(str);

	if (chrono > synctime)	/* �o�O�̪��o���峹�A���ݭn�[�J xpool[] �h sync */
	  continue;

	if (xhead >= xsize)
	{
	  xsize += (xsize >> 1);
	  xpool = (SyncData *) realloc(xpool, xsize * sizeof(SyncData));
	}

	xpool[xhead].chrono = chrono;
	xpool[xhead].prefix = prefix;
	xpool[xhead].exotic = 1;	/* �������O�� 1�A�� expire() ���A��^ 0 */
	xhead++;
      }

      closedir(dirp);
    }

    if (ch == 'W')
      break;

    if (ch == '9' + 1)
      ch = 'A';
  }

  if (xhead > 1)
    qsort(xpool, xhead, sizeof(SyncData), sync_cmp);

  sync_pool = xpool;
  sync_size = xsize;
  sync_head = xhead;
}


static void
sync_check(flog, fname)
  FILE *flog;
  char *fname;
{
  char *str, fpath[80];
  SyncData *xpool, *xtail;
  time_t cc;

  if ((cc = sync_head) <= 0)
    return;

  xpool = sync_pool;
  xtail = xpool + cc;

  sprintf(fpath, "%s/ /", fname);
  str = strchr(fpath, ' ');
  fname = str + 3;

  do
  {
    if (xtail->exotic)
    {
      cc = xtail->chrono;
      fname[-1] = xtail->prefix;
      *str = radix32[cc & 31];
      archiv32(cc, fname);
      unlink(fpath);

      fprintf(flog, "-\t%s\n", fpath);
    }
  } while (--xtail >= xpool);
}


static void
expire(flog, life, sync)
  FILE *flog;
  Life *life;
  int sync;
{
  HDR hdr;
  struct stat st;
  char fpath[128], fnew[128], index[128], *fname, *bname, *str;
  int done, keep, total;
  FILE *fpr, *fpw;

  int days, maxp, minp;
  int duetime;

  SyncData *xpool, *xsync;
  int xhead;

  days = life->days;
  maxp = life->maxp;
  minp = life->minp;
  bname = life->bname;

  fprintf(flog, "%s\n", bname);

  sprintf(index, "%s/.DIR", bname);
  if (!(fpr = fopen(index, "r")))
  {
    fprintf(flog, "\tError open file: %s\n", index);
    return;
  }

  fpw = f_new(index, fnew);
  if (!fpw)
  {
    fprintf(flog, "\tExclusive lock: %s\n", fnew);
    fclose(fpr);
    return;
  }

  if (sync)
  {
    sync_init(bname);
    xpool = sync_pool;
    xhead = sync_head;
    if (xhead <= 0)
      sync = 0;
    else
      fprintf(flog, "\t%d files to sync\n\n", xhead);
  }

  strcpy(fpath, index);
  str = (char *) strrchr(fpath, '.');
  fname = str + 1;
  *fname++ = '/';

  done = 1;
  duetime = synctime - days * 86400;

  fstat(fileno(fpr), &st);
  total = st.st_size / sizeof(hdr);

  while (fread(&hdr, sizeof(hdr), 1, fpr) == 1)
  {
    if (hdr.xmode & (POST_MARKED | POST_BOTTOM) || total <= minp)
      keep = 1;
    else if (hdr.chrono < duetime || total > maxp)
      keep = 0;
    else
      keep = 1;

    if (sync && (hdr.chrono < synctime))
    {
      if (xsync = (SyncData *) bsearch(&hdr.chrono, xpool, xhead, sizeof(SyncData), sync_cmp))
      {
	xsync->exotic = 0;	/* �o�g�b .DIR ���A�� sync */
      }
      else
      {
        keep = 0;		/* �@�ߤ��O�d */
      }
    }

    if (keep)
    {
      if (fwrite(&hdr, sizeof(hdr), 1, fpw) != 1)
      {
	fprintf(flog, "\tError in write DIR.n: %s\n", hdr.xname);
	done = 0;
        sync = 0; /* Thor.990127: �S�@��, �N�O��F�a */
	break;
      }
    }
    else
    {
      *str = hdr.xname[7];
      strcpy(fname, hdr.xname);
      unlink(fpath);
      fprintf(flog, "\t%s\n", fname);
      total--;
    }
  }
  fclose(fpr);
  fclose(fpw);

  if (done)
  {
    sprintf(fpath, "%s.o", index);
    if (!rename(index, fpath))
    {
      if (rename(fnew, index))
        rename(fpath, index);		/* ���^�� */
    }
  }
  unlink(fnew);

  if (sync)
    sync_check(flog, bname);
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  FILE *fp;
  int number, count;
  Life db, table[MAXBOARD], *key;
  char *ptr, *bname, buf[256];
  BRD *brdp, *bend;

  /* itoc.030325: �n�������w�ѼơA�n�����w�Ҧ��Ѽ� */
  if (argc != 1 && argc != 5)
  {
    printf("%s [day max min board]\n", argv[0]);
    exit(-1);
  }

  /* �Y�L���w�ѼơA�h�ιw�]�� */
  db.days = ((argc == 5) && (number = atoi(argv[1])) > 0) ? number : DEF_DAYS;
  db.maxp = ((argc == 5) && (number = atoi(argv[2])) > 0) ? number : DEF_MAXP;
  db.minp = ((argc == 5) && (number = atoi(argv[3])) > 0) ? number : DEF_MINP;

  /* --------------------------------------------------- */
  /* load expire.conf					 */
  /* --------------------------------------------------- */

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  init_bshm();

  count = 0;
  if (argc != 5)	/* �Y�S�����w�ѼơA�~�ݭn�hŪ expire.conf */
  {
    if (fp = fopen(FN_ETC_EXPIRE, "r"))
    {
      while (fgets(buf, sizeof(buf), fp))
      {
	if (buf[0] == '#')
	  continue;

	bname = (char *) strtok(buf, " \t\r\n");
	if (bname && *bname)
	{
	  ptr = (char *) strtok(NULL, " \t\r\n");
	  if (ptr && (number = atoi(ptr)) > 0)
	  {
	    key = &(table[count]);
	    strcpy(key->bname, bname);
	    key->days = number;
	    key->maxp = db.maxp;
	    key->minp = db.minp;

	    ptr = (char *) strtok(NULL, " \t\r\n");
	    if (ptr && (number = atoi(ptr)) > 0)
	    {
	      key->maxp = number;

	      ptr = (char *) strtok(NULL, " \t\r\n");
	      if (ptr && (number = atoi(ptr)) > 0)
		key->minp = number;
	    }

	    /* expire.conf �i��S�����@�ӶW�L MAXBOARD �ӪO */
	    if (++count >= MAXBOARD)
	      break;
	  }
	}
      }
      fclose(fp);
    }

    if (count > 1)
      qsort(table, count, sizeof(Life), life_cmp);
  }

  /* --------------------------------------------------- */
  /* visit all boards					 */
  /* --------------------------------------------------- */

  fp = fopen(EXPIRE_LOG, "w");

  chdir("brd");

  synctime = time(NULL) - 10 * 60;	/* �Q���������s�峹���ݭn sync */
  number = synctime / 86400;

  brdp = bshm->bcache;
  bend = brdp + bshm->number;
  do
  {
    bname = brdp->brdname;
    if (!*bname)
      continue;

    /* Thor.981027: �[�W board �ɡA�i expire+sync �Y�@ board */
    if (argc == 5)
    {
      if (str_cmp(argv[4], bname))
	continue;

      number = 0;	/* �j�� sync �o�O */
    }

    if (count)
    {
      key = (Life *) bsearch(bname, table, count, sizeof(Life), life_cmp);
      if (!key)
	key = &db;
    }
    else
    {
      key = &db;
    }

    strcpy(key->bname, bname);		/* �������T���j�p�g */
    expire(fp, key, !(number & 31));	/* �C�j 32 �� sync �@�� */
    number++;
    brdp->btime = -1;
  } while (++brdp < bend);

  fclose(fp);
  exit(0);
}
