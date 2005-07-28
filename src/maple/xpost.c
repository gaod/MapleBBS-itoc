/*-------------------------------------------------------*/
/* xpost.c      ( NTHU CS MapleBBS Ver 2.39 )		 */
/*-------------------------------------------------------*/
/* target : bulletin boards' routines		 	 */
/* create : 95/03/29				 	 */
/* update : 96/04/05				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern XZ xz[];

extern char xo_pool[];


/*------------------------------------------------------------------------
  Thor.980509:
  �s�� �峹�j�M�Ҧ� �i���w�@ keyword, �C�X�Ҧ�keyword�������峹�C��

  �b tmp/ �U�} xpost.{pid} �@�� folder, �t�ؤ@map�}�C, �Χ@�P�� post �@ map
  �O���Ӥ峹�O�b�� post ����B, �p���i�@ mark, gem, edit, title ���\��,
  �B�����}�ɦ^�ܹ����峹�B
  <�H�W�Q�k obsolete...>

  Thor.980510:
  �إߤ峹�Q�צ�, like tin, �N�峹�� index ��J memory ��,
  ���ϥ� thread, �]�� thread�n�� folder ��...

  ������� Mode, Title & post list

  ���Ҽ{����²�ƪ� �W�U�䲾��..

  O->O->O->...
  |  |  |
  o  o  o
  |  |  |

  index�tfield {next, text} ����int, �t�m�]�� int
  �Ĥ@�h sorted by title, ���J�ɥ� binary search
  �B MMAP only , �Ĥ@�h��� # and +

  �����ѥ���Ϭq�R���ʧ@, �קK�V��
-------------------------------------------------------------------------*/


/* ----------------------------------------------------- */
/* ��C�j�M�D�{��					 */
/* ----------------------------------------------------- */


#define MSG_XYDENY	"�Х��h�X�ϥ� ^Z �H�e���걵/�s�D�\\��"

extern KeyFunc xpost_cb[];
extern KeyFunc xmbox_cb[];

static int *xypostI;		/* Thor: first ypost pos in ypost_xo.key */
static int comebackPos;		/* Thor: first xpost pos in xpost_xo.key */


static char xypostHintword[TTLEN + 1];
static char xypostHintauthor[IDLEN + 1];


static int
XoXpost(xo, hdr, on, off, fchk)		/* Thor: eXtended post : call from post_cb */
  XO *xo;
  HDR *hdr;		/* �j�M������ */
  int on, off;		/* �j�M���d�� */
  int (*fchk) ();	/* �j�M���禡 */
{
  int *xlist, fsize, max, locus;
  char *fimage;
  HDR *head, *tail;
  XO *xt;
#ifdef HAVE_XYNEWS
  int returnPos;
#endif

  if ((max = xo->max) <= 0)	/* Thor.980911: ����: �H���U�@ */
    return XO_FOOT;
  
  /* build index according to input condition */

  fimage = f_map(xo->dir, &fsize);

  if (fimage == (char *) -1)
  {
    vmsg("�ثe�L�k�}�ү�����");
    return XO_FOOT;
  }

  /* allocate index memory, remember free first */

  /* Thor.990113: �Ȱ�title, author�������S���Hpost */
  max = fsize / sizeof(HDR);
  xlist = xypostI = (int *) malloc(sizeof(int) * max);

  max = 0;
  head = (HDR *) fimage;
  tail = (HDR *) (fimage + fsize);

  locus = -1;
  do
  {
    locus++;
    if (locus < on)
      continue;
    if (locus > off)
      break;

#ifdef HAVE_REFUSEMARK
    if ((head->xmode & POST_RESTRICT) && 
      strcmp(head->owner, cuser.userid) && !(bbstate & STAT_BM))
      continue;
#endif

    /* check condition */

    if (!(* fchk) (head, hdr))
      continue;

    xlist[max++] = locus;
  } while (++head < tail);

  munmap(fimage, fsize);

  if (max <= 0)
  {
    free(xlist);
    xypostI = NULL;
    vmsg(MSG_XY_NONE);
    return XO_FOOT;
  }

  /* build XO for xpost_xo */

  comebackPos = xo->pos;	/* Thor: record pos, future use */
#ifdef HAVE_XYNEWS
  returnPos = comebackPos;
#endif

  xz[XZ_XPOST - XO_ZONE].xo = xt = xo_new(xo->dir);
  xz[XZ_XPOST - XO_ZONE].cb = (xo->dir[0] == 'b') ? xpost_cb : xmbox_cb;
  xt->pos = 0;
  xt->max = max;
  xt->xyz = xo->xyz;
  xt->key = XZ_XPOST;

  xover(XZ_XPOST);

  /* set xo->pos for new location */

#ifdef HAVE_XYNEWS
  if (xz[XZ_NEWS - XO_ZONE].xo)
    xo->pos = returnPos;	/* �q XZ_XPOST �^�� XZ_NEWS ��в��h��Ӫ��a�� */
  else
#endif
    xo->pos = comebackPos;	/* �q XZ_XPOST �^�� XZ_POST ��в��h�ҿ���峹���u����m */

  /* free xpost_xo */

  if (xt = xz[XZ_XPOST - XO_ZONE].xo)
  {
    free(xt);
    xz[XZ_XPOST - XO_ZONE].xo = NULL;
  }

  /* free index memory, remember check free pointer */

  if (xlist = xypostI)
  {
    free(xlist);
    xypostI = NULL;
  }

  return XO_INIT;
}


  /* --------------------------------------------------- */
  /* �j�M�@��/���D					 */
  /* --------------------------------------------------- */


static int			/* 0:����������  !=0:�������� */
filter_select(head, hdr)
  HDR *head;	/* �ݴ��� */
  HDR *hdr;	/* ���� */
{
  char *title;

  /* �ɥ� hdr->xid �� strlen(hdr->owner) */

  /* Thor.981109: �S�O�`�N�A���F���C load�Aauthor �O�q�Y match�A���O substr match */
  if (hdr->xid && str_ncmp(head->owner, hdr->owner, hdr->xid))
    return 0;

  if (hdr->title[0])
  {
    title = head->title;
    if (STR4(title) == STR4(STR_REPLY))	/* Thor.980911: ���� Re: ���~ */
      title += 4;
    if (!str_sub(title, hdr->title))
      return 0;
  }

  return 1;
}


int
XoXselect(xo)
  XO *xo;
{
  HDR hdr;
  char *key;

  if (xz[XZ_XPOST - XO_ZONE].xo)	/* itoc.020308: ���o�ֿn�i�J�G�� */
  {
    vmsg(MSG_XYDENY);
    return XO_FOOT;
  }

  /* input condition */

  key = hdr.title;
  if (vget(b_lines, 0, MSG_XYPOST1, key, 30, DOECHO))
  {
    strcpy(xypostHintword, key);
    str_lowest(key, key);
  }
  else
  {
    xypostHintword[0] = '\0';
  }

  key = hdr.owner;
  if (vget(b_lines, 0, MSG_XYPOST2, key, IDLEN + 1, DOECHO))
  {
    strcpy(xypostHintauthor, key);
    str_lower(key, key);
    hdr.xid = strlen(key);
  }
  else
  {
    xypostHintauthor[0] = '\0';
    hdr.xid = 0;
  }
  
  if (!hdr.title[0] && !hdr.xid)
    return XO_FOOT;

  return XoXpost(xo, &hdr, -1, INT_MAX, filter_select);
}


  /* --------------------------------------------------- */
  /* �j�M�@��						 */
  /* --------------------------------------------------- */


int
XoXauthor(xo)
  XO *xo;
{
  HDR hdr;
  char *author;

  if (xz[XZ_XPOST - XO_ZONE].xo)	/* itoc.020308: ���o�ֿn�i�J�G�� */
  {
    vmsg(MSG_XYDENY);
    return XO_FOOT;
  }

  author = hdr.owner;
  if (!vget(b_lines, 0, MSG_XYPOST2, author, IDLEN + 1, DOECHO))
    return XO_FOOT;

  xypostHintword[0] = '\0';
  strcpy(xypostHintauthor, author);

  hdr.title[0] = '\0';
  str_lower(author, author);
  hdr.xid = strlen(author);

  return XoXpost(xo, &hdr, -1, INT_MAX, filter_select);
}


  /* --------------------------------------------------- */
  /* �j�M���D						 */
  /* --------------------------------------------------- */


int
XoXtitle(xo)
  XO *xo;
{
  HDR hdr;
  char *title;

  if (xz[XZ_XPOST - XO_ZONE].xo)	/* itoc.020308: ���o�ֿn�i�J�G�� */
  {
    vmsg(MSG_XYDENY);
    return XO_FOOT;
  }

  title = hdr.title;
  if (!vget(b_lines, 0, MSG_XYPOST1, title, 30, DOECHO))
    return XO_FOOT;

  strcpy(xypostHintword, title);
  xypostHintauthor[0] = '\0';

  str_lowest(title, title);
  hdr.xid = 0;

  return XoXpost(xo, &hdr, -1, INT_MAX, filter_select);
}


  /* --------------------------------------------------- */
  /* �j�M�ۦP���D					 */
  /* --------------------------------------------------- */


static int			/* 0:����������  !=0:�������� */
filter_search(head, hdr)
  HDR *head;	/* �ݴ��� */
  HDR *hdr;	/* ���� */
{
  char *title, buf[TTLEN + 1];

  title = head->title;
  if (STR4(title) == STR4(STR_REPLY))	/* Thor.980911: ���� Re: ���~ */
    title += 4;
  str_lowest(buf, title);
  return !strcmp(buf, hdr->title);
}


int
XoXsearch(xo)
  XO *xo;
{
  HDR hdr, *mhdr;
  char *title;

  if (xz[XZ_XPOST - XO_ZONE].xo)	/* itoc.020308: ���o�ֿn�i�J�G�� */
  {
    vmsg(MSG_XYDENY);
    return XO_FOOT;
  }

  mhdr = (HDR *) xo_pool + (xo->pos - xo->top);

  title = mhdr->title;
  if (STR4(title) == STR4(STR_REPLY))
    title += 4;

  strcpy(xypostHintword, title);
  xypostHintauthor[0] = '\0';

  str_lowest(hdr.title, title);

  return XoXpost(xo, &hdr, -1, INT_MAX, filter_search);
}


  /* --------------------------------------------------- */
  /* ����j�M						 */
  /* --------------------------------------------------- */


static char *search_folder;
static int search_fit;		/* >=0:���X�g -1:���_�j�M */
static int search_all;		/* �w�j�M�X�g */

static int			/* 0:����������  !=0:�������� */
filter_full(head, hdr)
  HDR *head;	/* �ݴ��� */
  HDR *hdr;	/* ���� */
{
  char buf[80], *fimage;
  int rc, fsize;
  struct timeval tv = {0, 10};

  if (search_fit < 0)		/* ���_�j�M */
    return 0;

  if (search_all % 100 == 0)	/* �C 100 �g�~���i�@���i�� */
  {
    sprintf(buf, "�ثe��� \033[1;33m%d / %d\033[m �g�A����j�M��\033[5m...\033[m�����N�䤤�_", 
      search_fit, search_all);
    outz(buf);
    refresh();
  }
  search_all++;

  hdr_fpath(buf, search_folder, head);

  fimage = f_map(buf, &fsize);
  if (fimage == (char *) -1)
    return 0;

  rc = 0;
  if (str_sub(fimage, hdr->title))
  {
    rc = 1;
    search_fit++;
  }

  munmap(fimage, fsize);

  /* �ϥΪ̥i�H���_�j�M */
  fsize = 1;
  if (select(1, (fd_set *) &fsize, NULL, NULL, &tv) > 0)
  {
    vkey();
    search_fit = -1;
  }

  return rc;
}


int
XoXfull(xo)
  XO *xo;
{
  HDR hdr;
  char *key, ans[8];
  int head, tail;

  if (xz[XZ_XPOST - XO_ZONE].xo)	/* itoc.020308: ���o�ֿn�i�J�G�� */
  {
    vmsg(MSG_XYDENY);
    return XO_FOOT;
  }

  /* input condition */

  key = hdr.title;
  if (!vget(b_lines, 0, "��������r�G", key, 30, DOECHO))
    return XO_FOOT;

  vget(b_lines, 0, "[�]�w�j�M�d��] �_�I�G(Enter)�q�Y�}�l ", ans, 6, DOECHO);
  if ((head = atoi(ans)) <= 0)
    head = 1; 

  vget(b_lines, 44, "���I�G(Enter)���̫� ", ans, 6, DOECHO);
  if ((tail = atoi(ans)) < head)
    tail = INT_MAX;

  head--;
  tail--;

  sprintf(xypostHintword, "[����j�M] %s", key);
  xypostHintauthor[0] = '\0';
  str_lowest(key, key);

  search_folder = xo->dir;
  search_fit = 0;
  search_all = 0;

  return XoXpost(xo, &hdr, head, tail, filter_full);
}


  /* --------------------------------------------------- */
  /* �j�M mark						 */
  /* --------------------------------------------------- */


static int			/* 0:����������  !=0:�������� */
filter_mark(head, hdr)
  HDR *head;	/* �ݴ��� */
  HDR *hdr;	/* ���� */
{
  return (head->xmode & POST_MARKED);
}


int
XoXmark(xo)
  XO *xo;
{
  if (xz[XZ_XPOST - XO_ZONE].xo)	/* itoc.020308: ���o�ֿn�i�J�G�� */
  {
    vmsg(MSG_XYDENY);
    return XO_FOOT;
  }

  strcpy(xypostHintword, "\033[1;33m�Ҧ� mark �峹\033[m");
  xypostHintauthor[0] = '\0';

  return XoXpost(xo, NULL, -1, INT_MAX, filter_mark);
}


  /* --------------------------------------------------- */
  /* �j�M���a						 */
  /* --------------------------------------------------- */


static int			/* 0:����������  !=0:�������� */
filter_local(head, hdr)
  HDR *head;	/* �ݴ��� */
  HDR *hdr;	/* ���� */
{
  return !(head->xmode & POST_INCOME);
}


int
XoXlocal(xo)
  XO *xo;
{
  if (currbattr & BRD_NOTRAN)
  {
    vmsg("���O������H�O�A�������O���a�峹");
    return XO_FOOT;
  }

  if (xz[XZ_XPOST - XO_ZONE].xo)	/* itoc.020308: ���o�ֿn�i�J�G�� */
  {
    vmsg(MSG_XYDENY);
    return XO_FOOT;
  }

  strcpy(xypostHintword, "\033[1;33m�Ҧ��D��i�峹\033[m");
  xypostHintauthor[0] = '\0';

  return XoXpost(xo, NULL, -1, INT_MAX, filter_local);
}


/* ----------------------------------------------------- */
/* ��C�j�M�ɭ�						 */
/* ----------------------------------------------------- */


int
xpost_head(xo)
  XO *xo;
{
  vs_head("�D�D��C", xo->xyz);

  /* itoc.010323: �P�ɴ��ܧ@��/�D�D */
  outs("[�걵�t�C] ");
  if (*xypostHintauthor)
    prints("�@�̡G%-13s   ", xypostHintauthor);
  if (*xypostHintword)
    prints("���D�G%.30s", xypostHintword);

  prints(NECKER_XPOST, d_cols, "", currbattr & BRD_NOSCORE ? "��" : "��");

  return XO_BODY;
}


static void
xypost_pick(xo)
  XO *xo;
{
  int *xyp, fsize, pos, max, top, num;
  HDR *fimage, *hdr;

  fimage = (HDR *) f_map(xo->dir, &fsize);
  if (fimage == (HDR *) - 1)
    return;

  hdr = (HDR *) xo_pool;
  xyp = xypostI;

  pos = xo->pos;
  xo->top = top = (pos / XO_TALL) * XO_TALL;
  max = xo->max;
  pos = top + XO_TALL;
  if (max > pos)
    max = pos;
  num = fsize / sizeof(HDR);

  do
  {
    pos = xyp[top++];
    if (pos >= num)	/* hightman.030528: �קK .DIR �Q�R��ɡA�|�S���峹�i�H��� */
      continue;
    *hdr = fimage[pos];
    hdr->xid = pos;		/* �� hdr->xid �ӰO�������b�ݪO���� pos */
    hdr++;
  } while (top < max);

  munmap(fimage, fsize);
}


int
xpost_init(xo)
  XO *xo;
{
  /* load into pool */

  xypost_pick(xo);

  return xpost_head(xo);
}


int
xpost_load(xo)
  XO *xo;
{
  /* load into pool */

  xypost_pick(xo);

  return XO_BODY;
}


static void
xpost_history(xo, fhdr)		/* �N fhdr �o�g�[�J brh */
  XO *xo;
  HDR *fhdr;
{
  time_t prev, chrono, next;
  int pos;
  char *dir;
  HDR buf;

  chrono = fhdr->chrono;
  if (!brh_unread(chrono))	/* �p�G�w�b brh ���A�N�L�ݰʧ@ */
    return;

  dir = xo->dir;
  pos = fhdr->xid;

  if (!rec_get(dir, &buf, sizeof(HDR), pos - 1))
    prev = buf.chrono;
  else
    prev = chrono;

  if (!rec_get(dir, &buf, sizeof(HDR), pos + 1))
    next = buf.chrono;
  else
    next = chrono;

  brh_add(prev, chrono, next);
}


int
xpost_browse(xo)
  XO *xo;
{
  HDR *hdr;
  int key;
  char *dir, fpath[64];

  dir = xo->dir;

  for (;;)
  {
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);

#if 0	/* itoc.010822: ���ݭn�A�b XoXpost() ���w�Q�簣 */
#ifdef HAVE_REFUSEMARK
    xmode = hdr->xmode;
    if ((xmode & POST_RESTRICT) && 
      strcmp(hdr->owner, cuser.userid) && !(bbstate & STAT_BM))
      continue;
#endif
#endif

    hdr_fpath(fpath, dir, hdr);

    /* Thor.990204: ���Ҽ{more �Ǧ^�� */   
    if ((key = more(fpath, FOOTER_POST)) < 0)
      break;

    comebackPos = hdr->xid; 
    /* Thor.980911: �q�걵�Ҧ��^�Ӯɭn�^��ݹL�����g�峹��m */

    xpost_history(xo, hdr);
    strcpy(currtitle, str_ttl(hdr->title));

re_key:
    /* Thor.990204: ���Ҽ{more �Ǧ^�� */   
    if (!key)
      key = vkey();

    switch (key)
    {
    case KEY_UP:
    case KEY_PGUP:
    case '[':	/* itoc.000227: ��C�j�M���A���ɷQ�� [ �ݤW�@�g�峹 */
    case 'k':	/* itoc.000227: ��C�j�M���A���ɷQ�� k �ݤW�@�g�峹 */
      {
	int pos = xo->pos - 1;

	/* itoc.000227: �קK�ݹL�Y */
	if (pos < 0)
	  return xpost_head(xo);

	xo->pos = pos;

	if (pos <= xo->top)
	  xypost_pick(xo);
  
	continue;
      }

    case KEY_DOWN:
    case KEY_PGDN:
    case ']':	/* Thor.990204: ��C�j�M���A���ɷQ�� ] �ݤU�@�g�峹 */
    case 'j':	/* Thor.990204: ��C�j�M���A���ɷQ�� j �ݤU�@�g�峹 */
    case ' ':
      {
	int pos = xo->pos + 1;

	/* Thor.980727: �ץ��ݹL�Y��bug */

	if (pos >= xo->max)
    	  return xpost_head(xo);

	xo->pos = pos;

	if (pos >= xo->top + XO_TALL)
  	  xypost_pick(xo);

	continue;
      }

    case 'y':
    case 'r':
      if (bbstate & STAT_POST)
      {
	strcpy(quote_file, fpath);
	if (do_reply(xo, hdr) == XO_INIT)	/* �����\�a post �X�h�F */
	  return xpost_init(xo);
      }
      break;

    case 'm': 
      if ((bbstate & STAT_BOARD) && !(hdr->xmode & POST_MARKED))
      {
	/* �b xpost_browse �ɬݤ��� m �O���A�ҥH����u�� mark */
	hdr->xmode ^= POST_MARKED;
	currchrono = hdr->chrono;
	rec_put(dir, hdr, sizeof(HDR), hdr->xid, cmpchrono);
      } 
      break;

#ifdef HAVE_SCORE
    case '%': 
      post_score(xo);
      return xpost_init(xo);
#endif

    case '/':
      if (vget(b_lines, 0, "�j�M�G", hunt, sizeof(hunt), DOECHO))
      {
	key = more(fpath, FOOTER_POST);
	goto re_key;
      }
      continue;

    case 'E':
      return post_edit(xo);

    case 'C':	/* itoc.000515: xpost_browse �ɥi�s�J�Ȧs�� */
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
      xo_help("post");
      break;
    }
    break;
  }

  return xpost_head(xo);
}


int
xmbox_browse(xo)
  XO *xo;
{
  HDR *hdr;
  char *dir, fpath[64];

  int key;

  dir = xo->dir;

  for (;;)
  {
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);

    hdr_fpath(fpath, dir, hdr);

    /* Thor.990204: ���Ҽ{more �Ǧ^�� */   
    if ((key = more(fpath, FOOTER_MAILER)) < 0)
      break;

    comebackPos = hdr->xid; 
    /* Thor.980911: �q�걵�Ҧ��^�Ӯɭn�^��ݹL�����g�峹��m */

    strcpy(currtitle, str_ttl(hdr->title));

re_key:
    /* Thor.990204: ���Ҽ{more �Ǧ^�� */   
    if (!key)
      key = vkey();

    switch (key)
    {
    case KEY_UP:
    case KEY_PGUP:
    case '[':	/* itoc.000227: ��C�j�M���A���ɷQ�� [ �ݤW�@�g�峹 */
    case 'k':	/* itoc.000227: ��C�j�M���A���ɷQ�� k �ݤW�@�g�峹 */
      {
	int pos = xo->pos - 1;

	/* itoc.000227: �קK�ݹL�Y */
	if (pos < 0)
	  return xpost_head(xo);

	xo->pos = pos;

	if (pos <= xo->top)
	  xypost_pick(xo);
  
	continue;
      }

    case KEY_DOWN:
    case KEY_PGDN:
    case ']':	/* Thor.990204: ��C�j�M���A���ɷQ�� ] �ݤU�@�g�峹 */
    case 'j':	/* Thor.990204: ��C�j�M���A���ɷQ�� j �ݤU�@�g�峹 */
    case ' ':
      {
	int pos = xo->pos + 1;

	/* Thor.980727: �ץ��ݹL�Y��bug */

	if (pos >= xo->max)
    	  return xpost_head(xo);

	xo->pos = pos;

	if (pos >= xo->top + XO_TALL)
  	  xypost_pick(xo);

	continue;
      }

    case 'y':
    case 'r':
      strcpy(quote_file, fpath);
      do_mreply(hdr, 1);
      break;

    case 'm': 
      if (!(hdr->xmode & POST_MARKED))
      {
	/* �b xmbox_browse �ɬݤ��� m �O���A�ҥH����u�� mark */
	hdr->xmode ^= POST_MARKED;
	currchrono = hdr->chrono;
	rec_put(dir, hdr, sizeof(HDR), hdr->xid, cmpchrono);
      } 
      break;

    case '/':
      if (vget(b_lines, 0, "�j�M�G", hunt, sizeof(hunt), DOECHO))
      {
	key = more(fpath, FOOTER_MAILER);
	goto re_key;
      }
      continue;

    case 'E':
      return mbox_edit(xo);

    case 'C':	/* itoc.000515: xmbox_browse �ɥi�s�J�Ȧs�� */
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
      xo_help("mbox");
      break;
    }
    break;
  }

  return xpost_head(xo);
}


#ifdef HAVE_XYNEWS	/* itoc.010822: news �\Ū�Ҧ� */

#if 0	/* �c�Q */

�b�j�ݪO(�s�u�ݪO)���`�`���ܦh����峹�A��� news ���\Ū�覡�A
��Ҧ� reply ���峹�������áA�u��ܲĤ@�ʵo��C

�Ĥ@���Q�� XoNews �ӭ簣 reply �峹
�ĤG���Q�έ즳�� XoXsearch �j�M�P�D�D�峹
�p���N�i�H�F��s�D�\Ū�Ҧ����ĪG

#endif


static int *xynewsI;

extern KeyFunc news_cb[];


int
news_head(xo)
  XO *xo;
{
  vs_head("�s�D�\\Ū", xo->xyz);
  prints(NECKER_NEWS, d_cols, "");
  return XO_BODY;
}


static void
news_pick(xo)
  XO *xo;
{
  int *xyp, fsize, pos, max, top;
  HDR *fimage, *hdr;

  fimage = (HDR *) f_map(xo->dir, &fsize);
  if (fimage == (HDR *) - 1)
    return;

  hdr = (HDR *) xo_pool;
  xyp = xynewsI;

  pos = xo->pos;
  xo->top = top = (pos / XO_TALL) * XO_TALL;
  max = xo->max;
  pos = top + XO_TALL;
  if (max > pos)
    max = pos;

  do
  {
    pos = xyp[top++];
    *hdr = fimage[pos];
    /* hdr->xid = pos; */	/* �b XZ_NEWS �S�Ψ� xid�A�i�H�Ҽ{�O�d�� reply �g�� */
    hdr++;
  } while (top < max);

  munmap(fimage, fsize);
}


int
news_init(xo)
  XO *xo;
{
  /* load into pool */
  news_pick(xo);
  return news_head(xo);
}


int
news_load(xo)
  XO *xo;
{
  /* load into pool */
  news_pick(xo);
  return XO_BODY;
}


int
XoNews(xo)			/* itoc: News reader : call from post_cb */
  XO *xo;
{
  int returnPos;
  int *xlist, fsize, max, locus;
  char *fimage;
  HDR *head, *tail;
  XO *xt;

  if ((max = xo->max) <= 0)	/* Thor.980911: ����: �H���U�@ */
    return XO_FOOT;

  /* itoc.020308: ���o�ֿn�i�J�G�� */
  if (xz[XZ_NEWS - XO_ZONE].xo)
  {
    vmsg(MSG_XYDENY);
    return XO_FOOT;
  }

  /* build index according to input condition */

  fimage = f_map(xo->dir, &fsize);

  if (fimage == (char *) -1)
  {
    vmsg("�ثe�L�k�}�ү�����");
    return XO_FOOT;
  }

  /* allocate index memory, remember free first */

  /* Thor.990113: �Ȱ�title, author�������S���Hpost */
  max = fsize / sizeof(HDR);
  xlist = xynewsI = (int *) malloc(sizeof(int) * max);
  
  max = 0;
  head = (HDR *) fimage;
  tail = (HDR *) (fimage + fsize);

  locus = -1;
  do
  {
    locus++;

#ifdef HAVE_REFUSEMARK
    if ((head->xmode & POST_RESTRICT) && 
      strcmp(head->owner, cuser.userid) && !(bbstate & STAT_BM))
      continue;
#endif

    /* check condition */

    if (STR4(head->title) == STR4(STR_REPLY)) /* reply ���峹���n */
      continue;

    xlist[max++] = locus;
  } while (++head < tail);

  munmap(fimage, fsize);

  if (max <= 0)
  {
    free(xlist);
    xynewsI = NULL;
    vmsg(MSG_XY_NONE);
    return XO_FOOT;
  }

  /* build XO for news_xo */

  returnPos = xo->pos;	/* Thor: record pos, future use */
  xz[XZ_NEWS - XO_ZONE].xo = xt = xo_new(xo->dir);
  xz[XZ_NEWS - XO_ZONE].cb = news_cb;
  xt->pos = 0;
  xt->max = max;
  xt->xyz = xo->xyz;
  xt->key = XZ_NEWS;

  xover(XZ_NEWS);

  /* set xo->pos for new location */

  xo->pos = returnPos;

  /* free news_xo */

  if (xt = xz[XZ_NEWS - XO_ZONE].xo)
  {
    free(xt);
    xz[XZ_NEWS - XO_ZONE].xo = NULL;
  }

  /* free index memory, remember check free pointer */

  if (xlist = xynewsI)
  {
    free(xlist);
    xynewsI = NULL;
  }

  return XO_INIT;
}
#endif	/* HAVE_XYNEWS */
