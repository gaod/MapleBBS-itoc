/*-------------------------------------------------------*/
/* util/account.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : �W���H���έp�B�t�θ�Ƴƥ��B�}��		 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/
/* syntax : ���{���y�H crontab ����A�]�w�ɶ����C�p��	 */
/* 1-5 �� ����						 */
/*-------------------------------------------------------*/
/* notice : brdshm (board shared memory) synchronize	 */
/*-------------------------------------------------------*/


#include "bbs.h"

#include <sys/ipc.h>
#include <sys/shm.h>


#define	MAX_LINE	16
#define	ADJUST_M	10	/* adjust back 10 minutes */


/* itoc.011004.����: �� - �N��o�����A�� = �N��W�����C�b gem/@/ �U���ܦh�o�˪��d�� */

static char fn_today[] = "gem/@/@-act";		/* ����W���H���έp */
static char fn_yesday[] = "gem/@/@=act";	/* �Q��W���H���έp */
static char log_file[] = FN_RUN_USIES "=";

static time_t now;			/* ����{�����ɶ� */


/* ----------------------------------------------------- */
/* �}���Gshm �������P cache.c �ۮe			 */
/* ----------------------------------------------------- */


static BCACHE *bshm;


static int
boardname_cmp(a, b)
  BRD *a, *b;
{
  return str_cmp(a->brdname, b->brdname);
}


static void
fix_brd()
{
  BRD allbrd[MAXBOARD], brd;
  FILE *fp;
  int i, num;

  if (!(fp = fopen(FN_BRD, "r")))
    return;

  num = 0;
  for (i = 0; i < MAXBOARD; i++)
  {
    if (fread(&brd, sizeof(BRD), 1, fp) != 1)
      break;

    if (!*brd.brdname)	/* ���O�w�Q�R�� */
      continue;

    memcpy(&allbrd[num], &brd, sizeof(BRD));
    num++;
  }

  fclose(fp);

  /* itoc.041110: �b�Ĥ@�����J bshm �ɡA�N bno �̪O�W�Ƨ� */
  if (num > 1)
    qsort(allbrd, num, sizeof(BRD), boardname_cmp);

  unlink(FN_BRD);

  if (num)
    rec_add(FN_BRD, allbrd, sizeof(BRD) * num);
}


#ifdef HAVE_MODERATED_BOARD
static int
int_cmp(a, b)
  int *a, *b;
{
  return *a - *b;
}
#endif


static void
init_allbrd()
{
  BRD *head, *tail;
#ifdef HAVE_MODERATED_BOARD
  int fd;
  char fpath[64];
  BPAL *bpal;
#endif

  head = bshm->bcache;
  tail = head + bshm->number;
#ifdef HAVE_MODERATED_BOARD
  bpal = bshm->pcache;
#endif

  do
  {
    /* itoc.040314: �O�D���ݪO�ԭz�άO�������ݪO�ɤ~�|�� bpost/blast �g�i .BRD �� 
       �ҥH .BRD �̪� bpost/blast �����O�諸�A�n���s initial�C
       initial ����k�O�N btime �]�� -1�A�� class_item() �h��s */
    head->btime = -1;

#ifdef HAVE_MODERATED_BOARD
    brd_fpath(fpath, head->brdname, FN_PAL);
    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
      struct stat st;
      PAL *pal, *up;
      int count;

      fstat(fd, &st);
      if (pal = (PAL *) malloc(count = st.st_size))
      {
	count = read(fd, pal, count) / sizeof(PAL);
	if (count > 0 && count <= PAL_MAX)
	{
	  int *userno;
	  int c = count;

	  userno = bpal->pal_spool;
	  up = pal;
	  do
	  {
	    *userno++ = (up->ftype & PAL_BAD) ? -(up->userno) : up->userno;
	    up++;
	  } while (--c);

	  if (count > 1)
	    qsort(bpal->pal_spool, count, sizeof(int), int_cmp);
	  bpal->pal_max = count;
	}
	free(pal);
      }
      close(fd);
    }

    bpal++;
#endif

  } while (++head < tail);     
}


static void
init_bshm()
{
  time_t *uptime;
  int n, turn;

  bshm = shm_new(BRDSHM_KEY, sizeof(BCACHE));
  uptime = &(bshm->uptime);

  turn = 0;
  for (;;)
  {
    n = *uptime;
    if (n > 0)		/* bshm �w initial ���� */
      return;

    if (n < 0)
    {
      if (++turn < 30)
      {
	sleep(2);
	continue;
      }
    }

    *uptime = -1;

    fix_brd();			/* itoc.030725: �b�Ĥ@�����J bshm �e�A����z .BRD */

    if ((n = open(FN_BRD, O_RDONLY)) >= 0)
    {
      turn = read(n, bshm->bcache, MAXBOARD * sizeof(BRD)) / sizeof(BRD);
      close(n);
      bshm->number = bshm->numberOld = turn;

      init_allbrd();
    }

    /* ���Ҧ� boards ��Ƨ�s��A�]�w uptime */

    time(uptime);
    fprintf(stderr, "[account]\tCACHE\treload bcache\n");
    return;
  }
}


/* ----------------------------------------------------- */
/* keep log in board					 */
/* ----------------------------------------------------- */


static void
update_btime(brdname)
  char *brdname;
{
  BRD *brdp, *bend;

  brdp = bshm->bcache;
  bend = brdp + bshm->number;
  do
  {
    if (!strcmp(brdname, brdp->brdname))
    {
      brdp->btime = -1;
      break;
    }
  } while (++brdp < bend);
}


static void
keeplog(fnlog, board, title, mode)
  char *fnlog;
  char *board;
  char *title;
  int mode;			/* 0:load  1:rename  2:unlink */
{
  HDR hdr;
  char folder[64], fpath[64];
  int fd;
  FILE *fp;

  if (!dashf(fnlog))	/* Kudo.010804: �ɮ׬O�Ū��N�� keeplog */
    return;

  if (!board)
    board = BN_RECORD;

  brd_fpath(folder, board, FN_DIR);
  fd = hdr_stamp(folder, 'A', &hdr, fpath);
  if (fd < 0)
    return;

  if (mode == 1)
  {
    close(fd);
    /* rename(fnlog, fpath); */
    f_mv(fnlog, fpath);		/* Thor.990409: �i��partition */
  }
  else
  {
    fp = fdopen(fd, "w");
    fprintf(fp, "�@��: %s (%s)\n���D: %s\n�ɶ�: %s\n\n",
      STR_SYSOP, SYSOPNICK, title, Btime(&hdr.chrono));
    f_suck(fp, fnlog);
    fclose(fp);
    if (mode)
      unlink(fnlog);
  }

  strcpy(hdr.title, title);
  strcpy(hdr.owner, STR_SYSOP);
  rec_bot(folder, &hdr, sizeof(HDR));

  update_btime(board);
}


/* ----------------------------------------------------- */
/* build vote result					 */
/* ----------------------------------------------------- */


struct Tchoice
{
  int count;
  vitem_t vitem;
};


static int
TchoiceCompare(i, j)
  struct Tchoice *i, *j;
{
  return j->count - i->count;
}


static int
draw_vote(brd, fpath, vch)
  BRD *brd;			/* Thor: �ǤJ BRD, �i�d battr */
  char *fpath;
  VCH *vch;
{
  struct Tchoice choice[MAX_CHOICES];
  FILE *fp;
  char *fname, *bid, buf[80];
  int total, items, num, fd, ticket, bollt;
  VLOG vlog;
  char *list = "@IOLGZ";	/* itoc.����: �M vote file */

  bid = brd->brdname;
  fname = strrchr(fpath, '@');

  /* vote item */

  *fname = 'I';

  items = 0;
  if (fp = fopen(fpath, "r"))
  {
    while (fread(&choice[items].vitem, sizeof(vitem_t), 1, fp) == 1)
    {
      choice[items].count = 0;
      items++;
    }
    fclose(fp);
  }

  if (items == 0)
    return 0;

  /* �֭p�벼���G */

  *fname = 'G';
  bollt = 0;		/* Thor: �`�����k�s */
  total = 0;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    while (read(fd, &vlog, sizeof(VLOG)) == sizeof(VLOG))
    {
      for (ticket = vlog.choice, num = 0; ticket && num < items; ticket >>= 1, num++)
      {
	if (ticket & 1)
	{
	  choice[num].count += vlog.numvotes;
	  bollt += vlog.numvotes;
	}
      }
      total++;
    }
    close(fd);
  }

  /* ���Ͷ}�����G */

  *fname = 'Z';
  if (!(fp = fopen(fpath, "w")))
    return 0;

  fprintf(fp, "\n\033[1;34m" MSG_SEPERATOR "\033[m\n\n"
    "\033[1;32m�� [%s] �ݪO�벼�G%s\033[m\n\n�|��O�D�G%s\n\n�|�����G%s\n\n",
    bid, vch->title, vch->owner, Btime(&vch->chrono));
  fprintf(fp, "�}������G%s\n\n\033[1;32m�� �벼�D�D�G\033[m\n\n", Btime(&vch->vclose));

  *fname = '@';
  f_suck(fp, fpath);

  fprintf(fp, "\n\033[1;32m�� �벼���G�G�C�H�i�� %d ���A�@ %d �H�ѥ[�A��X %d ��\033[m\n\n",
    vch->maxblt, total, bollt);

  if (vch->vsort == 's')
    qsort(choice, items, sizeof(struct Tchoice), TchoiceCompare);

  if (vch->vpercent == '%')
    fd = BMAX(1, bollt);
  else
    fd = 0;

  for (num = 0; num < items; num++)
  {
    ticket = choice[num].count;
    if (fd)
      fprintf(fp, "    %-36s%5d �� (%4.1f%%)\n", &choice[num].vitem, ticket, 100.0 * ticket / fd);
    else
      fprintf(fp, "    %-36s%5d ��\n", &choice[num].vitem, ticket);
  }

  /* other opinions */

  *fname = 'O';
  fputs("\n\033[1;32m�� �ڦ��ܭn���G\033[m\n\n", fp);
  f_suck(fp, fpath);
  fputs("\n", fp);
  fclose(fp);

  fp = fopen(fpath, "w");	/* Thor: �� O_ �Ȧs�@�U�U... */
  *fname = 'Z';
  f_suck(fp, fpath);
  sprintf(buf, "brd/%s/@/@vote", bid);
  f_suck(fp, buf);
  fclose(fp);
  *fname = 'O';
  rename(fpath, buf);

  /* �N�}�����G post �� [BN_RECORD] �P ���ݪO */

  *fname = 'Z';

  /* Thor: �Y�ݪO�ݩʬ� BRD_NOVOTE�A�h�� post �� [BN_RECORD] */

  if (!(brd->battr & BRD_NOVOTE))
  {
    sprintf(buf, "[�O��] %s <<�ݪO�ﱡ����>>", bid);
    keeplog(fpath, NULL, buf, 0);
  }

  keeplog(fpath, bid, "[�O��] �ﱡ����", 2);

  while (*fname = *list++)
    unlink(fpath); /* Thor: �T�w�W�r�N�� */

  return 1;
}


static int		/* 0:���ݼg�^.BRD !=0:�ݼg�^.BRD */
draw_board(brd)
  BRD *brd;
{
  int fd, fsize, alive;
  int oldbvote, newbvote;
  VCH *vch, *head, *tail;
  char *fname, fpath[64], buf[64];
  struct stat st;

  /* �� account �� maintain brd->bvote */

  oldbvote = brd->bvote;

  brd_fpath(fpath, brd->brdname, FN_VCH);

  if ((fd = open(fpath, O_RDWR | O_APPEND, 0600)) < 0 || fstat(fd, &st) || (fsize = st.st_size) <= 0)
  {
    if (fd >= 0)
    {
      close(fd);
      unlink(fpath);
    }
    brd->bvote = 0;
    return oldbvote;
  }

  vch = (VCH *) malloc(fsize);

  /* flock(fd, LOCK_EX); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_exlock(fd);

  read(fd, vch, fsize);

  strcpy(buf, fpath);
  fname = strrchr(buf, '.');
  *fname++ = '@';
  *fname++ = '/';

  head = vch;
  tail = (VCH *) ((char *)vch + fsize);

  alive = 0;
  newbvote = 0;

  do
  {
    if (head->vclose < now && head->vgamble == ' ')	/* ��L���� account �} */
    {
      strcpy(fname, head->xname);
      draw_vote(brd, buf, head);/* Thor: �ǤJ BRD, �i�d battr */
      head->chrono = 0;
    }
    else
    {
      alive++;

      if (head->vgamble == '$')
	newbvote = -1;
    }
  } while (++head < tail);


  if (alive && alive != fsize / sizeof(VCH))
  {
    ftruncate(fd, 0);
    head = vch;
    do
    {
      if (head->chrono)
      {
	write(fd, head, sizeof(VCH));
      }
    } while (++head < tail);
  }

  /* flock(fd, LOCK_UN);  */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_unlock(fd);

  close(fd);

  free(vch);

  if (!alive)
    unlink(fpath);
  else if (!newbvote)
    newbvote = 1;	/* �u���벼�A�S����L */

  if (oldbvote != newbvote)
  {
    brd->bvote = newbvote;
    return 1;
  }

  return 0;
}


static void
closepolls()
{
  BRD *bcache, *head, *tail;
  int dirty;

  dirty = 0;

  head = bcache = bshm->bcache;
  tail = head + bshm->number;
  do
  {
    dirty |= draw_board(head);
  } while (++head < tail);

  if (!dirty)
    return;

  /* write back the shm cache data */

  if ((dirty = open(FN_BRD, O_WRONLY | O_CREAT, 0600)) < 0)
    return;

  /* flock(dirty, LOCK_EX); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_exlock(dirty);

  write(dirty, bcache, (char *)tail - (char *)bcache);

  /* flock(dirty, LOCK_UN); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_unlock(dirty);

  close(dirty);
  time(&bshm->uptime);
}


/* ----------------------------------------------------- */
/* build Class image					 */
/* ----------------------------------------------------- */


#define CLASS_RUNFILE	"run/class.run"


static ClassHeader *chx[CH_MAX];
static int chn;
static BRD *bhead, *btail;


static int
class_parse(key)
  char *key;
{
  char *str, *ptr, fpath[64];
  ClassHeader *chp;
  HDR hdr;
  int i, len, count;
  FILE *fp;

  strcpy(fpath, "gem/@/@");
  str = fpath + sizeof("gem/@/@") - 1;
  for (ptr = key;; ptr++)
  {
    i = *ptr;
    if (i == '/')
      i = 0;
    *str = i;
    if (!i)
      break;
    str++;
  }

  len = ptr - key;

  /* search classes */

  for (i = 1; i < chn; i++)
  {
    str = chx[i]->title;
    if (str[len] == '/' && !memcmp(key, str, len))
      return CH_END - i;
  }

  /* build classes */

  if (fp = fopen(fpath, "r"))
  {
    int ans;
    struct stat st;

    if (fstat(fileno(fp), &st) || (count = st.st_size / sizeof(HDR)) <= 0)
    {
      fclose(fp);
      return CH_END;
    }

    /* itoc.030723: �ˬd Class �ƶq�O�_�W�L CH_MAX */
    if (chn >= CH_MAX - 1)
    {
      static int show_error = 0;
      if (!show_error)		/* ���~�T���u�L�@�� */
      {
	fprintf(stderr, "[account]\t�Х[�j�z�� CH_MAX �w�q�ACH_MAX �����W�L Class ���ƶq\n");
	show_error = 1;
      }
      fclose(fp);
      return CH_END;
    }

    chx[chn++] = chp = (ClassHeader *) malloc(sizeof(ClassHeader) + count * sizeof(short));
    memset(chp->title, 0, CH_TTLEN);
    strcpy(chp->title, key);

    ans = chn;
    count = 0;

    while (fread(&hdr, sizeof(hdr), 1, fp) == 1)
    {
      if (hdr.xmode & GEM_BOARD)
      {
	BRD *bp;

	i = -1;
	str = hdr.xname;
	bp = bhead;

	for (;;)
	{
	  i++;
	  if (!str_cmp(str, bp->brdname))
	    break;

	  if (++bp >= btail)
	  {
	    i = -1;
	    break;
	  }
	}

	if (i < 0)
	  continue;
      }
      else
      {
	/* recursive �a�@�h�@�h�i�h�� Class */
	i = class_parse(hdr.title);

	if (i == CH_END)
	  continue;
      }

      chp->chno[count++] = i;
    }

    fclose(fp);

    chp->count = count;
    return -ans;
  }

  return CH_END;
}


static int
brdname_cmp(i, j)
  short *i, *j;
{
  return str_cmp(bhead[*i].brdname, bhead[*j].brdname);
}


static int
brdtitle_cmp(i, j)		/* itoc.010413: �̬ݪO����ԭz�Ƨ� */
  short *i, *j;
{
  /* return strcmp(bhead[*i].title, bhead[*j].title); */

  /* itoc.010413: ����/�O�W��e��� */
  int k = strcmp(bhead[*i].class, bhead[*j].class);
  return k ? k : str_cmp(bhead[*i].brdname, bhead[*j].brdname);
}


static void
class_sort(cmp)
  int (*cmp) ();
{
  ClassHeader *chp;
  int i, j, max;
  BRD *bp;

  max = bshm->number;
  bhead = bp = bshm->bcache;
  btail = bp + max;

  chp = (ClassHeader *) malloc(sizeof(ClassHeader) + max * sizeof(short));

  for (i = j = 0; i < max; i++, bp++)
  {
    if (bp->brdname)
      chp->chno[j++] = i;
  }

  chp->count = j;

  qsort(chp->chno, j, sizeof(short), cmp);

  memset(chp->title, 0, CH_TTLEN);
  strcpy(chp->title, "Boards");
  chx[chn++] = chp;
}


static void
class_image()
{
  int i, times;
  FILE *fp;
  short len, pos[CH_MAX];
  ClassHeader *chp;

  for (times = 2; times > 0; times--)	/* itoc.010413: ���ͤG�� class image */
  {
    chn = 0;
    class_sort(times == 1 ? brdname_cmp : brdtitle_cmp);
    class_parse(CLASS_INIFILE);

    if (chn < 2)		/* lkchu.990106: �|�S������ */
      return;

    len = sizeof(short) * (chn + 1);

    for (i = 0; i < chn; i++)
    {
      pos[i] = len;
      len += CH_TTLEN + chx[i]->count * sizeof(short);
    }

    pos[i++] = len;

    if (fp = fopen(CLASS_RUNFILE, "w"))
    {
      fwrite(pos, sizeof(short), i, fp);
      for (i = 0; i < chn; i++)
      {
	chp = chx[i];
	fwrite(chp->title, 1, CH_TTLEN + chp->count * sizeof(short), fp);
	free(chp);
      }
      fclose(fp);

      rename(CLASS_RUNFILE, times == 1 ? CLASS_IMGFILE_NAME : CLASS_IMGFILE_TITLE);
    }
  }

  bshm->min_chn = -chn;
}


/* ----------------------------------------------------- */
/* �W���H�Ʋέp						 */
/* ----------------------------------------------------- */


static void
error(fpath)
  char *fpath;
{
  printf("can not open [%s]\n", fpath);
  /* exit(1); */	/* itoc.011004: �W���H���έp���ѡA�L�ݤ��_ account ���� */
}


static void
ansi_puts(fp, buf, mode)
  FILE *fp;
  char *buf, mode;
{
  static char state = '0';

  if (state != mode)
    fprintf(fp, "\033[3%cm", state = mode);
  if (*buf)
  {
    fprintf(fp, buf);
    *buf = '\0';
  }
}


static void
draw_usies(ptime)
  struct tm *ptime;
{
  int fact, hour, max, item, total, i, j, over;
  char buf[256];
  FILE *fp, *fpw;
  int act[26];			/* act[0~23]:0~23�ɦU�p�ɪ��W���H�� act[24]:���d�֭p�ɶ� act[25]:�ֿn�H�� */

  static char act_file[] = "run/var/act";
  static char run_file[] = FN_RUN_USIES;
  static char tmp_file[] = FN_RUN_USIES ".tmp";

  rename(run_file, tmp_file);
  if (!(fp = fopen(tmp_file, "r")))
  {
    /* error(tmp_file); */	/* itoc.011004.����: �S�� tmp_file ��ܨS�� run_file�A��ܱq�W���] account ��{�b�A */
    return;			/* �S���H login �L bbs�C�q�`�o�ͦb��ʶ] account �W�c�ɡC */
  }

  if (!(fpw = fopen(log_file, "a")))
  {
    fclose(fp);
    error(log_file);		/* itoc.011004.����: log_file �O�Q�� run_file�C�p�G�Q�Ѿ�ѳ��S���H login �L bbs�A */
    return;			/* �N�|�o�ͨS�� log_file �����p */
  }

  if ((fact = open(act_file, O_RDWR | O_CREAT, 0600)) < 0)
  {
    fclose(fp);
    fclose(fpw);
    error(act_file);		/* itoc.011004.����: ���w�g�� O_CREAT �p�G�٨S�� act_file ����..�n�۬����a :P */
    return;
  }

  memset(act, 0, sizeof(act));

  if (ptime->tm_hour != 0)
  {
    read(fact, act, sizeof(act));
    lseek(fact, 0, SEEK_SET);
  }

  while (fgets(buf, sizeof(buf), fp))
  {
    fputs(buf, fpw);

    if (!memcmp(buf + 24, "ENTER", 5))
    {
      hour = atoi(buf + 15);
      if (hour >= 0 && hour <= 23)
	act[hour]++;
      continue;
    }

    if (!memcmp(buf + 43, "Stay:", 5))
    {
      if (hour = atoi(buf + 49))
      {
	act[24] += hour;
	act[25]++;
      }
      continue;
    }
  }
  fclose(fp);
  fclose(fpw);
  unlink(tmp_file);

  write(fact, act, sizeof(act));
  close(fact);

  for (i = max = total = 0; i < 24; i++)
  {
    total += act[i];		/* itoc.030415.����: act[25] �������� total�A���H�]�\�����`���� */
    if (act[i] > max)
      max = act[i];
  }

  item = max / MAX_LINE + 1;
  over = max > 1000;

  if (!(fp = fopen(fn_today, "w")))
  {
    error(fn_today);
    return;
  }

  /* Thor.990329: y2k */
  fprintf(fp, "\t\t\t   \033[1;33;46m [%02d/%02d/%02d] �W���H���έp \033[40m\n",
    ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);

  for (i = MAX_LINE + 1; i > 0; i--)
  {
    strcpy(buf, "   ");
    for (j = 0; j < 24; j++)
    {
      max = item * i;
      hour = act[j];
      if (hour && (max > hour) && (max - item <= hour))
      {
	ansi_puts(fp, buf, '3');
	if (over)
	  hour = (hour + 5) / 10;
	fprintf(fp, "%-3d", hour);
      }
      else if (max <= hour)
      {
	ansi_puts(fp, buf, '1');
	fprintf(fp, "�i ");
      }
      else
	strcat(buf, "   ");
    }
    fprintf(fp, "\n");
  }

  if (act[25] == 0)
    act[25] = 1;		/* Thor.980928: lkchu patch: ����Ƭ�0 */

  fprintf(fp, "\033[34m"
    "  ��������������������������������������������������������������������������\n  \033[32m"
    "0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23\n\n"
    "\t%s\t\033[35m�`�@�W���H���G\033[37m%-9d\033[35m�����ϥήɶ��G\033[37m%d\033[m\n",
    over ? "\033[35m���G\033[37m10 �H" : "", total, act[24] / act[25] + 1);
  fclose(fp);
}


/* ----------------------------------------------------- */
/* ���Y�{��						 */
/* ----------------------------------------------------- */


static void
gzip(source, target, stamp)
  char *source, *target, *stamp;
{
  char buf[128];

  if (dashf(source))
  {
    sprintf(buf, "/usr/bin/gzip -n log/%s%s", target, stamp);
    /* rename(source, buf + 17); */
    f_mv(source, buf + 17);	/* Thor.990409: �i�� partition */
    system(buf);
  }
}


/* ----------------------------------------------------- */
/* �������ҫH�� private key				 */
/* ----------------------------------------------------- */


#ifdef HAVE_SIGNED_MAIL
static void
private_key(ymd)
  char *ymd;
{
  srandom(time(NULL));

#if (PRIVATE_KEY_PERIOD == 0)
  if (!dashf(FN_RUN_PRIVATE))
#else
  if (!dashf(FN_RUN_PRIVATE) || (random() % PRIVATE_KEY_PERIOD) == 0)
#endif
  {
    int i, j;
    char buf[80];

    sprintf(buf, "log/prikey%s", ymd);
    f_mv(FN_RUN_PRIVATE, buf);
    i = 8;
    for (;;)
    {
      j = random() & 0xff;
      if (!j)
	continue;
      buf[--i] = j;
      if (i == 0)
	break;
    }
    rec_add(FN_RUN_PRIVATE, buf, 8);
  }
}
#endif


/* ----------------------------------------------------- */
/* �D�{��						 */
/* ----------------------------------------------------- */


int
main(argc, argv)
  int argc;
  char *argv[];
{
  struct tm ntime, *ptime;
  FILE *fp;

  now = time(NULL);	/* �@�}�l�N�n���W�O���ɶ� */

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);
  umask(077);

  /* --------------------------------------------------- */
  /* ��l�� board shm �Ψӫ� Class �ζ}��		 */
  /* --------------------------------------------------- */

  init_bshm();

  /* --------------------------------------------------- */
  /* build Class image					 */
  /* --------------------------------------------------- */

  class_image();

  /* --------------------------------------------------- */
  /* �t�ζ}��						 */
  /* --------------------------------------------------- */

  closepolls();

  /* --------------------------------------------------- */
  /* ��Ʋέp�ɶ�					 */
  /* --------------------------------------------------- */

  /* itoc.030911: �Y�[�F�ѼơA��ܤ��O�b crontab �̶]���A���򤣰���Ʋέp */
  if (argc != 1)
    exit(0);

  /* ntime �O���� */
  ptime = localtime(&now);
  memcpy(&ntime, ptime, sizeof(ntime));	/* ���s�_�ӡA�]���٭n���@�� localtime() */

  /* ptime �O�Q�� */
  /* itoc.011004.����: �ѩ� account �O��e�@�p�ɲέp�A�ҥH�b�s�ɮɭn�������^ 10 �����A��Q�ѥh */
  /* itoc.030911.����: �ҥH account �����b�C�p�ɪ� 1-10 �������� */
  now -= ADJUST_M * 60;		/* back to ancent */
  ptime = localtime(&now);

  /* --------------------------------------------------- */
  /* �W���H���έp					 */
  /* --------------------------------------------------- */

  draw_usies(ptime);

  /* --------------------------------------------------- */
  /* ������Y�ƥ��B�������D�έp				 */
  /* --------------------------------------------------- */

  if (ntime.tm_hour == 0)
  {
    char date[16], ymd[16];
    char title[80];

    sprintf(ymd, "-%02d%02d%02d",
      ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);	/* Thor.990329: y2k */

    sprintf(date, "[%d �� %d ��] ", ptime->tm_mon + 1, ptime->tm_mday);


    /* ------------------------------------------------- */
    /* �C����s�ɰ�����				 */
    /* ------------------------------------------------- */

    gzip(log_file, "usies", ymd);		/* �ƥ��Ҧ� [�W��] �O�� */

#ifdef HAVE_SIGNED_MAIL
    private_key(ymd);
#endif

    sprintf(title, "%s�峹�g�Ʋέp", date);
    keeplog(FN_RUN_POST_LOG, BN_SECURITY, title, 2);

    sprintf(title, "%s�H�H�O��", date);
    keeplog(FN_RUN_MAIL_LOG, BN_SECURITY, title, 2);

#ifdef HAVE_ANONYMOUS
    sprintf(title, "%s�ΦW�峹�o��", date);
    keeplog(FN_RUN_ANONYMOUS, BN_SECURITY, title, 2);
#endif

#ifdef HAVE_BUY
    sprintf(title, "%s�׿��O��", date);
    keeplog(FN_RUN_BANK_LOG, BN_SECURITY, title, 2);
#endif

    system("grep OVER " BMTA_LOGFILE " | cut -f2 | cut -d' ' -f2- | sort | uniq -c > run/over.log");
    sprintf(title, "%sE-Mail over max connection �έp", date);
    keeplog("run/over.log", BN_SECURITY, title, 2);

    sprintf(title, "%s�Ĳ��W���d���O", date);
    keeplog(FN_RUN_NOTE_ALL, NULL, title, 2);

    if (fp = fopen(fn_yesday, "w"))
    {
      f_suck(fp, fn_today);
      fclose(fp);
    }
    sprintf(title, "%s�W���H���έp", date);
    keeplog(fn_today, NULL, title, 1);


    /* ------------------------------------------------- */
    /* �C�g�@���s�ɰ�����				 */
    /* ------------------------------------------------- */

    if (ntime.tm_wday == 0)
    {
      sprintf(title, "%s���g�������D", date);
      keeplog("gem/@/@-week", NULL, title, 0);

#ifdef LOG_ADMIN
      sprintf(title, "%s�����ק��v��", date);
      keeplog(FN_RUN_PERM, BN_SECURITY, title, 2);
#endif

      sprintf(title, "%s���i�O�D�έp", date);
      keeplog(FN_RUN_LAZYBM, BN_SECURITY, title, 2);

      sprintf(title, "%s�S���v���ϥΪ̦C��", date);
      keeplog(FN_RUN_MANAGER, BN_SECURITY, title, 2);

      sprintf(title, "%s�������W���Q�M�����ϥΪ̦C��", date);
      keeplog(FN_RUN_REAPER, BN_SECURITY, title, 2);

      sprintf(title, "%s�P�@ email �{�Ҧh��", date);
      keeplog(FN_RUN_EMAILADDR, BN_SECURITY, title, 2);
    }


    /* ------------------------------------------------- */
    /* �C��@����s�ɰ�����				 */
    /* ------------------------------------------------- */

    if (ntime.tm_mday == 1)
    {
      sprintf(title, "%s����������D", date);
      keeplog("gem/@/@-month", NULL, title, 0);
    }


    /* ------------------------------------------------- */
    /* �C�~�@��@����s�ɰ�����			 */
    /* ------------------------------------------------- */

    if (ntime.tm_yday == 1)
    {
      sprintf(title, "%s�~�׼������D", date);
      keeplog("gem/@/@-year", NULL, title, 0);
    }
  }

  exit(0);
}
