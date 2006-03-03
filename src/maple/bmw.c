/*-------------------------------------------------------*/
/* bmw.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : bmw routines		 		 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern UCACHE *ushm;
extern XZ xz[];
extern char xo_pool[];


/* ----------------------------------------------------- */
/* BMW : bbs message write routines			 */
/* ----------------------------------------------------- */


#define BMW_FORMAT	"\033[1;33;46m��%s \033[37;45m %s \033[m"	/* ���쪺���y */
#define BMW_FORMAT2	"\033[1;33;41m��%s \033[34;47m %s \033[m"	/* �e�X�����y */

static int bmw_locus = 0;		/* �`�@�O�s�X�Ӥ��y (�O�d�̪񦬨� BMW_LOCAL_MAX ��) */
static BMW bmw_lslot[BMW_LOCAL_MAX];	/* �O�d���쪺���y */

static int bmw_locat = 0;		/* �`�@�O�s�X�Ӥ��y (�O�d�̪�e�X BMW_LOCAL_MAX ��) */
static BMW bmw_lword[BMW_LOCAL_MAX];	/* �O�d�e�X�����y */


int			/* 1:�i�H�Ǥ��y�����/�P���Talk  0:����Ǥ��y�����/�P���Talk */
can_override(up)
  UTMP *up;
{
  int ufo;

  if (up->userno == cuser.userno)	/* ����Ǥ��y���ۤv(�Y�ϬO����) */
    return 0;

  ufo = up->ufo;

#ifdef HAVE_SUPERCLOAK
  if ((ufo & UFO_SUPERCLOAK) && !(cuser.ufo & UFO_SUPERCLOAK))	/* �����u���������~�ݪ��� */
    return 0;
#endif

  /* itoc.010909.����: �����i�H�Ǥ��y�� ��w/BBSNET... ���H�A�o�˦n�ܡH�[� */

  if (HAS_PERM(PERM_ALLACCT))	/* �����B�b���޲z���i�H�ǵ�����H */
    return 1;

  /* itoc.010909: ��w�ɤ���Q�Ǥ��y */
  if ((ufo & UFO_QUIET) || (up->status & STATUS_REJECT))	/* ��������/��w�� ����Q�� */
    return 0;

  if (!(up->ufo & UFO_CLOAK) || HAS_PERM(PERM_SEECLOAK))
  {
    /* itoc.001223: �� is_ogood/is_obad �Ӱ��P�_ */
    if (ufo & UFO_PAGER)
      return is_ogood(up);		/* pager �����ɥu���Q�]�n�ͯ�Ǥ��y */
    else
      return !is_obad(up);		/* pager ���}�ɥu�n�S���Q�]�a�H�Y�i�Ǥ��y */
  }
  else
  {
    /* itoc.020321: ���Y���ζǧڤ��y�A�ڤ]�i�H�Q�ʦ^ */
    BMW *bmw;

    for (ufo = bmw_locus - 1; ufo >= 0; ufo--)
    {
      bmw = &bmw_lslot[ufo];

      /* itoc.030718: �p�G�ڭ��s�W���F�A����Y�ϧڤW�@���W����������y�A���]���i�H�^��
         ���L�o�ˬd�٬O���Ӻ|�}�A�N�O�p�G���s�W���H��S��n���P�@�� ushm ����m�A�������٬O�i�H�^�� */
      if (bmw->caller == up && bmw->sender == up->userno)
	return 1;
    }
  }

  return 0;
}


int			/* 1:�i�ݨ� 0:���i�ݨ� */
can_see(my, up)
  UTMP *my;
  UTMP *up;
{
  usint mylevel, myufo, urufo;

  if (my == cutmp)	/* �� cuser. �ӥN�� cutmp-> */
  {
    mylevel = cuser.userlevel;
    myufo = cuser.ufo;
  }
  else
  {
    mylevel = my->userlevel;
    myufo = my->ufo;
  }
  urufo = up->ufo;

  if ((urufo & UFO_CLOAK) && !(mylevel & PERM_SEECLOAK))
    return 0;

#ifdef HAVE_SUPERCLOAK
  if ((urufo & UFO_SUPERCLOAK) && !(myufo & UFO_SUPERCLOAK))
    return 0;
#endif

#ifdef HAVE_BADPAL
  if (my == cutmp)	/* �ˬd�ڥi���i�H�ݨ��� */
  {
    if (!(mylevel & PERM_SEECLOAK) && is_obad(up))
      return 0;
  }
  else			/* �ˬd���i���i�H�ݨ�� */
  {
    if (!(mylevel & PERM_SEECLOAK) && is_mybad(my->userno))
      return 0;
  }
#endif

  return 1;
}


int
bmw_send(callee, bmw)
  UTMP *callee;
  BMW *bmw;
{
  BMW *mpool, *mhead, *mtail, **mslot;
  int i;
  pid_t pid;
  time_t texpire;

  if ((callee->userno != bmw->recver) || (pid = callee->pid) <= 0)
    return 1;

  /* sem_lock(BSEM_ENTER); */

  /* find callee's available slot */

  mslot = callee->mslot;
  i = 0;

  for (;;)
  {
    if (mslot[i] == NULL)
      break;

    if (++i >= BMW_PER_USER)
    {
      /* sem_lock(BSEM_LEAVE); */
      return 1;
    }
  }

  /* find available BMW slot in pool */

  texpire = time(&bmw->btime) - BMW_EXPIRE;

  mpool = ushm->mpool;
  mhead = ushm->mbase;
  if (mhead < mpool)
    mhead = mpool;
  mtail = mpool + BMW_MAX;

  do
  {
    if (++mhead >= mtail)
      mhead = mpool;
  } while (mhead->btime > texpire);

  *mhead = *bmw;
  ushm->mbase = mslot[i] = mhead;
  /* Thor.981206: �ݪ`�N, �Yushm mapping���P, 
                  �h���P�� bbsd ��call�|core dump,
                  ���D�o�]��offset, ���L���F -i, ���ӬO�D���n */


  /* sem_lock(BSEM_LEAVE); */
  return kill(pid, SIGUSR2);
}


#ifdef BMW_DISPLAY		
static void
bmw_display(max)	/* itoc.010313: display �H�e�����y */
  int max;
{
  int i;
  BMW *bmw;

  move(1, 0);
  clrtoeol();
  outs("\033[1;36m�~�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w\033[37;44m [Ctrl-T]���W���� \033[36;40m�w�w�w�w�w�w��\033[m");

  i = 2;
  for (; max >= 0; max--)
  {	/* �q���s�����y���U�L */
    bmw = &bmw_lslot[max];
    move(i, 0);
    clrtoeol();
    prints("  " BMW_FORMAT, bmw->userid, bmw->msg);
    i++;
  }

  move(i, 0);
  clrtoeol();
  outs("\033[1;36m���w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w\033[37;44m [Ctrl-R]���U���� \033[36;40m�w�w�w�w�w�w��\033[m");
}
#endif


static int bmw_pos;	/* �ثe���V bmw_lslot �����@�� */
static UTMP *bmw_up;	/* �ثe�^���� utmp */
static int bmw_request;	/* 1: ���s�����y�i�� */


void
bmw_edit(up, hint, bmw)
  UTMP *up;		/* �e����H�A�Y�O NULL ��ܼs�� */
  char *hint;
  BMW *bmw;
{
  int recver;
  screenline slp[3];
  char *userid, fpath[64];
  FILE *fp;

  if (bbsmode != M_BMW_REPLY)	/* �Y�O reply ���ܡA�b bmw_reply() �|�ۦ�B�z�e����ø */
    save_foot(slp);

  recver = up ? up->userno : 0;
  bmw->msg[0] = '\0';

  for (;;)
  {
    int ch;
    BMW *benz;

    ch = vget(0, 0, hint, bmw->msg, 62, GCARRY);

    if (!ch)		/* �S��J�F�� */
    {
      if (bbsmode != M_BMW_REPLY)
	restore_foot(slp, 1);
      return;
    }

    if (ch != Ctrl('R') && ch != Ctrl('T'))	/* �������y��J */
      break;

    /* ���s�����y�i�ӡA��ø���y�^�U�A�ñN bmw_pos ���V��Ө��Ӥ��y */
    if (bmw_request)
    {
      bmw_request = 0;
      bmw_pos = bmw_locus - 1;
      if (cuser.ufo & UFO_BMWDISPLAY)
	bmw_display(bmw_pos);
      bmw_reply_CtrlRT(ch);
      continue;
    }

    /* �b vget ���� ^R �� reply �O�����y */
    benz = &bmw_lslot[bmw_pos];
    if (benz->sender != up->userno)	/* reply ���P�H */
    {
      up = bmw_up;
      recver = up->userno;
      sprintf(hint, "��[%s]", up->userid);
    }
  }

  sprintf(fpath, "�T�w�n�e�X�m���y�n�� %s ��(Y/N)�H[Y] ", up ? up->userid : "�s��");
  if (vans(fpath) != 'n')
  {
    int i;

    bmw->caller = cutmp;
    bmw->sender = cuser.userno;
    userid = cuser.userid;

    if (up)	/* ���O�s�� */
    {
      /* �e�X���y */
      bmw->recver = recver;
      strcpy(bmw->userid, userid);
      if (bmw_send(up, bmw))	/* ���y�e���X�h�A���g�J���y������ */
      {
	vmsg(MSG_USR_LEFT);
	if (bbsmode != M_BMW_REPLY)
	  restore_foot(slp, 2);
	return;
      }

      /* lkchu.990103: �Y�O�ۤv�e�X�����y�A�s��誺 userid */
      strcpy(bmw->userid, up->userid);
    }
    else	/* �s�� */
    {
      /* �e�X�s�����{���A�b ulist_broadcast() �B�z */

      bmw->recver = 0;	/* �s 0 �Ϥ��� write �^�s�� */

      /* itoc.000213: �[ "> " ���F�P�@����y�Ϥ� */
      sprintf(bmw->userid, "%s> ", cuser.userid);
    }
      
    time(&bmw->btime);
    usr_fpath(fpath, userid, fn_bmw);
    rec_add(fpath, bmw, sizeof(BMW));

    /* itoc.020126: �[�J FN_AMW */
    usr_fpath(fpath, userid, fn_amw);
    if (fp = fopen(fpath, "a"))
    {
      fprintf(fp, BMW_FORMAT2 " %s\n", bmw->userid, bmw->msg, Btime(&bmw->btime));
      fclose(fp);
    }

    /* itoc.030621: �O�d�e�X�����y */
    if (bmw_locat >= BMW_LOCAL_MAX)
    {
      /* �ª����e�� */
      i = BMW_LOCAL_MAX - 1;
      memcpy(bmw_lword, bmw_lword + 1, i * sizeof(BMW));
    }
    else
    {
      i = bmw_locat;
      bmw_locat++;
    }
    bmw_lword[i].recver = recver;
    strcpy(bmw_lword[i].msg, bmw->msg);
  }

  if (bbsmode != M_BMW_REPLY)
    restore_foot(slp, 2);
}


static void
bmw_outz()
{
  int i;
  BMW *bmw, *benz;

  /* �C�L����m�n�M save/restore_foot �ҭ�ø�������O�ۦP�� */

  bmw = &bmw_lslot[bmw_pos];
  move(b_lines, 0);
  clrtoeol();
  prints(BMW_FORMAT, bmw->userid, bmw->msg);

  /* itoc.030621: �ѫO�d���e�X���y���A��X�W���^�o�H�����y�O���� */
  for (i = bmw_locat; i >= 0; i--)
  {
    benz = &bmw_lword[i];
    if (benz->recver == bmw->sender)
      break;
  }
  move(b_lines - 1, 0);
  clrtoeol();
  prints(BMW_FORMAT2, bmw->userid, i >= 0 ? benz->msg : "�i�z�̪�S���Ǥ��y���o��ϥΪ̡j");
}


static UTMP *
can_reply(uhead, pos)
  UTMP *uhead;
  int pos;
{
  int userno;
  BMW *bmw;
  UTMP *up;

  bmw = &bmw_lslot[pos];

  userno = bmw->sender;
  if (!userno)		/* Thor.980805: ����t�Ψ�M�^�� */
    return NULL;

  up = bmw->caller;
  if ((up < uhead) || (up > uhead + ushm->offset) || (up->userno != userno))
  {
    /* �p�G up-> ���b ushm ���A�άO up-> ���O call-in �ڪ��H�A��ܳo�H�U���F�A
       ���O�L�i��S�W���Φ� multi�A�ҥH����@�� */
    if (!(up = utmp_find(userno)))	/* �p�G�A��@���٬O�S�� */
      return NULL;
  }

  /* itoc.010909: �i�H�Q�ʦ^�� ����/��������/����pager ���H�A���O����^����w���H */
  if (bmw->caller != up || up->status & STATUS_REJECT)
    return NULL;

  return up;
}


static UTMP *
bmw_lastslot(pos)	/* ��X�̪�@�ӥi�H�^���y����H */
  int pos;
{
  int max, times;
  UTMP *up, *uhead;

  uhead = ushm->uslot;
  max = bmw_locus - 1;

  for (times = max; times >= 0; times--)
  {
    if (up = can_reply(uhead, pos))
    {
      bmw_pos = pos;
      return up;
    }

    /* ���U�`����@�� */
    pos = (pos == 0) ? max : pos - 1;
  }

  return NULL;
}


static UTMP *
bmw_firstslot(pos)	/* ��X�̻��@�ӥi�H�^���y����H */
  int pos;
{
  int max, times;
  UTMP *up, *uhead;

  uhead = ushm->uslot;
  max = bmw_locus - 1;

  for (times = max; times >= 0; times--)
  {
    if (up = can_reply(uhead, pos))
    {
      bmw_pos = pos;
      return up;
    }

    /* ���W�`����@�� */
    pos = (pos == max) ? 0 : pos + 1;
  }

  return NULL;
}


int
bmw_reply_CtrlRT(key)
  int key;
{
  int max, pos;

  max = bmw_locus - 1;
  if (max == 0)		/* �S��L�����y�i�H�� */
    return 0;

  pos = bmw_pos;	/* �ª� bmw_pos */

  if (key == Ctrl('R'))
    bmw_up = bmw_lastslot(pos == 0 ? max : pos - 1);	/* �ѥثe�Ҧb pos ���U��@�ӥi�H�^���y����H */
  else /* if (key == Ctrl('T')) */
    bmw_up = bmw_firstslot(pos == max ? 0 : pos + 1);	/* �ѥثe�Ҧb pos ���W��@�ӥi�H�^���y����H */

  if (!bmw_up)		/* �䤣��O�����y */
  {
    bmw_pos = pos;
    return 0;
  }

#ifdef BMW_DISPLAY
  if (cuser.ufo & UFO_BMWDISPLAY)
  {
    move(2 + max - pos, 0);
    outc(' ');
    move(2 + max - bmw_pos, 0);
    outc('>');
  }
#endif

  bmw_outz();
  return 1;
}


void
bmw_reply()
{
  int max, display, tmpmode;
  char buf[128];
  UTMP *up;
  BMW bmw;
#ifdef BMW_DISPLAY
  screenline slt[T_LINES];
#else
  screenline slt[3];
#endif

  cursor_save();

  max = bmw_locus - 1;
  if (!(up = bmw_lastslot(max)))
  {
    save_foot(slt);
    vmsg("���e�õL���y�I�s�A�ι��Ҥw�U��");
    restore_foot(slt, 2);
    cursor_restore();
    refresh();
    return;
  }

  tmpmode = bbsmode;	/* lkchu.981201: �x�s bbsmode */
  utmp_mode(M_BMW_REPLY);

#ifdef BMW_DISPLAY
  display = cuser.ufo & UFO_BMWDISPLAY;
  if (display)
  {
    vs_save(slt);	/* itoc.010313: �O�� bmd_display ���e�� screen */
    bmw_display(max);	/* itoc.010313: display �H�e�����y */
    move(2 + max - bmw_pos, 0);
    outc('>');
    bmw_request = 0;
  }
  else
#endif
    save_foot(slt);

  bmw_outz();

  sprintf(buf, "��[%s]", up->userid);
  bmw_edit(up, buf, &bmw);

#ifdef BMW_DISPLAY
  if (display)
  {
    cursor_restore();
    vs_restore(slt);	/* itoc.010313: �٭� bmw_display ���e�� screen */
  }
  else  
#endif
  {
    restore_foot(slt, 3);	/* �w bmw_outz�A�n�٭�T�C */
    cursor_restore();
    refresh();
  }

  utmp_mode(tmpmode);	/* lkchu.981201: �^�_ bbsmode */
}


void
bmw_rqst()
{
  int i, j, userno, locus;
  BMW bmw[BMW_PER_USER], *mptr, **mslot;

  /* download BMW slot first */

  i = j = 0;
  userno = cuser.userno;
  mslot = cutmp->mslot;

  while (mptr = mslot[i])
  {
    mslot[i] = NULL;
    if (mptr->recver == userno)
    {
      bmw[j++] = *mptr;
    }
    mptr->btime = 0;

    if (++i >= BMW_PER_USER)
      break;
  }

  /* process the request */

  if (j)
  {
    char buf[128];
    FILE *fp;

    locus = bmw_locus;
    i = locus + j - BMW_LOCAL_MAX;
    if (i >= 0)
    {
      locus -= i;
      memcpy(bmw_lslot, bmw_lslot + i, locus * sizeof(BMW));
    }

    /* itoc.020126: �[�J FN_AMW */
    usr_fpath(buf, cuser.userid, fn_amw);
    fp = fopen(buf, "a");

    i = 0;
    do
    {
      mptr = &bmw[i];

      /* lkchu.981230: �Q�� xover ��X bmw */
      usr_fpath(buf, cuser.userid, fn_bmw);
      rec_add(buf, mptr, sizeof(BMW));

      /* itoc.020126: �[�J FN_AMW */
      fprintf(fp, BMW_FORMAT " %s\n", mptr->userid, mptr->msg, Btime(&mptr->btime));

      bmw_lslot[locus++] = *mptr;	/* structure copy */
    } while (++i < j);

    fclose(fp);

    bmw_locus = locus;
    if (bbsmode == M_BMW_REPLY)
      bmw_request = 1;		/* �n�D��s */

    /* Thor.980827: ���F����C�L�@�b(more)�ɤ��y�ӫ�C�L�W�L�d���H, �G�s�U��Ц�m */
    cursor_save(); 

    sprintf(buf, BMW_FORMAT, mptr->userid, mptr->msg);
    outz(buf);

    /* Thor.980827: ���F����C�L�@�b(more)�ɤ��y�ӫ�C�L�W�L�d���H, �G�٭��Ц�m */
    cursor_restore();

    refresh();
    bell();

#ifdef BMW_COUNT
    /* itoc.010312: �h���@�Ӥ��y */
    cutmp->bmw_count++;
#endif
  }
}


void
do_write(up)
  UTMP *up;
{
  if (can_override(up))
  {
    BMW bmw;
    char buf[20];

    sprintf(buf, "��[%s]", up->userid);
    bmw_edit(up, buf, &bmw);
  }
}


/* ----------------------------------------------------- */
/* ���y�C��: ��榡�ާ@�ɭ��y�z by lkchu		 */
/* ----------------------------------------------------- */


static void
bmw_item(num, bmw)
  int num;
  BMW *bmw;
{
  struct tm *ptime = localtime(&bmw->btime);

  if (bmw->sender == cuser.userno)	/* �e�X�����y */
  {
    prints("%6d%c\033[33m%-13s\033[36m%-*.*s\033[33m%02d:%02d\033[m\n",
      num, tag_char(bmw->btime), bmw->userid, d_cols + 53, d_cols + 53, bmw->msg, ptime->tm_hour, ptime->tm_min);
  }
  else					/* ���쪺���y */
  {
    prints("%6d%c%-13s\033[32m%-*.*s\033[m%02d:%02d\n",
      num, tag_char(bmw->btime), bmw->userid, d_cols + 53, d_cols + 53, bmw->msg, ptime->tm_hour, ptime->tm_min);
  }
}


static int
bmw_body(xo)
  XO *xo;
{
  BMW *bmw;
  int num, max, tail;

  max = xo->max;
  if (max <= 0)
  {
    vmsg("���e�õL���y�I�s");
    return XO_QUIT;
  }

  bmw = (BMW *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  move(3, 0);
  do
  {
    bmw_item(++num, bmw++);
  } while (num < max);
  clrtobot();

  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: �� b_lines ��W feeter */
}


static int
bmw_head(xo)
  XO *xo;
{
  vs_head("��ݤ��y", str_site);
  prints(NECKER_BMW, d_cols, "");
  return bmw_body(xo);
}


static int
bmw_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(BMW));
  return bmw_body(xo);
}


static int
bmw_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(BMW));
  return bmw_head(xo);
}


static int
bmw_delete(xo)
  XO *xo;
{
  if (vans(msg_del_ny) == 'y')
  {
    if (!rec_del(xo->dir, sizeof(BMW), xo->pos, NULL))
      return bmw_load(xo);
  }

  return XO_FOOT;
}


static int
bmw_rangedel(xo)	/* itoc.001126: �s�W���y�Ϭq�R�� */
  XO *xo;
{
  return xo_rangedel(xo, sizeof(BMW), NULL, NULL);
}


static int
vfybmw(bmw, pos)
  BMW *bmw;
  int pos;
{
  return Tagger(bmw->btime, pos, TAG_NIN);
}


static int
bmw_prune(xo)
  XO *xo;
{
  return xo_prune(xo, sizeof(BMW), vfybmw, NULL);
}


static int
bmw_mail(xo)
  XO *xo;
{
  BMW *bmw;
  char *str, userid[IDLEN + 1];

  bmw = (BMW *) xo_pool + (xo->pos - xo->top);
  strcpy(userid, bmw->userid);
  if (str = strchr(userid, '>'))	/* �s�� */
    *str = '\0';
  return my_send(userid);
}


static int
bmw_query(xo)
  XO *xo;
{
  BMW *bmw;
  char *str, userid[IDLEN + 1];

  bmw = (BMW *) xo_pool + (xo->pos - xo->top);
  move(1, 0);
  clrtobot();
  strcpy(userid, bmw->userid);
  if (str = strchr(userid, '>'))	/* �s�� */
    *str = '\0';
  my_query(userid);
  return bmw_head(xo);
}


static int
bmw_write(xo)
  XO *xo;
{
  if (HAS_PERM(PERM_PAGE))
  {
    int userno;
    UTMP *up;
    BMW *bmw;

    bmw = (BMW *) xo_pool + (xo->pos - xo->top);

    /* itoc.010304: ���ǰT�� bmw �]�i�H�^ */
    /* �ڰe���y���O�H�A�^�����T�̡F�O�H�e���y���ڡA�^���e�T�� */
    userno = (bmw->sender == cuser.userno) ? bmw->recver : bmw->sender;
    if (!userno)
      return XO_NONE;

    if (up = utmp_find(userno))
      do_write(up);
  }
  return XO_NONE;
}


static void
bmw_store(fpath)
  char *fpath;
{
  int fd;
  FILE *fp;
  char buf[64], folder[64];
  HDR fhdr;

  /* itoc.020126.����: �i�H������ FN_AMW ���x�s�Y�i�A
     �i�O�p�G�� FN_BMW �����@�����ܡA�i�H���ϥΪ̦b t_bmw() ���ۥ� d �����n�����y */

  if ((fd = open(fpath, O_RDONLY)) < 0)
    return;

  usr_fpath(folder, cuser.userid, fn_dir);
  if (fp = fdopen(hdr_stamp(folder, 0, &fhdr, buf), "w"))
  {
    BMW bmw;

    fprintf(fp, "              == ���y�O�� %s ==\n\n", Now());

    while (read(fd, &bmw, sizeof(BMW)) == sizeof(BMW)) 
    {
      fprintf(fp, bmw.sender == cuser.userno ? BMW_FORMAT2 " %s\n" : BMW_FORMAT " %s\n",
	bmw.userid, bmw.msg, Btime(&bmw.btime));
    }
    fclose(fp);
  }

  close(fd);

  fhdr.xmode = MAIL_READ | MAIL_NOREPLY;
  strcpy(fhdr.title, "[�� �� ��] ���y����");
  strcpy(fhdr.owner, cuser.userid);
  rec_add(folder, &fhdr, sizeof(HDR));
}


static int
bmw_save(xo)
  XO *xo;
{
  if (vans("�z�T�w�n����y�s��H�c�̶�(Y/N)�H[N] ") == 'y')
  {
    char fpath[64];

    usr_fpath(fpath, cuser.userid, fn_bmw);
    bmw_store(fpath);
    unlink(fpath);
    usr_fpath(fpath, cuser.userid, fn_amw);
    unlink(fpath);
    return bmw_init(xo);
  }
  return XO_FOOT;
}


static int
bmw_save_user(xo)
  XO *xo;
{
  int fd;
  FILE *fp;
  char buf[64], folder[64];
  HDR fhdr;
  ACCT acct;

  if (acct_get(msg_uid, &acct) > 0 && acct.userno != cuser.userno)
  {
    usr_fpath(buf, cuser.userid, fn_bmw);
    if ((fd = open(buf, O_RDONLY)) >= 0)
    {
      usr_fpath(folder, cuser.userid, fn_dir);
      if (fp = fdopen(hdr_stamp(folder, 0, &fhdr, buf), "w"))
      {
	BMW bmw;

	fprintf(fp, "       == �P %s �᪺���y���� %s ==\n\n", acct.userid, Now());

	while (read(fd, &bmw, sizeof(BMW)) == sizeof(BMW)) 
	{
	  if (bmw.sender == acct.userno || bmw.recver == acct.userno)
	  {
	    fprintf(fp, bmw.sender == cuser.userno ? BMW_FORMAT2 " %s\n" : BMW_FORMAT " %s\n",
	      bmw.userid, bmw.msg, Btime(&bmw.btime));
	  }
	}
	fclose(fp);
      }
      close(fd);

      fhdr.xmode = MAIL_READ | MAIL_NOREPLY;
      strcpy(fhdr.title, "[�� �� ��] ���y����");
      strcpy(fhdr.owner, cuser.userid);
      rec_add(folder, &fhdr, sizeof(HDR));
      vmsg("���y�����w�H��H�c");
    }
  }

  return bmw_head(xo);
}


static int
bmw_clear(xo)
  XO *xo;
{
  if (vans("�O�_�R���Ҧ����y����(Y/N)�H[N] ") == 'y')
  {
    char fpath[64];

    usr_fpath(fpath, cuser.userid, fn_bmw);
    unlink(fpath);
    usr_fpath(fpath, cuser.userid, fn_amw);
    unlink(fpath);
    return XO_QUIT;
  }

  return XO_FOOT;
}


static int
bmw_tag(xo)
  XO *xo;
{
  BMW *bmw;
  int tag, pos, cur;

  pos = xo->pos;
  cur = pos - xo->top;
  bmw = (BMW *) xo_pool + cur;

  if (tag = Tagger(bmw->btime, pos, TAG_TOGGLE))
  {
    move(3 + cur, 6);
    outc(tag > 0 ? '*' : ' ');
  }

  /* return XO_NONE; */
  return xo->pos + 1 + XO_MOVE;	/* lkchu.981201: ���ܤU�@�� */
}


static int
bmw_help(xo)
  XO *xo;
{
  xo_help("bmw");
  return bmw_head(xo);
}


KeyFunc bmw_cb[] =
{
  XO_INIT, bmw_init,
  XO_LOAD, bmw_load,
  XO_HEAD, bmw_head,
  XO_BODY, bmw_body,
  
  'd', bmw_delete,
  'D', bmw_rangedel,
  'm', bmw_mail,
  'w', bmw_write,
  'r', bmw_query,
  Ctrl('Q'), bmw_query,
  's', bmw_init,
  'M', bmw_save,
  'u', bmw_save_user,
  't', bmw_tag,
  Ctrl('D'), bmw_prune,
  'C', bmw_clear,
  
  'h', bmw_help
};


int
t_bmw()
{
#if 0	/* itoc.010715: �ѩ� every_Z �n�ΡA�h�h talk_main �`�n */
  XO *xo;
  char fpath[64];

  usr_fpath(fpath, cuser.userid, fn_bmw);
  xz[XZ_BMW - XO_ZONE].xo = xo = xo_new(fpath);
  xover(XZ_BMW);
  free(xo);
#endif

  xover(XZ_BMW);
  return 0;
}


int
t_display()		/* itoc.020126: display FN_AMW */
{
  char fpath[64];

  usr_fpath(fpath, cuser.userid, fn_amw);
  return more(fpath, NULL);	/* Thor.990204: �u�n���O XEASY �N�i�H reload menu �F */
}


#ifdef RETAIN_BMW
static void
bmw_retain(fpath)
  char *fpath;
{
  char folder[64];
  HDR fhdr;

  usr_fpath(folder, str_sysop, fn_dir);
  hdr_stamp(folder, HDR_COPY, &fhdr, fpath);
  strcpy(fhdr.owner, cuser.userid);
  strcpy(fhdr.title, "���y�s��");
  fhdr.xmode = 0;
  rec_add(folder, &fhdr, sizeof(HDR));
}
#endif


#ifdef LOG_BMW
void
bmw_log()
{
  int op;
  char fpath[64], buf[64];
  struct stat st;

  /* lkchu.981201: ��i�p�H�H�c��/�M��/�O�d */  
  usr_fpath(fpath, cuser.userid, fn_bmw);

  if (!stat(fpath, &st) && S_ISREG(st.st_mode))
  {
    usr_fpath(buf, cuser.userid, fn_amw);

    if ((cuser.ufo & UFO_NWLOG) || !st.st_size)	/* itoc.000512: ���x�s���y�O�� */
    {						/* itoc.020711: �p�G bmw size �O 0 �N�M�� */
      op = 'c';
    }
    else
    {
      more(buf, (char *) -1);
#ifdef RETAIN_BMW
      op = vans("�����W�����y�B�z (M)���ܳƧѿ� (R)�O�d (C)�M�� (S)�s�ҡH[R] ");
#else
      op = vans("�����W�����y�B�z (M)���ܳƧѿ� (R)�O�d (C)�M���H[R] ");
#endif
    }
      
    switch (op)
    {
    case 'm':
      bmw_store(fpath);

    case 'c':
      unlink(fpath);
      unlink(buf);
      break;

#ifdef RETAIN_BMW
    case 's':
      if (vans("�s�ҬO����y��H�������H���|��L�ϥΪ̡A�z�T�w�n�s�Ҷ�(Y/N)�H[N] ") == 'y')
	bmw_retain(buf);	/* �Τ���諸 amw �Ӧs�� */
      break;
#endif

    default:
      break;
    }
  }
}
#endif
