/*-------------------------------------------------------*/
/* aloha.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : aloha routines			 	 */
/* create : 95/03/29				 	 */
/* update : 00/01/02				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_ALOHA

extern XZ xz[];
extern char xo_pool[];


/* ----------------------------------------------------- */
/* �W���q���W��						 */
/* ----------------------------------------------------- */


static int
cmpfrienz(frienz)
  FRIENZ *frienz;
{
  return frienz->userno == cuser.userno;
}


static void
delbenz(xo, aloha)
  XO *xo;
  ALOHA *aloha;
{
  char fpath[64];

  usr_fpath(fpath, aloha->userid, FN_FRIENZ);
  while (!rec_del(fpath, sizeof(FRIENZ), 0, cmpfrienz))
    ;
}


static int
aloha_find(fpath, userno)
  char *fpath;
  int userno;
{
  ALOHA old;
  int fd;
  int rc = 0;
  
  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    while (read(fd, &old, sizeof(ALOHA)) == sizeof(ALOHA))
    {
      if (userno == old.userno)
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
chkaloha(aloha)
  ALOHA *aloha;
{
  int userno;

  userno = aloha->userno;
  return (userno > 0 && userno == acct_userno(aloha->userid));
}


static void
aloha_sync(fpath)
  char *fpath;
{
  int fsize;

  outz(MSG_CHKDATA);
  refresh();

  fsize = rec_sync(fpath, sizeof(ALOHA), str_cmp, chkaloha);

  if (fsize > ALOHA_MAX * sizeof(ALOHA))
    vmsg(msg_list_over);
}


/* ----------------------------------------------------- */
/* �W���q���W��G��榡�ާ@�ɭ��y�z			 */
/* ----------------------------------------------------- */


static int aloha_add();


static void
aloha_item(num, aloha)
  int num;
  ALOHA *aloha;
{
#ifdef CHECK_ONLINE
  UTMP *online = utmp_get(aloha->userno, NULL);

  prints("%6d%c   %s%-14s%s\n", num, tag_char(aloha->userno), 
  online ? COLOR7 : "", aloha->userid, online ? str_ransi : "");
#else
  prints("%6d%c   %-14s\n", num, tag_char(aloha->userno), aloha->userid);
#endif
}


static int
aloha_body(xo)
  XO *xo;
{
  ALOHA *aloha;
  int num, max, tail;

  max = xo->max;
  if (max <= 0)
  {
    if (vans("�n�s�W�W���(Y/N)�H[N] ") == 'y')
      return aloha_add(xo);
    return XO_QUIT;
  }

  aloha = (ALOHA *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  move(3, 0);
  do
  {
    aloha_item(++num, aloha++);
  } while (num < max);
  clrtobot();

  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: �� b_lines ��W feeter */
}


static int
aloha_head(xo)
  XO *xo;
{
  vs_head("�W���q��", str_site);
  prints(NECKER_ALOHA, d_cols, "");
  return aloha_body(xo);
}


static int
aloha_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(ALOHA));
  return aloha_body(xo);
}


static int
aloha_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(ALOHA));
  return aloha_head(xo);
}


static int
aloha_loadpal(xo)
  XO *xo;
{
  int fd, quota;
  char fpath[64];
  FRIENZ frienz;
  PAL pal;
  ALOHA aloha;
  
  if (vans("�n�ޤJ�n�ͦW���(Y/N)�H[N] ") == 'y')
  {
    usr_fpath(fpath, cuser.userid, FN_PAL);
    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
      /* itoc.001224: �ޤJ�W��u�[�� ALOHA_MAX */
      quota = ALOHA_MAX - xo->max;

      memset(&frienz, 0, sizeof(FRIENZ));
      frienz.userno = cuser.userno;
      strcpy(frienz.userid, cuser.userid);

      while (read(fd, &pal, sizeof(PAL)) == sizeof(PAL))
      {
	if (!aloha_find(xo->dir, pal.userno))
	{
	  memset(&aloha, 0, sizeof(ALOHA));
	  aloha.userno = pal.userno;
	  strcpy(aloha.userid, pal.userid);
	  rec_add(xo->dir, &aloha, sizeof(ALOHA));

	  usr_fpath(fpath, aloha.userid, FN_FRIENZ);
	  rec_add(fpath, &frienz, sizeof(FRIENZ));

	  /* �u�n���ޤJ����@�H�A�N���Щ�b�̫� */
	  xo->pos = XO_TAIL;

	  if (quota < 0)
	    break;
	}
      }
      close(fd);
      return aloha_load(xo);
    }
  }
  return XO_FOOT;
}


static int
aloha_add(xo)
  XO *xo;
{
  int userno;
  char fpath[64];
  FRIENZ frienz;
  ACCT acct;
  ALOHA aloha;

  if (xo->max >= ALOHA_MAX)
  {
    vmsg(msg_list_over);
    return XO_FOOT;
  }

  if ((userno = acct_get(msg_uid, &acct)) <= 0)
    return aloha_head(xo);

  if (userno == cuser.userno)   
  {
    vmsg("�ۤv�����[�J�W���q���W�椤");
    return aloha_head(xo);
  }

  if (!aloha_find(xo->dir, userno))
  {
    memset(&aloha, 0, sizeof(ALOHA));
    aloha.userno = userno;
    strcpy(aloha.userid, acct.userid);
    rec_add(xo->dir, &aloha, sizeof(ALOHA));

    memset(&frienz, 0, sizeof(FRIENZ));  
    frienz.userno = cuser.userno;
    strcpy(frienz.userid, cuser.userid);
    usr_fpath(fpath, aloha.userid, FN_FRIENZ);
    rec_add(fpath, &frienz, sizeof(FRIENZ));
  }
  xo->pos = XO_TAIL;
  xo_load(xo, sizeof(ALOHA));
  
  return aloha_head(xo);
}


static int
aloha_delete(xo)
  XO *xo;
{
  if (vans(msg_del_ny) == 'y')
  {
    char fpath[64];
    ALOHA *aloha;

    aloha = (ALOHA *) xo_pool + (xo->pos - xo->top);

    usr_fpath(fpath, aloha->userid, FN_FRIENZ);
    /* itoc.030310: ����: �� frienz �̭������Ъ� */
    while (!rec_del(fpath, sizeof(FRIENZ), 0, cmpfrienz))
      ;

    rec_del(xo->dir, sizeof(ALOHA), xo->pos, NULL);
    return aloha_init(xo);
  }
  return XO_FOOT;
}


static int
aloha_rangedel(xo)
  XO *xo;
{
  return xo_rangedel(xo, sizeof(ALOHA), NULL, delbenz);
}


static int
vfyaloha(aloha, pos)
  ALOHA *aloha;
  int pos;
{
  return Tagger(aloha->userno, pos, TAG_NIN);
}


static int
aloha_prune(xo)
  XO *xo;
{
  return xo_prune(xo, sizeof(ALOHA), vfyaloha, delbenz);
}


static int
aloha_mail(xo)
  XO *xo;
{
  ALOHA *aloha;

  aloha = (ALOHA *) xo_pool + (xo->pos - xo->top);
  return my_send(aloha->userid);
}


static int
aloha_write(xo)
  XO *xo;
{
  if (HAS_PERM(PERM_PAGE))
  {
    ALOHA *aloha;
    UTMP *up; 

    aloha = (ALOHA *) xo_pool + (xo->pos - xo->top);

    if (up = utmp_find(aloha->userno))
      do_write(up);
  }
  return XO_NONE;
}


static int
aloha_query(xo)
  XO *xo;
{
  ALOHA *aloha;

  aloha = (ALOHA *) xo_pool + (xo->pos - xo->top);
  move(1, 0);
  clrtobot();

  my_query(aloha->userid);
  return aloha_head(xo);
}


static int
aloha_sort(xo)
  XO *xo;
{
  aloha_sync(xo->dir);
  return aloha_init(xo);
}


static int
aloha_tag(xo)
  XO *xo;
{
  ALOHA *aloha;
  int tag, pos, cur;

  pos = xo->pos;
  cur = pos - xo->top;
  aloha = (ALOHA *) xo_pool + cur;

  if (tag = Tagger(aloha->userno, pos, TAG_TOGGLE))
  {
    move(3 + cur, 6);
    outc(tag > 0 ? '*' : ' ');
  }

  /* return XO_NONE; */
  return xo->pos + 1 + XO_MOVE;	/* lkchu.981201: ���ܤU�@�� */
}

      
static int
aloha_help(xo)
  XO *xo;
{
  xo_help("aloha");
  return aloha_head(xo);
}


static KeyFunc aloha_cb[] =
{
  XO_INIT, aloha_init,
  XO_LOAD, aloha_load,
  XO_HEAD, aloha_head,
  XO_BODY, aloha_body,

  'a', aloha_add,
  'd', aloha_delete,
  'm', aloha_mail,
  'w', aloha_write,
  'D', aloha_rangedel,
  'f', aloha_loadpal,

  'r', aloha_query,
  Ctrl('Q'), aloha_query,
  
  's', aloha_sort,
  't', aloha_tag,
  Ctrl('D'), aloha_prune,

  'h', aloha_help
};


int
t_aloha()
{
  XO *xo;
  char fpath[64];

  usr_fpath(fpath, cuser.userid, FN_ALOHA);
  xz[XZ_ALOHA - XO_ZONE].xo = xo = xo_new(fpath);
  xz[XZ_ALOHA - XO_ZONE].cb = aloha_cb;
  xover(XZ_ALOHA);
  free(xo);
  return 0;
}
#endif	/* HAVE_ALOHA */
