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
  新的 文章搜尋模式 可指定一 keyword, 列出所有keyword相關之文章列表

  在 tmp/ 下開 xpost.{pid} 作為 folder, 另建一map陣列, 用作與原 post 作 map
  記載該文章是在原 post 的何處, 如此可作 mark, gem, edit, title 等功能,
  且能離開時回至對應文章處
  <以上想法 obsolete...>

  Thor.980510:
  建立文章討論串, like tin, 將文章串 index 放入 memory 中,
  不使用 thread, 因為 thread要用 folder 檔...

  分為兩種 Mode, Title & post list

  但考慮提供簡化的 上下鍵移動..

  O->O->O->...
  |  |  |
  o  o  o
  |  |  |

  index含field {next, text} 均為int, 配置也用 int
  第一層 sorted by title, 插入時用 binary search
  且 MMAP only , 第一層顯示 # and +

  不提供任何區段刪除動作, 避免混亂
-------------------------------------------------------------------------*/


/* ----------------------------------------------------- */
/* 串列搜尋主程式					 */
/* ----------------------------------------------------- */


#define MSG_XYDENY	"請先退出使用 ^Z 以前的串接/新聞功\能"

extern KeyFunc xpost_cb[];
extern KeyFunc xmbox_cb[];

static int *xypostI;		/* Thor: first ypost pos in ypost_xo.key */
static int comebackPos;		/* Thor: first xpost pos in xpost_xo.key */


static char xypostHintword[TTLEN + 1];
static char xypostHintauthor[IDLEN + 1];


static int
XoXpost(xo, hdr, on, off, fchk)		/* Thor: eXtended post : call from post_cb */
  XO *xo;
  HDR *hdr;		/* 搜尋的條件 */
  int on, off;		/* 搜尋的範圍 */
  int (*fchk) ();	/* 搜尋的函式 */
{
  int *xlist, fsize, max, locus;
  char *fimage;
  HDR *head, *tail;
  XO *xt;
#ifdef HAVE_XYNEWS
  int returnPos;
#endif

  if ((max = xo->max) <= 0)	/* Thor.980911: 註解: 以防萬一 */
    return XO_FOOT;
  
  /* build index according to input condition */

  fimage = f_map(xo->dir, &fsize);

  if (fimage == (char *) -1)
  {
    vmsg("目前無法開啟索引檔");
    return XO_FOOT;
  }

  /* allocate index memory, remember free first */

  /* Thor.990113: 怕問title, author的瞬間又有人post */
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
    xo->pos = returnPos;	/* 從 XZ_XPOST 回到 XZ_NEWS 游標移去原來的地方 */
  else
#endif
    xo->pos = comebackPos;	/* 從 XZ_XPOST 回到 XZ_POST 游標移去所選取文章的真正位置 */

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
  /* 搜尋作者/標題					 */
  /* --------------------------------------------------- */


static int			/* 0:不滿足條件  !=0:滿足條件 */
filter_select(head, hdr)
  HDR *head;	/* 待測物 */
  HDR *hdr;	/* 條件 */
{
  char *title;

  /* 借用 hdr->xid 當 strlen(hdr->owner) */

  /* Thor.981109: 特別注意，為了降低 load，author 是從頭 match，不是 substr match */
  if (hdr->xid && str_ncmp(head->owner, hdr->owner, hdr->xid))
    return 0;

  if (hdr->title[0])
  {
    title = head->title;
    if (STR4(title) == STR4(STR_REPLY))	/* Thor.980911: 先把 Re: 除外 */
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

  if (xz[XZ_XPOST - XO_ZONE].xo)	/* itoc.020308: 不得累積進入二次 */
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
  /* 搜尋作者						 */
  /* --------------------------------------------------- */


int
XoXauthor(xo)
  XO *xo;
{
  HDR hdr;
  char *author;

  if (xz[XZ_XPOST - XO_ZONE].xo)	/* itoc.020308: 不得累積進入二次 */
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
  /* 搜尋標題						 */
  /* --------------------------------------------------- */


int
XoXtitle(xo)
  XO *xo;
{
  HDR hdr;
  char *title;

  if (xz[XZ_XPOST - XO_ZONE].xo)	/* itoc.020308: 不得累積進入二次 */
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
  /* 搜尋相同標題					 */
  /* --------------------------------------------------- */


static int			/* 0:不滿足條件  !=0:滿足條件 */
filter_search(head, hdr)
  HDR *head;	/* 待測物 */
  HDR *hdr;	/* 條件 */
{
  char *title, buf[TTLEN + 1];

  title = head->title;
  if (STR4(title) == STR4(STR_REPLY))	/* Thor.980911: 先把 Re: 除外 */
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

  if (xz[XZ_XPOST - XO_ZONE].xo)	/* itoc.020308: 不得累積進入二次 */
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
  /* 全文搜尋						 */
  /* --------------------------------------------------- */


static char *search_folder;
static int search_fit;		/* >=0:找到幾篇 -1:中斷搜尋 */
static int search_all;		/* 已搜尋幾篇 */

static int			/* 0:不滿足條件  !=0:滿足條件 */
filter_full(head, hdr)
  HDR *head;	/* 待測物 */
  HDR *hdr;	/* 條件 */
{
  char buf[80], *fimage;
  int rc, fsize;
  struct timeval tv = {0, 10};

  if (search_fit < 0)		/* 中斷搜尋 */
    return 0;

  if (search_all % 100 == 0)	/* 每 100 篇才報告一次進度 */
  {
    sprintf(buf, "目前找到 \033[1;33m%d / %d\033[m 篇，全文搜尋中\033[5m...\033[m按任意鍵中斷", 
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

  /* 使用者可以中斷搜尋 */
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

  if (xz[XZ_XPOST - XO_ZONE].xo)	/* itoc.020308: 不得累積進入二次 */
  {
    vmsg(MSG_XYDENY);
    return XO_FOOT;
  }

  /* input condition */

  key = hdr.title;
  if (!vget(b_lines, 0, "內文關鍵字：", key, 30, DOECHO))
    return XO_FOOT;

  vget(b_lines, 0, "[設定搜尋範圍] 起點：(Enter)從頭開始 ", ans, 6, DOECHO);
  if ((head = atoi(ans)) <= 0)
    head = 1; 

  vget(b_lines, 44, "終點：(Enter)找到最後 ", ans, 6, DOECHO);
  if ((tail = atoi(ans)) < head)
    tail = INT_MAX;

  head--;
  tail--;

  sprintf(xypostHintword, "[全文搜尋] %s", key);
  xypostHintauthor[0] = '\0';
  str_lowest(key, key);

  search_folder = xo->dir;
  search_fit = 0;
  search_all = 0;

  return XoXpost(xo, &hdr, head, tail, filter_full);
}


  /* --------------------------------------------------- */
  /* 搜尋 mark						 */
  /* --------------------------------------------------- */


static int			/* 0:不滿足條件  !=0:滿足條件 */
filter_mark(head, hdr)
  HDR *head;	/* 待測物 */
  HDR *hdr;	/* 條件 */
{
  return (head->xmode & POST_MARKED);
}


int
XoXmark(xo)
  XO *xo;
{
  if (xz[XZ_XPOST - XO_ZONE].xo)	/* itoc.020308: 不得累積進入二次 */
  {
    vmsg(MSG_XYDENY);
    return XO_FOOT;
  }

  strcpy(xypostHintword, "\033[1;33m所有 mark 文章\033[m");
  xypostHintauthor[0] = '\0';

  return XoXpost(xo, NULL, -1, INT_MAX, filter_mark);
}


  /* --------------------------------------------------- */
  /* 搜尋本地						 */
  /* --------------------------------------------------- */


static int			/* 0:不滿足條件  !=0:滿足條件 */
filter_local(head, hdr)
  HDR *head;	/* 待測物 */
  HDR *hdr;	/* 條件 */
{
  return !(head->xmode & POST_INCOME);
}


int
XoXlocal(xo)
  XO *xo;
{
  if (currbattr & BRD_NOTRAN)
  {
    vmsg("本板為不轉信板，全部都是本地文章");
    return XO_FOOT;
  }

  if (xz[XZ_XPOST - XO_ZONE].xo)	/* itoc.020308: 不得累積進入二次 */
  {
    vmsg(MSG_XYDENY);
    return XO_FOOT;
  }

  strcpy(xypostHintword, "\033[1;33m所有非轉進文章\033[m");
  xypostHintauthor[0] = '\0';

  return XoXpost(xo, NULL, -1, INT_MAX, filter_local);
}


/* ----------------------------------------------------- */
/* 串列搜尋界面						 */
/* ----------------------------------------------------- */


int
xpost_head(xo)
  XO *xo;
{
  vs_head("主題串列", xo->xyz);

  /* itoc.010323: 同時提示作者/主題 */
  outs("[串接系列] ");
  if (*xypostHintauthor)
    prints("作者：%-13s   ", xypostHintauthor);
  if (*xypostHintword)
    prints("標題：%.30s", xypostHintword);

  prints(NECKER_XPOST, d_cols, "", currbattr & BRD_NOSCORE ? "╳" : "○");

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
    if (pos >= num)	/* hightman.030528: 避免 .DIR 被刪減時，會沒有文章可以顯示 */
      continue;
    *hdr = fimage[pos];
    hdr->xid = pos;		/* 用 hdr->xid 來記錄其原先在看板中的 pos */
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
xpost_history(xo, fhdr)		/* 將 fhdr 這篇加入 brh */
  XO *xo;
  HDR *fhdr;
{
  time_t prev, chrono, next;
  int pos;
  char *dir;
  HDR buf;

  chrono = fhdr->chrono;
  if (!brh_unread(chrono))	/* 如果已在 brh 中，就無需動作 */
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

#if 0	/* itoc.010822: 不需要，在 XoXpost() 中已被剔除 */
#ifdef HAVE_REFUSEMARK
    xmode = hdr->xmode;
    if ((xmode & POST_RESTRICT) && 
      strcmp(hdr->owner, cuser.userid) && !(bbstate & STAT_BM))
      continue;
#endif
#endif

    hdr_fpath(fpath, dir, hdr);

    /* Thor.990204: 為考慮more 傳回值 */   
    if ((key = more(fpath, FOOTER_POST)) < 0)
      break;

    comebackPos = hdr->xid; 
    /* Thor.980911: 從串接模式回來時要回到看過的那篇文章位置 */

    xpost_history(xo, hdr);
    strcpy(currtitle, str_ttl(hdr->title));

re_key:
    /* Thor.990204: 為考慮more 傳回值 */   
    if (!key)
      key = vkey();

    switch (key)
    {
    case KEY_UP:
    case KEY_PGUP:
    case '[':	/* itoc.000227: 串列搜尋中，有時想用 [ 看上一篇文章 */
    case 'k':	/* itoc.000227: 串列搜尋中，有時想用 k 看上一篇文章 */
      {
	int pos = xo->pos - 1;

	/* itoc.000227: 避免看過頭 */
	if (pos < 0)
	  return xpost_head(xo);

	xo->pos = pos;

	if (pos <= xo->top)
	  xypost_pick(xo);
  
	continue;
      }

    case KEY_DOWN:
    case KEY_PGDN:
    case ']':	/* Thor.990204: 串列搜尋中，有時想用 ] 看下一篇文章 */
    case 'j':	/* Thor.990204: 串列搜尋中，有時想用 j 看下一篇文章 */
    case ' ':
      {
	int pos = xo->pos + 1;

	/* Thor.980727: 修正看過頭的bug */

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
	if (do_reply(xo, hdr) == XO_INIT)	/* 有成功地 post 出去了 */
	  return xpost_init(xo);
      }
      break;

    case 'm': 
      if ((bbstate & STAT_BOARD) && !(hdr->xmode & POST_MARKED))
      {
	/* 在 xpost_browse 時看不到 m 記號，所以限制只能 mark */
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
      if (vget(b_lines, 0, "搜尋：", hunt, sizeof(hunt), DOECHO))
      {
	key = more(fpath, FOOTER_POST);
	goto re_key;
      }
      continue;

    case 'E':
      return post_edit(xo);

    case 'C':	/* itoc.000515: xpost_browse 時可存入暫存檔 */
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

    /* Thor.990204: 為考慮more 傳回值 */   
    if ((key = more(fpath, FOOTER_MAILER)) < 0)
      break;

    comebackPos = hdr->xid; 
    /* Thor.980911: 從串接模式回來時要回到看過的那篇文章位置 */

    strcpy(currtitle, str_ttl(hdr->title));

re_key:
    /* Thor.990204: 為考慮more 傳回值 */   
    if (!key)
      key = vkey();

    switch (key)
    {
    case KEY_UP:
    case KEY_PGUP:
    case '[':	/* itoc.000227: 串列搜尋中，有時想用 [ 看上一篇文章 */
    case 'k':	/* itoc.000227: 串列搜尋中，有時想用 k 看上一篇文章 */
      {
	int pos = xo->pos - 1;

	/* itoc.000227: 避免看過頭 */
	if (pos < 0)
	  return xpost_head(xo);

	xo->pos = pos;

	if (pos <= xo->top)
	  xypost_pick(xo);
  
	continue;
      }

    case KEY_DOWN:
    case KEY_PGDN:
    case ']':	/* Thor.990204: 串列搜尋中，有時想用 ] 看下一篇文章 */
    case 'j':	/* Thor.990204: 串列搜尋中，有時想用 j 看下一篇文章 */
    case ' ':
      {
	int pos = xo->pos + 1;

	/* Thor.980727: 修正看過頭的bug */

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
	/* 在 xmbox_browse 時看不到 m 記號，所以限制只能 mark */
	hdr->xmode ^= POST_MARKED;
	currchrono = hdr->chrono;
	rec_put(dir, hdr, sizeof(HDR), hdr->xid, cmpchrono);
      } 
      break;

    case '/':
      if (vget(b_lines, 0, "搜尋：", hunt, sizeof(hunt), DOECHO))
      {
	key = more(fpath, FOOTER_MAILER);
	goto re_key;
      }
      continue;

    case 'E':
      return mbox_edit(xo);

    case 'C':	/* itoc.000515: xmbox_browse 時可存入暫存檔 */
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


#ifdef HAVE_XYNEWS	/* itoc.010822: news 閱讀模式 */

#if 0	/* 構想 */

在大看板(連線看板)中常常有很多灌水文章，仿照 news 的閱讀方式，
把所有 reply 的文章都先隱藏，只顯示第一封發文。

第一輪利用 XoNews 來剔除 reply 文章
第二輪利用原有的 XoXsearch 搜尋同主題文章
如此就可以達到新聞閱讀模式的效果

#endif


static int *xynewsI;

extern KeyFunc news_cb[];


int
news_head(xo)
  XO *xo;
{
  vs_head("新聞閱\讀", xo->xyz);
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
    /* hdr->xid = pos; */	/* 在 XZ_NEWS 沒用到 xid，可以考慮保留給 reply 篇數 */
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

  if ((max = xo->max) <= 0)	/* Thor.980911: 註解: 以防萬一 */
    return XO_FOOT;

  /* itoc.020308: 不得累積進入二次 */
  if (xz[XZ_NEWS - XO_ZONE].xo)
  {
    vmsg(MSG_XYDENY);
    return XO_FOOT;
  }

  /* build index according to input condition */

  fimage = f_map(xo->dir, &fsize);

  if (fimage == (char *) -1)
  {
    vmsg("目前無法開啟索引檔");
    return XO_FOOT;
  }

  /* allocate index memory, remember free first */

  /* Thor.990113: 怕問title, author的瞬間又有人post */
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

    if (STR4(head->title) == STR4(STR_REPLY)) /* reply 的文章不要 */
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
