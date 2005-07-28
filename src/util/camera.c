/*-------------------------------------------------------*/
/* util/camera.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : 建立 [動態看板] cache			 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


static char *list[] = 		/* src/include/struct.h */
{
  "opening.0",			/* FILM_OPENING0 */
  "opening.1",			/* FILM_OPENING1 */
  "opening.2",			/* FILM_OPENING2 */
  "goodbye", 			/* FILM_GOODBYE  */
  "notify",			/* FILM_NOTIFY   */
  "mquota",			/* FILM_MQUOTA   */
  "mailover",			/* FILM_MAILOVER */
  "mgemover",			/* FILM_MGEMOVER */
  "birthday",			/* FILM_BIRTHDAY */
  "apply",			/* FILM_APPLY    */
  "justify",			/* FILM_JUSTIFY  */
  "re-reg",			/* FILM_REREG    */
  "e-mail",			/* FILM_EMAIL    */
  "newuser",			/* FILM_NEWUSER  */
  "tryout",			/* FILM_TRYOUT   */
  "post",			/* FILM_POST     */
  NULL				/* FILM_MOVIE    */
};


static FCACHE image;
static int number;
static int tail;


static int		/* 1:成功 0:已超過容量 */
play(data, movie, size)
  char *data;
  int movie;
  int size;
{
  int line, ch;
  char *head;

  head = data;

  if (movie)		/* 動態看板，要限制行數 */
  {
    line = 0;
    while (ch = *data)		/* at most MOVIE_LINES lines */
    {
      data++;
      if (ch == '\n')
      {
	if (++line >= MOVIE_LINES)
	  break;
      }
    }

    while (line < MOVIE_LINES)	/* at lease MOVIE_LINES lines */
    {
      *data++ = '\n';
      line++;
    }

    *data = '\0';
    size = data - head;
  }

  ch = size + 1;	/* Thor.980804: +1 將結尾的 '\0' 也算入 */

  line = tail + ch;
  if (line >= MOVIE_SIZE)	/* 若加入這篇會超過全部容量，則不 mirror */
    return 0;

  data = image.film + tail;
  memcpy(data, head, ch);
  image.shot[++number] = tail = line;
  return 1;
}


static int		/* 1:成功 0:已超過篇數 */
mirror(fpath, movie)
  char *fpath;
  int movie;		/* 0:系統文件，不限制列數  !=0:動態看板，要限制列數 */
{
  int fd, size;
  char buf[FILM_SIZ + 1];

  /* 若已經超過最大篇數，則不再繼續 mirror */
  if (number >= MOVIE_MAX - 1)
    return 0;

  size = 0;
  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    /* 讀入檔案 */
    size = read(fd, buf, FILM_SIZ);
    close(fd);
  }

  if (size <= 0)
  {
    if (movie)		/* 如果是 動態看板/點歌 缺檔案，就不 mirror */
      return 1;

    /* 如果是系統文件缺檔案的話，要補上去 */
    sprintf(buf, "請告訴站長缺檔案 %s", fpath);
    size = strlen(buf);
  }

  buf[size] = '\0';
  return play(buf, movie, size);
}


static void
do_gem(folder)		/* itoc.011105: 把看板/精華區的文章收進 movie */
  char *folder;		/* index 路徑 */
{
  char fpath[64];
  FILE *fp;
  HDR hdr;

  if (fp = fopen(folder, "r"))
  {
    while (fread(&hdr, sizeof(HDR), 1, fp) == 1)
    {
      if (hdr.xmode & (GEM_RESTRICT | GEM_RESERVED | GEM_BOARD))	/* 限制級、看板 不放入 movie 中 */
	continue;

      hdr_fpath(fpath, folder, &hdr);

      if (hdr.xmode & GEM_FOLDER)	/* 遇到卷宗則迴圈進去收 movie */
      {
	do_gem(fpath);
      }
      else				/* plain text */
      {
	if (!mirror(fpath, FILM_MOVIE))
	  break;
      }
    }
    fclose(fp);
  }
}


static void
lunar_calendar(key, now, ptime)	/* itoc.050528: 由陽曆算農曆日期 */
  char *key;
  time_t *now;
  struct tm *ptime;
{
#if 0	/* Table 的意義 */

  (1) "," 只是分隔，方便給人閱讀而已
  (2) 前 12 個 byte 分別代表農曆 1-12 月為大月或是小月。"L":大月三十天，"-":小月二十九天
  (3) 第 14 個 byte 代表今年農曆閏月。"X":無閏月，"123456789:;<" 分別代表農曆 1-12 是閏月
  (4) 第 16-20 個 bytes 代表今年農曆新年是哪天。例如 "02:15" 表示陽曆二月十五日是農曆新年

#endif

  #define TABLE_INITAIL_YEAR	2005
  #define TABLE_FINAL_YEAR	2011

  char Table[TABLE_FINAL_YEAR - TABLE_INITAIL_YEAR + 1][21] = 
  {
    "-L-L-LL-L-L-,X,02:09",	/* 2005 雞年 */
    "L-L-L-L-LL-L,7,01:29",	/* 2006 狗年 */
    "--L--L-LLL-L,X,02:18",	/* 2007 豬年 */
    "L--L--L-LL-L,X,02:07",	/* 2008 鼠年 */
    "LL--L--L-L-L,5,01:26",	/* 2009 牛年 */
    "L-L-L--L-L-L,X,02:14",	/* 2010 虎年 */
    "L-LL-L--L-L-,X,02:14",	/* 2011 兔年 */
  };

  char year[21];

  time_t nyd;	/* 今年的農曆新年 */
  struct tm ntime;

  int i;
  int Mon, Day;
  int leap;	/* 0:本年無閏月 */

  /* 先找出今天是農曆哪一年 */

  memcpy(&ntime, ptime, sizeof(ntime));

  for (i = TABLE_FINAL_YEAR - TABLE_INITAIL_YEAR; i >= 0; i--)
  {
    strcpy(year, Table[i]);
    ntime.tm_year = TABLE_INITAIL_YEAR - 1900 + i;
    ntime.tm_mday = atoi(year + 18);
    ntime.tm_mon = atoi(year + 15) - 1;
    nyd = mktime(&ntime);

    if (*now >= nyd)
      break;
  }

  /* 再從農曆正月初一開始數到今天 */

  leap = (year[13] == 'X') ? 0 : 1;

  Mon = Day = 1;
  for (i = (*now - nyd) / 86400; i > 0; i--)
  {
    if (++Day > (year[Mon - 1] == 'L' ? 30: 29))
    {
      Mon++;
      Day = 1;

      if (leap == 2)
      {
	leap = 0;
	Mon--;
	year[Mon - 1] = '-';	/* 閏月必小月 */
      }
      else if (year[13] == Mon + '0')
      {
	if (leap == 1)		/* 下月是閏月 */
	  leap++;
      }
    }
  }

  sprintf(key, "%02d:%02d", Mon, Day);
}


static void
do_today(fshm)
  FCACHE *fshm;
{
  FILE *fp;
  char buf[80], *str, *today;
  char key1[6];			/* mm/dd: 陽曆 mm月dd日 */
  char key2[6];			/* mm/#A: 陽曆 mm月的第#個星期A */
  char key3[6];			/* MM\DD: 農曆 MM月DD日 */
  time_t now;
  struct tm *ptime;

  time(&now);
  ptime = localtime(&now);
  sprintf(key1, "%02d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
  sprintf(key2, "%02d/%d%c", ptime->tm_mon + 1, (ptime->tm_mday - 1) / 7 + 1, ptime->tm_wday + 'A');
  lunar_calendar(key3, &now, ptime);

  today = fshm->today;
  sprintf(today, "%s %.2s", key1, "日一二三四五六" + (ptime->tm_wday << 1));

  if (fp = fopen(FN_ETC_FEAST, "r"))
  {
    while (fgets(buf, 80, fp))
    {
      if (buf[0] == '#')
	continue;

      if (str = (char *) strchr(buf, ' '))
      {
	*str = '\0';
	if (!strcmp(key1, buf) || !strcmp(key2, buf) || !strcmp(key3, buf))
	{
	  /* 跳過空白分隔 */
	  for (str++; *str && isspace(*str); str++)
	    ;

	  str_ncpy(today, str, sizeof(fshm->today));
	  if (str = (char *) strchr(today, '\n'))	/* 最後的 '\n' 不要 */
	    *str = '\0';
	  break;
	}
      }
    }
    fclose(fp);
  }
}


int
main()
{
  int i;
  char *fname, *str, fpath[64];
  FCACHE *fshm;

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  number = 0;		/* 計算有幾篇動態看板用的 */
  tail = 0;

  /* --------------------------------------------------- */
  /* 載入常用的文件及 help			 	 */
  /* --------------------------------------------------- */

  strcpy(fpath, "gem/@/@");
  fname = fpath + 7;

  for (i = 0; str = list[i]; i++)
  {
    strcpy(fname, str);
    mirror(fpath, 0);
  }

  /* itoc.註解: 到這裡以後，應該已有 FILM_MOVIE 篇 */

  /* --------------------------------------------------- */
  /* 載入動態看板					 */
  /* --------------------------------------------------- */

  /* itoc.註解: 動態看板及點歌本合計只有 MOVIE_MAX - FILM_MOVIE - 1 篇才會被收進 movie */

  sprintf(fpath, "gem/brd/%s/@/@note", BN_CAMERA);	/* 動態看板的群組檔案名稱應命名為 @note */
  do_gem(fpath);					/* 把 [note] 精華區收進 movie */

#ifdef HAVE_SONG_CAMERA
  brd_fpath(fpath, BN_KTV, FN_DIR);
  do_gem(fpath);					/* 把被點的歌收進 movie */
#endif

  /* --------------------------------------------------- */
  /* resolve shared memory				 */
  /* --------------------------------------------------- */

  fshm = (FCACHE *) shm_new(FILMSHM_KEY, sizeof(FCACHE));
  memcpy(fshm, &image, sizeof(image.shot) + tail);
  /* Thor.980805: 再加上 shot的部分 */
  fshm->shot[0] = number;	/* 總共有幾片 */

  /* printf("%d/%d films, %d/%d bytes\n", number, MOVIE_MAX, tail, MOVIE_SIZE); */

  do_today(fshm);

  exit(0);
}
