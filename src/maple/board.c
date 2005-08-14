/*-------------------------------------------------------*/
/* board.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : �ݪO�B�s�ե\��	 			 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern BCACHE *bshm;
extern XZ xz[];
extern char xo_pool[];


char brd_bits[MAXBOARD];

#ifndef ENHANCED_VISIT
time_t brd_visit[MAXBOARD];		/* �̪��s���ɶ� */
#endif


static char *class_img = NULL;
static XO board_xo;


/* ----------------------------------------------------- */
/* �ݪO�\Ū�O�� .BRH (Board Reading History)		 */
/* ----------------------------------------------------- */


typedef struct BoardReadingHistory
{
  time_t bstamp;		/* �إ߬ݪO���ɶ�, unique */	/* Thor.brh_tail */
  time_t bvisit;		/* �W���\Ū�ɶ� */		/* Thor.980904: �S�bŪ�ɩ�W��Ū���ɶ�, ���bŪ�ɩ� bhno */
  int bcount;			/* Thor.980902: �S�Ψ� */

  /* --------------------------------------------------- */
  /* time_t {final, begin} / {final | BRH_SIGN}		 */
  /* --------------------------------------------------- */
                           /* Thor.980904.����: BRH_SIGN�N��final begin �ۦP */
                           /* Thor.980904.����: �Ѥj��p�ƦC,�s��wŪinterval */
}                   BRH;


#define	BRH_EXPIRE	180		/* Thor.980902.����: �O�d�h�֤� */
#define BRH_MAX		200		/* Thor.980902.����: �C�O�̦h���X�Ӽ��� */
#define BRH_PAGE	2048		/* Thor.980902.����: �C���h�t�q, �Τ���F */
#define	BRH_MASK	0x7fffffff	/* Thor.980902.����: �̤j�q��2038�~1�뤤*/
#define	BRH_SIGN	0x80000000	/* Thor.980902.����: zap����final�M�� */
#define	BRH_WINDOW	(sizeof(BRH) + sizeof(time_t) * BRH_MAX * 2)


static int *brh_base;		/* allocated memory */
static int *brh_tail;		/* allocated memory */
static int brh_size;		/* allocated memory size */
static time_t brh_expire;


static int *
brh_alloc(tail, size)
  int *tail;
  int size;
{
  int *base, n;

  base = brh_base;
  n = (char *) tail - (char *) base;
  size += n;
  if (size > brh_size)
  {
    /* size = (size & -BRH_PAGE) + BRH_PAGE; */
    size += n >> 4;		/* �h�w���@�ǰO���� */
    base = (int *) realloc((char *) base, size);

    if (base == NULL)
      abort_bbs();

    brh_base = base;
    brh_size = size;
    tail = (int *) ((char *) base + n);
  }

  return tail;
}


static void
brh_put()
{
  int *list;

  /* compact the history list */

  list = brh_tail;

  if (*list)
  {
    int *head, *tail, n, item, chrono;

    n = *++list;   /* Thor.980904: ��Ū�ɬObhno */
    brd_bits[n] |= BRD_H_BIT;
    time((time_t *) list);    /* Thor.980904.����: bvisit time */

    item = *++list;
    head = ++list;
    tail = head + item;

    while (head < tail)
    {
      chrono = *head++;
      n = *head++;
      if (n == chrono) /* Thor.980904.����: �ۦP���ɭ����_�� */
      {
	n |= BRH_SIGN;
	item--;
      }
      else
      {
	*list++ = chrono;
      }
      *list++ = n;
    }

    list[-item - 1] = item;
    *list = 0;
    brh_tail = list;  /* Thor.980904:�s����brh */
  }
}


void
brh_get(bstamp, bhno)
  time_t bstamp;		/* board stamp */
  int bhno;
{
  int *head, *tail;
  int size, bcnt, item;
  char buf[BRH_WINDOW];

  if (bstamp == *brh_tail) /* Thor.980904.����: �ӪO�w�b brh_tail�W */
    return;

  brh_put();

  bcnt = 0;
  tail = brh_tail;

  if (brd_bits[bhno] & BRD_H_BIT)
  {
    head = brh_base;
    while (head < tail)
    {
      item = head[2];
      size = item * sizeof(time_t) + sizeof(BRH);

      if (bstamp == *head)
      {
	bcnt = item;
	memcpy(buf, head + 3, size);
	tail = (int *) ((char *) tail - size);
	if (item = (char *) tail - (char *) head)
	  memcpy(head, (char *) head + size, item);
	break;
      }
      head = (int *) ((char *) head + size);
    }
  }

  brh_tail = tail = brh_alloc(tail, BRH_WINDOW);

  *tail++ = bstamp;
  *tail++ = bhno;

  if (bcnt)			/* expand history list */
  {
    int *list;

    size = bcnt;
    list = tail;
    head = (int *) buf;

    do
    {
      item = *head++;
      if (item & BRH_SIGN)
      {
	item ^= BRH_SIGN;
	*++list = item;
	bcnt++;
      }
      *++list = item;
    } while (--size);
  }

  *tail = bcnt;
}


int
brh_unread(chrono)
  time_t chrono;
{
  int *head, *tail, item;

  /* itoc.010407.����: BRH_EXPIRE (180) �ѫe���峹���]���wŪ */
  if (chrono <= brh_expire)
    return 0;

  head = brh_tail + 2;
  if ((item = *head) > 0)
  {
    /* check {final, begin} history list */

    head++;
    tail = head + item;
    do
    {
      if (chrono > *head)
	return 1;

      head++;
      if (chrono >= *head)
	return 0;

    } while (++head < tail);
  }
  return 1;
}


void
brh_visit(mode)
  int mode;			/* 0 : visit, 1: un-visit */
{				/* itoc.010207: �άO�ǤJchrono, �N��Ū�ܭ� */
  int *list;

  list = (int *) brh_tail + 2;
  *list++ = 2;
  if (mode)
  {
    *list = mode;
  }
  else
  {
    time((time_t *)list);
  }
  /* *++list = mode; */
  *++list = 0;	/* itoc.010207: �j�w�� 0, for ���� visit */
}


int
brh_add(prev, chrono, next)
  time_t prev, chrono, next;
{
  int *base, *head, *tail, item, final, begin;

  head = base = brh_tail + 2;
  item = *head++;
  tail = head + item;

  begin = BRH_MASK;

  while (head < tail)
  {
    final = *head;
    if (chrono > final)
    {
      if (prev <= final)
      {
	if (next < begin)	/* increase */
	  *head = chrono;
	else
	{			/* merge */
	  *base = item - 2;
	  base = head - 1;
	  do
	  {
	    *base++ = *++head;
	  } while (head < tail);
	}
	return;
      }

      if (next >= begin)
      {
	head[-1] = chrono;
	return;
      }

      break;
    }

    begin = *++head;
    head++;
  }

  /* insert or append */

  /* [21, 22, 23] ==> [32, 30] [15, 10] */

  if (item < BRH_MAX)
  {
    /* [32, 30] [22, 22] [15, 10] */

    *base = item + 2;
    tail += 2;
  }
  else
  {
    /* [32, 30] [22, 10] */  /* Thor.980923: how about [6, 7, 8] ? [15, 7] ? */

    tail--;
  }

  prev = chrono;
  for (;;)
  {
    final = *head;
    *head++ = chrono;

    if (head >= tail)
      return;

    begin = *head;
    *head++ = prev;

    if (head >= tail)
      return;

    chrono = final;
    prev = begin;
  }
}


/* ----------------------------------------------------- */
/* board permission check				 */
/* ----------------------------------------------------- */


int			/* >=1:�ĴX�ӪO�D 0:���O�O�D */
is_bm(list, userid)
  char *list;		/* �O�D�GBM list */
  char *userid;
{
  return str_has(list, userid, strlen(userid));
}


static inline int
Ben_Perm(bno, ulevel)
  int bno;
  usint ulevel;
{
  usint readlevel, postlevel, bits;
  char *blist, *bname;
  BRD *brd;
#ifdef HAVE_MODERATED_BOARD
  BPAL *bpal;
  int ftype;	/* 0:�@��ID 1:�O�n 2:�O�a */

  /* itoc.040103: �ݪO�\Ū���Ż�����

  �z�w�w�w�w�s�w�w�w�w�s�w�w�w�w�s�w�w�w�w�{
  �x        �x�@��Τ�x�ݪO�n�͢x�ݪO�a�H�x
  �u�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�t
  �x�@��ݪO�x�v���M�w�x  ����  �x �ݤ��� �x    �ݤ����G�b�ݪO�C���L�k�ݨ�o�ӪO�A�]�i���h
  �u�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�t    �i���h�G�b�ݪO�C���i�H�ݨ�o�ӪO�A���O�i���h
  �x�n�ͬݪO�x �i���h �x  ����  �x  ����  �x    ��  ��G�b�ݪO�C���i�H�ݨ�o�ӪO�A�]�i�o�h�A���O����o��
  �u�w�w�w�w�q�w�w�w�w�q�w�w�w�w�q�w�w�w�w�t    ��  ��G�b�ݪO�C���i�H�ݨ�o�ӪO�A�]�i�o�h�εo��
  �x���K�ݪO�x �ݤ��� �x  ����  �x  ����  �x
  �|�w�w�w�w�r�w�w�w�w�r�w�w�w�w�r�w�w�w�w�}
  */

  static int bit_data[9] =
  {                /* �@��Τ�   �ݪO�n��                           �ݪO�a�H */
    /* ���}�ݪO */    0,         BRD_L_BIT | BRD_R_BIT,             0,
    /* �n�ͬݪO */    BRD_L_BIT, BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT,
    /* ���K�ݪO */    0,         BRD_L_BIT | BRD_R_BIT | BRD_W_BIT, BRD_L_BIT | BRD_R_BIT,
  };
#endif

  brd = bshm->bcache + bno;
  bname = brd->brdname;
  if (!*bname)
    return 0;

  readlevel = brd->readlevel;

#ifdef HAVE_MODERATED_BOARD
  bpal = bshm->pcache + bno;
  ftype = is_bgood(bpal) ? 1 : is_bbad(bpal) ? 2 : 0;

  if (readlevel == PERM_SYSOP)		/* ���K�ݪO */
    bits = bit_data[6 + ftype];
  else if (readlevel == PERM_BOARD)	/* �n�ͬݪO */
    bits = bit_data[3 + ftype];
  else if (ftype)			/* ���}�ݪO�A�Y�b�O�n/�O�a�W�椤 */
    bits = bit_data[ftype];
  else					/* ���}�ݪO�A��L���v���P�w */
#endif

  if (!readlevel || (readlevel & ulevel))
  {
    bits = BRD_L_BIT | BRD_R_BIT;

    postlevel = brd->postlevel;
    if (!postlevel || (postlevel & ulevel))
      bits |= BRD_W_BIT;
  }
  else
  {
    bits = 0;
  }

  /* Thor.980813.����: �S�O�� BM �Ҷq�A�O�D���ӪO���Ҧ��v�� */
  blist = brd->BM;
  if ((ulevel & PERM_BM) && blist[0] > ' ' && is_bm(blist, cuser.userid))
    bits = BRD_L_BIT | BRD_R_BIT | BRD_W_BIT | BRD_X_BIT | BRD_M_BIT;

  /* itoc.030515: �ݪO�`�ޭ��s�P�_ */
  else if (ulevel & PERM_ALLBOARD)
    bits = BRD_L_BIT | BRD_R_BIT | BRD_W_BIT | BRD_X_BIT;

  return bits;
}


/* ----------------------------------------------------- */
/* ���J currboard �i��Y�z�]�w				 */
/* ----------------------------------------------------- */


int
bstamp2bno(stamp)
  time_t stamp;
{
  BRD *brd;
  int bno, max;

  bno = 0;
  brd = bshm->bcache;
  max = bshm->number;
  for (;;)
  {
    if (stamp == brd->bstamp)
      return bno;
    if (++bno >= max)
      return -1;
    brd++;
  }
}


static inline void
brh_load()
{
  BRD *brdp;
  usint ulevel;
  int n, cbno;
  char *bits;

  int size, *base;
  time_t expire;
  char fpath[64];

#ifndef ENHANCED_VISIT
  time_t *bstp;
#endif

  memset(bits = brd_bits, 0, sizeof(brd_bits));
#ifndef ENHANCED_VISIT
  memset(bstp = brd_visit, 0, sizeof(brd_visit));
#endif

  ulevel = cuser.userlevel;
  n = 0;
  cbno = bshm->number;

  do
  {
    *bits++ = Ben_Perm(n, ulevel);
  } while (++n < cbno);

  /* --------------------------------------------------- */
  /* �N .BRH ���J memory				 */
  /* --------------------------------------------------- */

  size = 0;
  cbno = -1;
  brh_expire = expire = time(0) - BRH_EXPIRE * 86400;

  if (ulevel)
  {
    struct stat st;

    usr_fpath(fpath, cuser.userid, FN_BRH);
    if (!stat(fpath, &st))
      size = st.st_size;
  }

  /* --------------------------------------------------- */
  /* �h�O�d BRH_WINDOW ���B�@�Ŷ�			 */
  /* --------------------------------------------------- */

  /* brh_size = n = ((size + BRH_WINDOW) & -BRH_PAGE) + BRH_PAGE; */
  brh_size = n = size + BRH_WINDOW;
  brh_base = base = (int *) malloc(n);

  if (size && ((n = open(fpath, O_RDONLY)) >= 0))
  {
    int *head, *tail, *list, bstamp, bhno;

    size = read(n, base, size);
    close(n);

    /* compact reading history : remove dummy/expired record */

    head = base;
    tail = (int *) ((char *) base + size);
    bits = brd_bits;

    while (head < tail && head >= brh_base)
    {
      bstamp = *head;

      if (bstamp & BRH_SIGN)	/* zap */
      {
	bstamp ^= BRH_SIGN;
	bhno = bstamp2bno(bstamp);
	if (bhno >= 0)
	{
	  /* itoc.001029: NOZAP��, ���|�X�{ */
	  brdp = bshm->bcache + bhno;
	  if (!(brdp->battr & BRD_NOZAP))
	    bits[bhno] |= BRD_Z_BIT;
	}
	head++;
	continue;
      }

      bhno = bstamp2bno(bstamp);
      list = head + 2;

      if (list > tail)
	break;

      n = *list;
      size = n + 3;

      /* �o�ӬݪO�s�b�B�S���Q zap ���B�i�H list */

      if (bhno >= 0 && (bits[bhno] & BRD_L_BIT))
      {
	bits[bhno] |= BRD_H_BIT;/* �w���\Ū�O�� */

#ifndef ENHANCED_VISIT
	bstp[bhno] = head[1];	/* �W���\Ū�ɶ� */
#endif

	cbno = bhno;

	if (n > 0)
	{
	  list += n;	/* Thor.980904.����: �̫�@�� tag */

	  if (list > tail)
	    break;

	  do
	  {
	    bhno = *list;
	    if ((bhno & BRH_MASK) > expire)
	      break;

	    if (!(bhno & BRH_SIGN))
	    {
	      if (*--list > expire)
		break;
	      n--;
	    }

	    list--;
	    n--;
	  } while (n > 0);

	  head[2] = n;
	}

	n = n * sizeof(time_t) + sizeof(BRH);
	if (base != head)
	  memcpy(base, head, n);
	base = (int *) ((char *) base + n);
      }
      head += size;
    }
  }

  *base = 0;
  brh_tail = base;

  /* --------------------------------------------------- */
  /* �]�w default board					 */
  /* --------------------------------------------------- */

  strcpy(currboard, BN_NULL);
  currbno = -1;
}


void
brh_save()
{
  int *base, *head, *tail, bhno, size;
  BRD *bhdr, *bend;
  char *bits;

  /* Thor.980830: lkchu patch:  �٨S load �N���� save */ 
  if (!(base = brh_base))
    return;

  brh_put();

  /* save history of un-zapped boards */

  bits = brd_bits;
  head = base;
  tail = brh_tail;
  while (head < tail)
  {
    bhno = bstamp2bno(*head);
    size = head[2] * sizeof(time_t) + sizeof(BRH);
    if (bhno >= 0 && !(bits[bhno] & BRD_Z_BIT))
    {
      if (base != head)
	memcpy(base, head, size);
      base = (int *) ((char *) base + size);
    }
    head = (int *) ((char *) head + size);
  }

  /* save zap record */

  tail = brh_alloc(base, sizeof(time_t) * MAXBOARD);

  bhdr = bshm->bcache;
  bend = bhdr + bshm->number;
  do
  {
    if (*bits++ & BRD_Z_BIT)
    {
      *tail++ = bhdr->bstamp | BRH_SIGN;
    }
  } while (++bhdr < bend);

  /* OK, save it */

  base = brh_base;
  if ((size = (char *) tail - (char *) base) > 0)
  {
    char fpath[64];
    int fd;

    usr_fpath(fpath, cuser.userid, FN_BRH);
    if ((fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0)
    {
      write(fd, base, size);
      close(fd);
    }
  }
}


/* ----------------------------------------------------- */
/* ���� zap �O�� .CZH (Class Zap History)		 */
/* ----------------------------------------------------- */


typedef struct ClassZapHistory
{
  char brdname[BNLEN + 1];	/* ������ brdname */
}                   CZH;


static char class_bits[CH_MAX + 3];


static int			/* >=0:pos  -1:���b .CZH */
czh_find(fpath, brdname)	/* �ˬd�O�_�w�g�b .CZH �� */
  char *fpath;
  char *brdname;
{
  CZH czh;
  int fd, pos;
  int rc = -1;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    pos = 0;
    while (read(fd, &czh, sizeof(CZH)) == sizeof(CZH))
    {
      if (!strcmp(czh.brdname, brdname))
      {
	rc = pos;
	break;
      }
      pos++;
    }
    close(fd);
  }
  return rc;
}


static int		/* >=0:�b .CZH  -1:���b .CZH */
czh_put(brdname)
  char *brdname;
{
  char fpath[64];
  int pos;

  usr_fpath(fpath, cuser.userid, FN_CZH);

  /* �Y�w�g�b .CZH �����ܡA�h�R��������ơF�Y���b .CZH �����ܡA�h�[�J������� */
  if ((pos = czh_find(fpath, brdname)) >= 0)
    rec_del(fpath, sizeof(CZH), pos, NULL);
  else
    rec_add(fpath, brdname, sizeof(CZH));

  return pos;
}


static void
czh_load()
{
  int fsize, chn, min_chn;
  short *chx;
  char *img, fpath[64];
  CZH *chead, *ctail, *czh;

  memset(class_bits, 0, sizeof(class_bits));

  usr_fpath(fpath, cuser.userid, FN_CZH);
  if (chead = (CZH *) f_img(fpath, &fsize))
  {
    min_chn = bshm->min_chn;

    czh = chead;
    ctail = chead + fsize / sizeof(CZH);
    img = class_img;
    do
    {
      for (chn = CH_END - 2;; chn--)	/* �b�Ҧ�����������@�ӪO�W�ۦP�� */
      {
	chx = (short *) img + (CH_END - chn);
	if (!strncmp(czh->brdname, img + *chx, BNLEN))
	{
	  class_bits[-chn] |= BRD_Z_BIT;
	  break;
	}
	if (chn <= min_chn)	/* �p�G�䤣��A��ܸӤ����w�����A�q .CZH �R�� */
	{
	  czh_put(czh->brdname);
	  break;
	}
      }
    } while (++czh < ctail);
    free(chead);
  }
}


/*-------------------------------------------------------*/


int
XoPost(bno)
  int bno;
{
  XO *xo;
  BRD *brd;
  int bits;
  char *str, fpath[64];

  bbstate = STAT_STARTED;

  brd = bshm->bcache + bno;
  str = &brd_bits[bno];
  bits = *str;

#ifdef HAVE_MODERATED_BOARD
  if (!(bits & BRD_R_BIT))
  {
    vmsg("�藍�_�A���O�u��O�Ͷi�J�A�ЦV�O�D�ӽФJ�ҳ\\�i");
    return -1;
  }
#endif

  if (!brd->brdname[0])	/* �w�R�����ݪO */
    return -1;

  if (currbno != bno)
  {
    /* itoc.050613.����: �H�𪺴�֤��O�b���}�ݪO�ɡA�ӬO�b�i�J�s���ݪO�άO�����ɡA
       �o�O���F�קK switch ���ݪO�|����H�� */
    if (currbno >= 0 && bshm->mantime[currbno] > 0)
      bshm->mantime[currbno]--;	/* �h�X�W�@�ӪO */
    bshm->mantime[bno]++;	/* �i�J�s���O */
  }

  if (bits & BRD_M_BIT)
    bbstate |= (STAT_BM | STAT_BOARD | STAT_POST);
  else if (bits & BRD_X_BIT)
    bbstate |= (STAT_BOARD | STAT_POST);
  else if (bits & BRD_W_BIT)
    bbstate |= STAT_POST;

  currbno = bno;
  currbattr = brd->battr;
  strcpy(currboard, brd->brdname);

  brh_get(brd->bstamp, bno);

  /* itoc.011113: �令�Ĥ@���i�O�n�ݳƧѿ� */
  if (!(bits & BRD_V_BIT) || (cuser.ufo & UFO_BRDNOTE))
  {
    *str = bits | BRD_V_BIT;
    brd_fpath(fpath, currboard, fn_note);
    more(fpath, NULL);
  }

  brd_fpath(fpath, currboard, fn_dir);

#ifdef AUTO_JUMPPOST
  xz[XZ_POST - XO_ZONE].xo = xo = xo_get_post(fpath, brd);	/* itoc.010910: �� XoPost �q�����y�@�� xo_get() */
#else
  xz[XZ_POST - XO_ZONE].xo = xo = xo_get(fpath);
#endif
  xo->key = XZ_POST;
  xo->xyz = brd->title;
  str = brd->BM;
  if (*str <= ' ')
    str = "�x�D��";
  sprintf(currBM, "�O�D�G%s", str);

#ifdef HAVE_BRDMATE
  strcpy(cutmp->reading, currboard);
#endif

  return 0;
}


#ifdef HAVE_FORCE_BOARD
void
brd_force()			/* itoc.010407: �j��\Ū���i�O�A�B�j��Ū�̫�@�g */
{
  if (cuser.userlevel)		/* guest ���L */
  {
    int bno;
    BRD *brd;

    if ((bno = brd_bno(BN_ANNOUNCE)) < 0)
      return;
    brd = bshm->bcache + bno;
    if (brd->btime < 0)		/* �|����s brd->blast �N���j��\Ū���i�O */
      return;

#ifdef ENHANCED_VISIT
    brh_get(brd->bstamp, bno);
    while (brh_unread(brd->blast))
#else
    if (brd->blast > brd_visit[bno])
#endif
    {
      vmsg("���s���i�I�Х��\\Ū���s���i��A���}");
      XoPost(bno);
      xover(XZ_POST);

#ifndef ENHANCED_VISIT
      time(&brd_visit[bno]);
#endif
    }
  }
}
#endif	/* HAVE_FORCE_BOARD */


/* ----------------------------------------------------- */
/* Class [�����s��]					 */
/* ----------------------------------------------------- */


#ifdef MY_FAVORITE
int class_flag = 0;			/* favorite.c �n�� */
#else
static int class_flag = 0;
#endif


#ifdef AUTO_JUMPBRD
static int class_jumpnext = 0;	/* itoc.010910: �O�_���h�U�@�ӥ�Ū�O 1:�n 0:���n */
#endif


#define	BFO_YANK	0x01


static int
class_load(xo)
  XO *xo;
{
  short *cbase, *chead, *ctail;
  int chn;			/* ClassHeader number */
  int pos, max, val, zap;
  BRD *brd;
  char *bits;

  /* lkchu.990106: ���� ���� account �y�X class.img �ΨS�� class �����p */
  if (!class_img)
    return 0;

  chn = CH_END - xo->key;

  cbase = (short *) class_img;
  chead = cbase + chn;

  pos = chead[0] + CH_TTLEN;
  max = chead[1];

  chead = (short *) ((char *) cbase + pos);
  ctail = (short *) ((char *) cbase + max);

  max -= pos;

  if (cbase = (short *) xo->xyz)
    cbase = (short *) realloc(cbase, max);
  else
    cbase = (short *) malloc(max);

  xo->xyz = (char *) cbase;

  max = 0;
  brd = bshm->bcache;
  bits = brd_bits;
  zap = (class_flag & BFO_YANK) ? 0 : BRD_Z_BIT;

  do
  {
    chn = *chead++;
    if (chn >= 0)	/* �@��ݪO */
    {
      val = bits[chn];
      if (!(val & BRD_L_BIT) || (val & zap) || !(brd[chn].brdname[0]))
	continue;
    }
    else		/* �����s�� */
    {
      if (class_bits[-chn] & zap)
	continue;
    }

    max++;
    *cbase++ = chn;
  } while (chead < ctail);

  xo->max = max;
  if (xo->pos >= max)
    xo->pos = xo->top = 0;

  return max;
}


static int
XoClass(chn)
  int chn;
{
  XO xo, *xt;

  /* Thor.980727: �ѨM XO xo�����T�w��, 
                  class_load�����| initial xo.max, ��L���T�w */
  xo.pos = xo.top = 0;

  xo.key = chn;
  xo.xyz = NULL;
  if (!class_load(&xo))
  {
    if (xo.xyz)
      free(xo.xyz);
    return 0;
  }

  xt = xz[XZ_CLASS - XO_ZONE].xo;
  xz[XZ_CLASS - XO_ZONE].xo = &xo;

#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    class_jumpnext = 1;	/* itoc.010910: �D�ʸ��h�U�@�ӥ�Ū�ݪO */
#endif
  xover(XZ_CLASS);

  free(xo.xyz);
  xz[XZ_CLASS - XO_ZONE].xo = xt;

  return 1;
}


static inline void
btime_refresh(brd)
  BRD *brd;
{
  /* itoc.020123: ���ަ��L UFO_BRDPOST�A�@����s�A�H�K��Ū�O���|�G */
  if (brd->btime < 0)
  {
    int fd, fsize;
    char folder[64];
    struct stat st;

    brd->btime = 1;
    brd_fpath(folder, brd->brdname, fn_dir);
    if ((fd = open(folder, O_RDONLY)) >= 0)
    {
      if (!fstat(fd, &st) && (fsize = st.st_size) >= sizeof(HDR))
      {
#ifdef ENHANCED_BSHM_UPDATE
	HDR hdr;

	brd->bpost = fsize / sizeof(HDR);
	/* itoc.020829: ��̫�@�g���Q�[�K�B���O�m���� HDR */
	while ((fsize -= sizeof(HDR)) >= 0)
	{
	  lseek(fd, fsize, SEEK_SET);
	  read(fd, &hdr, sizeof(HDR));
	  if (!(hdr.xmode & (POST_RESTRICT | POST_BOTTOM)))
	    break;
	}
	brd->blast = hdr.chrono;
#else
	brd->bpost = fsize / sizeof(HDR);
	lseek(fd, fsize - sizeof(HDR), SEEK_SET);
	read(fd, &brd->blast, sizeof(time_t));
#endif
      }
      else
      {
	brd->blast = brd->bpost = 0;
      }
      close(fd);
    }
  }
}


void
class_item(num, bno, brdpost)
  int num, bno, brdpost;
{
  BRD *brd;
  char *str1, *str2, *str3, token, buf[16];

  brd = bshm->bcache + bno;

  btime_refresh(brd);

  /* �B�z �s��/�g�� */
  if (brdpost)
    num = brd->bpost;

  /* �B�z zap/friend/secret �O���Ÿ� */
  if (brd_bits[bno] & BRD_Z_BIT)
    token = TOKEN_ZAP_BRD;
  else if (brd->readlevel == PERM_SYSOP)
    token = TOKEN_SECRET_BRD;
  else if (brd->readlevel == PERM_BOARD)
    token = TOKEN_FRIEND_BRD;
  else
    token = ' ';

  /* �B�z �wŪ/��Ū */
#ifdef ENHANCED_VISIT
  /* itoc.010407: ��γ̫�@�g�wŪ/��Ū�ӧP�_ */
  brh_get(brd->bstamp, bno);
  str1 = brh_unread(brd->blast) ? ICON_UNREAD_BRD : ICON_READ_BRD;
#else
  str1 = brd->blast > brd_visit[bno] ? ICON_UNREAD_BRD : ICON_READ_BRD;
#endif

  /* �B�z �벼/��H */
  if (brd->bvote)
    str2 = (brd->bvote > 0) ? ICON_VOTED_BRD : ICON_GAMBLED_BRD;
  else
    str2 = (brd->battr & BRD_NOTRAN) ? ICON_NOTRAN_BRD : ICON_TRAN_BRD;

  /* �B�z �H�� */
  bno = bshm->mantime[bno];
  if (bno > 99)
    str3 = "\033[1;31m�z\033[m";
  else if (bno > 0)
    sprintf(str3 = buf, "%2d", bno);
  else
    str3 = "  ";

  /* itoc.010909: �O�W�Ӫ����R���B�[�����C��C���] BCLEN = 4 */
  prints("%6d%c%s%-13s\033[1;3%dm%-5s\033[m%s %-*.*s %s %.*s\n",
    num, token, str1, brd->brdname,
    brd->class[3] & 7, brd->class, str2,
    (d_cols >> 1) + 31, (d_cols >> 1) + 30, brd->title, str3,
    d_cols - (d_cols >> 1) + 13, brd->BM);
}


static int
class_body(xo)
  XO *xo;
{
  short *chp;
  BRD *bcache;
  int n, cnt, max, chn, brdpost;
#ifdef AUTO_JUMPBRD
  int nextpos;
#endif

  bcache = bshm->bcache;
  max = xo->max;
  cnt = xo->top;

#ifdef AUTO_JUMPBRD
  nextpos = 0;

  /* itoc.010910: �j�M�U�@�ӥ�Ū�ݪO */
  if (class_jumpnext)
  {
    class_jumpnext = 0;
    n = xo->pos;
    chp = (short *) xo->xyz + n;

    while (n < max)
    {
      chn = *chp++;
      if (chn >= 0)
      {
	BRD *brd;

	brd = bcache + chn;

#ifdef ENHANCED_VISIT
	/* itoc.010407: ��γ̫�@�g�wŪ/��Ū�ӧP�_ */
	brh_get(brd->bstamp, chn);
	if (brh_unread(brd->blast))
#else
	if (brd->blast > brd_visit[chn])
#endif
	{
	  nextpos = n;
	  break;
	}
      }
      n++;
    }

    /* �U�@�ӥ�Ū�O�b�O���A�n½�L�h */
    if (nextpos >= cnt + XO_TALL)
      return nextpos + XO_MOVE;
  }
#endif

  brdpost = class_flag & UFO_BRDPOST;
  chp = (short *) xo->xyz + cnt;

  n = 3;
  move(3, 0);
  do
  {
    chn = *chp;
    if (cnt < max)
    {
      clrtoeol();
      cnt++;
      if (chn >= 0)		/* �@��ݪO */
      {
	class_item(cnt, chn, brdpost);
      }
      else			/* �����s�� */
      {
	short *chx;
	char *img, *str;

	img = class_img;
	chx = (short *) img + (CH_END - chn);
	str = img + *chx;
	prints("%6d%c  %-13.13s\033[1;3%dm%-5.5s\033[m%s\n", 
	  cnt, class_bits[-chn] & BRD_Z_BIT ? TOKEN_ZAP_BRD : ' ', 
	  str, str[BNLEN + 4] & 7, str + BNLEN + 1, str + BNLEN + 1 + BCLEN + 1);
      }
      chp++;
    }
    else
    {
      clrtobot();
      break;
    }
  } while (++n < b_lines);

#ifdef AUTO_JUMPBRD
  /* itoc.010910: �U�@�ӥ�Ū�O�b�����A�n���в��L�h */
  outf(FEETER_CLASS);   
  return nextpos ? nextpos + XO_MOVE : XO_NONE;
#else
  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: �� b_lines ��W feeter */
#endif
}


static int
class_neck(xo)
  XO *xo;
{
  move(1, 0);
  prints(NECKER_CLASS, 
    class_flag & UFO_BRDPOST ? "�`��" : "�s��", 
    d_cols >> 1, "", d_cols - (d_cols >> 1), "");
  return class_body(xo);
}


static int
class_head(xo)
  XO *xo;
{
  vs_head("�ݪO�C��", str_site);
  return class_neck(xo);
}


static int
class_init(xo)			/* re-init */
  XO *xo;
{
  class_load(xo);
  return class_head(xo);
}


static int
class_postmode(xo)
  XO *xo;
{
  cuser.ufo ^= UFO_BRDPOST;
  cutmp->ufo = cuser.ufo;
  class_flag ^= UFO_BRDPOST;
  return class_neck(xo);
}


static int
class_namemode(xo)		/* itoc.010413: �ݪO�̷Ӧr��/�����ƦC */
  XO *xo;
{
  static time_t last = 0;
  time_t now;
 
  if (time(&now) - last < 10)
  {
    vmsg("�C�Q�����u������@��");
    return XO_FOOT;
  }
  last = now;

  if (cuser.userlevel)
    brh_save();			/* itoc.010711: �x�s�\Ū�O���� */
  cuser.ufo ^= UFO_BRDNAME;
  cutmp->ufo = cuser.ufo;
  board_main();			/* ���s���J class_img */
  return class_neck(xo);
}


static int
class_help(xo)
  XO *xo;
{
  xo_help("class");
  return class_head(xo);
}


static int
class_search(xo)
  XO *xo;
{
  int num, pos, max;
  char buf[BNLEN + 1];

  if (vget(b_lines, 0, MSG_BID, buf, BNLEN + 1, DOECHO))
  {
    short *chp, chn;
    BRD *bcache, *brd;

    str_lowest(buf, buf);

    bcache = bshm->bcache;
    pos = num = xo->pos;
    max = xo->max;
    chp = (short *) xo->xyz;

    do
    {
      if (++pos >= max)
	pos = 0;
      chn = chp[pos];
      if (chn >= 0)
      {
	brd = bcache + chn;
	if (str_str(brd->brdname, buf) || str_sub(brd->title, buf))
	{
	  outf(FEETER_CLASS);	/* itoc.010913: �� b_lines ��W feeter */
	  return pos + XO_MOVE;
	}
      }
    } while (pos != num);
  }

  return XO_FOOT;
}


static int
class_searchBM(xo)
  XO *xo;
{
  int num, pos, max;
  char buf[IDLEN + 1];

  if (vget(b_lines, 0, "�п�J�O�D�G", buf, IDLEN + 1, DOECHO))
  {
    short *chp, chn;
    BRD *bcache, *brd;

    str_lower(buf, buf);

    bcache = bshm->bcache;
    pos = num = xo->pos;
    max = xo->max;
    chp = (short *) xo->xyz;

    do
    {
      if (++pos >= max)
	pos = 0;
      chn = chp[pos];
      if (chn >= 0)
      {
	brd = bcache + chn;
	if (str_str(brd->BM, buf))
	{
	  outf(FEETER_CLASS);	/* itoc.010913: �� b_lines ��W feeter */
	  return pos + XO_MOVE;
	}
      }
    } while (pos != num);
  }

  return XO_FOOT;
}


static int
class_yank(xo)
  XO *xo;
{
  /* itoc.001029: �Ҧ���class,board�C��U, key < 0, 1 �h����M�@�̼Ҧ�
                  �Ϩ䤣��] XO_INIT(�ح���class_load), �p class_yank,
                  ���F�����X���@�̬ݪO�C�����, �]����H  */
  if (xo->key >= 0)
    return XO_NONE;
    
  class_flag ^= BFO_YANK;
  return class_init(xo);
}


static int
class_zap(xo)
  XO *xo;
{
  BRD *brd;
  short *chp;
  int pos, num, chn;
  char token;

  pos = xo->pos;
  chp = (short *) xo->xyz + pos;
  chn = *chp;
  if (chn >= 0)		/* �@��ݪO */
  {
    brd = bshm->bcache + chn;
    if (!(brd->battr & BRD_NOZAP))
    {
      /* itoc.010909: �n�H class_item() ������ */
      move(3 + pos - xo->top, 6);
      num = brd_bits[chn] ^= BRD_Z_BIT;

      /* �B�z zap/friend/secret �O���Ÿ� */
      if (num & BRD_Z_BIT)
	token = TOKEN_ZAP_BRD;
      else if (brd->readlevel == PERM_SYSOP)
	token = TOKEN_SECRET_BRD;
      else if (brd->readlevel == PERM_BOARD)
	token = TOKEN_FRIEND_BRD;
      else
	token = ' ';

      outc(token);
    }
  }
  else			/* �����s�� */
  {
    short *chx;
    char *img, brdname[BNLEN + 1];

    /* itoc.010909: �n�H class_body() ������ */
    move(3 + pos - xo->top, 6);
    num = class_bits[-chn] ^= BRD_Z_BIT;
    outc(num & BRD_Z_BIT ? TOKEN_ZAP_BRD : ' ');

    img = class_img;
    chx = (short *) img + (CH_END - chn);
    str_ncpy(brdname, img + *chx, BNLEN + 1);
    czh_put(brdname);
  }

  /* return XO_NONE; */
  return XO_MOVE + pos + 1;	/* itoc.020219: ���ܤU�@�� */
}


static int
class_zapall(xo)
  XO *xo;
{
  BRD *brdp, *bend;
  int ans, bno;

  ans = vans("�]�w�Ҧ��ݪO (U)�q�\\ (Z)���q�\\ (Q)�����H [Q] ");
  if (ans != 'z' && ans != 'u')
    return XO_FOOT;

  brdp = bshm->bcache;
  bend = brdp + bshm->number;
  bno = 0;
  do
  {
    if (ans == 'z')
    {
      if (!(brdp->battr & BRD_NOZAP))
	brd_bits[bno] |= BRD_Z_BIT;
    }
    else
    {
      brd_bits[bno] &= ~BRD_Z_BIT;
    }

    bno++;
  } while (++brdp < bend);

  class_flag |= BFO_YANK;	/* �j�� yank �_�Ӭݵ��G */
  return class_init(xo);
}


static int
class_visit(xo)		/* itoc.010128: �ݪO�C��]�w�ݪO�wŪ */
  XO *xo;
{
  short *chp;
  int chn;

  chp = (short *) xo->xyz + xo->pos;
  chn = *chp;
  if (chn >= 0)
  {
    BRD *brd;
    brd = bshm->bcache + chn;
    brh_get(brd->bstamp, chn);
    brh_visit(0);
#ifndef ENHANCED_VISIT
    time(&brd_visit[chn]);
#endif
  }
  return class_body(xo);
}


static int
class_unvisit(xo)		/* itoc.010129: �ݪO�C��]�w�ݪO��Ū */
  XO *xo;
{
  short *chp;
  int chn;

  chp = (short *) xo->xyz + xo->pos;
  chn = *chp;
  if (chn >= 0)
  {
    BRD *brd;
    brd = bshm->bcache + chn;
    brh_get(brd->bstamp, chn);
    brh_visit(1);
#ifndef ENHANCED_VISIT
    brd_visit[chn] = 0;	/* itoc.010402: �̪��s���ɶ��k�s�A�ϬݪO�C����ܥ�Ū */
#endif
  }
  return class_body(xo);
}


static int
class_nextunread(xo)
  XO *xo;
{
  int max, pos, chn;
  short *chp;
  BRD *bcache, *brd;

  bcache = bshm->bcache;
  max = xo->max;
  pos = xo->pos;
  chp = (short *) xo->xyz + pos;

  while (++pos < max)
  {
    chn = *(++chp);
    if (chn >= 0 && !(brd_bits[chn] & BRD_Z_BIT))	/* ���L������ zap �����ݪO */
    {
      brd = bcache + chn;

#ifdef ENHANCED_VISIT
      /* itoc.010407: ��γ̫�@�g�wŪ/��Ū�ӧP�_ */
      brh_get(brd->bstamp, chn);
      if (brh_unread(brd->blast))
#else
      if (brd->blast > brd_visit[chn])
#endif
	return pos + XO_MOVE;
    }
  }

  return XO_NONE;
}


static int
class_edit(xo)
  XO *xo;
{
  if (HAS_PERM(PERM_ALLBOARD | PERM_BM))
  {
    short *chp;
    int chn;

    chp = (short *) xo->xyz + xo->pos;
    chn = *chp;
    if (chn >= 0)
    {
      if (!HAS_PERM(PERM_ALLBOARD))
	brd_title(chn);		/* itoc.000312: �O�D�ק襤��ԭz */
      else
	brd_edit(chn);
      return class_init(xo);
    }
  }
  return XO_NONE;
}


static int
hdr_cmp(a, b)
  HDR *a;
  HDR *b;
{
  /* ���������A�A���O�W */
  int k = strncmp(a->title + BNLEN + 1, b->title + BNLEN + 1, BCLEN);
  return k ? k : str_cmp(a->xname, b->xname);
}


static int
class_newbrd(xo)
  XO *xo;
{
  BRD newboard;

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  memset(&newboard, 0, sizeof(BRD));

  /* itoc.010211: �s�ݪO�w�] postlevel = PERM_POST; battr = ����H */
  newboard.postlevel = PERM_POST;
  newboard.battr = BRD_NOTRAN;

  if (brd_new(&newboard) < 0)
    return class_head(xo);

  if (xo->key < CH_END)		/* �b�����s�ո̭� */
  {
    short *chx;
    char *img, *str;
    char xname[BNLEN + 1], fpath[64];
    HDR hdr;

    img = class_img;
    chx = (short *) img + (CH_END - xo->key);
    str = img + *chx;

    str_ncpy(xname, str, sizeof(xname));
    if (str = strchr(xname, '/'))
      *str = '\0';

    /* �[�J�����s�� */
    sprintf(fpath, "gem/@/@%s", xname);
    brd2gem(&newboard, &hdr);
    rec_add(fpath, &hdr, sizeof(HDR));
    rec_sync(fpath, sizeof(HDR), hdr_cmp, NULL);

    vmsg("�s�O����");
  }
  else				/* �b�ݪO�C��̭� */
  {
    vmsg("�s�O���ߡA�O�ۥ[�J�����s��");
  }

  return class_init(xo);
}


static int
class_browse(xo)
  XO *xo;
{
  short *chp;
  int chn;

  chp = (short *) xo->xyz + xo->pos;
  chn = *chp;
  if (chn < 0)		/* �i�J���� */
  {
    if (!XoClass(chn))
      return XO_NONE;
  }
  else			/* �i�J�ݪO */
  {
    if (XoPost(chn))	/* �L�k�\Ū�ӪO */
      return XO_FOOT;
    xover(XZ_POST);
#ifndef ENHANCED_VISIT
    time(&brd_visit[chn]);
#endif
  }

#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    class_jumpnext = 1;		/* itoc.010910: �u���b���}�ݪO�^��ݪO�C��ɤ~�ݭn���h�U�@�ӥ�Ū�ݪO */
#endif

  return class_head(xo);	/* Thor.980701: �L�k�M�֤@�I, �]�� XoPost */
}


int
Select()
{
  int bno;
  BRD *brd;
  char bname[BNLEN + 1];

  if (brd = ask_board(bname, BRD_R_BIT, NULL))
  {
    bno = brd - bshm->bcache;
    XoPost(bno);
    xover(XZ_POST);
#ifndef ENHANCED_VISIT
    time(&brd_visit[bno]);
#endif
  }
  else
  {
    vmsg(err_bid);
  }

  return 0;
}


static int
class_switch(xo)
  XO *xo;
{
  Select();
  return class_head(xo);
}


#ifdef MY_FAVORITE

/* ----------------------------------------------------- */
/* MyFavorite [�ڪ��̷R]				 */
/* ----------------------------------------------------- */


static int 
class_addMF(xo)
  XO *xo;  
{    
  short *chp;
  int chn;

  if (!cuser.userlevel)
    return XO_NONE;
  
  chp = (short *) xo->xyz + xo->pos;
  chn = *chp;
      
  if (chn >= 0)
  {
    BRD *bhdr;

    bhdr = bshm->bcache + chn;

    if (bhdr->brdname[0])	/* ���O�w�R�����ݪO */
    {
      MF mf;
      char fpath[64];

      memset(&mf, 0, sizeof(MF));
      time(&mf.chrono);
      mf.mftype = MF_BOARD;
      strcpy(mf.xname, bhdr->brdname);

      mf_fpath(fpath, cuser.userid, FN_MF);
      rec_add(fpath, &mf, sizeof(MF));
      vmsg("�w�[�J�ڪ��̷R");
    }
  }

  return XO_FOOT;
}
  
#endif  /* MY_FAVORITE */


#ifdef AUTHOR_EXTRACTION
/* Thor.980818: �Q�令�H�ثe���ݪO�C��Τ����ӧ�, ���n����� */


/* opus.1127 : �p�e���g, �i extract author/title */


static int
XoAuthor(xo)
  XO *xo;
{
  int chn, len, max, tag;
  short *chp, *chead, *ctail;
  BRD *brd;
  char key[30], author[IDLEN + 1];
  XO xo_a, *xoTmp;

  vget(b_lines, 0, MSG_XYPOST1, key, 30, DOECHO);
  vget(b_lines, 0, MSG_XYPOST2, author, IDLEN + 1, DOECHO);

  if (!*key && !*author)
    return XO_FOOT;

  str_lowest(key, key);
  str_lower(author, author);
  len = strlen(author);

  chead = (short *) xo->xyz;
  max = xo->max;
  ctail = chead + max;

  tag = 0;
  chp = (short *) malloc(max * sizeof(short));
  brd = bshm->bcache;

  do
  {
    if ((chn = *chead++) >= 0)	/* Thor.980818: ���� group */
    {
      /* Thor.980701: �M����w�@�̤峹, ���h����m, �é�J */

      int fsize;
      char *fimage;

      char folder[80];
      HDR *head, *tail;

      sprintf(folder, "�m�M����w���D�@�̡n�ݪO�G%s \033[5m...\033[m", brd[chn].brdname);
      outz(folder);
      refresh();
      brd_fpath(folder, brd[chn].brdname, fn_dir);

      fimage = f_map(folder, &fsize);

      if (fimage == (char *) -1)
	continue;

      head = (HDR *) fimage;
      tail = (HDR *) (fimage + fsize);

      while (head <= --tail)
      {
	if ((!*key || str_sub(tail->title, key)) &&
	  (!len || !str_ncmp(tail->owner, author, len)))
	{
	  xo_get(folder)->pos = tail - head;
	  chp[tag++] = chn;
	  break;
	}
      }

      munmap(fimage, fsize);
    }
  } while (chead < ctail);

  if (!tag)
  {
    free(chp);
    vmsg("�ŵL�@��");
    return XO_FOOT;
  }

  xo_a.pos = xo_a.top = 0;
  xo_a.max = tag;
  xo_a.key = 1;			/* all boards */
  /* Thor.990621: �Ҧ���class,board�C��U, key < 0, �H 1 �P���`�Ҧ��Ϥ�
                  �Ϩ䤣��] XO_INIT(�ح���class_load), �p class_yank,
                  ���F�����X���@�̬ݪO�C�����, �]����H */ 
  xo_a.xyz = (char *) chp;

  xoTmp = xz[XZ_CLASS - XO_ZONE].xo;	/* Thor.980701: �O�U��Ӫ�class_xo */
  xz[XZ_CLASS - XO_ZONE].xo = &xo_a;

#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    class_jumpnext = 1;	/* itoc.010910: �D�ʸ��h�U�@�ӥ�Ū�ݪO */
#endif
  xover(XZ_CLASS);

  free(chp);
  xz[XZ_CLASS - XO_ZONE].xo = xoTmp;	/* Thor.980701: �٭� class_xo */

  return class_body(xo);
}
#endif


static KeyFunc class_cb[] =
{
  XO_INIT, class_head,
  XO_LOAD, class_body,
  XO_HEAD, class_head,
  XO_BODY, class_body,

  'r', class_browse,
  '/', class_search,
  '?', class_searchBM,
  's', class_switch,
  'c', class_postmode,
  'S', class_namemode,

  'y', class_yank,
  'z', class_zap,
  'Z', class_zapall,
  'v', class_visit,
  'V', class_unvisit,
  '`', class_nextunread,
  'E', class_edit,

#ifdef AUTHOR_EXTRACTION
  'A', XoAuthor,
#endif

#ifdef MY_FAVORITE
  'a', class_addMF,
  'f', class_addMF,
#endif

  Ctrl('P'), class_newbrd,

  'h', class_help
};


int
Class()
{
  /* XoClass(CH_END - 1); */
  /* Thor.980804: ���� ���� account �y�X class.img �ΨS�� class �����p */
  if (!class_img || !XoClass(CH_END - 1))
  {
    vmsg("���w�q���հQ�װ�");
    return XEASY;
  }
  return 0;
}


void
board_main()
{
  int fsize;

  brh_load();

  if (class_img)	/* itoc.030416: �ĤG���i�J board_main �ɡA�n free �� class_img */
  {
    free(class_img);
  }
  else			/* itoc.040102: �Ĥ@���i�J board_main �ɡA�~�ݭn��l�� class_flag */
  {
    class_flag = cuser.ufo & UFO_BRDPOST;	/* �ݪO�C�� 1:�峹�� 0:�s�� */
    if (!cuser.userlevel)			/* guest yank all boards */
      class_flag |= BFO_YANK;
  }

  /* class_img = f_img(CLASS_IMGFILE, &fsize); */
  /* itoc.010413: �̷� ufo �Ӹ��J���P�� class image */
  class_img = f_img(cuser.ufo & UFO_BRDNAME ? CLASS_IMGFILE_NAME : CLASS_IMGFILE_TITLE, &fsize);

  if (class_img == NULL)
    blog("CACHE", "class.img");
  else
    czh_load();

  board_xo.key = CH_END;
  class_load(&board_xo);

  xz[XZ_CLASS - XO_ZONE].xo = &board_xo;	/* Thor: default class_xo */
  xz[XZ_CLASS - XO_ZONE].cb = class_cb;		/* Thor: default class_xo */
}


int
Boards()
{
  /* class_xo = &board_xo; *//* Thor: �w�� default, ���ݧ@�� */

#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    class_jumpnext = 1;	/* itoc.010910: �D�ʸ��h�U�@�ӥ�Ū�ݪO */
#endif
  xover(XZ_CLASS);

  return 0;
}
