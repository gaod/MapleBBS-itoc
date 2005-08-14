/*-------------------------------------------------------*/
/* gem.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : ��ذϾ\Ū�B�s��			     	 */
/* create : 95/03/29				     	 */
/* update : 97/02/02				     	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern XZ xz[];
extern char xo_pool[];
extern char brd_bits[];

extern int TagNum;
extern TagItem TagList[];


#define	GEM_WAY		3
static int gem_way;		/* 0:�u�L���D 1:���D�[�ɦW 2:���D�[�s�� */

static int GemBufferNum;	/* Thor.990414: ���e�ŧi�A�Ω�gem_head */

static char GemAnchor[64];	/* �w��Ϫ����| */
static char GemSailor[20];	/* �w��Ϫ����D */

static int gem_add_all();
static int gem_paste();
static int gem_anchor();


static void
gem_item(num, hdr, level)
  int num;
  HDR *hdr;
  int level;
{
  int xmode, gtype;

  /* ������������������ : A1B7 ... */

  xmode = hdr->xmode;
  gtype = (char) 0xba;

  /* �ؿ��ι�ߡA���O�ؿ��ΪŤ� */
  if (xmode & GEM_FOLDER)		/* �峹:�� ���v:�� */
    gtype += 1;

  if (hdr->xname[0] == '@')		/* ���:�� ����:�� */
    gtype -= 2;
  else if (xmode & GEM_BOARD)		/*         �ݪO:�� */
    gtype += 2;

  prints("%6d%c%c\241%c ", num, xmode & GEM_RESTRICT ? ')' : ' ',
    TagNum && !Tagger(hdr->chrono, num - 1, TAG_NIN) ? '*' : ' ', gtype);

  if ((xmode & GEM_RESTRICT) && !(level & GEM_M_BIT))
    outs(MSG_DATA_CLOAK);				/* itoc.000319: ����Ť峹�O�K */
  else if (gem_way == 0)
    prints("%.*s\n", d_cols + 64, hdr->title);
  else
    prints("%-*.*s%-13s%s\n", d_cols + 46, d_cols + 45, hdr->title, (gem_way == 1 ? hdr->xname : hdr->owner), hdr->date);
}


static int
gem_body(xo)
  XO *xo;
{
  HDR *hdr;
  int num, max, tail;

  max = xo->max;
  if (max <= 0)
  {
    outs("\n\n�m��ذϡn�|�b�l���Ѧa��������� :)");

    if (xo->key & GEM_W_BIT)
    {
      switch (vans("(A)�s�W��� (P)�K�� (G)����\\�� [N]�L�Ҩƨ� "))
      {
      case 'a':
	max = gem_add_all(xo);
    	if (xo->max > 0)
	  return max;
	break;

      case 'p':
	max = gem_paste(xo);
	if (xo->max > 0)
	  return max;
	break;

      case 'g':
	gem_anchor(xo);
	break;
      }
    }
    else
    {
      vmsg(NULL);
    }
    return XO_QUIT;
  }

  hdr = (HDR *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  move(3, 0);
  tail = xo->key;	/* �ɥ� tail */
  do
  {
    gem_item(++num, hdr++, tail);
  } while (num < max);
  clrtobot();

  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: �� b_lines ��W feeter */
}


static int
gem_head(xo)
  XO *xo;
{
  char buf[20];

  vs_head("��ؤ峹", xo->xyz);

  if ((xo->key & GEM_W_BIT) && GemBufferNum > 0)
    sprintf(buf, "(�ŶKï %d �g)", GemBufferNum);
  else
    buf[0] = '\0';

  prints(NECKER_GEM, buf, d_cols, "");
  return gem_body(xo);
}


static int
gem_toggle(xo)
  XO *xo;
{
  gem_way++;
  gem_way %= GEM_WAY;

  /* �u��������ݨ��ɦW */
  if (!(xo->key & GEM_X_BIT) && gem_way == 1)
    gem_way++;

  return gem_body(xo);
}


static int
gem_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(HDR));
  return gem_head(xo);
}


static int
gem_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(HDR));
  return gem_body(xo);
}


/* ----------------------------------------------------- */
/* gem_check : attribute check out		 	 */
/* ----------------------------------------------------- */


#define	GEM_PLAIN	0x01	/* �w���O plain text */


static HDR *		/* NULL:�L�vŪ�� */
gem_check(xo, fpath, op)
  XO *xo;
  char *fpath;
  int op;
{
  HDR *hdr;
  int gtype;

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  gtype = hdr->xmode;

  if ((gtype & GEM_RESTRICT) && !(xo->key & GEM_M_BIT))
    return NULL;

  if ((op & GEM_PLAIN) && (gtype & GEM_FOLDER))
    return NULL;

  if (fpath)
  {
    if (gtype & GEM_BOARD)
      gem_fpath(fpath, hdr->xname, fn_dir);
    else
      hdr_fpath(fpath, xo->dir, hdr);
  }
  return hdr;
}


#if 0
static int	/* -1:���O�ݪO��ذ�  >=0:bno */
gem_bno(xo)
  XO *xo;
{
  char *dir, *str;
  int bno;

  /* �� xo->dir ��X�ثe�b���@�ӪO����ذ� */
  dir = xo->dir;

  /* �ˬd�O�_�� gem/brd/brdname/.DIR ���榡�A
     �קK�Y�b gem/.DIR �� usr/u/userid/gem/.DIR �|�y�����~ */
  if (dir[0] == 'g' && dir[4] == 'b')
  {
    dir += 8;	/* ���L "gem/brd/" */
    if (str = strchr(dir, '/'))
    {
      *str = '\0';
      bno = brd_bno(dir);
      *str = '/';
      return bno;
    }
  }

  return -1;
}
#endif


/* ----------------------------------------------------- */
/* ��Ƥ��s�W�Gappend / insert				 */
/* ----------------------------------------------------- */


void
brd2gem(brd, gem)
  BRD *brd;
  HDR *gem;
{
  memset(gem, 0, sizeof(HDR));
  time(&gem->chrono);
  str_stamp(gem->date, &gem->chrono);
  strcpy(gem->xname, brd->brdname);
  sprintf(gem->title, "%-13s%-5s%s", brd->brdname, brd->class, brd->title);
  gem->xmode = GEM_BOARD | GEM_FOLDER;
}


#if 0	/* itoc.010218: ���s�� gem_log() */
static void
gem_log(folder, action, hdr)
  char *folder;
  char *action;
  HDR *hdr;
{
  char fpath[64], buf[256];

  if (hdr->xmode & (GEM_RESTRICT | GEM_RESERVED))
    return;

  str_folder(fpath, folder, "@/@log");
  sprintf(buf, "[%s] %s (%s) %s\n%s\n\n",
    action, hdr->xname, Now(), cuser.userid, hdr->title);
  f_cat(fpath, buf);
}
#endif


static void
gem_log(folder, action, hdr)
  char *folder;
  char *action;
  HDR *hdr;
{
  char fpath1[64], fpath2[64];
  FILE *fp1, *fp2;

  if (hdr->xmode & (GEM_RESTRICT | GEM_RESERVED))
    return;

  /* mv @log @log.old */
  str_folder(fpath1, folder, "@/@log");
  str_folder(fpath2, folder, "@/@log.old");
  f_mv(fpath1, fpath2);  

  if (!(fp1 = fopen(fpath1, "a")))
    return;

  /* ��s�����ʩ�b�̤W���A�è̲��ʶ��ǽs�� */

  fprintf(fp1, "<01> %s %-12s [%s] %s\n     %s\n\n", Now(),
    cuser.userid, action, hdr->xname, hdr->title);

  if (fp2 = fopen(fpath2, "r"))
  {
    char buf[STRLEN];
    int i = 6;				/* �q�ĤG�g�}�l */
    int j;

    while (fgets(buf, STRLEN, fp2))
    {
      if (++i > 63)			/* �u�O�d�̷s 20 ������ */
	break;

      j = i % 3;
      if (j == 1)			/* �Ĥ@�� */
	fprintf(fp1, "<%02d> %s", i / 3, buf + 5);
      else if (j == 2)			/* �ĤG�� */
	fprintf(fp1, "%s\n", buf);
					/* �ĤT��O�Ŧ� */
    }
    fclose(fp2);
  }
  fclose(fp1);
}


static int
gem_add(xo, gtype)
  XO *xo;
  int gtype;
{
  int level, fd, ans;
  char title[80], fpath[64], *dir;
  HDR hdr;

  level = xo->key;
  if (!(level & GEM_W_BIT))
    return XO_NONE;

  if (!gtype)
  {
    gtype = vans((level & GEM_X_BIT) ?
      /* "�s�W A)rticle B)oard C)lass D)ata F)older P)aste Q)uit [Q] " : */
      "�s�W (A)�峹 (B)�ݪO (C)���� (D)��� (F)���v (P)�K�� (Q)�����H[Q] " : 
      "�s�W (A)�峹 (F)���v (P)�K�� (Q)�����H[Q] ");
  }

  if (gtype == 'p')
    return gem_paste(xo);

  if (gtype != 'a' && gtype != 'f' && 
    (!(level & GEM_X_BIT) || (gtype != 'b' && gtype != 'c' && gtype != 'd')))
    return XO_FOOT;

  dir = xo->dir;
  fd = -1;

  if (gtype == 'b')
  {
    BRD *brd;

    if (!(brd = ask_board(fpath, BRD_L_BIT, NULL)))
      return gem_head(xo);

    brd2gem(brd, &hdr);
    gtype = 0;
  }
  else
  {
    if (!vget(b_lines, 0, "���D�G", title, TTLEN + 1, DOECHO))
      return XO_FOOT;

    if (gtype == 'c' || gtype == 'd')
    {
      if (!vget(b_lines, 0, "�ɦW�G", fpath, BNLEN + 1, DOECHO))
	return XO_FOOT;

      if (strchr(fpath, '/'))
      {
	zmsg("���X�k���ɮצW��");
	/* return XO_NONE; */
	return XO_FOOT;		/* itoc.010726: �� b_lines ��W feeter */
      }

      memset(&hdr, 0, sizeof(HDR));
      time(&hdr.chrono);
      str_stamp(hdr.date, &hdr.chrono);
      sprintf(hdr.xname, "@%s", fpath);
      if (gtype == 'c')
      {
	strcat(fpath, "/");
	sprintf(hdr.title, "%-13s���� �� %.50s", fpath, title);
	hdr.xmode = GEM_FOLDER;
      }
      else
      {
	strcpy(hdr.title, title);
	hdr.xmode = 0;
      }
      gtype = 1;
    }
    else
    {
      if ((fd = hdr_stamp(dir, gtype, &hdr, fpath)) < 0)
	return XO_FOOT;
      close(fd);

      if (gtype == 'a')
      {
	if (vedit(fpath, 0))	/* Thor.981020: �`�N�Qtalk�����D */
	{
	  unlink(fpath);
	  zmsg(msg_cancel);
	  return gem_head(xo);
	}
	gtype = 0;
      }
      else if (gtype == 'f')
      {
	gtype = GEM_FOLDER;
      }

      hdr.xmode = gtype;
      strcpy(hdr.title, title);
    }
  }

  /* ans = vans("�s���m A)ppend I)nsert N)ext Q)uit [A] "); */
  ans = vans("�s���m A)�[��̫� I/N)���J�ثe��m Q)���} [A] ");

  if (ans == 'q')
  {
    if (fd >= 0)
      unlink(fpath);
    return (gtype ? XO_FOOT : gem_head(xo));
  }

  strcpy(hdr.owner, cuser.userid);

  if (ans == 'i' || ans == 'n')
    rec_ins(dir, &hdr, sizeof(HDR), xo->pos + (ans == 'n'), 1);
  else
    rec_add(dir, &hdr, sizeof(HDR));

  gem_log(dir, "�s�W", &hdr);

  return (gtype ? gem_load(xo) : gem_init(xo));
}


static int
gem_add_all(xo)
  XO *xo;
{
  return gem_add(xo, 0);
}


static int
gem_add_article(xo)		/* itoc.010419: �ֳt�� */
  XO *xo;
{
  return gem_add(xo, 'a');
}


static int
gem_add_folder(xo)		/* itoc.010419: �ֳt�� */
  XO *xo;
{
  return gem_add(xo, 'f');
}


/* ----------------------------------------------------- */
/* ��Ƥ��ק�Gedit / title				 */
/* ----------------------------------------------------- */


static int
gem_edit(xo)
  XO *xo;
{
  int level;
  char fpath[64];
  HDR *hdr;

  if (!(hdr = gem_check(xo, fpath, GEM_PLAIN)))
    return XO_NONE;

  level = xo->key;

  if (!(level & GEM_W_BIT) || ((hdr->xmode & GEM_RESERVED) && !(level & GEM_X_BIT)))
  {
    vedit(fpath, -1);
  }
  else
  {
    if (vedit(fpath, 0) >= 0)
      gem_log(xo->dir, "�ק�", hdr);
  }

  return gem_head(xo);
}


static int
gem_title(xo)
  XO *xo;
{
  HDR *fhdr, mhdr;
  int pos, cur;

  if (!(xo->key & GEM_W_BIT) || !(fhdr = gem_check(xo, NULL, 0)))
    return XO_NONE;

  memcpy(&mhdr, fhdr, sizeof(HDR));

  vget(b_lines, 0, "���D�G", mhdr.title, TTLEN + 1, GCARRY);

  if (xo->key & GEM_X_BIT)
  {
    vget(b_lines, 0, "�s�̡G", mhdr.owner, IDLEN + 1, GCARRY);
    /* vget(b_lines, 0, "�ʺ١G", mhdr.nick, sizeof(mhdr.nick), GCARRY); */	/* ��ذϦ���쬰�� */
    vget(b_lines, 0, "����G", mhdr.date, sizeof(mhdr.date), GCARRY);
  }

  if (memcmp(fhdr, &mhdr, sizeof(HDR)) && vans(msg_sure_ny) == 'y')
  {
    pos = xo->pos;
    cur = pos - xo->top;

    memcpy(fhdr, &mhdr, sizeof(HDR));
    rec_put(xo->dir, fhdr, sizeof(HDR), pos, NULL);

    move(3 + cur, 0);
    gem_item(++pos, fhdr, xo->key);

    gem_log(xo->dir, "���D", fhdr);
  }
  return XO_FOOT;
}


static int
gem_refuse(xo)
  XO *xo;
{
  HDR *hdr;
  int num;

  if ((xo->key & GEM_M_BIT) && (hdr = gem_check(xo, NULL, 0)))
  {
    hdr->xmode ^= GEM_RESTRICT;

    num = xo->pos;
    rec_put(xo->dir, hdr, sizeof(HDR), num, NULL);
    num++;
    move(num - xo->top + 2, 0);
    gem_item(num, hdr, xo->key);
  }

  return XO_NONE;
}


static int
gem_state(xo)
  XO *xo;
{
  HDR *hdr;
  char fpath[64];
  struct stat st;

  if ((xo->key & GEM_W_BIT) && (hdr = gem_check(xo, fpath, 0)))
  {
    move(12, 0);
    clrtobot();
    prints("\nDir : %s", xo->dir);
    prints("\nName: %s", hdr->xname);
    prints("\nFile: %s", fpath);

    if (!stat(fpath, &st))
    {
      prints("\nTime: %s", Btime(&st.st_mtime));
      prints("\nSize: %d", st.st_size);
    }

    vmsg(NULL);
    return gem_body(xo);
  }

  return XO_NONE;
}


/* ----------------------------------------------------- */
/* ��Ƥ��s���Gedit / title				 */
/* ----------------------------------------------------- */


int			/* -1:�L�v�� */
gem_link(brdname)	/* �ˬd�s���h��L�ݪO��ذϪ��v�� */
  char *brdname;
{
  int bno, level;

  if ((bno = brd_bno(brdname)) < 0 || !((bno = brd_bits[bno]) & BRD_R_BIT))
    return -1;

  level = 0;
  if (bno & BRD_X_BIT)
    level ^= GEM_W_BIT;
  if (HAS_PERM(PERM_SYSOP))
    level ^= GEM_X_BIT;
  if (bno & BRD_M_BIT)
    level ^= GEM_M_BIT;

  return level;
}


static int
gem_browse(xo)
  XO *xo;
{
  HDR *hdr;
  int op, xmode;
  char fpath[64], title[TTLEN + 1], *ptr;

  op = 0;

  for (;;)
  {    
    if (!(hdr = gem_check(xo, fpath, op)))
      break;

    xmode = hdr->xmode;

    /* browse folder */

    if (xmode & GEM_FOLDER)
    {
      strcpy(title, hdr->title);

      if (xmode & GEM_BOARD)
      {
	if ((op = gem_link(hdr->xname)) < 0)
	{
	  vmsg("�藍�_�A���O��ذϥu��O�Ͷi�J�A�ЦV�O�D�ӽФJ�ҳ\\�i");
	  return XO_FOOT;
	}
      }
      else			/* �@����v�~���p�O�D */
      {
	op = xo->key;		/* �~�ӥ����v���v�� */

	/* itoc.011217: [userA/userB ���h��p�O�D�Ҧ��]�A�� */
	if ((ptr = strrchr(title, '[')) && is_bm(ptr + 1, cuser.userid))
	  op |= GEM_W_BIT;
      }

      XoGem(fpath, title, op);
      return gem_init(xo);
    }

    /* browse article */

    /* Thor.990204: ���Ҽ{more �Ǧ^�� */   
    if ((xmode = more(fpath, FOOTER_GEM)) < 0)
      break;

    op = GEM_PLAIN;

re_key:
    switch (xo_getch(xo, xmode))
    {
    case XO_BODY:
      continue;

    case '/':
      if (vget(b_lines, 0, "�j�M�G", hunt, sizeof(hunt), DOECHO))
      {
	more(fpath, FOOTER_GEM);
	goto re_key;
      }
      continue;

    case 'E':
      return gem_edit(xo);

    case 'C':
      {
	FILE *fp;
	if (fp = tbf_open())
	{
	  f_suck(fp, fpath);
	  fclose(fp);
	}
      }
      break;

    case 'h':
      xo_help("gem");
      break;
    }
    break;
  }

  return gem_head(xo);
}


/* ----------------------------------------------------- */
/* ��ذϤ��R��						 */
/* ----------------------------------------------------- */


static int
chkgem(hdr)
  HDR *hdr;
{
  return (hdr->xmode & (GEM_RESTRICT | GEM_RESERVED));
}


static int
vfygem(hdr, pos)
  HDR *hdr;
  int pos;
{
  return (Tagger(hdr->chrono, pos, TAG_NIN) || chkgem(hdr));
}


static void
delgem(xo, hdr)
  XO *xo;
  HDR *hdr;
{
  char folder[64];
  HDR fhdr;
  FILE *fp;

  if (hdr->xmode & GEM_FOLDER)		/* ���v/����/�ݪO */
  {
    hdr_fpath(folder, xo->dir, hdr);

    /* Kyo.050328: �w��ϳQ�R���ɭn���� */
    if (!strcmp(GemAnchor, folder))
      GemAnchor[0] = '\0';

    /* ���v�n�i�l�ؿ��R���F�ݪO/�����h���ݭn */
    if (hdr->xmode == GEM_FOLDER && hdr->xname[0] != '@')
    {
      if (fp = fopen(folder, "r"))
      {
	while (fread(&fhdr, sizeof(HDR), 1, fp) == 1)
	  delgem(xo, &fhdr);

	fclose(fp);
	unlink(folder);
      }
    }
  }
  else					/* �峹/��� */
  {
    /* �峹�n�R���ɮסF��ƫh���R���ɮ� */
    if (hdr->xname[0] != '@')
    {
      hdr_fpath(folder, xo->dir, hdr);
      unlink(folder);
    }
  }
}


static int
gem_delete(xo)
  XO *xo;
{
  HDR *hdr;
  int xmode;

  if (!(xo->key & GEM_W_BIT) || !gem_check(xo, NULL, 0))
    return XO_NONE;

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  xmode = hdr->xmode;

  if (hdr->xmode & (GEM_RESTRICT | GEM_RESERVED))
    return XO_NONE;

  if (vans(msg_del_ny) == 'y')
  {
    delgem(xo, hdr);

    if (!rec_del(xo->dir, sizeof(HDR), xo->pos, NULL))
    {
      gem_log(xo->dir, "�R��", hdr);
      return gem_load(xo);
    }
  }

  return XO_FOOT;
}


static int
gem_rangedel(xo)	/* itoc.010726: ���ѰϬq�R�� */
  XO *xo;
{
  if (!(xo->key & GEM_W_BIT) || !gem_check(xo, NULL, 0))
    return XO_NONE;

  return xo_rangedel(xo, sizeof(HDR), chkgem, delgem);
}


static int
gem_prune(xo)
  XO *xo;
{
  if (!(xo->key & GEM_W_BIT))
    return XO_NONE;
  return xo_prune(xo, sizeof(HDR), vfygem, delgem);
}


/* ----------------------------------------------------- */
/* ��ذϤ��ƻs�B�K�W�B����				 */
/* ----------------------------------------------------- */


static char GemFolder[64];

static HDR *GemBuffer;
static int GemBufferSiz;
/* static int GemBufferNum; */	/* Thor.990414: ���e�ŧi��gem_head�� */


/* �t�m�������Ŷ���J header */


static HDR *
gbuf_malloc(num)
  int num;
{
  HDR *gbuf;

  GemBufferNum = num;
  if (gbuf = GemBuffer)
  {
    if (GemBufferSiz < num)
    {
      num += (num >> 1);
      GemBufferSiz = num;
      GemBuffer = gbuf = (HDR *) realloc(gbuf, sizeof(HDR) * num);
    }
  }
  else
  {
    GemBufferSiz = num;
    GemBuffer = gbuf = (HDR *) malloc(sizeof(HDR) * num);
  }

  return gbuf;
}


/* static */			/* itoc.010924: �� post_copy() �� */
void
gem_buffer(dir, hdr)
  char *dir;
  HDR *hdr;			/* NULL �N���J TagList, �_�h�N�ǤJ����J */
{
  int num, locus;
  HDR *gbuf;

  if (hdr)
  {
    num = 1;
  }
  else
  {
    num = TagNum;
    if (num <= 0)
      return;
  }

  gbuf = gbuf_malloc(num);

  if (hdr)
  {
    memcpy(gbuf, hdr, sizeof(HDR));
  }
  else
  {
    locus = 0;
    do
    {
      EnumTag(&gbuf[locus], dir, locus, sizeof(HDR));
    } while (++locus < num);
  }

  strcpy(GemFolder, dir);
}


static int
gem_copy(xo)
  XO *xo;
{
  HDR *hdr;
  int tag;

  if (!(hdr = gem_check(xo, NULL, 0)))
    return XO_NONE;

  tag = AskTag("��ذϫ���");

  if (tag < 0)
    return XO_FOOT;

  gem_buffer(xo->dir, tag ? NULL : hdr);

  zmsg("���������A���O�[�K�峹���|�Q�����C[�`�N] �K�W��~��R�����I");

  /* return XO_FOOT; */
  return gem_head(xo);		/* Thor.990414: ���ŶK�g�Ƨ�s */
}


static inline int
gem_extend(xo, num)
  XO *xo;
  int num;
{
  char *dir, fpath[64], gpath[64];
  FILE *fp;
  time_t chrono;
  HDR *hdr;

  if (!(hdr = gem_check(xo, fpath, GEM_PLAIN)))
    return -1;

  if (!(fp = fopen(fpath, "a")))
    return -1;

  dir = xo->dir;
  chrono = hdr->chrono;

  for (hdr = GemBuffer; num--; hdr++)
  {
    if ((hdr->chrono != chrono) && !(hdr->xmode & (GEM_FOLDER | GEM_RESTRICT | GEM_RESERVED)))
    {
      hdr_fpath(gpath, GemFolder, hdr);	/* itoc.010924: �ץ����P�ؿ��| extend ���� */
      fputs(str_line, fp);
      f_suck(fp, gpath);
    }
  }

  fclose(fp);
  return 0;
}


static int					/* 1: �L�a�j��  0: �X�k */
invalid_loop(srcDir, dstDir, hdr, depth)	/* itoc.010727: �ˬd�O�_�|�y���L�a�j�� for gem_paste() */
  char *srcDir, *dstDir;
  HDR *hdr;
  int depth;				/* 0: ���j�Ĥ@��  1: ���j�� */
{
  static int valid;

  int fd;
  char fpath1[64], fpath2[64];
  HDR fhdr;

  if (!depth)
  {
    if (!(hdr->xmode & GEM_FOLDER))	/* plain text */
      return 0;

    str_folder(fpath1, srcDir, fn_dir);
    str_folder(fpath2, dstDir, fn_dir);

    if (strcmp(fpath1, fpath2))		/* ��ϫ����@�w���|�y���L�a�j�� */
      return 0;

    hdr_fpath(fpath1, srcDir, hdr);	/* ��ۤv����ۤv�̭� */
    if (!strcmp(fpath1, dstDir))
      return 1;

    valid = 0;
  }
  else
  {
    if (valid)		/* �b�Y�@�ӻ��j�����D�k�ҾڴN����j�Ҥu�@ */
      return 1;

    hdr_fpath(fpath1, srcDir, hdr);
  }

  if ((fd = open(fpath1, O_RDONLY)) >= 0)
  {
    while (read(fd, &fhdr, sizeof(HDR)) == sizeof(HDR))
    {  
      if (fhdr.xmode & GEM_FOLDER)	/* plain text ���|�y���L�a�j�� */
      {
	hdr_fpath(fpath2, srcDir, &fhdr);
	if (!strcmp(fpath2, dstDir))
	{
	  valid = 1;
	  return 1;
	}

	/* recursive �a�@�h�@�h�ؿ��i�h�ˬd�O�_�|�y���L�a�j�� */
	invalid_loop(fpath1, dstDir, &fhdr, 1);
      }
    }
    close(fd);
  }

  return valid;
}


static void
gem_do_paste(srcDir, dstDir, hdr, pos)		/* itoc.010725: for gem_paste() */
  char *srcDir;		/* source folder */
  char *dstDir;		/* destination folder */
  HDR *hdr;		/* source hdr */
  int pos;		/* -1: ���[�b�̫�  >=0: �K�W����m */
{
  int xmode, fsize;
  char folder[64], fpath[64];
  HDR fhdr, *data, *head, *tail;

  xmode = hdr->xmode;

  if (xmode & (GEM_RESTRICT | GEM_RESERVED))	/* ����ź�ذϤ���ƻs/�K�W */
    return;

  if (xmode & GEM_BOARD)			/* �ݪO����Q�ƻs/�K�W */
    return;

  if (xmode & GEM_FOLDER)	/* ���v/���� */
  {
    /* ���s�ɫإߦۤv�o�Ӥ���/���v */

    /* �b�ƻs/�K�W��@���ܦ����v�A�]�������O�����M�ίS��γ~�� */
    if ((fsize = hdr_stamp(dstDir, 'F', &fhdr, fpath)) < 0)
      return;
    close(fsize);

    fhdr.xmode = GEM_FOLDER;
  }
  else				/* �峹/��� */
  {
    hdr_fpath(folder, srcDir, hdr);

    /* �b�ƻs/�K�W��@���ܦ��峹�A�]����ƬO�����M�ίS��γ~�� */
    hdr_stamp(dstDir, HDR_COPY | 'A', &fhdr, folder);
  }

  strcpy(fhdr.owner, cuser.userid);
  strcpy(fhdr.title, hdr->title);
  if (pos < 0)
    rec_add(dstDir, &fhdr, sizeof(HDR));
  else
    rec_ins(dstDir, &fhdr, sizeof(HDR), pos, 1);
  gem_log(dstDir, "�ƻs", &fhdr);

  if (xmode & GEM_FOLDER)	/* ���v/���� */
  {
    /* �إߧ��ۤv�o�Ө��v/�����H��A�A recursive �a�@�h�@�h�ؿ��i�h�@�g�@�g�t�s�s�� */
    hdr_fpath(folder, srcDir, hdr);
    if (data = (HDR *) f_img(folder, &fsize))
    {
      head = data;
      tail = data + (fsize / sizeof(HDR));
      do
      {
	gem_do_paste(folder, fpath, head, -1);
      } while (++head < tail);

      free(data);
    }
  }
}


static int
gem_paste(xo)
  XO *xo;
{
  int num, ans, pos;
  char *dir;
  HDR *head, *tail;

  if (!(xo->key & GEM_W_BIT))
    return XO_NONE;

  if (!(num = GemBufferNum))
  {
    zmsg("�Х����� copy �R�O��A paste");
    /* return XO_NONE; */
    return XO_FOOT;		/* itoc.010726: �� b_lines ��W feeter */
  }

  dir = xo->dir;

  /* switch (ans = vans("�s���m A)ppend I)nsert N)ext E)xtend Q)uit [A] ")) */
  switch (ans = vans("�s���m A)�[��̫� I/N)���J�ثe��m E)���[�ɮ� Q)���} [A] "))
  {
  case 'q':
    return XO_FOOT;

  case 'e':
    if (gem_extend(xo, num))
      zmsg("[Extend �ɮת��[] �ʧ@�å��������\\");
    return XO_FOOT;

  default:
    pos = (ans == 'n') ? xo->pos + 1 : (ans == 'i') ? xo->pos : -1;

    head = GemBuffer;
    tail = head + num;
    do
    {
      if (invalid_loop(GemFolder, dir, head, 0))	/* itoc.010727: �y���j��̤����\�K�W */
      {
	vmsg("�y���j�骺���v�N�L�k����");
	continue;
      }
      else
      {
	gem_do_paste(GemFolder, dir, head, pos);
	if (pos >= 0)		/* Insert/Next:�n�~�򩹤U�K Append:�@���K�b�̫� */
	  pos++;
      }
    } while (++head < tail);
  }

  return gem_load(xo);
}


static int
gem_move(xo)
  XO *xo;
{
  HDR *hdr;
  char *dir, buf[40];
  int pos, newOrder;

  if (!(xo->key & GEM_W_BIT) || !(hdr = gem_check(xo, NULL, 0)))
    return XO_NONE;

  pos = xo->pos;
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
    if (!rec_del(dir, sizeof(HDR), pos, NULL))
    {
      rec_ins(dir, hdr, sizeof(HDR), newOrder, 1);
      xo->pos = newOrder;
      return gem_load(xo);
    }
  }
  return XO_FOOT;
}


static int
gem_anchor(xo)
  XO *xo;
{
  int ans;
  char *folder;

  if (!(xo->key & GEM_W_BIT))	/* Thor.981020: �u�n�O�D�H�W�Y�i�ϥ�anchor */
    return XO_NONE; 		/* Thor.981020: ���}��@�� user �ϥάO���F����O�D�եX�t�@�Ӥp bug :P */
  
  ans = vans("��ذ� A)�w�� D)���� J)�N�� Q)���� [A] ");
  if (ans != 'q')
  {
    folder = GemAnchor;

    if (ans == 'j')
    {
      if (!*folder)			/* �S���w�� */
	return XO_FOOT;

      XoGem(folder, "�� ��ةw��� ��", xo->key);
      return gem_init(xo);
    }
    else if (ans == 'd')
    {
      *folder = '\0';
    }
    else
    {
      strcpy(folder, xo->dir);
      str_ncpy(GemSailor, xo->xyz, sizeof(GemSailor));
    }

    zmsg("��ʧ@����");
  }

  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010726: �� b_lines ��W feeter */
}


int
gem_gather(xo)
  XO *xo;
{
  HDR *hdr, *gbuf, ghdr, xhdr;
  int tag, locus, index, rc, xmode;
  char *dir, *folder, fpath[80];
  FILE *fp;

  folder = GemAnchor;

  if (!*folder)
  {
    zmsg("�Х��w��H��A���������ܩw���");
    return XO_FOOT;
  }

  sprintf(fpath, "�����ܩw��� (%s)", GemSailor);
  tag = AskTag(fpath);

  if (tag < 0)
    return XO_FOOT;

  fp = NULL;

  if (tag > 0)
  {
    switch (vans("��C�峹 1)�X���@�g 2)���O���� Q)���� [1] "))
    {
    case 'q':
      return XO_FOOT;

    case '2':
      break;

    default:
      strcpy(xhdr.title, currtitle);
      if (!vget(b_lines, 0, "���D�G", xhdr.title, TTLEN + 1, GCARRY))
	return XO_FOOT;
      fp = fdopen(hdr_stamp(folder, 'A', &ghdr, fpath), "w");
      strcpy(ghdr.owner, cuser.userid);
      strcpy(ghdr.title, xhdr.title);
    }
  }

  dir = xo->dir;
  hdr = tag ? &xhdr : (HDR *) xo_pool + xo->pos - xo->top;
  rc = (*dir == 'g') ? XO_NONE : XO_FOOT;

  /* gather ���P copy�A�i�ǳƧ@ paste */

  strcpy(GemFolder, folder);
  gbuf = gbuf_malloc((fp != NULL || tag == 0) ? 1 : tag);

  /* itoc.010727.����: �Y�X���@�g�Υu�����@�g�A�h GemBufferNum = 1�A�Ϥ��O tag �ƶq */

  index = 0;
  locus = 0;

  do
  {
    if (tag)
      EnumTag(hdr, dir, locus, sizeof(HDR));

    xmode = hdr->xmode;

    if (xmode & (GEM_RESTRICT | GEM_RESERVED))	/* ����ź�ذϤ���w�㦬�� */
      continue;
    
    if (!(xmode & GEM_FOLDER))	/* �d hdr �O�_ plain text (�Y�峹/���/����) */
    {
      hdr_fpath(fpath, dir, hdr);

      if (fp)	/* �X���@�g */
      {
	f_suck(fp, fpath);
	fputs(str_line, fp);
      }
      else	/* ���O���� */
      {
	hdr_stamp(folder, HDR_COPY | 'A', &ghdr, fpath);
	strcpy(ghdr.owner, cuser.userid);	/* �����ΪO�D id */
	strcpy(ghdr.title, hdr->title);
	ghdr.xmode = 0;				/* xmode �O�峹 */

	gbuf[index] = ghdr;		/* itoc.010727: ���L���O plain text �� */
	index++;
	rec_add(folder, &ghdr, sizeof(HDR));
	gem_log(folder, "����", &ghdr);
      }
    }
  } while (++locus < tag);

  if (fp)
  {
    fclose(fp);
    gbuf[0] = ghdr;
    rec_add(folder, &ghdr, sizeof(HDR));
    gem_log(folder, "�s�W", &ghdr);
  }
  else
  {
    /* itoc.010727: tag �ƶq�������O plain text ���~�O�u�O�ŶKï���g�� */
    /* itoc.010727.����: �p�G GemBufferNum �ƥؤ���A���� gem_paste() �ɴN�|�o�Ϳ��~ */
    GemBufferNum = index;
  }

  zmsg("���������A���O�[�K�峹���|�Q����");

  if (rc == XO_NONE)	/* �b��ذϤ� gem_gather() �~�n��ø�ŶKï�g�ơA�b�ݪO/�H�c�̳����� */
  {
    /* itoc.010727: ��ø�ŶKï�g�� */
    move(1, 59);
    clrtoeol();
    prints("(�ŶKï %d �g)\n", GemBufferNum);
  }

  return XO_FOOT;
}


static int
gem_tag(xo)
  XO *xo;
{
  HDR *hdr;
  int pos, tag;

  if ((hdr = gem_check(xo, NULL, 0)) &&
    (tag = Tagger(hdr->chrono, pos = xo->pos, TAG_TOGGLE)))
  {
    move(3 + pos - xo->top, 7);
    outc(tag > 0 ? '*' : ' ');
  }

  /* return XO_NONE; */
  return xo->pos + 1 + XO_MOVE; /* lkchu.981201: ���ܤU�@���A��K�s�� tag */
}


static int
gem_help(xo)
  XO *xo;
{
  xo_help("gem");
  return gem_head(xo);
}


static KeyFunc gem_cb[] =
{
  XO_INIT, gem_init,
  XO_LOAD, gem_load,
  XO_HEAD, gem_head,
  XO_BODY, gem_body,

  'r', gem_browse,

  Ctrl('P'), gem_add_all,	/* itoc.010723: gem_cb ���޼ƥu�� xo */
  'a', gem_add_article,
  'f', gem_add_folder,

  'E', gem_edit,
  'T', gem_title,
  'd', gem_delete,
  'D', gem_rangedel,		/* itoc.010726: ���ѰϬq�R�� */

  'c', gem_copy,
  'g', gem_gather,

  Ctrl('G'), gem_anchor,
  Ctrl('V'), gem_paste,
  'p', gem_paste,		/* itoc.010223: �ϥΪ̲ߺD c/p ������ذ� */

  't', gem_tag,
  'x', post_cross,		/* �b post/mbox �����O�p�g x ��ݪO�A�j�g X ��ϥΪ� */
  'X', post_forward,
  'B', gem_toggle,
  'o', gem_refuse,
  'm', gem_move,
  'M', gem_move,

  'S', gem_state,

  Ctrl('D'), gem_prune,
#if 0
  Ctrl('Q'), xo_uquery,		/* ��ذϪ� hdr.owner �@��O���������O�D�A�Ӥ��O�@�� */
  Ctrl('O'), xo_usetup,		/* ��ذϪ� hdr.owner �@��O���������O�D�A�Ӥ��O�@�� */
#endif

  'h', gem_help
};


void
XoGem(folder, title, level)
  char *folder;
  char *title;
  int level;
{
  XO *xo, *last;

  last = xz[XZ_GEM - XO_ZONE].xo;	/* record */

  xz[XZ_GEM - XO_ZONE].xo = xo = xo_new(folder);
  xo->pos = 0;
  xo->key = level;
  xo->xyz = title;

  xover(XZ_GEM);

  free(xo);

  xz[XZ_GEM - XO_ZONE].xo = last;	/* restore */
}


void
gem_main()
{
  XO *xo;

  xz[XZ_GEM - XO_ZONE].xo = xo = xo_new("gem/"FN_DIR);
  xz[XZ_GEM - XO_ZONE].cb = gem_cb;
  xo->pos = 0;
  /* �ݪO�`�ަb (A)nnounce �̭��� GEM_X_BIT �ӷs�W�ݪO���| */
  xo->key = (HAS_PERM(PERM_ALLBOARD) ? (GEM_W_BIT | GEM_X_BIT | GEM_M_BIT) : 0);
  xo->xyz = "";
}
