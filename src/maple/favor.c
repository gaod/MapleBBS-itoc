/*-------------------------------------------------------*/
/* favorite.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : �ڪ��̷R					 */
/* create : 00/06/16					 */
/* update :   /  /  					 */
/* author : weichung.bbs@bbs.ntit.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef MY_FAVORITE

static int mf_add();
static int mf_paste();
static int mf_load();
static void XoMF();


extern XZ xz[];
extern char xo_pool[];
extern char brd_bits[];		/* itoc.010821: �P�_�O�_���\Ū�ݪO���v�� */
extern BCACHE *bshm;

#ifndef ENHANCED_VISIT
extern time_t brd_visit[];
#endif

#ifdef AUTO_JUMPBRD
static int mf_jumpnext = 0;     /* itoc.020615: �O�_���h�U�@�ӥ�Ū�O 1:�n 0:���n */
#endif


static MF mftmp;		/* for copy & paste */

static int mf_depth;


void
mf_fpath(fpath, userid, fname)
  char *fpath;
  char *userid;
  char *fname;
{
  char buf[64];

  sprintf(buf, "MF/%s", fname);
  usr_fpath(fpath, userid, buf);
}


static void
mf_item(num, mf)
  int num;
  MF *mf;
{
  char folder[64];
  int mftype, brdpost, bno;

  mftype = mf->mftype;
  brdpost = cuser.ufo & UFO_BRDPOST;

  if (mftype & MF_FOLDER)
  {
    if (brdpost)
    {
      mf_fpath(folder, cuser.userid, mf->xname);
      num = rec_num(folder, sizeof(MF));
    }
    prints("%6d%c  %s %s\n", num, mftype & MF_MARK ? ')' : ' ', "��", mf->title);
  }
  else if (mftype & MF_BOARD)
  {
    if ((bno = brd_bno(mf->xname)) >= 0)
      class_item(num, bno, brdpost);
    else
      /* itoc.010821: �������ݪO�A�� user �ۤv�M���A�p�� user �~���D���ǬݪO�Q��F */
      prints("         \033[36m<%s �w��W�γQ�R���A�бN�����|�R��>\033[m\n", mf->xname);
  }
  else if (mftype & MF_GEM)
  {
    prints("%6d%c  %s %s\n",
      brdpost ? 0 : num, 
      mftype & MF_MARK ? ')' : ' ', "��", mf->title);
  }
  else  if (mftype & MF_LINE)		/* qazq.040721: ���j�u */
  {
    prints("%6d   %s\n", 
      brdpost ? 0 : num, mf->title);
  }
  else /* if (mftype & MF_CLASS) */	/* LHD.051007: �����s�� */
  {
    char cname[BNLEN + 2];

    sprintf(cname, "%s/", mf->xname);
    prints("%6d   %-13.13s\033[1;3%dm%-5.5s\033[m%s\n",
      num, cname, mf->class[3] & 7, mf->class, mf->title);
  }
}


static int
mf_body(xo)
  XO *xo;
{
  MF *mf;
  int max, num, tail;
#ifdef AUTO_JUMPBRD
  int nextpos;
  static int originpos = -1;
#endif

  max = xo->max;
  if (max <= 0)
  {
    max = vans("�ڪ��̷R (A)�s�W (P)�K�W (Q)���} [Q] ");
    switch (max)
    {
    case 'a':

      max = mf_add(xo);
      if (xo->max > 0)
	return max;
      break;

    case 'p':

      mf_paste(xo);
      return mf_load(xo);
    }
 
    return XO_QUIT;
  }

  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

#ifdef AUTO_JUMPBRD
  nextpos = 0;

  /* itoc.020615: �j�M�U�@�ӥ�Ū�ݪO */
  if (mf_jumpnext)
  {
    BRD *bcache;

    tail = xo->pos;	/* �ɥ� tail */
    if (originpos < 0)	/* �b�h�U���j�M��Ū�ݪO�e�A�O���@�}�l��ЩҦb */
      originpos = tail;
    mf = (MF *) xo_pool + tail - num;
    bcache = bshm->bcache;

    while (tail < max)	/* �u��䥻��������Ū�ݪO�A�]���U���٨S���J */
    {
      if (mf->mftype & MF_BOARD)
      {
	int chn;
	BRD *brd;

	chn = brd_bno(mf->xname);
	brd = bcache + chn;

#ifdef ENHANCED_VISIT
	/* itoc.010407: ��γ̫�@�g�wŪ/��Ū�ӧP�_ */
	brh_get(brd->bstamp, chn);
	if (brh_unread(brd->blast))
#else
	if (brd->blast > brd_visit[chn])
#endif
	{
	  nextpos = tail;
	  mf_jumpnext = 0;
	  originpos = -1;
	  break;
	}
      }
      tail++;
      mf++;
    }

    if (mf_jumpnext)	/* �p�G�b�����S����쥼Ū�ݪO */
    {
      if (max < rec_num(xo->dir, sizeof(MF)))	/* �A�h�U���� */
	return num + XO_TALL + XO_MOVE;
 
      /* �w�g�O�̫�@���F�٬O�䤣�쥼Ū�ݪO */
      mf_jumpnext = 0;
      tail = originpos;
      originpos = -1;
      if (tail < num)		/* �^���Ө��� */
	return tail + XO_MOVE;
    }
  }
#endif

  mf = (MF *) xo_pool;

  move(3, 0);
  do
  {
    mf_item(++num, mf++);
  } while (num < max);
  clrtobot();

#ifdef AUTO_JUMPBRD
  /* itoc.020615: �U�@�ӥ�Ū�O�b�����A�n���в��L�h */
  outf(FEETER_MF);
  return nextpos ? nextpos + XO_MOVE : XO_NONE;
#else
  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: �� b_lines ��W feeter */
#endif
}


static int
mf_head(xo)
  XO *xo;
{
  vs_head("�ڪ��̷R", str_site);
  prints(NECKER_MF, 
    cuser.ufo & UFO_BRDPOST ? "�`��" : "�s��", 
    d_cols >> 1, "", d_cols - (d_cols >> 1), "");
  return mf_body(xo);
}


static int
mf_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(MF));
  return mf_head(xo);
}


static int
mf_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(MF));
  return mf_body(xo);
}


static int
mf_stamp(mf)
  MF *mf;
{
  char fpath[64];
  int fd;

  mf->xname[0] = 'F';
  archiv32(mf->chrono, mf->xname + 1);

  mf_fpath(fpath, cuser.userid, mf->xname);

  if ((fd = open(fpath, O_WRONLY | O_CREAT | O_EXCL, 0600)) >= 0)
    close(fd);

  return fd;
}


static int
mf_add(xo)
  XO *xo;
{
  MF mf;
  int ans;

  ans = vans("�s�W (B)�ݪO���| (F)���v (G)��ذϱ��| (L)���j�u [Q] ");

  if (ans != 'b' && ans != 'f' && ans != 'g' && ans != 'l')
    return XO_FOOT;

  time(&mf.chrono);

  if (ans == 'b' || ans == 'g')
  {
    BRD *brd;
    char bname[BNLEN + 1];

    if (!(brd = ask_board(bname, BRD_R_BIT, NULL)))
      return mf_head(xo);

    mf.mftype = (ans == 'b') ? MF_BOARD : MF_GEM;
    strcpy(mf.xname, brd->brdname);
    if (ans == 'g')
      sprintf(mf.title, "%s �O ��ذϱ��|", mf.xname);
  }
  else /* if (ans == 'f' || ans == 'l') */
  {
    if (!vget(b_lines, 0, "���D�G", mf.title, BTLEN + 1, DOECHO))
      return XO_FOOT;

    mf.mftype = (ans == 'f') ? MF_FOLDER : MF_LINE;
    if (ans == 'f')
    {
      if (mf_stamp(&mf) < 0)
	return XO_FOOT;
    }
  }

  ans = vans("�s���m A)�s�W I)���J N)�U�@�� Q)���} [A] ");
  switch (ans)
  {
  case 'q':  
    break;

  case 'i':
  case 'n':

    rec_ins(xo->dir, &mf, sizeof(MF), xo->pos + (ans == 'n'), 1);
    break;

  default:

    rec_add(xo->dir, &mf, sizeof(MF));
    break;
  }

  return mf_init(xo);
}


static void
mf_do_delete(folder)
  char *folder;
{
  MF mf;
  char fpath[64];
  FILE *fp;

  if (!(fp = fopen(folder, "r")))
    return;

  while (fread(&mf, sizeof(MF), 1, fp) == 1)
  {
    if (mf.mftype & MF_FOLDER)
    {
      mf_fpath(fpath, cuser.userid, mf.xname);
      mf_do_delete(fpath);
    }
  }

  fclose(fp);
  unlink(folder);
}


static int
mf_delete(xo)
  XO *xo;
{
  MF *mf;
  int mftype;
  char fpath[64];

  mf = (MF *) xo_pool + (xo->pos - xo->top);
  mftype = mf->mftype;

  if (mftype & MF_MARK)
    return XO_NONE;

  if (vans(msg_del_ny) == 'y')
  {
    if (mftype & MF_FOLDER)
    {
      mf_fpath(fpath, cuser.userid, mf->xname);
      mf_do_delete(fpath);
    }
    if (!rec_del(xo->dir, sizeof(MF), xo->pos, NULL))
      return mf_load(xo);
  }

  return XO_FOOT;
}


static void
delmf(xo, mf)
  XO *xo;
  MF *mf;
{
  if (mf->mftype & MF_FOLDER)
  {
    char fpath[64];

    mf_fpath(fpath, cuser.userid, mf->xname);
    mf_do_delete(fpath);
  }
}


static int
mf_rangedel(xo)		/* amaki.030910: ���ѧڪ��̷R�Ϭq�R�� */
  XO *xo;
{
  return xo_rangedel(xo, sizeof(MF), NULL, delmf);
}


static int
mf_title(xo)
  XO *xo;
{
  MF *mf, xmf;

  mf = (MF *) xo_pool + (xo->pos - xo->top);
  xmf = *mf;

  if (!(mf->mftype & (MF_FOLDER | MF_LINE)))
    return XO_NONE;

  vget(b_lines, 0, "���D�G", xmf.title, BTLEN + 1, GCARRY);

  if (memcmp(mf, &xmf, sizeof(MF)) && vans(msg_sure_ny) == 'y')
  {
    int num;

    *mf = xmf;
    num = xo->pos;
    rec_put(xo->dir, mf, sizeof(MF), xo->pos, NULL);
    num++;
    move(num - xo->top + 2, 0);
    mf_item(num, mf);
  }

  return XO_FOOT;
}


static int
mf_move(xo)
  XO *xo;
{
  MF *mf;
  char *dir, buf[40];
  int pos, newOrder;

  pos = xo->pos;
  mf = (MF *) xo_pool + (pos - xo->top);

  sprintf(buf, "�п�J�� %d �ﶵ���s��m�G", pos + 1);
  if (!vget(b_lines, 0, buf, buf, 5, DOECHO))
    return XO_FOOT;

  newOrder = atoi(buf) - 1;
  if (newOrder < 0)
    newOrder = 0;
  else if (newOrder >= xo->max)
    newOrder = xo->max - 1;

  if (newOrder != pos)
  {
    dir = xo->dir;
    if (!rec_del(dir, sizeof(MF), pos, NULL))
    {
      rec_ins(dir, mf, sizeof(MF), newOrder, 1);
      xo->pos = newOrder;
      return mf_load(xo);
    }
  }

  return XO_FOOT;
}


static int
mf_mark(xo)
  XO *xo;
{
  MF *mf;

  mf = (MF *) xo_pool + (xo->pos - xo->top);

  if (mf->mftype & MF_FOLDER)
  {
    int num;

    mf->mftype ^= MF_MARK;
    num = xo->pos;
    rec_put(xo->dir, mf, sizeof(MF), num, NULL);
    num++;
    move(num - xo->top + 2, 0);
    mf_item(num, mf);
  }

  return XO_NONE;
}


static int
mf_browse(xo)
  XO *xo;
{
  int type, bno;
  char *xname, fpath[64];
  BRD *brd;
  MF *mf;

  mf = (MF *) xo_pool + (xo->pos - xo->top);
  type = mf->mftype;
  xname = mf->xname;

  if (type & MF_BOARD)		/* �ݪO���| */
  {
    /* itoc.010726: �Y�O�ݪO�w�g�Q����v���S���F�A�h�n�������| */
    if ((bno = brd_bno(xname)) < 0 || !(brd_bits[bno] & BRD_R_BIT))
    {
      rec_del(xo->dir, sizeof(MF), xo->pos, NULL);
      vmsg("���ݪO�w�Q�R���αz�S���v���\\Ū���ݪO�A�t�αN�۰ʲ������|");
      return mf_load(xo);
    }

    brd = bshm->bcache + bno;
    XoPost(bno);
    xover(XZ_POST);
#ifndef ENHANCED_VISIT
    time(&brd_visit[bno]);
#endif

#ifdef AUTO_JUMPBRD
    if (cuser.ufo & UFO_JUMPBRD)
      mf_jumpnext = 1;	/* itoc.010910: �u���b���}�ݪO�^��ݪO�C��ɤ~�ݭn���h�U�@�ӥ�Ū�ݪO */
#endif

    return mf_init(xo);
  }
  else if (type & MF_GEM)	/* ��ذϱ��| */
  {
    /* itoc.010726: �Y�O�ݪO�w�g�Q����v���S���F�A�h�n�������| */
    if ((type = gem_link(xname)) < 0)
    {
      rec_del(xo->dir, sizeof(MF), xo->pos, NULL);
      vmsg("���ݪO�w�Q�R���αz�S���v���\\Ū���ݪO�A�t�αN�۰ʲ������|");
      return mf_load(xo);
    }

    gem_fpath(fpath, xname, fn_dir);
    XoGem(fpath, "��ذ�", type);
    return mf_init(xo);
  }
  else if (type & MF_FOLDER)	/* ���v */
  {
    mf_depth++;
    mf_fpath(fpath, cuser.userid, xname);
    XoMF(fpath);
    mf_depth--;
    return mf_load(xo);
  }
  else if (type & MF_CLASS)	/* �����s�� */
  {
    if (!MFclass_browse(xname))
    {
      rec_del(xo->dir, sizeof(MF), xo->pos, NULL);
      vmsg("�������s�դw�Q�R���A�t�αN�۰ʲ������|");
      return mf_load(xo);
    }
    return mf_init(xo);
  }

  return XO_NONE;
}


static int
mf_copy(xo)
  XO *xo;
{
  MF *mf;

  mf = (MF *) xo_pool + (xo->pos - xo->top);

  memcpy(&mftmp, mf, sizeof(MF));
  zmsg("���������A���O���v������Ƥ��|�Q����");

  return XO_FOOT;
}


static int
mf_paste(xo)
  XO *xo;
{
  MF mf;
  int ans;

  if (!mftmp.chrono)
  {
    zmsg("�Х����� copy �R�O��A paste");
    return XO_FOOT;
  }

  memcpy(&mf, &mftmp, sizeof(MF));
  time(&mf.chrono);			/* �y�@�ӷs�� chrono */

  /* itoc.010726.����: �Y�O MF_FOLDER�A�h�����ɦW�A�K�W�A�@�Ө��v�@���ɮ� */
  /* itoc.010726.����: ���v�ƻs�K�W�A�̭����F��èS���K�W�A�i�o�g recursive ���{�� :p */
  if (mf.mftype & MF_FOLDER)
  {
    if (mf_stamp(&mf) < 0)
    {
      vmsg("��Ʀ��~�A�Э��s�ƻs��A�K�W");
      return XO_FOOT;
    }
  }

  ans = vans("�s���m A)�s�W I)���J N)�U�@�� Q)���} [A] ");
  switch (ans)
  {
  case 'q':
    return XO_FOOT;

  case 'i':
  case 'n':

    rec_ins(xo->dir, &mf, sizeof(MF), xo->pos + (ans == 'n'), 1);
    break;

  default:

    rec_add(xo->dir, &mf, sizeof(MF));
    break;
  }

  return mf_load(xo);
}


static int
mf_namemode(xo)
  XO *xo;
{
  cuser.ufo ^= UFO_BRDPOST;
  cutmp->ufo = cuser.ufo;
  return mf_head(xo);
}


static int
mf_edit(xo)		/* itoc.010110: �ڪ��̷R���ݪO�ק� */
  XO *xo;
{
  MF *mf;

  mf = (MF *) xo_pool + (xo->pos - xo->top);

  if ((mf->mftype & MF_BOARD) && (HAS_PERM(PERM_ALLBOARD | PERM_BM)))
  {
    int bno;

    bno = brd_bno(mf->xname);
    if (bno >= 0)
    {
      if (!HAS_PERM(PERM_ALLBOARD))
	brd_title(bno);
      else
	brd_edit(bno);  
      return mf_init(xo);
    }
  }
  return XO_NONE;
}


static int
mf_switch(xo)
  XO *xo;
{
  Select();
  return mf_init(xo);
}


static int
mf_visit(xo)		/* itoc.010402: �ݪO�C��]�w�ݪO�wŪ */
  XO *xo;
{
  int bno;
  MF *mf;

  mf = (MF *) xo_pool + (xo->pos - xo->top);
  bno = brd_bno(mf->xname);

  if (bno >= 0)		/* itoc.010110: ���Ӥ��| < 0 ? */
  {
    BRD *brd;
    brd = bshm->bcache + bno;
    brh_get(brd->bstamp, bno);
    brh_visit(0);
#ifndef ENHANCED_VISIT
    time(&brd_visit[bno]);
#endif
  }
  return mf_body(xo);
}


static int
mf_unvisit(xo)		/* itoc.010402: �ݪO�C��]�w�ݪO��Ū */
  XO *xo;
{
  int bno;
  MF *mf;

  mf = (MF *) xo_pool + (xo->pos - xo->top);
  bno = brd_bno(mf->xname);

  if (bno >= 0)		/* itoc.010110: ���Ӥ��| < 0 ? */
  {
    BRD *brd;
    brd = bshm->bcache + bno;
    brh_get(brd->bstamp, bno);
    brh_visit(1);
#ifndef ENHANCED_VISIT
    brd_visit[bno] = 0;	/* itoc.010402: �̪��s���ɶ��k�s�A�ϬݪO�C����ܥ�Ū */
#endif
  }
  return mf_body(xo);
}


static int
mf_nextunread(xo)
  XO *xo;
{
  int max, pos, bno;
  MF *mf;

  max = xo->max;
  pos = xo->pos;
  mf = (MF *) xo_pool + (xo->pos - xo->top);

  while (++pos < max)
  {
    bno = brd_bno((++mf)->xname);
    if (bno >= 0 && !(brd_bits[bno] & BRD_Z_BIT))	/* ���L������ zap �����ݪO */
    {
      BRD *brd;
      brd = bshm->bcache + bno;

#ifdef ENHANCED_VISIT
      /* itoc.010407: ��γ̫�@�g�wŪ/��Ū�ӧP�_ */
      brh_get(brd->bstamp, bno);

      if (brh_unread(brd->blast))
#else
      if (brd->blast > brd_visit[bno])
#endif
	return pos + XO_MOVE;
    }
  }

  return XO_NONE;
}


static int
mf_help(xo)
  XO *xo;
{
  xo_help("mf");
  return mf_head(xo);
}


static KeyFunc mf_cb[] =
{
  XO_INIT, mf_init,
  XO_LOAD, mf_load,
  XO_HEAD, mf_head,
  XO_BODY, mf_body,

  'r', mf_browse,
  'd', mf_delete,
  'D', mf_rangedel,
  'o', mf_mark,
  'm', mf_move,
  'T', mf_title,
  'E', mf_edit,
  's', mf_switch,
  'c', mf_namemode,
  'v', mf_visit,
  'V', mf_unvisit,
  '`', mf_nextunread,

  Ctrl('P'), mf_add,
  'C', mf_copy,
  'g', mf_copy,
  'p', mf_paste,
  Ctrl('V'), mf_paste,

  'h', mf_help
};


static void
XoMF(folder)
  char *folder;
{
  XO *xo, *last;

  /* itoc.060706: �� mf_depth �� 0 �ɡA��ܭn�i FN_MF�A�b mf_main() �w�B�z�L�A
     ���ݭn���s xo_new()�A�p���i�O�d������ xo->pos */

  if (mf_depth)
  {
    last = xz[XZ_MF - XO_ZONE].xo;	/* record */

    xz[XZ_MF - XO_ZONE].xo = xo = xo_new(folder);
    xo->pos = 0;
  }

#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    mf_jumpnext = 1;	/* itoc.020615: �D�ʸ��h�U�@�ӥ�Ū�ݪO */
#endif
  xover(XZ_MF);

  if (mf_depth)
  {
    free(xo);
    xz[XZ_MF - XO_ZONE].xo = last;	/* restore */
  }
}


int
MyFavorite()
{
  /* �q�D���i�J�ڪ��̷R�Amf_depth �@�w�O 0 */
  XoMF(NULL);

  return 0;
}


void
mf_main()
{
  char fpath[64];
  XO *xo;

  /* itoc.060706.����: �i���ɴN�n��l�ơA�]���ϥΪ̥i��@�W���N every_Z ���h�ڪ��̷R */

  mf_fpath(fpath, cuser.userid, FN_MF);
  xz[XZ_MF - XO_ZONE].xo = xo = xo_new(fpath);
  xz[XZ_MF - XO_ZONE].cb = mf_cb;

  xo->pos = 0;

#ifdef AUTO_JUMPBRD
  if (cuser.ufo & UFO_JUMPBRD)
    mf_jumpnext = 1;	/* itoc.020615: �D�ʸ��h�U�@�ӥ�Ū�ݪO */
#endif
}
#endif				/* MY_FAVORITE */
