/*-------------------------------------------------------*/
/* ulist.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : ulist routines	 			 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern UCACHE *ushm;
extern XZ xz[];


/*-------------------------------------------------------*/
/* ��榡��Ѥ���					 */
/*-------------------------------------------------------*/


static int pickup_ship = 0;	/* 0:�G�m !=0:�ͽ˱ԭz */

typedef UTMP *pickup;

/* �����ǧY���ƧǪ����� */
#define FTYPE_SELF	0x01
#define FTYPE_BOTHGOOD	0x02
#define FTYPE_MYGOOD	0x04
#define FTYPE_OGOOD	0x08
#define FTYPE_NORMAL	0x10
#define FTYPE_MYBAD	0x20

static int mygood_num;		/* ���]�ڬ��n�� */
static int ogood_num;		/* �ڳ]��謰�n�� */

static pickup ulist_pool[MAXACTIVE];
/* static */ int ulist_userno[MAXACTIVE];	/* ���� ushm ���U�檺 userno */
static int ulist_ftype[MAXACTIVE];		/* ���� ushm ���U�檺�B�ͺ��� */

static int ulist_init();
static int ulist_head();
static XO ulist_xo;


#if 0
static char *
pal_ship(ftype, userno)	/* itoc.020811: �Ǧ^�B�ͱԭz */
  int ftype, userno;
{
  int fd;
  PAL *pal;
  char fpath[64];
  static char palship[46];

  if (ftype & (FTYPE_BOTHGOOD | FTYPE_MYGOOD | FTYPE_MYBAD))	/* ���]�n�͡B�ڪ��n�͡B�a�H�~���ͽ˱ԭz */
  {
    usr_fpath(fpath, cuser.userid, fn_pal);
    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
      mgets(-1);
      while (pal = mread(fd, sizeof(PAL)))
      {
	if (userno == pal->userno)
	{
	  strcpy(palship, pal->ship);
	  close(fd);
	  return palship;
	}
      }
      close(fd);
    }
  }
  return "";
}
#endif


#if 1	/* itoc.020901: �� cache ���M����n�A���O�Z���O�O���骺 */
typedef struct
{
  int userno;
  char ship[20];	/* ���ݭn�M PAL.ship �@�ˤj�A�u�n�� ulist_body() ��ܧY�i */
}	PALSHIP;

        
static char *
pal_ship(ftype, userno)	/* itoc.020811: �Ǧ^�B�ͱԭz */
  int ftype, userno;
{
  static PALSHIP palship[PAL_MAX] = {0};
  PALSHIP *pp;

  /* itoc.020901: �� palship ���i�O����A���n�@�� I/O �F */
  if (!palship[0].userno)	/* initialize *palship[] */
  {
    int fd;
    char fpath[64];
    PAL *pal;

    /* ���D�Ĳv�A�C���W���Ȱ��@���A�G�Y���ܪB�ͱԭz�A�n���s�W���~�ͮ� */
    usr_fpath(fpath, cuser.userid, fn_pal);
    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
      pp = palship;
      mgets(-1);
      while (pal = mread(fd, sizeof(PAL)))
      {
	if (pal->ship[0])	/* ���ͽˤ~���J palship[] */
	{
	  pp->userno = pal->userno;
	  str_ncpy(pp->ship, pal->ship, sizeof(pp->ship));
	  pp++;
	}
      }
      close(fd);
    }    
  }

  if (ftype & (FTYPE_BOTHGOOD | FTYPE_MYGOOD | FTYPE_MYBAD))	/* ���]�n�͡B�ڪ��n�͡B�a�H�~���ͽ˱ԭz */
  {
    /* �g pal_sync �H�᪺�B�ͦW��O�� ID �ƧǪ��A�Ҽ{�O�_�� binary search? */
    pp = palship;
    while (pp->userno)
    {
      if (pp->userno == userno)
	return pp->ship;
      pp++;
    }
  }
  return "";
}
#endif


static void
ulist_item(num, up, slot, now, sysop)
  int num;
  UTMP *up;
  int slot;
  time_t now;
  int sysop;
{
  time_t diff, ftype;
  int userno, ufo;
  char pager, buf[15], *fcolor;

  if (!(userno = up->userno))
  {
    outs("      < ������ͥ������} >\n");
    return;
  }

  /* itoc.011022: �Y�ͤ��ѤW���A�ɥ� idle ���ө�جP */
  if (up->status & STATUS_BIRTHDAY)
  {
    strcpy(buf, "\033[1;31m�جP\033[m");
  }
  else
  {
#ifdef DETAIL_IDLETIME
    if ((diff = now - up->idle_time) >= 60)	/* �W�L 60 ��~�ⶢ�m */
      sprintf(buf, "%3d'%02d", diff / 60, diff % 60);
#else
    if (diff = up->idle_time)
      sprintf(buf, "%2d", diff);
#endif
    else
      buf[0] = '\0';
  }

  ufo = up->ufo;

  /*         pager ���A                       */
  /*  #�G����������H�I�s�A�]����������H�s�� */
  /*  *�G�u�����n�ͩI�s�A�B�u�����n�ͼs��     */
  /*  !�G�u�����n�ͩI�s�A������������H�s��   */
  /*  -�G��������H�I�s�A������������H�s��   */
  /*   �G���S���N�O�S�������                 */
  if (ufo & UFO_QUIET)
  {
    pager = '#';
  }
  else if (ufo & UFO_PAGER)
  {
#ifdef HAVE_NOBROAD
    if (ufo & UFO_RCVER)
      pager = '!';
    else
#endif
      pager = '*';
  }
#ifdef HAVE_NOBROAD
  else if (ufo & UFO_RCVER)
  {
    pager = '-';
  }
#endif
  else
  {
    pager = ' ';
  }

  ftype = ulist_ftype[slot];

  fcolor = 
#ifdef HAVE_BRDMATE
# ifdef HAVE_ANONYMOUS
    up->mode == M_READA && !(currbattr & BRD_ANONYMOUS) && !strcmp(currboard, up->reading) ? COLOR_BRDMATE :
# else
    up->mode == M_READA && !strcmp(currboard, up->reading) ? COLOR_BRDMATE :
#  endif
#endif
    ftype & FTYPE_NORMAL ? COLOR_NORMAL : 
    ftype & FTYPE_BOTHGOOD ? COLOR_BOTHGOOD : 
    ftype & FTYPE_MYGOOD ? COLOR_MYGOOD : 
    ftype & FTYPE_OGOOD ? COLOR_OGOOD : 
    ftype & FTYPE_SELF ? COLOR_SELF : 
    ftype & FTYPE_MYBAD ? COLOR_MYBAD : 
    "";

  prints("%6d%c%c%s%-13s%-*.*s\033[m%-*.*s%-11.10s%s\n",
    num, ufo & UFO_CLOAK ? ')' : ' ', pager, 
    fcolor, up->userid, 
    (d_cols >> 1) + 21, (d_cols >> 1) + 20, up->username, 
    d_cols - (d_cols >> 1) + 19, d_cols - (d_cols >> 1) + 18, 
    pickup_ship ? pal_ship(ftype, up->userno) : 
#ifdef GUEST_WHERE
    (pager == ' ' || sysop || ftype & (FTYPE_SELF | FTYPE_BOTHGOOD | FTYPE_OGOOD) || !up->userlevel) ? 	/* �i�ݨ� guest ���G�m */
#else
    (pager == ' ' || sysop || ftype & (FTYPE_SELF | FTYPE_BOTHGOOD | FTYPE_OGOOD)) ?			/* ���]�ڬ��n�ͥi�ݨ����ӷ� */
#endif
    up->from : "*", bmode(up, 0), buf);
}


static int
ulist_body(xo)
  XO *xo;
{
  pickup *pp;
  UTMP *up;
  int num, max, tail, sysop, seecloak, slot;
#ifdef HAVE_SUPERCLOAK
  int seesupercloak;
#endif
#ifdef DETAIL_IDLETIME
  time_t now;
#endif

  max = xo->max;
  if (max <= 0)
  {
    if (vans("�ثe�S���n�ͤW���A�n�ݬݨ�L�ϥΪ̶�(Y/N)�H[Y] ") != 'n')
    {
      cuser.ufo ^= UFO_PAL;
      cutmp->ufo = cuser.ufo;
      return ulist_init(xo);
    }
    return XO_QUIT;
  }

  num = xo->top;
  pp = &ulist_pool[num];
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  sysop = HAS_PERM(PERM_ALLACCT);
  seecloak = HAS_PERM(PERM_SEECLOAK);
#ifdef HAVE_SUPERCLOAK
  seesupercloak = cuser.ufo & UFO_SUPERCLOAK;
#endif
#ifdef DETAIL_IDLETIME
  time(&now);
#endif

  move(3, 0);
  do
  {
    up = *pp;
    slot = up - ushm->uslot;

    /* itoc.011124: �p�G�b ulist_body() ���o�{ userno ���X�A��ܤ�W�o���W���b���¤F�A�j����s */
    if (ulist_userno[slot] != up->userno)
      return ulist_init(xo);

#ifdef DETAIL_IDLETIME
    ulist_item(++num, up, slot, now, sysop);
#else
    ulist_item(++num, up, slot, NULL, sysop);
#endif

    pp++;
  } while (num < max);
  clrtobot();

  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: �� b_lines ��W feeter */
}


static int
ulist_cmp_userid(i, j)
  UTMP **i, **j;
{
  int k = ulist_ftype[(*i) - ushm->uslot] - ulist_ftype[(*j) - ushm->uslot];
  return k ? k : str_cmp((*i)->userid, (*j)->userid);
}


static int
ulist_cmp_host(i, j)
  UTMP **i, **j;
{
  int k = ulist_ftype[(*i) - ushm->uslot] - ulist_ftype[(*j) - ushm->uslot];
  /* return k ? k : (*i)->in_addr - (*j)->in_addr; */
  /* Kyo.050112: in_addr �O unsigned int (u_long)�A������|�y�� int �~�P */
  return k ? k : (*i)->in_addr > (*j)->in_addr ? 1 : (*i)->in_addr < (*j)->in_addr ? -1 : 0;
}


static int
ulist_cmp_mode(i, j)
  UTMP **i, **j;
{
  int k = ulist_ftype[(*i) - ushm->uslot] - ulist_ftype[(*j) - ushm->uslot];
  return k ? k : (*i)->mode - (*j)->mode;
}


#ifdef HAVE_BRDMATE
static int
ulist_cmp_brdmate(i, j)
  UTMP **i, **j;
{
#ifdef HAVE_ANONYMOUS
  if (!(currbattr & BRD_ANONYMOUS) || HAS_PERM(PERM_SYSOP))	/* �\Ū�ΦW�O�h���C�J */
#endif
  {
    int ibrdmate = (*i)->mode == M_READA && !strcmp(currboard, (*i)->reading);
    int jbrdmate = (*j)->mode == M_READA && !strcmp(currboard, (*j)->reading);

    /* �O���u�� */
    if (ibrdmate && !jbrdmate)
      return -1;
    if (jbrdmate && !ibrdmate)
      return 1;
  }

  /* �����O�γ��O�O�񪺸ܡA�� ID �Ƨ� */
  return ulist_cmp_userid(i, j);
}
#endif


#ifdef HAVE_BRDMATE
#define PICKUP_WAYS	4
#else
#define PICKUP_WAYS	3
#endif

static int pickup_way = 0;	/* �w�]�ƦC�覡 0:�N�� 1:�G�m 2:�ʺA 3:�O�� */


static int (*ulist_cmp[PICKUP_WAYS]) () =
{
  ulist_cmp_userid,
  ulist_cmp_host,
  ulist_cmp_mode,
#ifdef HAVE_BRDMATE
  ulist_cmp_brdmate,
#endif
};


static char *msg_pickup_way[PICKUP_WAYS] =
{
  "���ͥN��",
  "�ȳ~�G�m",
  "���ͰʺA",
#ifdef HAVE_BRDMATE
  "�O��N��",
#endif
};


static int
ulist_paltype(up)		/* �B�ͺ��� */
  UTMP *up;
{
  const int userno = up->userno;

  if (userno == cuser.userno)
    return FTYPE_SELF;
  if (is_mybad(userno))
    return FTYPE_MYBAD;
  if (is_mygood(userno))
    return is_ogood(up) ? FTYPE_BOTHGOOD : FTYPE_MYGOOD;
  return is_ogood(up) ? FTYPE_OGOOD : FTYPE_NORMAL;
}


#if 0	/* itoc.041001: ulist_init() ���� */

  1. ushm->uslot �O�������� UTMP (�ϥΪ̦W����)�A�o�O���Ҧ��H�@�Ϊ�

  2. �C�ӤH��W�A�U���H�U�T���G
     ulist_pool �O ushm->uslot �����ޡA�O���ۧڥi�H�ݨ���ǤH
     ulist_userno[i] �O���� ushm->uslot[i] �o��l���F��
     ulist_ftype[i] �O���� ushm->uslot[i] �o��l�O�ڪ� �n��/�a�H/�@��H

  3. �b ulist_init() �̭��h ushm->uslot �s���Ҧ���l�A�Ҽ{ ushm->uslot[i]
     �p�G�ڥi�H�ݨ�o�ӤH�A�N�⥦�۶i�� ulist_pool
     �Y ushm->uslot[i].userno != ulist_userno[i]�A��ܳo�Ӧ�l���F�@�ӷs�W�����H�A
     �ڴN�h�d�L�O���O�ڪ��n��/�a�H�A�ðO���b ulist_ftype[i]�F
     �Y ushm->uslot[i].userno == ulist_userno[i]�A��ܳo�Ӧ�l�S���H�A
     �ڴN������ ulist_ftype[i] �ӷ�@�B�ͺ���

  4. ���F ulist_pool[] �H��A�A�� ulist_pool[] �̧ڷQ�n���覡�ƧǡA
     �b ulist_item() �L�X�Ӫ��C��h�O�ѷ� ulist_ftype[]

  5. �Y���H�N�ڦb�L���B�ͦW�椤���ʡA���L�|��ʧڪ� cutmp->status�A
     �ҥH����ˬd�� HAS_STATUS(STATUS_PALDIRTY)�A�ڴN�n����ڪ� ulist_ftype[]

#endif


static int
ulist_init(xo)
  XO *xo;
{
  UTMP *up, *uceil;
  pickup *pp;
  int filter, slot, userno, paldirty;

  pp = ulist_pool;
  filter = cuser.ufo & UFO_PAL;
  if (paldirty = HAS_STATUS(STATUS_PALDIRTY))
    cutmp->status ^= STATUS_PALDIRTY;

  slot = 0;
  up = ushm->uslot;
  uceil = (void *) up + ushm->offset;

  mygood_num = ogood_num = 0;

  /* �q ushm->uslot[] �ۨ� ulist_pool[] */
  do
  {
    userno = up->userno;

    if (userno > 0)
    {
      /* �s�W�����ϥΪ̡A�ݬݥL�O���@�����B�͡FSTATUS_PALDIRTY ���� ulist_ftype[] */
      if (ulist_userno[slot] != userno || paldirty)
      {
	ulist_userno[slot] = userno;
	ulist_ftype[slot] = ulist_paltype(up);
      }

      if (can_see(cutmp, up))
      {
	userno = ulist_ftype[slot];
	if (!filter || userno & (FTYPE_SELF | FTYPE_BOTHGOOD | FTYPE_MYGOOD))
	  *pp++ = up;

	/* �⦳�X�Ӧn�� */
	if (userno & (FTYPE_BOTHGOOD | FTYPE_MYGOOD))
	  mygood_num++;
	if (userno & (FTYPE_BOTHGOOD | FTYPE_OGOOD))
	  ogood_num++;
      }
    }
    slot++;
  } while (++up <= uceil);

  xo->max = slot = pp - ulist_pool;

  if (xo->pos >= slot)
    xo->pos = xo->top = 0;

  if (slot > 1)
    qsort(ulist_pool, slot, sizeof(pickup), ulist_cmp[pickup_way]);

  /* itoc.010928: �ѩ� ushm->count �`����A�ҥH�� total_user �Ӯե��A
     ��W���ɴN�ҩl�� total_user �� ushm->count�A����Y�ϥΪ̨S���ӨϥΪ̦W��A�N����s total_user */
  if (!filter)
    total_user = slot;

  return ulist_head(xo);
}


static int
ulist_neck(xo)
  XO *xo;
{
  move(1, 0);

  prints("  �ƦC�覡�G[\033[1m%s/%s\033[m] ���W�H�ơG%d "
    COLOR_MYGOOD " �ڪ��n�͡G%d " COLOR_OGOOD " �P�ڬ��͡G%d\033[m", 
    msg_pickup_way[pickup_way], 
    cuser.ufo & UFO_PAL ? "�n��" : "����", 
    total_user, mygood_num, ogood_num);

  prints(NECKER_ULIST, d_cols >> 1, "", d_cols - (d_cols >> 1) + 4, pickup_ship ? "�ͽ�" : "�G�m");
  return ulist_body(xo);
}


static int
ulist_head(xo)
  XO *xo;
{
  vs_head("���ͦC��", str_site);
  return ulist_neck(xo);
}


static int
ulist_toggle(xo)
  XO *xo;
{
  int ans, max;

#ifdef HAVE_BRDMATE
  ans = vans("�ƦC�覡 [1]�N�� [2]�ӷ� [3]�ʺA [4]�O�� ") - '1';
#else
  ans = vans("�ƦC�覡 [1]�N�� [2]�ӷ� [3]�ʺA ") - '1';
#endif
  if (ans >= 0 && ans < PICKUP_WAYS && ans != pickup_way)	/* Thor.980705: from 0 .. PICKUP_WAYS-1 */
  {
    pickup_way = ans;
    max = xo->max;

    if (max > 1)
    {
      qsort(ulist_pool, max, sizeof(pickup), ulist_cmp[pickup_way]);
      return ulist_neck(xo);
    }
  }

  return XO_FOOT;
}


static int
ulist_pal(xo)
  XO *xo;
{
  cuser.ufo ^= UFO_PAL;
  cutmp->ufo = cuser.ufo;
  return ulist_init(xo);
}


static int
ulist_search(xo, step)
  XO *xo;
  int step;
{
  int num, pos, max;
  pickup *pp;
  char buf[IDLEN + 1];

  if (vget(b_lines, 0, "�п�J�N���μʺ١G", buf, IDLEN + 1, DOECHO))
  {
    str_lowest(buf, buf);
    
    pos = num = xo->pos;
    max = xo->max;
    pp = ulist_pool;
    do
    {
      pos += step;
      if (pos < 0) /* Thor.990124: ���] max ����0 */
	 pos = max - 1;
      else if (pos >= max)
	pos = 0;

      if (str_str(pp[pos]->userid, buf) ||	/* lkchu.990127: �䳡�� id �n������n�� :p */
	str_sub(pp[pos]->username, buf)) 	/* Thor.990124: �i�H�� ���� nickname */
      {
	outf(FEETER_ULIST);	/* itoc.010913: �� b_lines ��W feeter */	
	return pos + XO_MOVE;
      }

    } while (pos != num);
  }

  return XO_FOOT;
}


static int
ulist_search_forward(xo)
  XO *xo;
{
  return ulist_search(xo, 1); /* step = +1 */
}


static int
ulist_search_backward(xo)
  XO *xo;
{
  return ulist_search(xo, -1); /* step = -1 */
}


static int
ulist_addpal(xo)
  XO *xo;
{
  if (cuser.userlevel)
  {
    UTMP *up;
    int userno;

    up = ulist_pool[xo->pos];
    userno = up->userno;
    if (userno > 0 && (userno != cuser.userno) &&	/* lkchu.981217: �ۤv���i���B�� */
      !is_mygood(userno) && !is_mybad(userno))		/* �|���C�J�B�ͦW�� */
    {
      PAL pal;
      char fpath[64];

      pal_edit(PALTYPE_PAL, &pal, DOECHO);
      pal.userno = userno;
      strcpy(pal.userid, up->userid);
      usr_fpath(fpath, cuser.userid, fn_pal);

      /* itoc.001222: �ˬd�B�ͭӼ� */
      if (rec_num(fpath, sizeof(PAL)) < PAL_MAX)
      {
	rec_add(fpath, &pal, sizeof(PAL));
	pal_cache();				/* �B�ͦW��P�B */
	utmp_admset(userno, STATUS_PALDIRTY);
	return ulist_init(xo);
      }
      else
      {
	vmsg("�z���B�ͦW��Ӧh�A�е��[��z");
	return XO_FOOT;
      }
    }
  }
  return XO_NONE;
}


static int
cmppal(pal)
  PAL *pal;
{
  return pal->userno == currchrono;
}


static int
ulist_delpal(xo)
  XO *xo;
{
  if (cuser.userlevel)
  {
    UTMP *up;
    int userno;

    up = ulist_pool[xo->pos];
    userno = up->userno;
    if (userno > 0 && (is_mygood(userno) || is_mybad(userno)))	/* �b�B�ͦW�椤 */
    {
      if (vans(msg_del_ny) == 'y')
      {
	char fpath[64];

	usr_fpath(fpath, cuser.userid, fn_pal);

	currchrono = userno;
	if (!rec_del(fpath, sizeof(PAL), 0, cmppal))
	{
	  pal_cache();				/* �B�ͦW��P�B */
	  utmp_admset(userno, STATUS_PALDIRTY);
	  return ulist_init(xo);
	}
      }
      return XO_FOOT;
    }
  }
  return XO_NONE;
}


static int
ulist_mail(xo)
  XO *xo;
{
  char userid[IDLEN + 1];

  /* ���ƻs�@�����W�A�H�K�b�g�H�� ushm �ܰʤF */
  strcpy(userid, ulist_pool[xo->pos]->userid);

  if (!*userid)
  {
    vmsg(MSG_USR_LEFT);
    return XO_FOOT;
  }

  return my_send(userid);
}


static int
ulist_query(xo)
  XO *xo;
{
  move(1, 0);
  clrtobot();
  my_query(ulist_pool[xo->pos]->userid);
  return ulist_neck(xo);
}


static int
ulist_broadcast(xo)
  XO *xo;
{
  int num, sysop;
  pickup *pp;
  UTMP *up;
  BMW bmw;

  num = cuser.userlevel;
  sysop = num & PERM_ALLADMIN;
  if (!sysop && (!(num & PERM_PAGE) || !(cuser.ufo & UFO_PAL)))
    return XO_NONE;

  num = xo->max;
  if (num <= 1)		/* �p�G�u���ۤv�A����s�� */
    return XO_NONE;

  /* itoc.030101: �p�G�����Ϊ��O�n�ͼs���A���P�@�� ID �s�� */
  sysop = sysop && !(cuser.ufo & UFO_PAL);

  bmw.caller = NULL;
  bmw_edit(NULL, "���s���G", &bmw);

  if (bmw.caller)	/* bmw_edit() ���^�� Yes �n�e�X�s�� */
  {
    /* itoc.000213: �[ "> " ���F�P�@����y�Ϥ� */
    sprintf(bmw.userid, "%s> ", cuser.userid);

    pp = ulist_pool;
    while (--num >= 0)
    {
      up = pp[num];

      if (!sysop)
      {
#ifdef HAVE_NOBROAD
	if (up->ufo & UFO_RCVER)
	  continue;
#endif

	/* itoc.011126: �Y up-> �w�U���A�Q��L user �Ҩ��N�ɡA
	   �|���s���~�Ӫ����D�A�o���s�ˬd�O�_���ڪ��n�� */
	if (!is_mygood(up->userno))
	  continue;
      }

      if (can_override(up))
      {
	bmw.recver = up->userno;
	bmw_send(up, &bmw);
      }
    }
  }

  return XO_NONE;
}


static int
ulist_talk(xo)
  XO *xo;
{
  if (HAS_PERM(PERM_PAGE))
  {
    UTMP *up;

    up = ulist_pool[xo->pos];
    if (can_override(up))
      return talk_page(up) ? ulist_head(xo) : XO_FOOT;
  }
  return XO_NONE;
}


static int
ulist_write(xo)
  XO *xo;
{
  if (HAS_PERM(PERM_PAGE))
  {
    UTMP *up;

    if (up = ulist_pool[xo->pos])
      do_write(up);
  }
  return XO_NONE;
}


static int
ulist_edit(xo)			/* Thor: �i�u�W�d�ݤέק�ϥΪ� */
  XO *xo;
{
  ACCT acct;

  if (!HAS_PERM(PERM_ALLACCT) || acct_load(&acct, ulist_pool[xo->pos]->userid) < 0)
    return XO_NONE;

  vs_bar("�ϥΪ̳]�w");
  acct_setup(&acct, 1);
  return ulist_head(xo);
}


static int
ulist_kick(xo)
  XO *xo;
{
  if (HAS_PERM(PERM_ALLACCT))
  {
    UTMP *up;
    pid_t pid;
    char buf[80];

    up = ulist_pool[xo->pos];
    if (pid = up->pid)
    {
      if (vans(msg_sure_ny) != 'y' || pid != up->pid)
	return XO_FOOT;

      sprintf(buf, "%s (%s)", up->userid, up->username);

      if ((kill(pid, SIGTERM) == -1) && (errno == ESRCH))
	utmp_free(up);
      else
	sleep(3);		/* �Q�𪺤H�o�ɭԥ��b�ۧڤF�_ */

      blog("KICK", buf);
      return ulist_init(xo);
    }
  }
  return XO_NONE;
}


#ifdef HAVE_CHANGE_NICK
static int
ulist_nickchange(xo)
  XO *xo;
{
  char *str, buf[UNLEN + 1];

  if (!cuser.userlevel)
    return XO_NONE;

  strcpy(buf, str = cutmp->username);
  if (vget(b_lines, 0, "�п�J�s���ʺ١G", buf, UNLEN + 1, GCARRY))
  {  
    if (strcmp(buf, str))
    {
      strcpy(str, buf);
      strcpy(cuser.username, buf);	/* �ʺ٤]�@�֧� cuser. */
      return ulist_body(xo);
    }
  }
  return XO_FOOT;
}
#endif


#ifdef HAVE_CHANGE_FROM
static int
ulist_fromchange(xo)
  XO *xo;
{
  char *str, buf[34];
  
  if (!cuser.userlevel)
    return XO_NONE;
  
  strcpy(buf, str = cutmp->from);
  if (vget(b_lines, 0, "�п�J�s���G�m�G", buf, sizeof(cutmp->from), GCARRY))
  {
    if (strcmp(buf, str))
    {
      strcpy(str, buf);
      return ulist_body(xo);
    }
  }

  return XO_FOOT;
}
#endif


#ifdef HAVE_CHANGE_ID
static int
ulist_idchange(xo)
  XO *xo;
{
  char *str, buf[IDLEN + 1];

  /* itoc.010717.����: �o�\�ണ�ѯ����i�H�b�ϥΪ̦W��Ȯɧ�ۤv�� ID�A
     ���O�ѩ� ulist �j�����O�� userno �ӧP�_�A�ҥH�u���n�ݦӤw */
  
  if (!HAS_PERM(PERM_ALLADMIN))
    return XO_NONE;
  
  strcpy(buf, str = cutmp->userid);
  if (vget(b_lines, 0, "�п�J�s���עҡG", buf, IDLEN + 1, GCARRY))
  {
    if (strcmp(buf, str))
    {
      strcpy(str, buf);
      return ulist_body(xo);
    }
  }

  return XO_FOOT;
}
#endif


static int  
ulist_cloak(xo)			/* itoc.010908: �ֳt���� */
  XO *xo;
{
  if (HAS_PERM(PERM_CLOAK))
  {
    cuser.ufo ^= UFO_CLOAK;
    cutmp->ufo = cuser.ufo;
    return ulist_init(xo);
  }
  return XO_NONE;
}


#ifdef HAVE_SUPERCLOAK
static int
ulist_supercloak(xo)		/* itoc.010908: �ֳt���� */
  XO *xo;
{
  if (cuser.ufo & UFO_SUPERCLOAK)	/* ���������A�������v�� */
  {
    cuser.ufo &= ~(UFO_CLOAK | UFO_SUPERCLOAK);
    cutmp->ufo = cuser.ufo;
    return ulist_init(xo);
  }
  else if (HAS_PERM(PERM_ALLADMIN))	/* �i�J���� */
  {
    cuser.ufo |= (UFO_CLOAK | UFO_SUPERCLOAK);
    cutmp->ufo = cuser.ufo;
    return ulist_init(xo);
  }
  return XO_NONE;
}
#endif


static int
ulist_ship(xo)
  XO *xo;
{
  pickup_ship = ~pickup_ship;
  return ulist_neck(xo);
}


static int
ulist_recall(xo)
  XO *xo;
{
  if (cuser.userlevel)
  {
    t_bmw();
    return ulist_head(xo);
  }
  return XO_NONE;
}


static int
ulist_display(xo)
  XO *xo;
{
  if (cuser.userlevel)
  {
    t_display();
    return ulist_head(xo);
  }
  return XO_NONE;
}


static int
ulist_help(xo)
  XO *xo;
{
  xo_help("ulist");
  return ulist_head(xo);
}


static KeyFunc ulist_cb[] =
{
  XO_INIT, ulist_init,
  XO_LOAD, ulist_body,
  XO_HEAD, ulist_head,
  /* XO_BODY, ulist_body, */	/* �S���Ψ� */

  'f', ulist_pal,
  'y', ulist_pal,		/* itoc.010205: ���H�|�� yank ���N��Φb�o */
  'a', ulist_addpal,
  'd', ulist_delpal,
  't', ulist_talk,
  'w', ulist_write,
  'l', ulist_recall,		/* ���y�^�U */
  'L', ulist_display,
  'r', ulist_query,
  'q', ulist_query,		/* itoc.020109: �ϥΪ̲ߺD�� q �d�� */
  'B', ulist_broadcast,
  's', ulist_init,		/* refresh status Thor: ��user�n�D */
  'S', ulist_ship,

  Ctrl('K'), ulist_kick,
  Ctrl('O'), ulist_edit,
  Ctrl('Q'), ulist_query,

#ifdef HAVE_CHANGE_NICK
  Ctrl('N'), ulist_nickchange,
#endif
#ifdef HAVE_CHANGE_FROM
  Ctrl('F'), ulist_fromchange,
#endif
#ifdef HAVE_CHANGE_ID
  Ctrl('D'), ulist_idchange,
#endif

#if 0
  '/', ulist_search,
#endif
  /* Thor.990125: �i�e��j�M, id or nickname */
  '/', ulist_search_forward,
  '?', ulist_search_backward,

  'm', ulist_mail,
  KEY_TAB, ulist_toggle,

  'i', ulist_cloak,
#ifdef HAVE_SUPERCLOAK
  'H', ulist_supercloak,
#endif

  'h', ulist_help
};


void
talk_main()
{
  char fpath[64];

  xz[XZ_ULIST - XO_ZONE].xo = &ulist_xo;
  xz[XZ_ULIST - XO_ZONE].cb = ulist_cb;

  /* itoc.010715: �ѩ� erevy_Z �i�H�����i�J bmw�A�ҥH�@�W���N�n���J */
  usr_fpath(fpath, cuser.userid, fn_bmw);
  xz[XZ_BMW - XO_ZONE].xo = xo_new(fpath);
}
