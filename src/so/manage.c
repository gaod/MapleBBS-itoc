/*-------------------------------------------------------*/
/* manage.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : �ݪO�޲z				 	 */
/* create : 95/03/29				 	 */
/* update : 96/04/05				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern BCACHE *bshm;


#ifdef HAVE_TERMINATOR
/* ----------------------------------------------------- */
/* �����\�� : �ط�������				 */
/* ----------------------------------------------------- */


extern char xo_pool[];


#define MSG_TERMINATOR	"�m�ط������١n"

int
post_terminator(xo)		/* Thor.980521: �׷��峹�R���j�k */
  XO *xo;
{
  int mode, type;
  HDR *hdr;
  char keyOwner[80], keyTitle[TTLEN + 1], buf[80];

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_FOOT;

  mode = vans(MSG_TERMINATOR "�R�� (1)����@�� (2)������D (3)�۩w�H[Q] ") - '0';

  if (mode == 1)
  {
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    strcpy(keyOwner, hdr->owner);
  }
  else if (mode == 2)
  {
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    strcpy(keyTitle, str_ttl(hdr->title));		/* ���� Re: */
  }
  else if (mode == 3)
  {
    if (!vget(b_lines, 0, "�@�̡G", keyOwner, 73, DOECHO))
      mode ^= 1;
    if (!vget(b_lines, 0, "���D�G", keyTitle, TTLEN + 1, DOECHO))
      mode ^= 2;
  }
  else
  {
    return XO_FOOT;
  }

  type = vans(MSG_TERMINATOR "�R�� (1)��H�O (2)�D��H�O (3)�Ҧ��ݪO�H[Q] ");
  if (type < '1' || type > '3')
    return XO_FOOT;

  sprintf(buf, "�R��%s�G%.35s ��%s�O�A�T�w��(Y/N)�H[N] ", 
    mode == 1 ? "�@��" : mode == 2 ? "���D" : "����", 
    mode == 1 ? keyOwner : mode == 2 ? keyTitle : "�۩w", 
    type == '1' ? "��H" : type == '2' ? "�D��H" : "�Ҧ���");

  if (vans(buf) == 'y')
  {
    BRD *bhdr, *head, *tail;
    char tmpboard[BNLEN + 1];

    /* Thor.980616: �O�U currboard�A�H�K�_�� */
    strcpy(tmpboard, currboard);

    head = bhdr = bshm->bcache;
    tail = bhdr + bshm->number;
    do				/* �ܤ֦� note �@�O */
    {
      int fdr, fsize, xmode;
      FILE *fpw;
      char fpath[64], fnew[64], fold[64];
      HDR *hdr;

      xmode = head->battr;
      if ((type == '1' && (xmode & BRD_NOTRAN)) || (type == '2' && !(xmode & BRD_NOTRAN)))
	continue;

      /* Thor.980616: ��� currboard�A�H cancel post */
      strcpy(currboard, head->brdname);

      sprintf(buf, MSG_TERMINATOR "�ݪO�G%s \033[5m...\033[m", currboard);
      outz(buf);
      refresh();

      brd_fpath(fpath, currboard, fn_dir);

      if ((fdr = open(fpath, O_RDONLY)) < 0)
	continue;

      if (!(fpw = f_new(fpath, fnew)))
      {
	close(fdr);
	continue;
      }

      fsize = 0;
      mgets(-1);
      while (hdr = mread(fdr, sizeof(HDR)))
      {
	xmode = hdr->xmode;

	if ((xmode & POST_MARKED) || 
	  ((mode & 1) && strcmp(keyOwner, hdr->owner)) ||
	  ((mode & 2) && strcmp(keyTitle, str_ttl(hdr->title))))
	{
	  if ((fwrite(hdr, sizeof(HDR), 1, fpw) != 1))
	  {
	    fclose(fpw);
	    close(fdr);
	    goto contWhileOuter;
	  }
	  fsize++;
	}
	else
	{
	  /* �Y���ݪO�N�s�u��H */

	  cancel_post(hdr);
	  hdr_fpath(fold, fpath, hdr);
	  unlink(fold);
	}
      }
      close(fdr);
      fclose(fpw);

      sprintf(fold, "%s.o", fpath);
      rename(fpath, fold);
      if (fsize)
	rename(fnew, fpath);
      else
  contWhileOuter:
	unlink(fnew);
    } while (++head < tail);

    /* �٭� currboard */
    strcpy(currboard, tmpboard);
    return XO_LOAD;
  }

  return XO_FOOT;
}
#endif	/* HAVE_TERMINATOR */


/* ----------------------------------------------------- */
/* �O�D�\�� : �ק�O�W					 */
/* ----------------------------------------------------- */


static int
post_brdtitle(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  /* itoc.����: ���I�s brd_title(bno) �N�i�H�F�A�S�t�A�Z�F�@�U�n�F :p */
  if (vans("�O�_�ק襤��O�W�ԭz(Y/N)�H[N] ") == 'y')
  {
    vget(b_lines, 0, "�ݪO�D�D�G", newbrd.title, BTLEN + 1, GCARRY);

    if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
    {
      memcpy(oldbrd, &newbrd, sizeof(BRD));
      rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
      return XO_HEAD;	/* itoc.011125: �n��ø����O�W */
    }
  }

  return XO_FOOT;
}


/* ----------------------------------------------------- */
/* �O�D�\�� : �ק�i�O�e��				 */
/* ----------------------------------------------------- */


static int
post_memo_edit(xo)
  XO *xo;
{
  int mode;
  char fpath[64];

  mode = vans("�i�O�e�� (D)�R�� (E)�ק� (Q)�����H[E] ");

  if (mode != 'q')
  {
    brd_fpath(fpath, currboard, fn_note);

    if (mode == 'd')
    {
      unlink(fpath);
      return XO_FOOT;
    }

    if (vedit(fpath, 0))	/* Thor.981020: �`�N�Qtalk�����D */
      vmsg(msg_cancel);
    return XO_HEAD;
  }
  return XO_FOOT;
}


/* ----------------------------------------------------- */
/* �O�D�\�� : �ݪO�ݩ�					 */
/* ----------------------------------------------------- */


#ifdef HAVE_SCORE
static int
post_battr_noscore(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  switch (vans("�}����� (1)���\\ (2)���\\ (Q)�����H[Q] "))
  {
  case '1':
    newbrd.battr &= ~BRD_NOSCORE;
    break;
  case '2':
    newbrd.battr |= BRD_NOSCORE;
    break;
  default:
    return XO_FOOT;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_FOOT;
}
#endif	/* HAVE_SCORE */


/* ----------------------------------------------------- */
/* �O�D�\�� : �ק�O�D�W��				 */
/* ----------------------------------------------------- */


static int
post_changeBM(xo)
  XO *xo;
{
  char buf[80], uid[IDLEN + 1], *blist;
  BRD *oldbrd, newbrd;
  ACCT acct;
  int dirty, len;

  oldbrd = bshm->bcache + currbno;

  blist = oldbrd->BM;
  if (is_bm(blist, cuser.userid) != 1)	/* �u�����O�D�i�H�]�w�O�D�W�� */
    return XO_FOOT;

  memcpy(&newbrd, oldbrd, sizeof(BRD));

  move(3, 0);
  clrtobot();

  move(8, 0);
  prints("�ثe�O�D�� %s\n�п�J�s���O�D�W��A�Ϋ� [Return] ����", oldbrd->BM);

  strcpy(buf, oldbrd->BM);
  dirty = strlen(buf);

  while (vget(10, 0, "�п�J�ƪO�D�A�����Ы� Enter�A�M���Ҧ��ƪO�D�Х��u�L�v�G", uid, IDLEN + 1, DOECHO))
  {
    if (!strcmp(uid, "�L"))
    {
      strcpy(buf, cuser.userid);
      dirty = strlen(buf);
    }
    else if (acct_load(&acct, uid) >= 0 && !is_bm(buf, acct.userid))   /* ��J�s�O�D */
    {
      len = strlen(acct.userid) + 1;  /* '/' + userid */
      if (dirty + len > BMLEN)
      {
	vmsg("�O�D�W��L���A�L�k�N�o ID �]���O�D");
	continue;
      }
      sprintf(buf + dirty, "/%s", acct.userid);
      dirty += len;

      acct_setperm(&acct, PERM_BM, 0);
    }
    move(8, 0);
    prints("�ثe�O�D�� %s", buf);
    clrtoeol();
  }
  strcpy(newbrd.BM, buf);

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);

    sprintf(currBM, "�O�D�G%s", newbrd.BM);
    return XO_HEAD;	/* �n��ø���Y���O�D */
  }

  return XO_BODY;
}


#ifdef HAVE_MODERATED_BOARD
/* ----------------------------------------------------- */
/* �O�D�\�� : �ݪO�v��					 */
/* ----------------------------------------------------- */


static int
post_brdlevel(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  switch (vans("1)���}�ݪO 2)���K�ݪO 3)�n�ͬݪO�H[Q] "))
  {
  case '1':				/* ���}�ݪO */
    newbrd.readlevel = 0;
    newbrd.postlevel = PERM_POST;
    newbrd.battr &= ~(BRD_NOSTAT | BRD_NOVOTE);
    break;

  case '2':				/* ���K�ݪO */
    newbrd.readlevel = PERM_SYSOP;
    newbrd.postlevel = 0;
    newbrd.battr |= (BRD_NOSTAT | BRD_NOVOTE);
    break;

  case '3':				/* �n�ͬݪO */
    newbrd.readlevel = PERM_BOARD;
    newbrd.postlevel = 0;
    newbrd.battr |= (BRD_NOSTAT | BRD_NOVOTE);
    break;

  default:
    return XO_FOOT;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_FOOT;
}
#endif	/* HAVE_MODERATED_BOARD */


#ifdef HAVE_MODERATED_BOARD
/* ----------------------------------------------------- */
/* �O�ͦW��Gmoderated board				 */
/* ----------------------------------------------------- */


static void
bpal_cache(fpath)
  char *fpath;
{
  BPAL *bpal;

  bpal = bshm->pcache + currbno;
  bpal->pal_max = image_pal(fpath, bpal->pal_spool);
}


extern XZ xz[];


static int
XoBM(xo)
  XO *xo;
{
  XO *xt;
  char fpath[64];

  brd_fpath(fpath, currboard, fn_pal);
  xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath);
  xt->key = PALTYPE_BPAL;
  xover(XZ_PAL);		/* Thor: �ixover�e, pal_xo �@�w�n ready */

  /* build userno image to speed up, maybe upgreade to shm */

  bpal_cache(fpath);

  free(xt);

  return XO_INIT;
}
#endif	/* HAVE_MODERATED_BOARD */


/* ----------------------------------------------------- */
/* �O�D���						 */
/* ----------------------------------------------------- */


int
post_manage(xo)
  XO *xo;
{
#ifdef POPUP_ANSWER
  char *menu[] = 
  {
    "BQ",
    "BTitle  �ק�ݪO�D�D",
    "WMemo   �s��i�O�e��",
    "Manager �W��ƪO�D",
#  ifdef HAVE_SCORE
    "Score   �]�w�i�_����",
#  endif
#  ifdef HAVE_MODERATED_BOARD
    "Level   ���}/�n��/���K",
    "OPal    �O�ͦW��",
#  endif
    NULL
  };
#else
  char *menu = "�� �O�D��� (B)�D�D (W)�i�O (M)�ƪO"
#  ifdef HAVE_SCORE
    " (S)����"
#  endif
#  ifdef HAVE_MODERATED_BOARD
    " (L)�v�� (O)�O��"
#  endif
    "�H[Q] ";
#endif

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

#ifdef POPUP_ANSWER
  switch (pans(3, 20, "�O�D���", menu))
#else
  switch (vans(menu))
#endif
  {
  case 'b':
    return post_brdtitle(xo);

  case 'w':
    return post_memo_edit(xo);

  case 'm':
    return post_changeBM(xo);

#ifdef HAVE_SCORE
  case 's':
    return post_battr_noscore(xo);
#endif

#ifdef HAVE_MODERATED_BOARD
  case 'l':
    return post_brdlevel(xo);

  case 'o':
    return XoBM(xo);
#endif
  }

  return XO_FOOT;
}
