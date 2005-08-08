/*-------------------------------------------------------*/
/* xover.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : board/mail interactive reading routines 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef MY_FAVORITE
#define MSG_ZONE_SWITCH	"�ֳt�����G(A)��� (B)�峹 (C)�ݪO (F)�̷R (M)�H�� (U)�ϥΪ� (W)���y�G"
#else
#define MSG_ZONE_SWITCH	"�ֳt�����G(A)��� (B)�峹 (C)�ݪO (M)�H�� (U)�ϥΪ� (W)���y�G"
#endif


/* ----------------------------------------------------- */
/* keep xover record					 */
/* ----------------------------------------------------- */


static XO *xo_root;		/* root of overview list */


XO *
xo_new(path)
  char *path;
{
  XO *xo;
  int len;

  len = strlen(path) + 1;

  xo = (XO *) malloc(sizeof(XO) + len);

  memcpy(xo->dir, path, len);

  return xo;
}


XO *
xo_get(path)
  char *path;
{
  XO *xo;

  for (xo = xo_root; xo; xo = xo->nxt)
  {
    if (!strcmp(xo->dir, path))
      return xo;
  }

  xo = xo_new(path);
  xo->nxt = xo_root;
  xo_root = xo;
  xo->xyz = NULL;
  xo->pos = XO_TAIL;		/* �Ĥ@���i�J�ɡA�N��Щ�b�̫᭱ */

  return xo;
}


#ifdef AUTO_JUMPPOST
XO *
xo_get_post(path, brd)		/* itoc.010910: �Ѧ� xover.c xo_get()�A�� XoPost �q�����y */
  char *path;
  BRD *brd;
{
  XO *xo;
  time_t chrono;
  int fd;
  int pos, locus, mid;	/* locus:������ mid:������ pos:�k���� */

  for (xo = xo_root; xo; xo = xo->nxt)
  {
    if (!strcmp(xo->dir, path))
      return xo;
  }
   
  xo = xo_new(path);
  xo->nxt = xo_root;
  xo_root = xo;
  xo->xyz = NULL;

  /* �|����s brd->blast �� �̫�@�g�wŪ �� �u���@�g�A�h��Ъ�����̫� */
  if (brd->btime < 0 || !brh_unread(brd->blast) || 
    (pos = rec_num(path, sizeof(HDR))) <= 1 || (fd = open(path, O_RDONLY)) < 0)
  {
    xo->pos = XO_TAIL;	/* ��Щ�b�̫᭱ */
    return xo;
  }

  /* ��Ĥ@�g��Ū binary search */
  pos--;
  locus = 0;
  while (1)
  {
    if (pos <= locus + 1)
      break;

    mid = locus + ((pos - locus) >> 1);
    lseek(fd, (off_t) (sizeof(HDR) * mid), SEEK_SET);
    if (read(fd, &chrono, sizeof(time_t)) == sizeof(time_t))
    {
      if (brh_unread(chrono))
	pos = mid;
      else
	locus = mid;
    }
    else
    {
      break;
    }
  }

  /* �S��: �p�G�k���а��d�b 1�A���G�إi��A�@�O��Ū��ĤG�g�A�t�@���s�Ĥ@�g���SŪ */
  if (pos == 1)
  {
    /* �ˬd�Ĥ@�g�O�_�wŪ */
    lseek(fd, (off_t) 0, SEEK_SET);
    if (read(fd, &chrono, sizeof(time_t)) == sizeof(time_t))
    {
      if (brh_unread(chrono))	/* �Y�s�Ĥ@�g�]��Ū�Apos �զ^�h�Ĥ@�g */
	pos = 0;
    }
  }

  close(fd);
  xo->pos = pos;	/* �Ĥ@���i�J�ɡA�N��Щ�b�Ĥ@�g��Ū */

  return xo;
}
#endif


#if 0
void
xo_free(xo)
  XO *xo;
{
  char *ptr;

  if (ptr = xo->xyz)
    free(ptr);
  free(xo);
}
#endif


/* ----------------------------------------------------- */
/* interactive menu routines			 	 */
/* ----------------------------------------------------- */


char xo_pool[(T_LINES - 4) * XO_RSIZ];	/* XO's data I/O pool */


void
xo_load(xo, recsiz)
  XO *xo;
  int recsiz;
{
  int fd, max;

  max = 0;
  if ((fd = open(xo->dir, O_RDONLY)) >= 0)
  {
    int pos, top;
    struct stat st;

    fstat(fd, &st);
    max = st.st_size / recsiz;
    if (max > 0)
    {
      pos = xo->pos;
      if (pos <= 0)
      {
	pos = top = 0;
      }
      else
      {
	top = max - 1;
	if (pos > top)
	  pos = top;
	top = (pos / XO_TALL) * XO_TALL;
      }
      xo->pos = pos;
      xo->top = top;

      lseek(fd, (off_t) (recsiz * top), SEEK_SET);
      read(fd, xo_pool, recsiz * XO_TALL);
    }
    close(fd);
  }

  xo->max = max;
}


int					/* XO_LOAD:�R��  XO_FOOT:���� */
xo_rangedel(xo, size, fchk, fdel)	/* itoc.031001: �Ϭq�R�� */
  XO *xo;
  int size;
  int (*fchk) ();			/* �ˬd�Ϭq���O�_���Q�O�@���O��  0:�R�� 1:�O�@ */
  void (*fdel) ();			/* ���F�R���O���H�~�A�٭n���Ǥ���� */
{
  char ans[8];
  int head, tail;

  vget(b_lines, 0, "[�]�w�R���d��] �_�I�G", ans, 6, DOECHO);
  head = atoi(ans);
  if (head <= 0)
  {
    zmsg("�_�I���~");
    return XO_FOOT;
  }

  vget(b_lines, 28, "���I�G", ans, 6, DOECHO);
  tail = atoi(ans);
  if (tail < head)
  {
    zmsg("���I���~");
    return XO_FOOT;
  }

  if (vget(b_lines, 41, msg_sure_ny, ans, 3, LCECHO) == 'y')
  {
    int fd, total;
    char *data, *phead, *ptail;
    struct stat st;

    if ((fd = open(xo->dir, O_RDONLY)) < 0)
      return XO_FOOT;

    fstat(fd, &st);
    total = st.st_size;
    head = (head - 1) * size;
    tail = tail * size;
    if (head > total)
    {
      close(fd);
      return XO_FOOT;
    }
    if (tail > total)
      tail = total;

    data = (char *) malloc(total);
    read(fd, data, total);
    close(fd);

    total -= tail;
    phead = data + head;
    ptail = data + tail;

    if (fchk || fdel)
    {
      char *ptr;
      for (ptr = phead; ptr < ptail; ptr += size)
      {
	if (fchk && fchk(ptr))		/* �o���O���O�@���Q�� */
	{
	  memcpy(phead, ptr, size);
	  phead += size;
	  head += size;
	}
	else if (fdel)			/* ���F�R���O���A�٭n�� fdel() */
	{
	  fdel(xo, ptr);
	}
      }
    }

    memcpy(phead, ptail, total);

    if ((fd = open(xo->dir, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0)
    {
      write(fd, data, total + head);
      close(fd);
    }

    free(data);
    return XO_LOAD;
  }
  return XO_FOOT;
}


/* ----------------------------------------------------- */
/* Tag List ����					 */
/* ----------------------------------------------------- */


int TagNum;			/* tag's number */
TagItem TagList[TAG_MAX];	/* ascending list */


int
Tagger(chrono, recno, op)
  time_t chrono;
  int recno;
  int op;			/* op : TAG_NIN / TOGGLE / INSERT */
/* ----------------------------------------------------- */
/* return 0 : not found	/ full				 */
/* 1 : add						 */
/* -1 : remove						 */
/* ----------------------------------------------------- */
{
  int head, tail, pos, cmp;
  TagItem *tagp;

  for (head = 0, tail = TagNum - 1, tagp = TagList, cmp = 1; head <= tail;)
  {
    pos = (head + tail) >> 1;
    cmp = tagp[pos].chrono - chrono;
    if (!cmp)
    {
      break;
    }
    else if (cmp < 0)
    {
      head = pos + 1;
    }
    else
    {
      tail = pos - 1;
    }
  }

  if (op == TAG_NIN)
  {
    if (!cmp && recno)		/* �����Y�ԡG�s recno �@�_��� */
      cmp = recno - tagp[pos].recno;
    return cmp;
  }

  tail = TagNum;

  if (!cmp)
  {
    if (op != TAG_TOGGLE)
      return 0;

    TagNum = --tail;
    memcpy(&tagp[pos], &tagp[pos + 1], (tail - pos) * sizeof(TagItem));
    return -1;
  }

  if (tail < TAG_MAX)
  {
    TagItem buf[TAG_MAX];

    TagNum = tail + 1;
    tail = (tail - head) * sizeof(TagItem);
    tagp += head;
    memcpy(buf, tagp, tail);
    tagp->chrono = chrono;
    tagp->recno = recno;
    memcpy(++tagp, buf, tail);
    return 1;
  }

  /* TagList is full */

  bell();
  return 0;
}


void
EnumTag(data, dir, locus, size)
  void *data;
  char *dir;
  int locus;
  int size;
{
  rec_get(dir, data, size, TagList[locus].recno);
}


int
AskTag(msg)
  char *msg;
/* ----------------------------------------------------- */
/* return value :					 */
/* -1	: ����						 */
/* 0	: single article				 */
/* o.w.	: whole tag list				 */
/* ----------------------------------------------------- */
{
  char buf[80];
  int num;

  num = TagNum;

  if (num)	/* itoc.020130: �� TagNum �~�� */
  {  
    sprintf(buf, "�� %s A)��g�峹 T)�аO�峹 Q)���}�H[%c] ", msg, num ? 'T' : 'A');
    switch (vans(buf))
    {
    case 'q':
      return -1;

    case 'a':
      return 0;
    }
  }
  return num;
}


/* ----------------------------------------------------- */
/* tag articles according to title / author		 */
/* ----------------------------------------------------- */


static int
xo_tag(xo, op)
  XO *xo;
  int op;
{
  int fsize, count;
  char *token, *fimage;
  HDR *head, *tail;

  fimage = f_map(xo->dir, &fsize);
  if (fimage == (char *) -1)
    return XO_NONE;

  head = (HDR *) xo_pool + (xo->pos - xo->top);
  if (op == Ctrl('A'))
  {
    token = head->owner;
    op = 0;
  }
  else
  {
    token = str_ttl(head->title);
    op = 1;
  }

  head = (HDR *) fimage;
  tail = (HDR *) (fimage + fsize);

  count = 0;

  do
  {
    if (!strcmp(token, op ? str_ttl(head->title) : head->owner))
    {
      if (!Tagger(head->chrono, count, TAG_INSERT))
	break;
    }
    count++;
  } while (++head < tail);

  munmap(fimage, fsize);
  return XO_BODY;
}


int					/* XO_LOAD:�R��  XO_FOOT/XO_NONE:���� */
xo_prune(xo, size, fvfy, fdel)		/* itoc.031003: ���ҧR�� */
  XO *xo;
  int size;
  int (*fvfy) ();			/* �ˬd�Ϭq���O�_���Q�O�@���O��  0:�R�� 1:�O�@ */
  void (*fdel) ();			/* ���F�R���O���H�~�A�٭n���Ǥ���� */
{
  int fd, total, pos;
  char *data, *phead, *ptail, *ptr;
  char buf[80];
  struct stat st;

  if (!TagNum)
    return XO_NONE;

  sprintf(buf, "�T�w�n�R�� %d �g���Ҷ�(Y/N)�H[N] ", TagNum);
  if (vans(buf) != 'y')
    return XO_FOOT;

  if ((fd = open(xo->dir, O_RDONLY)) < 0)
    return XO_FOOT;

  fstat(fd, &st);
  data = (char *) malloc(total = st.st_size);
  total = read(fd, data, total);
  close(fd);

  phead = data;
  ptail = data + total;
  pos = 0;
  total = 0;

  for (ptr = phead; ptr < ptail; ptr += size)
  {
    if (fvfy(ptr, pos))			/* �o���O���O�@���Q�� */
    {
      memcpy(phead, ptr, size);
      phead += size;
      total += size;
    }
    else if (fdel)			/* ���F�R���O���A�٭n�� fdel() */
    {
      fdel(xo, ptr);
    }
    pos++;
  }

  if ((fd = open(xo->dir, O_WRONLY | O_CREAT | O_TRUNC, 0600)) >= 0)
  {
    write(fd, data, total);
    close(fd);
  }

  free(data);

  TagNum = 0;
  
  return XO_LOAD;
}


/* ----------------------------------------------------- */
/* Tag's batch operation routines			 */
/* ----------------------------------------------------- */


extern BCACHE *bshm;    /* lkchu.981229 */


static int
xo_tbf(xo)
  XO *xo;
{
  char fpath[128], *dir;
  HDR *hdr, xhdr;
  int tag, locus, xmode;
  FILE *fp;

  if (!cuser.userlevel)
    return XO_NONE;

  tag = AskTag("������Ȧs��");
  if (tag < 0)
    return XO_FOOT;

  if (!(fp = tbf_open()))
    return XO_FOOT;

  hdr = tag ? &xhdr : (HDR *) xo_pool + xo->pos - xo->top;

  locus = 0;
  dir = xo->dir;

  do
  {
    if (tag)
    {
      fputs(str_line, fp);
      EnumTag(hdr, dir, locus, sizeof(HDR));
    }

    xmode = hdr->xmode;

    /* itoc.000319: �ץ�����Ť峹���o�פJ�Ȧs�� */
    /* itoc.010602: GEM_RESTRICT �M POST_RESTRICT �ǰt�A�ҥH�[�K�峹�]���o�פJ�Ȧs�� */
    if (xmode & (GEM_RESTRICT | GEM_RESERVED))
      continue;

    if (!(xmode & GEM_FOLDER))		/* �d hdr �O�_ plain text */
    {
      hdr_fpath(fpath, dir, hdr);
      f_suck(fp, fpath);
    }
  } while (++locus < tag);

  fclose(fp);
  zmsg("��������");

  return XO_FOOT;
}


static int
xo_forward(xo)
  XO *xo;
{
  static char rcpt[64];
  char fpath[64], folder[64], *dir, *title, *userid;
  HDR *hdr, xhdr;
  int tag, locus, userno, cc, xmode;
  int method;		/* ��H�� 0:���~ >0:�ۤv <0:��L�����ϥΪ� */

  if (!cuser.userlevel || HAS_PERM(PERM_DENYMAIL))
    return XO_NONE;

  tag = AskTag("��H");
  if (tag < 0)
    return XO_FOOT;

  if (!rcpt[0])
    strcpy(rcpt, cuser.email);

  if (!vget(b_lines, 0, "�ت��a�G", rcpt, sizeof(rcpt), GCARRY))
    return XO_FOOT;

  userid = cuser.userid;
  userno = 0;

  if (!mail_external(rcpt))	/* ���~�d�I */
  {
    if (!str_cmp(rcpt, userid))
    {
      /* userno = cuser.userno; */	/* Thor.981027: �H��ﶰ���ۤv���q���ۤv */
      method = 1;
    }
    else
    {
      if (!HAS_PERM(PERM_LOCAL))
	return XO_FOOT;

      if ((userno = acct_userno(rcpt)) <= 0)
      {
	zmsg(err_uid);
	return XO_FOOT;
      }
      method = -1;
    }

    usr_fpath(folder, rcpt, fn_dir);
  }
  else
  {
    if (!HAS_PERM(PERM_INTERNET))
    {
      vmsg("�z�L�k�H�H�쯸�~");
      return XO_FOOT;
    }

    if (not_addr(rcpt))
    {
      zmsg(err_email);
      return XO_FOOT;
    }

    method = 0;
  }

  hdr = tag ? &xhdr : (HDR *) xo_pool + xo->pos - xo->top;

  dir = xo->dir;
  title = hdr->title;
  locus = 0;
  cc = -1;

  do
  {
    if (tag)
      EnumTag(hdr, dir, locus, sizeof(HDR));

    xmode = hdr->xmode;

    /* itoc.000319: �ץ�����Ť峹���o��H */
    /* itoc.010602: GEM_RESTRICT �M POST_RESTRICT �ǰt�A�ҥH�[�K�峹�]���o��H */
    if (xmode & (GEM_RESTRICT | GEM_RESERVED))
      continue;     

    if (!(xmode & GEM_FOLDER))		/* �d hdr �O�_ plain text */
    {
      hdr_fpath(fpath, dir, hdr);

      if (method)		/* ��H���� */
      {
	HDR mhdr;

	if ((cc = hdr_stamp(folder, HDR_COPY, &mhdr, fpath)) < 0)
	  break;

	if (method > 0)		/* ��H�ۤv */
	{
	  strcpy(mhdr.owner, "[�� �� ��]");
	  mhdr.xmode = MAIL_READ | MAIL_NOREPLY;
	}
	else			/* ��H��L�ϥΪ� */
	{
	  strcpy(mhdr.owner, userid);
	}
	strcpy(mhdr.nick, cuser.username);
	strcpy(mhdr.title, title);
	if ((cc = rec_add(folder, &mhdr, sizeof(HDR))) < 0)
	  break;
      }
      else			/* ��H���~ */
      {
	if ((cc = bsmtp(fpath, title, rcpt, 0)) < 0)
	  break;
      }
    }
  } while (++locus < tag);

  if (userno > 0 && cc >= 0)
    m_biff(userno);

  zmsg(cc >= 0 ? msg_sent_ok : "�����H��L�k�H�F");

  return XO_FOOT;
}


/* ----------------------------------------------------- */
/* �峹�@�̬d�ߡB�v���]�w				 */
/* ----------------------------------------------------- */


int
xo_uquery(xo)
  XO *xo;
{
  HDR *hdr;
  char *userid;

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  userid = hdr->owner;
  if (strchr(userid, '.'))
    return XO_NONE;

  move(1, 0);
  clrtobot();
  my_query(userid);
  return XO_HEAD;
}


int
xo_usetup(xo)
  XO *xo;
{
  HDR *hdr;
  char *userid;
  ACCT acct;

  if (!HAS_PERM(PERM_ALLACCT))
    return XO_NONE;

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  userid = hdr->owner;
  if (strchr(userid, '.') || (acct_load(&acct, userid) < 0))
    return XO_NONE;

  move(3, 0);
  acct_setup(&acct, 1);
  return XO_HEAD;
}


/* ----------------------------------------------------- */
/* �D�D���\Ū						 */
/* ----------------------------------------------------- */


#define RS_TITLE	0x001	/* author/title */
#define RS_FORWARD      0x002	/* backward */
#define RS_RELATED      0x004
#define RS_FIRST	0x008	/* find first article */
#define RS_CURRENT      0x010	/* match current read article */
#define RS_THREAD	0x020	/* search the first article */
#define RS_SEQUENT	0x040	/* sequential read */
#define RS_MARKED 	0x080	/* marked article */
#define RS_UNREAD 	0x100	/* unread article */
#define	RS_BOARD	0x1000	/* �Ω� RS_UNREAD�A��e�������i���| */

#define CURSOR_FIRST	(RS_RELATED | RS_TITLE | RS_FIRST)
#define CURSOR_NEXT	(RS_RELATED | RS_TITLE | RS_FORWARD)
#define CURSOR_PREV	(RS_RELATED | RS_TITLE)
#define RELATE_FIRST	(RS_RELATED | RS_TITLE | RS_FIRST | RS_CURRENT)
#define RELATE_NEXT	(RS_RELATED | RS_TITLE | RS_FORWARD | RS_CURRENT)
#define RELATE_PREV	(RS_RELATED | RS_TITLE | RS_CURRENT)
#define THREAD_NEXT	(RS_THREAD | RS_FORWARD)
#define THREAD_PREV	(RS_THREAD)

/* Thor: �e���mark�峹, ��K���D��������D���B�z */

#define MARK_NEXT	(RS_MARKED | RS_FORWARD | RS_CURRENT)
#define MARK_PREV	(RS_MARKED | RS_CURRENT)


typedef struct
{
  int key;			/* key stroke */
  int map;			/* the mapped threading op-code */
}      KeyMap;


static KeyMap keymap[] =
{
  /* search title / author */

  '?', RS_TITLE | RS_FORWARD,
  '|', RS_TITLE,
  'A', RS_FORWARD,
  'Q', 0,

  /* thread : currtitle */

  '[', RS_RELATED | RS_TITLE | RS_CURRENT,
  ']', RS_RELATED | RS_TITLE | RS_FORWARD | RS_CURRENT,
  '=', RS_RELATED | RS_TITLE | RS_FIRST | RS_CURRENT,

  /* i.e. < > : make life easier */

  ',', RS_THREAD,
  '.', RS_THREAD | RS_FORWARD,

  /* thread : cursor */

  '-', RS_RELATED | RS_TITLE,
  '+', RS_RELATED | RS_TITLE | RS_FORWARD,
  '\\', RS_RELATED | RS_TITLE | RS_FIRST,

  /* Thor: marked : cursor */
  '\'', RS_MARKED | RS_FORWARD | RS_CURRENT,
  ';', RS_MARKED | RS_CURRENT,

  /* Thor: �V�e��Ĥ@�g��Ū���峹 */
  /* Thor.980909: �V�e�䭺�g��Ū, �Υ��g�wŪ */
  '`', RS_UNREAD /* | RS_FIRST */,

  /* sequential */

  ' ', RS_SEQUENT | RS_FORWARD,
  KEY_RIGHT, RS_SEQUENT | RS_FORWARD,
  KEY_PGDN, RS_SEQUENT | RS_FORWARD,
  KEY_DOWN, RS_SEQUENT | RS_FORWARD,
  /* Thor.990208: ���F��K�ݤ峹�L�{��, ���ܤU�g, ���M�W�h�Qxover�Y���F:p */
  'j', RS_SEQUENT | RS_FORWARD,

  KEY_UP, RS_SEQUENT,
  KEY_PGUP, RS_SEQUENT,
  /* Thor.990208: ���F��K�ݤ峹�L�{��, ���ܤW�g, ���M�W�h�Qxover�Y���F:p */
  'k', RS_SEQUENT,

  /* end of keymap */

  (char) NULL, -1
};


static int
xo_keymap(key)
  int key;
{
  KeyMap *km;
  int ch;

  km = keymap;
  while (ch = km->key)
  {
    if (ch == key)
      break;
    km++;
  }
  return km->map;
}


/* itoc.010913: xo_thread() �^�ǭ�                */
/*  XO_NONE: �S���δN�O��ЩҦb�A���βM b_lines */
/*  XO_FOOT: �S���δN�O��ЩҦb�A�ݭn�M b_lines */
/*  XO_BODY: ���F�A���b�O��                     */
/* -XO_NONE: ���F�A�N�b�����A���βM b_lines     */
/* -XO_FOOT: ���F�A�N�b�����A�ݭn�M b_lines     */


static int
xo_thread(xo, op)
  XO *xo;
  int op;
{
  static char s_author[16], s_title[32], s_unread[2] = "0";
  char buf[80];

  char *tag, *query, *title;
  const int origpos = xo->pos, origtop = xo->top, max = xo->max;
  int pos, match, near, neartop;	/* Thor: neartop �P near ����� */
  int top, bottom, step, len;
  HDR *pool, *fhdr;

  match = XO_NONE;
  pos = origpos;
  top = origtop;
  pool = (HDR *) xo_pool;
  fhdr = pool + (pos - top);
  near = 0;
  step = (op & RS_FORWARD) - 1;		/* (op & RS_FORWARD) ? 1 : -1 */

  if (op & RS_RELATED)
  {
    tag = fhdr->title;
    if (op & RS_CURRENT)
    {
      query = currtitle;
      if (op & RS_FIRST)
      {
	if (!strcmp(query, tag))	/* �ثe���N�O�Ĥ@���F */
	  return XO_NONE;
	near = -1;
      }
    }
    else
    {
      title = str_ttl(tag);
      if (op & RS_FIRST)
      {
	if (title == tag)
	  return XO_NONE;
	near = -1;
      }
      strcpy(query = buf, title);
    }
  }
  else if (op & RS_UNREAD)
  {
    /* Thor.980909: �߰� "���g��Ū" �� "���g�wŪ" */

    near = xo->dir[0];
    if (near != 'b' && near != 'u')	/* itoc.010913: �u���\�b�ݪO/�H�c�j�M */
      return XO_NONE;			/* itoc.040916.bug: �S������b�H�c��ذϷj�M */

    if (!vget(b_lines, 0, "�V�e��M 0)���g��Ū 1)���g�wŪ ", s_unread, sizeof(s_unread), GCARRY))
      return XO_FOOT;

    if (*s_unread == '0')
      op |= RS_FIRST;

    if (near == 'b')		/* search board */
      op |= RS_BOARD;

    near = -1;
  }
  else if (!(op & (RS_THREAD | RS_SEQUENT | RS_MARKED)))
  {
    if (op & RS_TITLE)
    {
      title = "���D";
      tag = s_title;
      len = sizeof(s_title);
    }
    else
    {
      title = "�@��";
      tag = s_author;
      len = sizeof(s_author);
    }

    sprintf(query = buf, "�j�M%s(%s)�G", title, (step > 0) ? "��" : "��");
    if (!vget(b_lines, 0, query, tag, len, GCARRY))
      return XO_FOOT;

    str_lowest(query, tag);
  }

  bottom = top + XO_TALL;
  if (bottom > max)
    bottom = max;

  for (;;)
  {
    if (step > 0)
    {
      if (++pos >= max)
	break;
    }
    else
    {
      if (--pos < 0)
	break;
    }

    /* buffer I/O : shift sliding window scope */

    if (pos < top || pos >= bottom)
    {
      xo->pos = pos;
      xo_load(xo, sizeof(HDR));

      top = xo->top;
      bottom = top + XO_TALL;
      if (bottom > max)
	bottom = max;

      fhdr = pool + (pos - top);
    }
    else
    {
      fhdr += step;
    }

#ifdef HAVE_REFUSEMARK
    if ((fhdr->xmode & POST_RESTRICT) &&
      strcmp(fhdr->owner, cuser.userid) && !(bbstate & STAT_BM))
      continue;
#endif

    if (op & RS_SEQUENT)
    {
      match = -1;
      break;
    }

    /* Thor: �e�� search marked �峹 */

    if (op & RS_MARKED)
    {
      if (fhdr->xmode & POST_MARKED)
      {
	match = -1;
	break;
      }
      continue;
    }

    /* �V�e��M��M��Ū/�wŪ�峹 */

    if (op & RS_UNREAD)
    {
#define UNREAD_FUNC()   (op & RS_BOARD ? brh_unread(fhdr->chrono) : !(fhdr->xmode & MAIL_READ))
      if (op & RS_FIRST)	/* ���g��Ū */
      {
	if (UNREAD_FUNC())
	{
	  near = pos;
	  neartop = top;
	  continue;
	}
      }
      else			/* ���g�wŪ */
      {
	if (!UNREAD_FUNC())
	{
	  match = -1;
	  break;
	}
      }
      continue;
    }

    /* ------------------------------------------------- */
    /* �H�U�j�M title / author				 */
    /* ------------------------------------------------- */

    if (op & (RS_TITLE | RS_THREAD))
    {
      title = fhdr->title;	/* title ���V [title] field */
      tag = str_ttl(title);	/* tag ���V thread's subject */

      if (op & RS_THREAD)
      {
	if (tag == title)
	{
	  match = -1;
	  break;
	}
	continue;
      }
    }
    else
    {
      tag = fhdr->owner;	/* tag ���V [owner] field */
    }

    if (((op & RS_RELATED) && !strncmp(tag, query, 40)) ||
      (!(op & RS_RELATED) && str_sub(tag, query)))
    {
      if ((op & RS_FIRST) && tag != title)
      {
	near = pos;		/* �O�U�̱���_�I����m */
	neartop = top;
	continue;
      }

      match = -1;
      break;
    }
  }

  /* top = xo->top = buffering �� top */
  /* �p�G match = -1 ��ܧ��F�A�� pos, top = �n�h���a�� */
  /* �p�G RS_FIRST && near >= 0 ��ܧ��F�A�� near, neartop = �n�h���a�� */

#define CLEAR_FOOT()	(!(op & RS_RELATED) && ((op & RS_UNREAD) || !(op & (RS_THREAD | RS_SEQUENT | RS_MARKED))))
  
  if (match < 0)			/* ���F */
  {
    xo->pos = pos;			/* ��n�h����m��i�h */

    if (top != origtop)			/* �b�O�����F */
      match = XO_BODY;
    else				/* �b�������F */
      match = CLEAR_FOOT() ? -XO_FOOT : -XO_NONE;
  }
  else if ((op & RS_FIRST) && near >= 0)/* ���F */
  {
    xo->pos = near;			/* ��n�h����m��i�h */

    /* �ѩ�O RS_FIRST ��Ĥ@�g�A�ҥH buffering �� top �i�����̫ᵲ�G neartop ��e�� */
    if (top != neartop)
      xo_load(xo, sizeof(HDR));

    if (xo->top != origtop)		/* �b�O�����F */
      match = XO_BODY;
    else				/* �b�������F */
      match = CLEAR_FOOT() ? -XO_FOOT : -XO_NONE;
  }
  else					/* �䤣�� */
  {
    xo->pos = origpos;			/* �٭��Ӧ�m */

    if (top != origtop)			/* �^�ثe�Ҧb�� */
      xo_load(xo, sizeof(HDR));

    match = CLEAR_FOOT() ? XO_FOOT : XO_NONE;
  }

  return match;
}


/* Thor.990204: ���Ҽ{more �Ǧ^��, �H�K�ݤ@�b�i�H�� []... 
                ch �����emore()���ҫ���key */   
int
xo_getch(xo, ch)
  XO *xo;
  int ch;
{
  int op;

  if (!ch)
    ch = vkey();

  op = xo_keymap(ch);
  if (op >= 0)
  {
    ch = xo_thread(xo, op);
    if (ch != XO_NONE)
      ch = XO_BODY;		/* �~���s�� */
  }

  return ch;
}


/* ----------------------------------------------------- */
/* XZ							 */
/* ----------------------------------------------------- */


extern KeyFunc pal_cb[];
extern KeyFunc bmw_cb[];
extern KeyFunc post_cb[];


XZ xz[] =
{
  {NULL, NULL, M_BOARD, FEETER_CLASS},		/* XZ_CLASS */
  {NULL, NULL, M_LUSERS, FEETER_ULIST},		/* XZ_ULIST */
  {NULL, pal_cb, M_PAL, FEETER_PAL},		/* XZ_PAL */
  {NULL, NULL, M_PAL, FEETER_ALOHA},		/* XZ_ALOHA */
  {NULL, NULL, M_VOTE, FEETER_VOTE},		/* XZ_VOTE */
  {NULL, bmw_cb, M_BMW, FEETER_BMW},		/* XZ_BMW */
  {NULL, NULL, M_MF, FEETER_MF},		/* XZ_MF */
  {NULL, NULL, M_COSIGN, FEETER_COSIGN},	/* XZ_COSIGN */
  {NULL, NULL, M_SONG, FEETER_SONG},		/* XZ_SONG */
  {NULL, NULL, M_READA, FEETER_NEWS},		/* XZ_NEWS */
  {NULL, NULL, M_READA, FEETER_XPOST},		/* XZ_XPOST */
  {NULL, NULL, M_RMAIL, FEETER_MBOX},		/* XZ_MBOX */
  {NULL, post_cb, M_READA, FEETER_POST},	/* XZ_POST */
  {NULL, NULL, M_GEM, FEETER_GEM}		/* XZ_GEM */
};


static int
xo_jump(pos, zone)
  int pos;			/* ���ʴ�Ш� number �Ҧb���S�w��m */
  int zone;			/* itoc.010403: �� zone �]�Ƕi�� */
{
  char buf[6];

  buf[0] = pos;
  buf[1] = '\0';
  vget(b_lines, 0, "���ܲĴX���G", buf, sizeof(buf), GCARRY);

#if 0
  move(b_lines, 0);
  clrtoeol();
#endif
  outf(xz[zone].feeter);	/* itoc.010403: �� b_lines ��W feeter */

  pos = atoi(buf);

  if (pos > 0)
    return XO_MOVE + pos - 1;

  return XO_NONE;
}


/* ----------------------------------------------------- */
/* interactive menu routines			 	 */
/* ----------------------------------------------------- */


void
xover(cmd)
  int cmd;
{
  int pos, num, zone, sysmode;
  XO *xo;
  KeyFunc *xcmd, *cb;

  for (;;)
  {
    while (cmd != XO_NONE)
    {
      if (cmd == XO_FOOT)
      {
	outf(xz[zone].feeter);	/* itoc.010403: �� b_lines ��W feeter */
	break;
      }

      if (cmd >= XO_ZONE)
      {
	/* --------------------------------------------- */
	/* switch zone					 */
	/* --------------------------------------------- */

	zone = cmd;
	cmd -= XO_ZONE;
	xo = xz[cmd].xo;
	xcmd = xz[cmd].cb;
	sysmode = xz[cmd].mode;

	TagNum = 0;		/* clear TagList */
	cmd = XO_INIT;
	utmp_mode(sysmode);
      }
      else if (cmd >= XO_MOVE - XO_TALL)
      {
	/* --------------------------------------------- */
	/* calc cursor pos and show cursor correctly	 */
	/* --------------------------------------------- */

	/* cmd >= XO_MOVE - XO_TALL so ... :chuan: ���] cmd = -1 ?? */

	/* fix cursor's range */

	num = xo->max - 1;

	/* pos = (cmd | XO_WRAP) - (XO_MOVE + XO_WRAP); */
	/* cmd &= XO_WRAP; */
	/* itoc.020124: �ץ��b�Ĥ@���� PGUP�B�̫�@���� PGDN�B�Ĥ@���� UP�B�̫�@���� DOWN �|�� XO_WRAP �X�Ю��� */

	if (cmd > XO_MOVE + (XO_WRAP >> 1))     /* XO_WRAP >> 1 ���j�L�峹�� */
	{
	  pos = cmd - (XO_MOVE + XO_WRAP);
	  cmd = 1;				/* �� KEY_UP �� KEY_DOWN */
	}
	else
	{
	  pos = cmd - XO_MOVE;
	  cmd = 0;
	}

	/* pos: �n���h������  cmd: �O�_�� KEY_UP �� KEY_DOWN */

	if (pos < 0)
	{
	  /* pos = (zone == XZ_POST) ? 0 : num; *//* itoc.000304: �\Ū��Ĥ@�g�� KEY_UP �� KEY_PGUP ���|½��̫� */
	  pos = num;	/* itoc.020124: ½��̫�����K�A���Ӥ��|���H�O���WŪ�� :P */
	}
	else if (pos > num)
	{
	  if (zone == XZ_POST)
	    pos = num;	/* itoc.000304: �\Ū��̫�@�g�� KEY_DOWN �� KEY_PGDN ���|½��̫e */
	  else
	    pos = (cmd || pos == num + XO_TALL) ? 0 : num;	/* itoc.020124: �n�קK�p�G�b�˼ƲĤG���� KEY_PGDN�A
								   �ӳ̫�@���g�ƤӤַ|�������h�Ĥ@���A�ϥΪ̷|
								   �����D���̫�@���A�G���b�̫�@�����@�U */
	}

	/* check cursor's range */

	cmd = xo->pos;

	if (cmd == pos)
	  break;

	xo->pos = pos;
	num = xo->top;
	if ((pos < num) || (pos >= num + XO_TALL))
	{
	  xo->top = (pos / XO_TALL) * XO_TALL;
	  cmd = XO_LOAD;	/* ���J��ƨä��H��� */
	}
	else
	{
	  move(3 + cmd - num, 0);
	  outc(' ');

	  break;		/* �u���ʴ�� */
	}
      }

      /* ----------------------------------------------- */
      /* ���� call-back routines			 */
      /* ----------------------------------------------- */

      cb = xcmd;
      num = cmd | XO_DL; /* Thor.990220: for dynamic load */
      for (;;)
      {
	pos = cb->key;
#if 1
	/* Thor.990220: dynamic load , with key | XO_DL */
	if (pos == num)
	{
	  void *p = DL_get((char *) cb->func);
	  if (p) 
	  {
	    cb->func = p;
	    pos = cb->key = cmd;
	  }
	  else
	  {
	    cmd = XO_NONE;
	    break;
	  }
	}
#endif
	if (pos == cmd)
	{
	  cmd = (*(cb->func)) (xo);

	  if (cmd == XO_QUIT)
	    return;

	  break;
	}
	
	if (pos == 'h')		/* itoc.001029: 'h' �O�@�S�ҡA�N�� *_cb ������ */
	{
	  cmd = XO_NONE;	/* itoc.001029: �N��䤣�� call-back, ���@�F! */
	  break;
	}

	cb++;
      }

    } /* Thor.990220.����: end of while (cmd!=XO_NONE) */

    utmp_mode(sysmode); 
    /* Thor.990220:����:�ΨӦ^�_ event handle routine �^�ӫ᪺�Ҧ� */

    pos = xo->pos;

    if (xo->max > 0)		/* Thor: �Y�O�L�F��N��show�F */
    {
      num = 3 + pos - xo->top;
      move(num, 0);
      outc('>');
    }

    cmd = vkey();

    /* itoc.����: �H�U�w�q�F�򥻫���A�ҿװ򥻫���A�N�O���ʪ��������A�q�Ω�Ҧ� XZ_ ���a�� */  

    /* ------------------------------------------------- */
    /* �򥻪���в��� routines				 */
    /* ------------------------------------------------- */

    if (cmd == KEY_LEFT || cmd == 'e')
    {
      TagNum = 0;	/* itoc.050413: �q��ذϦ^��峹�C��ɭn�M�� tag */
      return;
    }
    else if (xo->max <= 0)	/* Thor: �L�F��h�L�k����� */
    {
      continue;
    }
    else if (cmd == KEY_UP || cmd == 'k')
    {
      cmd = pos - 1 + XO_MOVE + XO_WRAP;
    }
    else if (cmd == KEY_DOWN || cmd == 'j')
    {
      cmd = pos + 1 + XO_MOVE + XO_WRAP;
    }
    else if (cmd == ' ' || cmd == KEY_PGDN || cmd == 'N')
    {
      cmd = pos + XO_TALL + XO_MOVE;
    }
    else if (cmd == KEY_PGUP || cmd == 'P')
    {
      cmd = pos - XO_TALL + XO_MOVE;
    }
    else if (cmd == KEY_HOME || cmd == '0')
    {
      cmd = XO_MOVE;
    }
    else if (cmd == KEY_END || cmd == '$')
    {
      cmd = xo->max - 1 + XO_MOVE;
    }
    else if (cmd >= '1' && cmd <= '9')
    {
      cmd = xo_jump(cmd, zone);
    }
    else if (cmd == KEY_RIGHT || cmd == '\n')
    {
      cmd = 'r';
    }

    /* ------------------------------------------------- */
    /* switch Zone					 */
    /* ------------------------------------------------- */

#ifdef  EVERY_Z
    else if (cmd == Ctrl('Z'))
    {
      cmd = every_Z(zone);
    }
    else if (cmd == Ctrl('U'))
    {
      cmd = every_U(zone);
    }
#endif

    /* ------------------------------------------------- */
    /* ��l������					 */
    /* ------------------------------------------------- */

    else
    {
      if (zone >= XZ_XPOST)		/* xo_pool ���񪺬O HDR */
      {
	/* --------------------------------------------- */
	/* Tag						 */
	/* --------------------------------------------- */

	if (cmd == 'C')
	{
	  cmd = xo_tbf(xo);
	}
	else if (cmd == 'F')
	{
	  cmd = xo_forward(xo);
	}
	else if (cmd == Ctrl('C'))
	{
	  if (TagNum)
	  {
	    TagNum = 0;
	    cmd = XO_BODY;
	  }
	  else
	    cmd = XO_NONE;
	}
	else if (cmd == Ctrl('A') || cmd == Ctrl('T'))
	{
	  cmd = xo_tag(xo, cmd);
	}

	/* --------------------------------------------- */
	/* �D�D���\Ū					 */
	/* --------------------------------------------- */

	if (zone == XZ_XPOST)		/* �걵�����䴩�D�D���\Ū */
	  continue;

	pos = xo_keymap(cmd);
	if (pos >= 0)			/* �p�G���O����V�� */
	{
	  cmd = xo_thread(xo, pos);	/* �h�d�d�O���@�� thread �j�M */	  

	  if (cmd < 0)		/* �b������� match */
	  {
	    move(num, 0);
	    outc(' ');
	    /* cmd = XO_NONE; */
	    /* itoc.010913: �Y�Ƿj�M�n�� b_lines ��W feeter */
	    cmd = -cmd;
	  }
	}
      }

      /* ----------------------------------------------- */
      /* ��L���浹 call-back routine �h�B�z		 */
      /* ----------------------------------------------- */

    } /* Thor.990220.����: end of vkey() handling */
  }
}


/* ----------------------------------------------------- */
/* Thor.980725: ctrl Z everywhere			 */
/* ----------------------------------------------------- */


#ifdef	EVERY_Z
static int z_status = 0;	/* �i�J�X�h */

int
every_Z(zone)
  int zone;				/* �ǤJ�Ҧb XZ_ZONE�A�Y�ǤJ 0�A��ܤ��b xover() �� */
{
  int cmd, tmpbno, tmpmode;

  /* itoc.000319: �̦h every_Z �@�h */
  if (z_status >= 1)
    return XO_NONE;
  else
    z_status++;

  cmd = zone;
  outz(MSG_ZONE_SWITCH);

  tmpbno = vkey();	/* �ɥ� tmpbno �Ӵ����p�g */
  if (tmpbno >= 'A' && tmpbno <= 'Z')
    tmpbno |= 0x20;
  switch (tmpbno)
  {
  case 'a':
    cmd = XZ_GEM;
    break;

  case 'b':
    if (xz[XZ_POST - XO_ZONE].xo)	/* �Y�w��w�ݪO�A�i�J�ݪO�A�_�h��ݪO�C�� */
    {
      XoPost(currbno);
      cmd = XZ_POST;
      break;
    }

  case 'c':
    cmd = XZ_CLASS;
    break;

#ifdef MY_FAVORITE
  case 'f':
    if (cuser.userlevel)
      cmd = XZ_MF;
    break;
#endif

  case 'm':
    if (cuser.userlevel)
      cmd = XZ_MBOX;
    break;

  case 'u':
    cmd = XZ_ULIST;
    break;

  case 'w':
    if (cuser.userlevel)
      cmd = XZ_BMW;
    break;
  }

  if (cmd == zone)		/* �M�ثe�Ҧb zone �@�ˡA�Ψ��� */
  {
    z_status--;
    return XO_FOOT;		/* �Y�b xover() �������I�s every_Z() �h�e�^ XO_FOOT �Y�i��ø */
  }

#ifdef MY_FAVORITE
  if (zone == XZ_POST && (cmd == XZ_CLASS || cmd == XZ_MF))
#else
  if (zone == XZ_POST && cmd == XZ_CLASS)
#endif
    tmpbno = currbno;
  else
    tmpbno = -1;

  tmpmode = bbsmode;
  xover(cmd);
  utmp_mode(tmpmode);

  if (tmpbno >= 0)		/* itoc.030731: ���i��i�J�O���O�A�N�ݭn���s XoPost�A�|�A�ݤ@���i�O�e�� */
    XoPost(tmpbno);

  z_status--;
  return XO_INIT;		/* �ݭn���s���J xo_pool�A�Y�b xover() ���]�i�Ǧ���ø */
}


int
every_U(zone)
  int zone;			/* �ǤJ�Ҧb XZ_ZONE�A�Y�ǤJ 0�A��ܤ��b xover() �� */
{
  /* itoc.000319: �̦h every_Z �@�h */
  if (z_status >= 1)
    return XO_NONE;

  if (zone != XZ_ULIST)
  {
    int tmpmode;

    z_status++;
    tmpmode = bbsmode;
    xover(XZ_ULIST);
    utmp_mode(tmpmode);
    z_status--;
  }
  return XO_INIT;
}
#endif


/* ----------------------------------------------------- */
/* �� XZ_* ���c����в���				 */
/* ----------------------------------------------------- */


/* �ǤJ: ch, pagemax, num, pageno, cur, redraw */
/* �ǥX: ch, pageno, cur, redraw */
int
xo_cursor(ch, pagemax, num, pageno, cur, redraw)
  int ch, pagemax, num;
  int *pageno, *cur, *redraw;
{
  switch (ch)
  {
  case KEY_LEFT:
  case 'q':
    return 'q';

  case KEY_PGUP:
    if (pagemax != 0)
    {
      if (*pageno)
      {
	(*pageno)--;
      }
      else
      {
	*pageno = pagemax;
	*cur = num % XO_TALL;
      }
      *redraw = 1;
    }
    break;

  case KEY_PGDN:
    if (pagemax != 0)
    {
      if (*pageno == pagemax)
      {
	/* �b�̫�@�����@�U */
	if (*cur != num % XO_TALL)
	{
	  *cur = num % XO_TALL;
	}
	else
	{
	  *pageno = 0;
	  *cur = 0;
	}
      }
      else
      {
	(*pageno)++;
	if (*pageno == pagemax && *cur > num % XO_TALL)
	  *cur = num % XO_TALL;
      }
      *redraw = 1;
    }
    break;

  case KEY_UP:
  case 'k':
    if (*cur == 0)
    {
      if (*pageno != 0)
      {
	*cur = XO_TALL - 1;
	*pageno = *pageno - 1;
      }
      else
      {
	*cur = num % XO_TALL;
	*pageno = pagemax;
      }
      *redraw = 1;
    }
    else
    {
      move(3 + *cur, 0);
      outc(' ');
      (*cur)--;
      move(3 + *cur, 0);
      outc('>');
    }
    break;

  case KEY_DOWN:
  case 'j':
    if (*cur == XO_TALL - 1)
    {
      *cur = 0;
      *pageno = (*pageno == pagemax) ? 0 : *pageno + 1;
      *redraw = 1;
    }
    else if (*pageno == pagemax && *cur == num % XO_TALL)
    {
      *cur = 0;
      *pageno = 0;
      *redraw = 1;
    }
    else
    {
      move(3 + *cur, 0);
      outc(' ');
      (*cur)++;
      move(3 + *cur, 0);
      outc('>');
    }
    break;

  case KEY_HOME:
  case '0':
    *pageno = 0;
    *cur = 0;
    *redraw = 1;
    break;

  case KEY_END:
  case '$':
    *pageno = pagemax;
    *cur = num % XO_TALL;
    *redraw = 1;
    break;
  }

  return ch;
}


/* ----------------------------------------------------- */
/* �������						 */
/* ----------------------------------------------------- */


void
xo_help(path)			/* itoc.021122: ������� */
  char *path;
{
  /* itoc.030510: ��� so �̭� */
  DL_func("bin/help.so:vaHelp", path);
}
