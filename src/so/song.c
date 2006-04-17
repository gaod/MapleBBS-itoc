/*-------------------------------------------------------*/
/* song.c	( YZU_CSE WindTop BBS )			 */
/*-------------------------------------------------------*/
/* target : song ordering routines			 */
/* create :   /  /  					 */
/* update : 01/12/18					 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_SONG

extern BCACHE *bshm;
extern XZ xz[];
extern char xo_pool[];
extern char brd_bits[];


static void XoSong();


#define	SONG_SRC	"<~Src~>"
#define SONG_DES	"<~Des~>"
#define SONG_SAY	"<~Say~>"
#define SONG_END	"<~End~>"


#ifdef LOG_SONG_USIES
static int			/* -1:�S���  >=0:pos */
song_usies_find(fpath, chrono, songdata)
  char *fpath;
  time_t chrono;
  SONGDATA *songdata;
{
  int fd, pos;
  int rc = -1;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    pos = 0;
    while (read(fd, songdata, sizeof(SONGDATA)) == sizeof(SONGDATA))
    {
      if (songdata->chrono == chrono)
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


static void
song_usies_log(chrono, title)
  time_t chrono;
  char *title;
{
  SONGDATA songdata;
  int pos;
  char *dir;

  dir = FN_RUN_SONGUSIES;

  if ((pos = song_usies_find(dir, chrono, &songdata)) >= 0)
  {
    songdata.count++;
    rec_put(dir, &songdata, sizeof(SONGDATA), pos, NULL);
  }
  else
  {
    songdata.chrono = chrono;
    songdata.count = 1;
    strcpy(songdata.title, title);
    rec_add(dir, &songdata, sizeof(SONGDATA));
  }
}
#endif


static int swaped;		/* ����O�_�� <~Src~> �� */

static int
song_swap(str, src, des)
  char *str;
  char *src;
  char *des;
{
  char *ptr;
  char buf[ANSILINELEN];

  if (ptr = strstr(str, src))
  {
    *ptr = '\0';
    ptr += strlen(src);
    /* sprintf(buf, "%s%s%s", str, des, ptr); */
    snprintf(buf, ANSILINELEN, "%s%s%s", str, des, ptr);	/* swap �i��|�W�L ANSILINELEN */
    strcpy(str, buf);

    /* return 1; */
    return ++swaped;
  }

  return 0;
}


static void
song_quote(fpr, fpw, src, des, say)	/* �q fpr Ū�X����A�� <~Src~> ���������A�g�J fpw */
  FILE *fpr, *fpw;
  char *src, *des, *say;
{
  char buf[ANSILINELEN];

  swaped = 0;
  while (fgets(buf, sizeof(buf), fpr))
  {
    if (strstr(buf, SONG_END))
      break;

    while (song_swap(buf, SONG_SRC, src));
    while (song_swap(buf, SONG_DES, des));
    while (song_swap(buf, SONG_SAY, say));
    fputs(buf, fpw);
  }

  if (!swaped)	/* itoc.011115: �p�G����S�� <~Src~>�A�h�b�̫�[�W */
  {
    /* �b�̫�@��[�W <~Src~> �Q�� <~Des~> �� <~Say~> */
    strcpy(buf, "\033[1;33m" SONG_SRC " �Q�� " SONG_DES " �� " SONG_SAY "\033[m\n");
    song_swap(buf, SONG_SRC, src);
    song_swap(buf, SONG_DES, des);
    song_swap(buf, SONG_SAY, say);
    fputs(buf, fpw);
  }

  /* �b�ɮ׳̫�[�W�I�q�ɶ� */
  fprintf(fpw, "\n--\n%s\n", Now());
}


#define GEM_FILE	0x01	/* �w���O�ɮ� */


static HDR *		/* NULL:�L�vŪ�� */
song_check(xo, fpath, op)
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

  if ((op & GEM_FILE) && (gtype & GEM_FOLDER))
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


static void
song_item(num, hdr, level)
  int num;
  HDR *hdr;
  int level;
{
  int xmode, gtype;

  xmode = hdr->xmode;
  gtype = (char) 0xba;

  /* �ؿ��ι�ߡA���O�ؿ��ΪŤ� */
  if (xmode & GEM_FOLDER)		/* �峹:�� ���v:�� */
    gtype += 1;

  if (hdr->xname[0] == '@')		/* ���:�� ����:�� */
    gtype -= 2;
  else if (xmode & GEM_BOARD)		/*         �ݪO:�� */
    gtype += 2;

  prints("%6d%c \241%c ", num, xmode & GEM_RESTRICT ? ')' : ' ', gtype);

  if ((xmode & GEM_RESTRICT) && !(level & GEM_M_BIT))
    outs(MSG_DATA_CLOAK);
  else
    prints("%.*s\n", d_cols + 64, hdr->title);
}


static int
song_body(xo)
  XO *xo;
{
  HDR *hdr;
  int num, max, tail;

  max = xo->max;
  if (max <= 0)
  {
    outs("\n\n�m�q���n�|�b�l���Ѧa��������� :)");
    vmsg(NULL);
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
    song_item(++num, hdr++, tail);
  } while (num < max);
  clrtobot();

  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: �� b_lines ��W feeter */  
}


static int
song_head(xo)
  XO *xo;
{
  vs_head("��ؤ峹", xo->xyz);
  prints(NECKER_SONG, d_cols, "");
  return song_body(xo);
}


static int
song_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(HDR));
  return song_head(xo);
}


static int
song_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(HDR));
  return song_body(xo);
}


/* ----------------------------------------------------- */
/* ��Ƥ��s�W�Gappend / insert				 */
/* ----------------------------------------------------- */


static int
song_browse(xo)
  XO *xo;
{
  HDR *hdr;
  int op, xmode;
  char fpath[64], title[TTLEN + 1];

  op = 0;

  for (;;)
  {
    if (!(hdr = song_check(xo, fpath, op)))
      break;

    xmode = hdr->xmode;

    /* browse folder */

    if (xmode & GEM_FOLDER)
    {
      if (xmode & GEM_BOARD)
      {
	if ((op = gem_link(hdr->xname)) < 0)
	{
	  vmsg("�藍�_�A���O��ذϥu��O�Ͷi�J�A�ЦV�O�D�ӽФJ�ҳ\\�i");
	  return XO_FOOT;
	}
      }
      else			/* �@����v */
      {
	op = xo->key;		/* �~�ӥ����v���v�� */
      }

      strcpy(title, hdr->title);
      XoSong(fpath, title, op);
      return song_init(xo);
    }

    /* browse article */

    /* Thor.990204: ���Ҽ{more �Ǧ^�� */
    if ((xmode = more(fpath, FOOTER_GEM)) < 0)
      break;

    op = GEM_FILE;

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
      xo_help("song");
      break;
    }
    break;
  }

  return song_head(xo);
}


static int
count_ktv()	/* itoc.021102: ktv �O�̭��w���X�g���I�q�O�� */
{
  int count, fd;
  time_t yesterday;
  char folder[64];
  HDR *hdr;

  brd_fpath(folder, BN_KTV, fn_dir);
  if ((fd = open(folder, O_RDONLY)) < 0)
    return 0;

  mgets(-1);
  count = 0;
  yesterday = time(0) - 86400;
  while (hdr = mread(fd, sizeof(HDR)))
  {
    /* �Y�ϬO�ΦW�I�q�Auserid ���O���b hdr.owner + IDLEN + 1 */
    if (!strcmp(hdr->owner + IDLEN + 1, cuser.userid) &&
      hdr->chrono > yesterday)
    {
      if (++count >= 3)		/* �����I�T�� */
	break;
    }
  }
  close(fd);

  return count;
}


static int
song_order(xo)
  XO *xo;
{
#ifdef HAVE_ANONYMOUS
  int annoy;
#endif
  char fpath[64], des[20], say[57], buf[64];
  HDR *hdr, xpost;
  FILE *fpr, *fpw;

  /* itoc.����: song_order ���P post */
  if (!HAS_PERM(PERM_POST))
    return XO_NONE;

  /* itoc.010831: �I�q�n���A�ҥH�n�T�� multi-login */
  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XO_FOOT;
  }

  if (count_ktv() >= 3)		/* �����I�T�� */
  {
    vmsg("�L�h�G�Q�|�p�ɤ��z�w�I��L�h�q��");
    return XO_FOOT;
  }

  if (cuser.money < 1000)
  {
    vmsg("�n 1000 �ȹ��~���I�q��ݪO��");
    return XO_FOOT;
  }

  if (!(hdr = song_check(xo, fpath, GEM_FILE)))
    return XO_NONE;

  if (!vget(b_lines, 0, "�I�q���֡G", des, sizeof(des), DOECHO))
    return XO_FOOT;
  if (!vget(b_lines, 0, "�Q�����ܡG", say, sizeof(say), DOECHO))
    return XO_FOOT;

#ifdef HAVE_ANONYMOUS
  annoy = vans("�Q�n�ΦW��(Y/N)�H[N] ") == 'y';
#endif

  if (vans("�T�w�I�q��(Y/N)�H[Y] ") == 'n')
    return XO_FOOT;

  if (!(fpr = fopen(fpath, "r")))
    return XO_FOOT;

  cuser.money -= 1000;

#ifdef LOG_SONG_USIES
  song_usies_log(hdr->chrono, hdr->title);
#endif

  /* �[�J�峹�ɮ� */

  brd_fpath(fpath, BN_KTV, fn_dir);

  if (fpw = fdopen(hdr_stamp(fpath, 'A', &xpost, buf), "w"))
  {
#ifdef HAVE_ANONYMOUS
    song_quote(fpr, fpw, annoy ? STR_ANONYMOUS : cuser.userid, des, say);
#else
    song_quote(fpr, fpw, cuser.userid, des, say);
#endif
    fclose(fpw);
  }

  fclose(fpr);

  /* �[�J .DIR record */

#ifdef HAVE_ANONYMOUS
  strcpy(xpost.owner, annoy ? STR_ANONYMOUS : cuser.userid);
  /* �Y�ϬO�ΦW�I�q�Auserid ���O���b hdr.owner + IDLEN + 1 */
  strcpy(xpost.owner + IDLEN + 1, cuser.userid);
#else
  strcpy(xpost.owner, cuser.userid);
#endif
  strcpy(xpost.nick, xpost.owner);
  sprintf(xpost.title, "%s �I�� %s", xpost.owner, des);
  rec_bot(fpath, &xpost, sizeof(HDR));

  btime_update(brd_bno(BN_KTV));

  return XO_FOOT;
}


static int
song_send(xo)
  XO *xo;
{
  char fpath[64], say[57], buf[64];
  HDR *hdr, xpost;
  FILE *fpr, *fpw;
  ACCT acct;

  /* itoc.����: song_send ���P mail */
  if (!HAS_PERM(PERM_LOCAL))
    return XO_NONE;

  if (!(hdr = song_check(xo, fpath, GEM_FILE)))
    return XO_NONE;

  if (acct_get("�I�q���֡G", &acct) < 1)	/* acct_get �i��| clear�A�ҥH�n��ø */
    return song_head(xo);

  if (!vget(b_lines, 0, "�Q�����ܡG", say, sizeof(say), DOECHO))
    strcpy(say, ".........");

  if (vans("�T�w�I�q��(Y/N)�H[Y] ") == 'n')
    return song_head(xo);			/* acct_get �i��| clear�A�ҥH�n��ø */

#ifdef LOG_SONG_USIES
  song_usies_log(hdr->chrono, hdr->title);
#endif

  /* �[�J�峹�ɮ� */

  if (!(fpr = fopen(fpath, "r")))
    return song_head(xo);			/* acct_get �i��| clear�A�ҥH�n��ø */

  usr_fpath(fpath, acct.userid, fn_dir);

  if (fpw = fdopen(hdr_stamp(fpath, 0, &xpost, buf), "w"))
  {
    song_quote(fpr, fpw, cuser.userid, acct.userid, say);
    fclose(fpw);
  }

  fclose(fpr);

  /* �[�J .DIR record */

  strcpy(xpost.owner, cuser.userid);
  strcpy(xpost.nick, cuser.username);
  sprintf(xpost.title, "%s �I�q���z", cuser.userid);
  rec_add(fpath, &xpost, sizeof(HDR));

  mail_hold(buf, acct.userid, xpost.title, 0);
  m_biff(acct.userno);		/* �Y���b�u�W�A�h�n�q�����s�H */

  return song_head(xo);				/* acct_get �i��| clear�A�ҥH�n��ø */
}


static int
song_internet(xo)
  XO *xo;
{
  int rc;
  char fpath[64], rcpt[64], des[20], say[57];
  HDR *hdr;
  FILE *fpr, *fpw;

  /* itoc.����: song_internet ���P internet_mail */
  if (!HAS_PERM(PERM_INTERNET))
    return XO_NONE;

  if (!(hdr = song_check(xo, fpath, GEM_FILE)))
    return XO_NONE;

  if (!vget(b_lines, 0, "�ت��a�G", rcpt, sizeof(rcpt), DOECHO))
    return XO_FOOT;
  if (!strchr(rcpt, '@'))
    return XO_FOOT;

  if (!vget(b_lines, 0, "�I�q���֡G", des, sizeof(des), DOECHO))
    return XO_FOOT;
  if (!vget(b_lines, 0, "�Q�����ܡG", say, sizeof(say), DOECHO))
    strcpy(say, ".........");

  if (vans("�T�w�I�q��(Y/N)�H[Y] ") == 'n')
    return XO_FOOT;

#ifdef LOG_SONG_USIES
  song_usies_log(hdr->chrono, hdr->title);
#endif

  /* �[�J�峹�ɮ� */

  if (!(fpr = fopen(fpath, "r")))
    return XO_FOOT;

  sprintf(fpath, "tmp/song_internet.%s", cuser.userid);
  fpw = fopen(fpath, "w");

  song_quote(fpr, fpw, cuser.userid, des, say);

  fclose(fpr);
  fclose(fpw);

  /* �H����� */

  rc = bsmtp(fpath, "�I�q���z", rcpt, 0);
  vmsg(rc >= 0 ? msg_sent_ok : "�H��L�k�H�F�A���Z�ƥ��b�H�c");

  mail_hold(fpath, rcpt, hdr->title, rc);
  unlink(fpath);

  return XO_FOOT;
}


static int
song_edit(xo)
  XO *xo;
{
  char fpath[64];

  if (!song_check(xo, fpath, GEM_FILE))
    return XO_NONE;

  if (xo->key & GEM_W_BIT)
    vedit(fpath, 0);
  else
    vedit(fpath, -1);		/* itoc.010403: ���@��ϥΪ̤]�i�H edit �ݱ���X */
  return song_head(xo);
}


static int
song_title(xo)
  XO *xo;
{
  HDR *fhdr, mhdr;
  int pos, cur;

  if (!(xo->key & GEM_W_BIT) || !(fhdr = song_check(xo, NULL, 0)))
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
    song_item(++pos, fhdr, xo->key);

  }
  return XO_FOOT;
}


static int
song_help(xo)
  XO *xo;
{
  xo_help("song");
  return song_head(xo);
}


static KeyFunc song_cb[] =
{
  XO_INIT, song_init,
  XO_LOAD, song_load,
  XO_HEAD, song_head,
  XO_BODY, song_body,

  'r', song_browse,
  'o', song_order,
  'm', song_send,
  'M', song_internet,

  'E', song_edit,
  'T', song_title,

  'h', song_help
};


static void
XoSong(folder, title, level)
  char *folder;
  char *title;
  int level;
{
  XO *xo, *last;

  last = xz[XZ_SONG - XO_ZONE].xo;	/* record */

  xz[XZ_SONG - XO_ZONE].xo = xo = xo_new(folder);
  xz[XZ_SONG - XO_ZONE].cb = song_cb;
  xo->pos = 0;
  xo->key = level;
  xo->xyz = title;
  strcpy(currBM, "�O�D�G�t�κ޲z��");

  xover(XZ_SONG);

  free(xo);

  xz[XZ_SONG - XO_ZONE].xo = last;	/* restore */
}


int
XoSongMain()
{
  int level;
  char fpath[64];

  gem_fpath(fpath, BN_KTV, fn_dir);

  if (HAS_PERM(PERM_SYSOP))
    level = GEM_W_BIT | GEM_X_BIT | GEM_M_BIT;
  else if (HAS_PERM(PERM_ALLBOARD))
    level = GEM_W_BIT | GEM_M_BIT;
  else
    level = 0;

  XoSong(fpath, "�I�q�t��", level);
  return 0;
}


int
XoSongSub()
{
  int bno;

  if ((bno = brd_bno(BN_REQUEST)) >= 0)
  {
    XoPost(bno);
    xover(XZ_POST);
    return 0;
  }
  return XEASY;
}


int
XoSongLog()
{
  int bno;

  if ((bno = brd_bno(BN_KTV)) >= 0)
  {
    XoPost(bno);
    xover(XZ_POST);
    return 0;
  }
  return XEASY;
}
#endif	/* HAVE_SONG */
