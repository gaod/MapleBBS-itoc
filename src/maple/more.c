/*-------------------------------------------------------*/
/* more.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : simple & beautiful ANSI/Chinese browser	 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


/* ----------------------------------------------------- */
/* buffered file read					 */
/* ----------------------------------------------------- */


#define MORE_BUFSIZE	4096


static uschar more_pool[MORE_BUFSIZE];
static int more_base;		/* more_pool[more_base ~ more_base+more_size] ¦³­È */
static int more_size;


/* ----------------------------------------------------- */
/* mget ¾\Åª¤å¦rÀÉ¡Fmread ¾\Åª¤G¶i¦ìÀÉ			 */
/* ----------------------------------------------------- */


/* itoc.041226.µù¸Ñ: mgets() ©M more_line() ¤£¤@¼Ëªº³¡¤À¦³
   1. mgets ª½±µ¥Î more_pool ªºªÅ¶¡¡Fmore_line «h¬O·|§â­È¼g¤J¤@¶ô buffer
   2. mgets ¤£·|¦Û°ÊÂ_¦æ¡Fmore_line «h¬O·|¦Û°ÊÂ_¦æ¦b b_cols
   ©Ò¥H mgets ¬O®³¥Î¦b¤@¨Ç¨t²ÎÀÉ³B²z©Î¬O edit.c¡A¦Ó more_line ¥u¥Î¦b more()
 */


char *
mgets(fd)
  int fd;
{
  char *pool, *base, *head, *tail;
  int ch;

  if (fd < 0)
  {
    more_base = more_size = 0;
    return NULL;
  }

  pool = more_pool;
  base = pool + more_base;
  tail = pool + more_size;
  head = base;

  for (;;)
  {
    if (head >= tail)
    {
      if (ch = head - base)
	memcpy(pool, base, ch);

      head = pool + ch;
      ch = read(fd, head, MORE_BUFSIZE - ch);

      if (ch <= 0)
	return NULL;

      base = pool;
      tail = head + ch;
      more_size = tail - pool;
    }

    ch = *head;

    if (ch == '\n')
    {
      *head++ = '\0';
      more_base = head - pool;
      return base;
    }

    head++;
  }
}


/* use mgets(-1) to reset */


void *
mread(fd, len)
  int fd, len;
{
  char *pool;
  int base, size;

  base = more_base;
  size = more_size;
  pool = more_pool;

  if (size < len)
  {
    if (size)
      memcpy(pool, pool + base, size);

    base = read(fd, pool + size, MORE_BUFSIZE - size);

    if (base <= 0)
      return NULL;

    size += base;
    base = 0;
  }

  more_base = base + len;
  more_size = size - len;

  return pool + base;
}


/* ----------------------------------------------------- */
/* more ¾\Åª¤å¦rÀÉ					 */
/* ----------------------------------------------------- */


#define	STR_ANSICODE	"[0123456789;"


static off_t more_off;		/* more_base ¹ïÀ³¬O¦b fd ªº off_t ¦h¤Ö */


static void
more_goto(fd, off)
  int fd;
  off_t off;		/* off ­n¤ñ more_off ¤p */
{
  off_t diff;

  diff = more_off - off;
  if (diff <= more_base)	/* ­n«e¥hªº¦a¤è¦b more_pool[] ¸Ì­±¡A¤£»Ý­n­«·s read */
  {
    more_base -= diff;
    more_size += diff;
  }
  else
  {
    lseek(fd, off, SEEK_SET);
    more_base = more_size = 0;
  }
  more_off = off;
}


static int
more_line(fd, buf)
  int fd;
  char *buf;
{
  uschar *pool, *base, *tail;
  int ch, len, bytes, in_ansi, in_chi;

  pool = more_pool;
  base = pool + more_base;
  tail = base + more_size;

  len = bytes = in_ansi = in_chi = 0;

  for (;;)
  {
    if (base >= tail)
    {
      ch = read(fd, pool, MORE_BUFSIZE);
      if (ch <= 0)			/* end of file or error */
	break;

      base = pool;
      tail = pool + ch;
    }

    ch = *base;

    /* weiyu.040802: ¦pªG³o½X¬O¤¤¤å¦rªº­º½X¡A¦ý¬O¥u³Ñ¤U¤@½XªºªÅ¶¡¥i¥H¦L¡A¨º»ò¤£­n¦L³o½X */
    if (in_chi || ch & 0x80)
      in_chi ^= 1;
    if (in_chi && (len >= b_cols - 1 || bytes >= ANSILINELEN - 2))
      break;

    base++;
    bytes++;

    if (ch == '\n')
      break;

    if (ch == KEY_ESC)
    {
      in_ansi = 1;
    }
    else if (in_ansi)
    {
      if (!strchr(STR_ANSICODE, ch))
	in_ansi = 0;
    }
    else if (isprint2(ch))
    {
      len++;
    }
    else
    {
      ch = ' ';		/* ¦L¥X¤£¨Óªº³£´«¦¨ªÅ¥Õ */
      len++;
    }

    *buf++ = ch;

    /* ­Y¤£§t±±¨î½Xªºªø«×¤w¹F b_cols ¦r¡A©Î§t±±¨î½Xªºªø«×¤w¹F ANSILINELEN-1¡A¨º»òÂ÷¶}°j°é */
    if (len >= b_cols || bytes >= ANSILINELEN - 1)
    {
      /* itoc.031123: ¦pªG¬O±±¨î½X¡A§Y¨Ï¤£§t±±¨î½Xªºªø«×¤w¹F b_cols ¤F¡AÁÙ¥i¥HÄ~Äò¦Y */
      if ((in_ansi || (base < tail && *base == KEY_ESC)) && bytes < ANSILINELEN - 1)
	continue;

      /* itoc.031123: ¦AÀË¬d¤U¤@­Ó¦r¬O¤£¬O '\n'¡AÁ×§K«ê¦n¬O b_cols ©Î ANSILINELEN-1 ®É¡A·|¦h¸õ¤@¦æªÅ¥Õ¦æ */
      if (base < tail && *base == '\n')
      {
	base++;
	bytes++;
      }
      break;
    }
  }

  *buf = '\0';

  more_base = base - pool;
  more_size = tail - base;
  more_off += bytes;

  return bytes;
}


static void
outs_line(str)			/* ¦L¥X¤@¯ë¤º®e */
  char *str;
{
  int ch1, ch2, ansi;

  /* ¡°³B²z¤Þ¥ÎªÌ & ¤Þ¨¥ */

  ch1 = str[0];
  ch2 = str[1];

  if (ch2 == ' ' && (ch1 == QUOTE_CHAR1 || ch1 == QUOTE_CHAR2))	/* ¤Þ¨¥ */
  {
    ansi = 1;
    ch1 = str[2];
    outs((ch1 == QUOTE_CHAR1 || ch1 == QUOTE_CHAR2) ? "\033[33m" : "\033[36m");	/* ¤Þ¥Î¤@¼h/¤G¼h¤£¦PÃC¦â */
  }
  else if (ch1 == '¡' && ch2 == '°')		/* ¡° ¤Þ¨¥ªÌ */
  {
    ansi = 1;
    outs("\033[1;36m");
  }
  else
    ansi = 0;

  /* ¦L¥X¤º®e */

  if (!hunt[0])
  {
    outx(str);
  }
  else
  {
    int len;
    char buf[ANSILINELEN];
    char *ptr1, *ptr2;

    len = strlen(hunt);
    ptr2 = buf;
    while (1)
    {
      if (!(ptr1 = str_sub(str, hunt)))
      {
	strcpy(ptr2, str);
	break;
      }

      if (buf + ANSILINELEN - 1 <= ptr2 + (ptr1 - str) + (len + 7))	/* buf ªÅ¶¡¤£°÷ */
	break;

      str_ncpy(ptr2, str, ptr1 - str + 1);
      ptr2 += ptr1 - str;
      sprintf(ptr2, "\033[7m%.*s\033[m", len, ptr1);
      ptr2 += len + 7;
      str = ptr1 + len;
    }

    outx(buf);
  }

  if (ansi)
    outs(str_ransi);
}


#define LINE_HEADER	3	/* ÀÉÀY¦³¤T¦æ */

static void
outs_header(str, header_len)	/* ¦L¥XÀÉÀY */
  char *str;
  int header_len;
{
  static char header1[LINE_HEADER][LEN_AUTHOR1] = {"§@ªÌ",   "¼ÐÃD",   "®É¶¡"};
  static char header2[LINE_HEADER][LEN_AUTHOR2] = {"µo«H¤H", "¼Ð  ÃD", "µo«H¯¸"};
  int i;
  char *ptr, *word;

  /* ³B²zÀÉÀY */

  if ((header_len == LEN_AUTHOR1 && !memcmp(str, header1[0], LEN_AUTHOR1 - 1)) ||
    (header_len == LEN_AUTHOR2 && !memcmp(str, header2[0], LEN_AUTHOR2 - 1)))
  {
    /* §@ªÌ/¬ÝªO ÀÉÀY¦³¤GÄæ¡A¯S§O³B²z */
    word = str + header_len;
    if ((ptr = strstr(word, str_post1)) || (ptr = strstr(word, str_post2)))
    {
      ptr[-1] = ptr[4] = '\0';
      prints(COLOR5 " %s " COLOR6 "%-*.*s" COLOR5 " %s " COLOR6 "%-13s\033[m", 
	header1[0], d_cols + 53, d_cols + 53, word, ptr, ptr + 5);
    }
    else
    {
      /* ¤Ö¬ÝªO³oÄæ */
      prints(COLOR5 " %s " COLOR6 "%-*.*s\033[m", 
	header1[0], d_cols + 72, d_cols + 72, word);
    }
    return;
  }

  for (i = 1; i < LINE_HEADER; i++)
  {
    if ((header_len == LEN_AUTHOR1 && !memcmp(str, header1[i], LEN_AUTHOR1 - 1)) ||
      (header_len == LEN_AUTHOR2 && !memcmp(str, header2[i], LEN_AUTHOR2 - 1)))
    {
      /* ¨ä¥LÀÉÀY³£¥u¦³¤@Äæ */
      word = str + header_len;
      prints(COLOR5 " %s " COLOR6 "%-*.*s\033[m", 
	header1[i], d_cols + 72, d_cols + 72, word);
      return;
    }
  }

  /* ¦pªG¤£¬OÀÉÀY¡A´N·í¤@¯ë¤å¦r¦L¥X */
  outs_line(str);
}


static inline void
outs_footer(buf, lino, fsize)
  char *buf;
  int lino;
  off_t fsize;
{
  int i;

  /* P.1 ¦³ (PAGE_SCROLL + 1) ¦C¡A¨ä¥L Page ³£¬O PAGE_SCROLL ¦C */

  /* prints(FOOTER_MORE, (lino - 2) / PAGE_SCROLL + 1, (more_off * 100) / fsize); */

  /* itoc.010821: ¬°¤F©M FOOTER ¹ï»ô */
  sprintf(buf, FOOTER_MORE, (lino - 2) / PAGE_SCROLL + 1, (more_off * 100) / fsize);
  outs(buf);

  for (i = b_cols + sizeof(COLOR1) + sizeof(COLOR2) - strlen(buf); i > 3; i--)
  {
    /* ¶ñº¡³Ì«áªºªÅ¥ÕÃC¦â¡A³Ì«á¯d¤@­ÓªÅ¥Õ */
    outc(' ');
  }
  outs(str_ransi);
}


#ifdef SLIDE_SHOW
static int slideshow;		/* !=0: ¼½©ñ movie ªº³t«× */

static int
more_slideshow()
{
  int ch;

  if (!slideshow)
  {
    ch = vkey();

    if (ch == '@')
    {
      slideshow = vans("½Ð¿ï¾Ü©ñ¬Mªº³t«× 1(³ÌºC)¡ã9(³Ì§Ö)¡H¼½©ñ¤¤«ö¥ô·NÁä¥i°±¤î¼½©ñ¡G") - '0';
      if (slideshow < 1 || slideshow > 9)
	slideshow = 5;

      ch = KEY_PGDN;
    }
  }  
  else
  {
    struct timeval tv[9] =
    {
      {4, 0}, {3, 0}, {2, 0}, {1, 500000}, {1, 0},
      {0, 800000}, {0, 600000}, {0, 400000}, {0, 200000}
    };

    refresh();
    ch = 1;
    if (select(1, (fd_set *) &ch, NULL, NULL, tv + slideshow - 1) > 0)
    {
      /* ­Y¼½©ñ¤¤«ö¥ô·NÁä¡A«h°±¤î¼½©ñ */
      slideshow = 0;
      ch = vkey();
    }
    else
    {
      ch = KEY_PGDN;
    }
  }

  return ch;
}
#endif


#define END_MASK	0x200	/* «ö KEY_END ª½¹F³Ì«á¤@­¶ */

#define HUNT_MASK	0x400
#define HUNT_NEXT	0x001	/* «ö n ·j´M¤U¤@µ§ */
#define HUNT_FOUND	0x002	/* «ö / ¶}©l·j´M¡A¥B¤w¸g§ä¨ì match ªº¦r¦ê */
#define HUNT_START	0x004	/* «ö / ¶}©l·j´M¡A¥B©|¥¼§ä¨ì match ªº¦r¦ê */

#define MAXBLOCK	256	/* °O¿ý´X­Ó block ªº offset¡C¥i¥[³t MAXBLOCK*32 ¦C¥H¤ºªºªø¤å¦b¤W±²/Â½®Éªº³t«× */

/* Thor.990204: ¶Ç¦^­È -1 ¬°µLªkshow¥X
                        0 ¬°¥þ¼Æshow§¹
                       >0 ¬°¥¼¥þshow¡A¤¤Â_©Ò«öªºkey */
int
more(fpath, footer)
  char *fpath;
  char *footer;
{
  char buf[ANSILINELEN];
  struct stat st;
  int fd;
  int i;

  int shift;			/* ÁÙ»Ý­n©¹¤U²¾°Ê´X¦C */
  int lino;			/* ¥Ø«e line number */
  int header_len;		/* ÀÉÀYªºªø«×¡A¦P®É¤]¬O¯¸¤º/¯¸¥~«Hªº°Ï§O */
  int key;			/* «öÁä */
  int cmd;			/* ¤¤Â_®É©Ò«öªºÁä */

  off_t fsize;			/* ÀÉ®×¤j¤p */
  static off_t block[MAXBLOCK];	/* ¨C 32 ¦C¬°¤@­Ó block¡A°O¿ý¦¹ block ªº offset */

  if ((fd = open(fpath, O_RDONLY)) < 0)
    return -1;

  more_base = more_size = 0;
  more_off = 0;

  /* Åª¥XÀÉ®×²Ä¤@¦æ¡A¨Ó§PÂ_¯¸¤º«HÁÙ¬O¯¸¥~«H */
  if (fstat(fd, &st) || (fsize = st.st_size) <= 0 || !more_line(fd, buf))
  {
    close(fd);
    return -1;
  }

  lino = cmd = 0;
  block[0] = 0;

#ifdef SLIDE_SHOW
  slideshow = 0;
#endif

  if (hunt[0])		/* ¦b xxxx_browse() ½Ð¨D·j´M¦r¦ê */
  {
    str_lowest(hunt, hunt);
    shift = HUNT_MASK | HUNT_START;
  }
  else
  {
    shift = b_lines;
  }

  header_len = 
    !memcmp(buf, str_author1, LEN_AUTHOR1) ? LEN_AUTHOR1 :	/* ¡u§@ªÌ:¡vªí¯¸¤º¤å³¹ */
    !memcmp(buf, str_author2, LEN_AUTHOR2) ? LEN_AUTHOR2 : 	/* ¡uµo«H¤H:¡vªíÂà«H¤å³¹ */
    0;								/* ¨S¦³ÀÉÀY */

  /* Âk¹s */
  more_base = 0;
  more_size += more_off;
  more_off = 0;

  clear();

  while (more_line(fd, buf))
  {
    /* ------------------------------------------------- */
    /* ¦L¥X¤@¦Cªº¤å¦r					 */
    /* ------------------------------------------------- */

    /* ­º­¶«e´X¦C¤~»Ý­n³B²zÀÉÀY */
    if (lino < LINE_HEADER || (shift < 0 && lino <= b_lines + LINE_HEADER - 1))
      outs_header(buf, header_len);
    else
      outs_line(buf);

    outc('\n');

    /* ------------------------------------------------- */
    /* ¨Ì shift ¨Ó¨M©w°Ê§@				 */
    /* ------------------------------------------------- */

    /* itoc.030303.µù¸Ñ: shift ¦b¦¹ªº·N¸q
       >0: ÁÙ»Ý­n©¹¤U²¾´X¦æ
       <0: ÁÙ»Ý­n©¹¤W²¾´X¦æ
       =0: µ²§ô³o­¶¡Aµ¥«Ý¨Ï¥ÎªÌ«öÁä */

    if (shift > 0)		/* ÁÙ­n¤U²¾ shift ¦C */
    {
      if (lino >= b_lines)	/* ¥u¦³¦b­è¶i more¡A²Ä¤@¦¸¦L²Ä¤@­¶®É¤~¥i¯à lino <= b_lines */
	scroll();

      lino++;

      if ((lino % 32 == 0) && ((i = lino >> 5) < MAXBLOCK))
	block[i] = more_off;


      if (!(shift & (HUNT_MASK | END_MASK)))	/* ¤@¯ë¸ê®ÆÅª¨ú */
      {
	shift--;
      }
      else if (shift & HUNT_MASK)		/* ¦r¦ê·j´M */
      {
	if (shift & HUNT_NEXT)	/* «ö n ·j´M¤U¤@µ§ */
	{
	  /* ¤@§ä¨ì´N°±©ó¸Ó¦C */
	  if (str_sub(buf, hunt))
	    shift = 0;
	}
	else			/* «ö / ¶}©l·j´M */
	{
	  /* ­Y¦b²Ä¤G­¶¥H«á§ä¨ì¡A¤@§ä¨ì´N°±©ó¸Ó¦C¡F
	     ­Y¦b²Ä¤@­¶§ä¨ì¡A¥²¶·µ¥¨ìÅª§¹²Ä¤@­¶¤~¯à°±¤î */
	  if (shift & HUNT_START && str_sub(buf, hunt))
	    shift ^= HUNT_START | HUNT_FOUND;		/* ®³±¼ HUNT_START ¨Ã¥[¤W HUNT_FOUND */
	  if (shift & HUNT_FOUND && lino >= b_lines)
	    shift = 0;
	}
      }
    }
    else if (shift < 0)		/* ÁÙ­n¤W²¾ -shift ¦C */
    {
      shift++;

      if (!shift)
      {
	move(b_lines, 0);
	clrtoeol();

	/* ³Ñ¤U b_lines+shift ¦C¬O rscroll¡Aoffsect ¥h¥¿½T¦ì¸m¡F³o¸Ìªº i ¬OÁ`¦@­n shift ªº¦C¼Æ */
	for (i += b_lines; i > 0; i--)
	  more_line(fd, buf);
      }
    }

    if (more_off >= fsize)	/* ¤w¸gÅª§¹¥þ³¡ªºÀÉ®× */
    {
      /* ­Õ­Y¬O«ö End ²¾¨ì³Ì«á¤@­¶¡A¨º»ò°±¯d¦b 100% ¦Ó¤£µ²§ô¡F§_«h¤@«ßµ²§ô */
      if (!(shift & END_MASK))
	break;
      shift = 0;
    }

    if (shift)			/* ÁÙ»Ý­nÄ~ÄòÅª¸ê®Æ */
      continue;

    /* ------------------------------------------------- */
    /* ¨ì¦¹¦L§¹©Ò»Ýªº shift ¦C¡A±µ¤U¨Ó¦L¥X footer ¨Ãµ¥«Ý */
    /* ¨Ï¥ÎªÌ«öÁä					 */
    /* ------------------------------------------------- */

re_key:

    outs_footer(buf, lino, fsize);

#ifdef SLIDE_SHOW
    key = more_slideshow();
#else
    key = vkey();
#endif

    if (key == ' ' || key == KEY_PGDN || key == KEY_RIGHT || key == Ctrl('F'))
    {
      shift = PAGE_SCROLL;
    }

    else if (key == KEY_DOWN || key == '\n')
    {
      shift = 1;
    }

    else if (key == KEY_PGUP || key == Ctrl('B') || key == KEY_DEL)
    {
      /* itoc.010324: ¨ì¤F³Ì¶}©l¦A¤W±²ªí¥ÜÂ÷¶}¡A¨Ã¦^¶Ç 'k' (keymap[] ©w¸q¤W¤@½g) */
      if (lino <= b_lines)
      {
	cmd = 'k';
	break;
      }
      /* ³Ì¦h¥u¯à¤W±²¨ì¤@¶}©l */
      i = PAGE_SCROLL + 1 - lino;
      shift = BMAX(-PAGE_SCROLL, i);
    }

    else if (key == KEY_UP)
    {
      /* itoc.010324: ¨ì¤F³Ì¶}©l¦A¤W±²ªí¥ÜÂ÷¶}¡A¨Ã¦^¶Ç 'k' (keymap[] ©w¸q¤W¤@½g) */
      if (lino <= b_lines)
      {
	cmd = 'k';
	break;
      }
      shift = -1;
    }

    else if (key == KEY_END || key == '$')
    {
      shift = END_MASK;
    }

    else if (key == KEY_HOME || key == '0')
    {
      if (lino <= b_lines)	/* ¤w¸g¦b³Ì¶}©l¤F */
	shift = 0;
      else
	shift = -b_lines;
    }

    else if (key == '/' || key == 'n')
    {
      if (key == 'n' && hunt[0])	/* ¦pªG«ö n «o¥¼¿é¤J¹L·j´M¦r¦ê¡A¨º»òµø¦P«ö / */
      {
	shift = HUNT_MASK | HUNT_NEXT;
      }
      else if (vget(b_lines, 0, "·j´M¡G", hunt, sizeof(hunt), DOECHO))
      {
	str_lowest(hunt, hunt);
	shift = HUNT_MASK | HUNT_START;
      }
      else				/* ¦pªG¨ú®ø·j´Mªº¸Ü¡A­«Ã¸ footer §Y¥i */
      {
	shift = 0;
      }
    }

    else if (key == 'C')	/* Thor.980405: more ®É¥i¦s¤J¼È¦sÀÉ */
    {
      FILE *fp;
      if (fp = tbf_open())
      {
	f_suck(fp, fpath);
	fclose(fp);
      }
      shift = 0;		/* ­«Ã¸ footer */
    }

    else if (key == 'h')
    {
      screenline slt[T_LINES];
      vs_save(slt);
      xo_help("post");
      vs_restore(slt);
      shift = 0;
    }

    else		/* ¨ä¥LÁä³£¬O¨Ï¥ÎªÌ¤¤Â_ */
    {
      /* itoc.041006: ¨Ï¥ÎªÌ¤¤Â_ªº«öÁä­n > 0 (¦Ó KEY_LEFT ¬O < 0) */
      cmd = key > 0 ? key : 'q';
      break;
    }

    /* ------------------------------------------------- */
    /* ¨Ï¥ÎªÌ¤w«öÁä¡A­Y break «hÂ÷¶}°j°é¡F§_«h¨Ì·Ó shift */
    /* ªººØÃþ (¥ç§Y«öÁäªººØÃþ) ¦Ó°µ¤£¦Pªº°Ê§@		 */
    /* ------------------------------------------------- */

    if (shift > 0)			/* ·Ç³Æ¤U²¾ shift ¦C */
    {
      if (shift < (HUNT_MASK | HUNT_START))	/* ¤@¯ë¤U²¾ */
      {
	/* itoc.041114.µù¸Ñ: ¥Ø¼Ð¬O¨q¥X lino-b_lines+1+shift ~ lino+shift ¦Cªº¤º®e¡G
	   ´N¥u­n²M footer §Y¥i¡A¨ä¥Lªº´N¥æµ¹«e­±´`§Ç¦L shift ¦Cªºµ{¦¡ */

	move(b_lines, 0);
	clrtoeol();

#if 1
	/* itoc.041116: End ªº§@ªk¨ä¹ê©M¤@¯ë¤U²¾¥i¥H¬O§¹¥þ¤@¼Ëªº¡A¦ý¬O¦pªG¹J¨ì¶Wªø¤å³¹®É¡A
	   ·|³y¦¨«e­±´`§Ç¦L shift ¦Cªºµ{¦¡´N±o¤@ª½Â½¡Aª½¨ì§ä¨ì³Ì«á¤@­¶¡A³o¼Ë·|°µ¤Ó¦h outs_line() ¥Õ¤u¡A
	   ©Ò¥H¦b¦¹¯S§OÀË¬d¶Wªø¤å³¹®É¡A´N¥ý¥h§ä³Ì«á¤@­¶©Ò¦b */

	if ((shift & END_MASK) && (fsize - more_off >= MORE_BUFSIZE))	/* ÁÙ¦³¤@°ï¨SÅª¹L¡A¤~¯S§O³B²z */
	{
	  int totallino = lino;

	  /* ¥ýÅª¨ì³Ì«á¤@¦C¬Ý¬Ý¥þ³¡¦³´X¦C */
	  while (more_line(fd, buf))
	  {
	    totallino++;
	    if ((totallino % 32 == 0) && ((i = totallino >> 5) < MAXBLOCK))
	      block[i] = more_off;
	  }

	  /* ¥ý¦ì²¾¨ì¤W¤@­Ó block ªº§ÀºÝ */
	  i = (totallino - b_lines) >> 5;
	  if (i >= MAXBLOCK)
	    i = MAXBLOCK - 1;
	  more_goto(fd, (off_t) block[i]);
	  i = i << 5;

	  /* ¦A±q¤W¤@­Ó block ªº§ÀºÝ¦ì²¾¨ì totallino-b_lines+1 ¦C */
	  for (i = totallino - b_lines - i; i > 0; i--)
	    more_line(fd, buf);

	  lino = totallino - b_lines;
	}
#endif
      }
      else
      {
	/* '/' ±qÀY¶}©l·j´M */
	lino = 0;
	more_goto(fd, (off_t) 0);
	clear();
      }
    }
    else if (shift < 0)			/* ·Ç³Æ¤W²¾ -shift ¦C */
    {
      if (shift > -b_lines)	/* ¤W±²¼Æ¦C */
      {
	lino += shift;

	/* itoc.041114.µù¸Ñ: ¥Ø¼Ð¬O¨q¥X lino-b_lines+1 ~ lino ¦Cªº¤º®e¡G
	  1. ¥ý±qÀY¦ì²¾¨ì lino-b_lines+1 ¦C
	  2. ¨ä¤¤¦³ b_lines+shift ¦C¬O¤£ÅÜªº¤º®e¡A¥Î rscroll ¹F¦¨
	  3. ¦b«e­±ªº outs_line() ªº¦a¤è¦L¥X -shift ¦C
	  4. ³Ì«á¦A¦ì²¾­è¤~ rscroll ªº¦C¼Æ
	*/

	/* ¥ý¦ì²¾¨ì¤W¤@­Ó block ªº§ÀºÝ */
	i = (lino - b_lines) >> 5;
	if (i >= MAXBLOCK)
	  i = MAXBLOCK - 1;
	more_goto(fd, (off_t) block[i]);
	i = i << 5;

	/* ¦A±q¤W¤@­Ó block ªº§ÀºÝ¦ì²¾¨ì lino-b_lines+1 ¦C */
	for (i = lino - b_lines - i; i > 0; i--)
	  more_line(fd, buf);

	for (i = shift; i < 0; i++) 
	{
	  rscroll();
	  move(0, 0);
	  clrtoeol();
	}

	i = shift;
      }
      else			/* Home */
      {
	/* itoc.041226.µù¸Ñ: ¥Ø¼Ð¬O¨q¥X 1 ~ b_lines ¦Cªº¤º®e¡G
           §@ªk´N¬O¥þ³¡³£Âk¹s¡A±qÀY¦A¦L b_lines ¦C§Y¥i */

	clear();

	more_goto(fd, (off_t) 0);
	lino = 0;
	shift = b_lines;
      }
    }
    else				/* ­«Ã¸ footer ¨Ã re-key */
    {
      move(b_lines, 0);
      clrtoeol();
      goto re_key;
    }
  }	/* while °j°éªºµ²§ô */

  /* --------------------------------------------------- */
  /* ÀÉ®×¤w¸g¨q§¹ (cmd = 0) ©Î ¨Ï¥ÎªÌ¤¤Â_ (cmd != 0)	 */
  /* --------------------------------------------------- */

  close(fd);

  if (!cmd)	/* ÀÉ®×¥¿±`¨q§¹¡A­n³B²z footer */
  {
    if (footer)		/* ¦³ footer */
    {
      if (footer != (char *) -1)
	outf(footer);
      else
	outs(str_ransi);
    }
    else		/* ¨S¦³ footer ­n vmsg() */
    {
      /* lkchu.981201: ¥ý²M¤@¦¸¥H§K­«Å|Åã¥Ü */
      move(b_lines, 0);
      clrtoeol();

      if (vmsg(NULL) == 'C')	/* Thor.990204: ¯S§Oª`·N­Y¦^¶Ç 'C' ªí¥Ü¼È¦sÀÉ */
      {
	FILE *fp;

	if (fp = tbf_open()) 
	{
	  f_suck(fp, fpath); 
	  fclose(fp);
	}
      }
    }
  }
  else		/* ¨Ï¥ÎªÌ¤¤Â_¡Aª½±µÂ÷¶} */
  {
    outs(str_ransi);
  }

  hunt[0] = '\0';

  /* Thor.990204: Åýkey¥i¦^¶Ç¦Ümore¥~ */
  return cmd;
}
