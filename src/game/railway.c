/*-------------------------------------------------------*/
/* railway.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : 台鐵火車時刻表查詢				 */
/* create : 02/05/29					 */
/* update :   /  /  					 */
/* author : lp@micro.ee.nthu.edu.tw			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_NETTOOL

#define mouts(x,y,s)	{ move(x, y); outs(s); }


#define HTTP_PORT	80
#define SERVER_railway	"train2.twtraffic.com.tw"
#define CGI_railway	"/cgi-bin/taimain_tr4.fpl"
#define FORM_railway	"day=%s&from_station=%s&to_station=%s&from_time=%s&to_time=%s&ttime=%s&typer=%s&transit=%s&tr_station=%s"
#define REF		"http://www.railway.gov.tw/"

#define RAILWAY_XPOS	1
#define RAILWAY_YPOS	0
#define MAX_SITE	215

static char *siteid[MAX_SITE] =	/* 車站站台 */
{
  "台東", "山里", "鹿野", "瑞源", "瑞和", "月美", "關山", "海端", "池上", "富里", "東竹", "東里", "安通", 
  "玉里", "三民", "瑞穗", "瑞北", "富源", "大富", "光復", "萬榮", "鳳林", "南平", "溪口", "豐田", "壽豐", 
  "平和", "志學", "吉安", "花蓮", "北埔", "景美", "新城", "崇德", "和仁", "和平", "漢本", "武塔", "南澳", 
  "東澳", "永樂", "永春", "蘇澳","蘇澳新","新馬", "冬山", "羅東", "中里", "二結", "宜蘭", "四城", "礁溪", 
  "頂埔", "頭城", "外澳", "龜山", "大溪", "大里", "石城", "福隆", "貢寮", "雙溪", "牡丹","三貂嶺","侯硐", 
  "瑞芳","四腳亭","暖暖", "基隆", "三坑", "八堵", "七堵", "五堵", "汐止", "南港", "松山", "台北", "萬華", 
  "板橋", "樹林", "山佳", "鶯歌", "桃園", "內壢", "中壢", "埔心", "楊梅", "富岡", "湖口", "新豐", "竹北", 
  "新竹", "香山", "崎頂", "竹南", "談文", "大山", "後龍", "龍港","白沙屯","新埔", "通霄", "苑裡", "日南", 
  "大甲","台中港","清水", "沙鹿", "龍井", "大肚", "追分", "造橋", "豐富", "苗栗", "南勢", "銅鑼", "三義", 
  "泰安", "后里", "豐原", "潭子", "太原", "台中", "大慶", "烏日", "成功\","彰化", "花壇", "員林", "永靖", 
  "社頭", "田中", "二水", "林內", "石榴", "斗六", "斗南", "石龜", "大林", "民雄", "嘉義", "水上", "南靖", 
  "後壁", "新營", "柳營","林鳳營","隆田", "拔林", "善化", "新市", "永康", "大橋", "台南", "保安", "中洲", 
  "大湖", "路竹", "岡山", "橋頭", "楠梓", "左營", "高雄", "鳳山","後庄","九曲堂","六塊厝","屏東", "歸來", 
  "麟洛", "西勢", "竹田", "潮州", "崁頂", "南州", "鎮安", "林邊", "佳冬", "東海", "枋寮", "加祿", "內獅", 
  "枋山", "枋野", "中央", "古莊", "大武", "瀧溪", "多良", "金崙","太麻里","知本", "康樂", "大華", "十分", 
  "望古", "嶺腳", "平溪", "菁桐", "深澳", "竹中", "上員", "榮華", "竹東", "橫山","九讚頭","合興", "南河", 
  "內灣", "源泉", "濁水", "龍泉", "集集", "水里", "車埕"
};


static char *siteno[MAX_SITE] =	/* 車站代碼 */
{
  "004",  "006",  "008",  "009",  "010",  "011",  "012",  "014",  "015",  "018",  "020",  "022",  "023", 
  "025",  "027",  "029",  "030",  "031",  "032",  "034",  "035",  "036",  "037",  "039",  "040",  "041", 
  "042",  "043",  "045",  "051",  "052",  "053",  "054",  "055",  "056",  "057",  "058",  "060",  "062", 
  "063",  "064",  "065",  "066",  "067",  "068",  "069",  "070",  "071",  "072",  "073",  "074",  "075", 
  "076",  "077",  "078",  "079",  "080",  "081",  "082",  "083",  "084",  "085",  "086",  "087",  "088", 
  "089",  "090",  "091",  "092",  "093",  "094",  "095",  "096",  "097",  "098",  "100",  "101",  "102", 
  "103",  "104",  "105",  "106",  "107",  "108",  "109",  "110",  "111",  "112",  "113",  "114",  "115", 
  "116",  "117",  "118",  "119",  "120",  "121",  "122",  "123",  "124",  "125",  "126",  "127",  "128", 
  "129",  "130",  "131",  "132",  "133",  "134",  "135",  "136",  "137",  "138",  "139",  "140",  "142", 
  "143",  "144",  "145",  "146",  "147",  "148",  "149",  "150",  "151",  "152",  "153",  "154",  "155", 
  "156",  "157",  "158",  "159",  "160",  "161",  "162",  "163",  "164",  "165",  "166",  "167",  "168", 
  "169",  "170",  "171",  "172",  "173",  "174",  "175",  "176",  "177",  "178",  "179",  "180",  "181", 
  "182",  "183",  "184",  "185",  "186",  "187",  "188",  "189",  "190",  "191",  "192",  "193",  "194", 
  "195",  "196",  "197",  "198",  "199",  "200",  "201",  "202",  "203",  "204",  "205",  "206",  "207", 
  "208",  "209",  "210",  "211",  "213",  "214",  "215",  "217",  "219",  "220",  "221",  "222",  "223", 
  "224",  "225",  "226",  "227",  "228",  "229",  "230",  "231",  "232",  "233",  "234",  "235",  "236", 
  "237",  "238",  "239",  "240",  "241",  "242",  "243"
};


static int cx, cy;
static int fr_pos, to_pos, tr_pos;
static char fr_no[4], to_no[4], tr_no[4];	/* 起迄轉 代碼 */
static char fr_tm[3], to_tm[3];			/* 起迄轉 時刻 */
static char ttime[6];				/* 啟程/到達時間 */
static char typer[5];				/* 對號/非對號 */
static char transit[9];				/* 轉車/直達 */


static void
outs_site(pos)
  int pos;
{
  char *site;

  site = siteid[pos];
  if (site[4] == '\0')
    prints(" %s ", site);
  else
    outs(site);
}


static void
outs_info(color, msg)
  int color;
  char *msg;		/* strlen(msg) == 14 */
{
  move(b_lines, 0);
  clrtoeol();
  move(b_lines, 30);	/* 置中 */
  prints("\033[%dm %s \033[m", color, msg);
}


static void
site_drawrow(x)		/* 重繪 x 列 */
  int x;
{
  int i;

  move(x + RAILWAY_XPOS, RAILWAY_YPOS);
  i = x * 13;
  x = (x + 1) * 13;
  if (x > MAX_SITE)
    x = MAX_SITE;
  for (; i < x; i++)
  {
    if (i == fr_pos)
      outs("\033[41m");
    else if (i == to_pos)
      outs("\033[44m");
    else if (i == tr_pos)
      outs("\033[42m");
    outs_site(i);
    if (i == fr_pos || i == to_pos || i == tr_pos)
      outs("\033[m");
  }
}


static void
site_show()
{
  int i;

  clear();
  move(0, 23);
  outs("\033[1;37;44m◎ 台鐵火車時刻表查詢系統 ◎\033[m");

  for (i = 0; i < MAX_SITE; i++)
  {
    if (i % 13 == 0)
      move(i / 13 + RAILWAY_XPOS, RAILWAY_YPOS);
    outs_site(i);
  }

  move(b_lines - 4, 0);
  outs("啟程車站：         到達車站：         轉車車站：");
  move(b_lines - 3, 0);
  outs("查詢時間：00:00 至 24:00 1)開車 2)到達 ");
  move(b_lines - 2, 0);
  outs("查詢車種：1)對號快車 2)非對號車 3)全部車種      1)直達 2)轉車");

  cx = cy = 0;
  fr_pos = to_pos = tr_pos = -1;
  tr_no[0] = '\0';
}


static int		/* 1:成功 */
site_choose(flag)
  int flag;		/* 1:選起站  2:選迄站  4: 選轉站 */
{
  int pos;

  if (cx == 16 && cy >= 7)	/* 這幾格沒有站台 */
    return 0;

  pos = cx * 13 + cy;

  if (flag == 1)
  {
    fr_pos = pos;
    strcpy(fr_no, siteno[fr_pos]);
    move(b_lines - 4, 10);
    prints("%-6s", siteid[fr_pos]);
  }
  else if (flag == 4)
  {
    if (fr_pos == pos)
      return 0;
    to_pos = pos;
    strcpy(to_no, siteno[to_pos]);
    move(b_lines - 4, 29);
    outs(siteid[to_pos]);
  }
  else /* if (flag == 2) */
  {
    if (fr_pos == pos || to_pos == pos)
      return 0;
    tr_pos = pos;
    strcpy(tr_no, siteno[tr_pos]);
    move(b_lines - 4, 48);
    outs(siteid[tr_pos]);
  }

  site_drawrow(cx);
  return 1;	/* 選下一個 */
}


static int		/* 1: 已決定起迄站 */
site_menu(flag)		/* 目前在選 1:起 4:迄 2:轉 站 */
  int flag;
{
  for (;;)
  {
    /* itoc.031209: 若遇到三個字的站名，又偵測全形時，那麼會多跳一次，暫時不管了 :p */
    move(cx + RAILWAY_XPOS, cy * 6 + RAILWAY_YPOS + 2);
    switch (vkey())
    {
    case KEY_RIGHT:
      cy++;
      if (cy > 12)
	cy = 0;
      break;

    case KEY_LEFT:
      cy--;
      if (cy < 0)
	cy = 12;
      break;

    case KEY_DOWN:
      cx++;
      if (cx > 16)
	cx = 0;
      break;

    case KEY_UP:
      cx--;
      if (cx < 0)
	cx = 16;
      break;

    case '\n':
    case ' ':
      if (site_choose(flag))
	return 1;
      break;

    case 'q':
      return 0;
    }
  }
}


static void
time_query()
{
  char ans[3];

  outs_info(46, "請輸入查詢時間");
  if (!vget(b_lines - 3, 0, "查詢時間：", fr_tm, 3, DOECHO))
  {
    strcpy(fr_tm, "0");
    move(b_lines - 3, 10);
    outs("00");
  }
  move(b_lines - 3, 12);
  outs(":00 ");

  if (!vget(b_lines - 3, 16, "至 ", to_tm, 3, DOECHO))
  {
    strcpy(to_tm, "24");
    move(b_lines - 3, 19);
    outs("24");
  }
  move(b_lines - 3, 21);
  outs(":00 ");

  if (vget(b_lines - 3, 25, "之間 1)開車 2)到達：[1] ", ans, 3, DOECHO) != '2')
  {
    strcpy(ttime, "start");
    move(b_lines - 3, 37);
    clrtoeol();
  }
  else
  {
    strcpy(ttime, "arriv");
    move(b_lines - 3, 30);
    clrtoeol();
    outs("2)到達");
  }
}


static int		/* 1:轉車  0:直達 */
rail_query()
{
  char ans[3], *msg;

  outs_info(45, "請選擇查詢車種");

  switch (vget(b_lines - 2, 0, "查詢車種：1)對號快車 2)非對號車 3)全部車種：[3] ", ans, 3, DOECHO))
  {
  case '1':
    strcpy(typer, "fast");
    msg = "1)對號快車";
    break;

  case '2':
    strcpy(typer, "slow");
    msg = "2)非對號車";
    break;

  default:
    strcpy(typer, "all");
    msg = "3)全部車種";
    break;
  }

  move(b_lines - 2, 10);
  outs(msg);
  clrtoeol();

  refresh();	/* 再次 vget() 會出現前面的控制碼，要 refresh */

  if (vget(b_lines - 2, 48, "1)直達 2)轉車：[1] ", ans, 3, DOECHO) != '2')
  {
    strcpy(transit, "direct");
    tr_no[0] = '\0';
    move(b_lines - 2, 55);
    clrtoeol();
    return 0;
  }
  strcpy(transit, "undirect");
  move(b_lines - 2, 48);
  clrtoeol();
  outs("2)轉車");
  return 1;
}


static char
Tcolor(str)		/* 依車次來決定顏色 */
  char *str;
{
  if (!strncmp(str, "自強", 4))
    return '1';
  if (!strncmp(str, "莒光", 4))
    return '6';
  if (!strncmp(str, "復興", 4))
    return '4';
  return '0';
}


static void
html_parser(src, dst, undirect)
  char *src, *dst;
  int undirect;		/* 1: 轉車 */
{
  FILE *fpr, *fpw;
  char tag[256], data[256];
  int table_num, tag_count, in_table, flag;
  struct stat st;

  if ((!stat(src, &st) && !st.st_size) || !(fpr = fopen(src, "r")))
    return;

  fpw = fopen(dst, "w");

  table_num = tag_count = in_table = 0;
  flag = 1;

  while (str_cmp(tag, "/html"))
  {
    fscanf(fpr, "<%[^>]", tag);
    fscanf(fpr, "%[^<]", data);

    if (!str_ncmp(tag, "table", 5))
    {
      table_num++;
      in_table = 1;
    }
    else if (!str_ncmp(tag, "/table", 6))
    {
      in_table = 0;
    }

    if (table_num == 1)
    {
      tag_count++;
      if (tag_count == 3)
	fprintf(fpw, "──────────────────────────────────────\n   %s", data + 1);

      if (tag_count == 4 || tag_count == 13 || tag_count == 15)		/* 座車、乘車時間 */
	fprintf(fpw, "\033[45m%s\033[m", data + 1);

      if (tag_count == 6 || tag_count == 10)				/* 車站 */
	fprintf(fpw, "\033[44m%s\033[m", data + 1);

      if (tag_count == 5 || tag_count == 8 || tag_count == 12 || tag_count == 14)
	fprintf(fpw, "%s", data + 1);

      if (tag_count == 16)
	fprintf(fpw, "%s\n──────────────────────────────────────", data + 1);
    }
    else if (in_table && (table_num == 2 || (table_num == 3 && undirect)))
    {
      if (!str_ncmp(tag, "img", 3))
	continue;

      tag_count++;

      if (flag < table_num) /* 第一次進 table 才印。如果有轉車，再印一次 */
      {
	flag++;

	if (undirect)
	{
	  if (table_num == 3)
	  {
	    fprintf(fpw, "\n└───┴──┴─┴───┴─┴───┴───┴───┴──┴───────┘");
	    fprintf(fpw, "\n\n            有下列轉乘班車 (\033[42m%s\033[m-\033[44m%s\033[m) 可供您選擇\n",
	      siteid[tr_pos], siteid[to_pos]);
	  }
	  else
	  {
	    fprintf(fpw, "\n\n            有下列轉乘班車 (\033[41m%s\033[m-\033[42m%s\033[m) 可供您選擇\n",
	      siteid[fr_pos], siteid[tr_pos]);
	  }
	}

	fprintf(fpw, "\n"
	  "┌───┬──┬─┬───┬─┬───┬───┬───┬──┬───────┐\n"
	  "│ 乘坐 │車次│山│ 發車 │開│ 終點 │ 啟程 │ 到達 │全票│補充│\n"
	  "│ 車種 │編號│海│ 車站 │往│ 車站 │ 時間 │ 時間 │價錢│備註│");
      }


      if (!str_ncmp(tag, "a href", 2))	/* 車型，6 bytes */
      {
	fprintf(fpw, "\n├───┼──┼─┼───┼─┼───┼───┼───┼──┼───────┤");
	if (strlen(data) == 1 + 1 + 4)	/* '<' + ' ' + "站名" */
	  strcat(data, " ");
	fprintf(fpw, "\n│\033[1;4%c;37m%6.6s\033[m", Tcolor(data + 2), data + 2);	/* 冷氣柴客會超過 6 bytes */
	tag_count = 0;
      }

      if (tag_count == 4)	/* 車次代碼，4 bytes */
	fprintf(fpw, "│%4.4s", data + 1);		/* 冷氣柴客會超過 4 bytes */

      if (tag_count == 7)	/* 山線或海線或空的，2 bytes */
	fprintf(fpw, "│%2s", data + 1);

      if (tag_count == 10 || tag_count == 14)		/* 啟程站、到達站，6 bytes */
      {
	if (strlen(data) == 1 + 4)
	  strcat(data, " ");
	fprintf(fpw, "│%6s", data + 1);
      }

      if (tag_count == 12)	/* 至 */
	fprintf(fpw, "│至");

      if (tag_count == 16 || tag_count == 18)	/* 開車、到達時間 */
	fprintf(fpw, "│%6s", data + 1);

      if (tag_count == 20)	/* 全票價格 3 bytes */
	fprintf(fpw, "│%4s", data + 1);

      if (tag_count == 22)	/* 備註 */
	fprintf(fpw, "│%-14.14s│", data + 1);

    }		/* end of if (table_num) */
  }		/* end of while */

  if (table_num == 1)
    fprintf(fpw, "\n\n\n\t\t\t抱歉！沒有您欲搭乘的班車");
  else
    fprintf(fpw, "\n└───┴──┴─┴───┴─┴───┴───┴───┴──┴───────┘\n\n");

  fclose(fpr);
  fclose(fpw);
}


static inline int
data_ready(sockfd)
  int sockfd;
{
  static struct timeval tv = {1, 100};	/* Thor.980806: man page 假設 timeval struct是會改變的 */
  int cc;
  fd_set rset;

  for (;;)
  {
    struct timeval tmp_tv = tv;
    FD_ZERO(&rset);
    FD_SET(0, &rset);
    FD_SET(sockfd, &rset);
    cc = select(sockfd + 1, &rset, NULL, NULL, &tmp_tv);
    if (cc > 0)
    {
      if (FD_ISSET(0, &rset))
	return 0;	/* Thor.990401: user break */
      if (FD_ISSET(sockfd, &rset))
	return 1;	/* Thor.990401: data ready */
    }
  }
}


static int
http_conn(server, s)
  char *server;
  char *s;
{
  int sockfd, start_show;
  int cc, tlen;
  char *xhead, *xtail, src[30], dst[30];
  static char pool[2048];
  FILE *fp;

  mouts(b_lines - 1, 0, "連接伺服器中，請稍候.............");
  refresh();

  if ((sockfd = dns_open(server, HTTP_PORT)) < 0)
  {
    vmsg("無法與伺服器取得連結，查詢失敗");
    return 0;
  }

  write(sockfd, s, strlen(s));
  shutdown(sockfd, 1);

  /* parser return message from web server */
  xhead = pool;
  xtail = pool;
  tlen = 0;
  start_show = 0;

  sprintf(src, "tmp/%s.rail", cuser.userid);

  fp = fopen(src, "w");
  for (;;)
  {
    if (xhead >= xtail)
    {
      xhead = pool;
#if 1
      /* Thor.990401: 無聊一下 */
      if (!data_ready(sockfd))
      {
	close(sockfd);
	fclose(fp);
	unlink(src);
	return 0;
      }
#endif
      cc = read(sockfd, xhead, sizeof(pool));
      if (cc <= 0)
	break;
      xtail = xhead + cc;
    }

    cc = *xhead++;
    fputc(cc, fp);
  }

  close(sockfd);
  fclose(fp);

  sprintf(dst, "tmp/%s.way", cuser.userid);
  html_parser(src, dst, tr_pos >= 0);

  /* show message that return from the web server */

  if (more(dst, (char *) -1) >= 0)
  {
    if (vans("您要將查詢結果寄回信箱嗎？[N] ") == 'y')
      mail_self(dst, cuser.userid, "[備 忘 錄] 火車時刻查詢結果", MAIL_READ);
  }
  unlink(dst);
  unlink(src);

  return 0;
}



static void
railway(delay)
  int delay;
{
  char atrn[128], sendform[128];
  char day[20];
  struct tm *ptime;
  time_t now;

  time(&now);
  now += 86400 * delay;
  ptime = localtime(&now);
  sprintf(day, "%d%%20%02d/%d/%d星期%.2s", (delay + 1),
    ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday, "日一二三四五六" + (ptime->tm_wday << 1));

  sprintf(atrn, FORM_railway, day, fr_no, to_no, fr_tm, to_tm, ttime, typer, transit, tr_no);
  sprintf(sendform, "GET %s?%s", CGI_railway, atrn);
  http_conn(SERVER_railway, sendform);
}


int
main_railway()
{
  int ch;

  site_show();

  outs_info(41, "請選擇啟程車站");
  if (!site_menu(1))
    return 0;

  outs_info(44, "請選擇到達車站");
  if (!site_menu(4))
    return 0;

  time_query();		/* 決定啟程/到達時間 */

  if (rail_query())	/* 決定對號/直達/轉車 */
  {
    outs_info(42, "請選擇轉車車站");
    if (!site_menu(2))
      return 0;
  }

  ch = vans("查詢 0)今天 1)明天 2)後天 3456789)天後的時刻表 Q)離開？[0] ");
  if (ch == 'q')
  {
    vmsg("hmm.....不想查囉...^o^");
    return 0;
  }
  if (ch < '0' || ch > '9')
    ch = 0;
  else
    ch -= '0';

  railway(ch);

  return 0;
}
#endif	/* HAVE_NETTOOL */
