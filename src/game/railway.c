/*-------------------------------------------------------*/
/* railway.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : �x�K�����ɨ��d��				 */
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

static char *siteid[MAX_SITE] =	/* �������x */
{
  "�x�F", "�s��", "����", "�緽", "��M", "���", "���s", "����", "���W", "�I��", "�F��", "�F��", "�w�q", 
  "�ɨ�", "�T��", "���J", "��_", "�I��", "�j�I", "���_", "�U�a", "��L", "�n��", "�ˤf", "�ץ�", "����", 
  "���M", "�Ӿ�", "�N�w", "�Ὤ", "�_�H", "����", "�s��", "�R�w", "�M��", "�M��", "�~��", "�Z��", "�n�D", 
  "�F�D", "�ü�", "�ìK", "Ĭ�D","Ĭ�D�s","�s��", "�V�s", "ù�F", "����", "�G��", "�y��", "�|��", "�G��", 
  "���H", "�Y��", "�~�D", "�t�s", "�j��", "�j��", "�۫�", "�ֶ�", "�^�d", "����", "�d��","�T�I��","�Jֻ", 
  "���","�|�}�F","�x�x", "��", "�T�|", "�K��", "�C��", "����", "����", "�n��", "�Q�s", "�x�_", "�U��", 
  "�O��", "��L", "�s��", "�a�q", "���", "���c", "���c", "�H��", "����", "�I��", "��f", "�s��", "�˥_", 
  "�s��", "���s", "�T��", "�˫n", "�ͤ�", "�j�s", "���s", "�s��","�ըF��","�s�H", "�q�]", "�b��", "��n", 
  "�j��","�x����","�M��", "�F��", "�s��", "�j�{", "�l��", "�y��", "�״I", "�]��", "�n��", "���r", "�T�q", 
  "���w", "�Z��", "�׭�", "��l", "�ӭ�", "�x��", "�j�y", "�Q��", "���\\","����", "���", "���L", "�ùt", 
  "���Y", "�Ф�", "�G��", "�L��", "�ۺh", "�椻", "��n", "���t", "�j�L", "����", "�Ÿq", "���W", "�n�t", 
  "���", "�s��", "�h��","�L����","����", "�ުL", "����", "�s��", "�ñd", "�j��", "�x�n", "�O�w", "���w", 
  "�j��", "����", "���s", "���Y", "����", "����", "����", "��s","����","�E����","������","�̪F", "�k��", 
  "�ﬥ", "���", "�˥�", "��{", "�r��", "�n�{", "��w", "�L��", "�ΥV", "�F��", "�D�d", "�[�S", "����", 
  "�D�s", "�D��", "����", "�j��", "�j�Z", "�]��", "�h�}", "���[","�ӳ¨�","����", "�d��", "�j��", "�Q��", 
  "��j", "���}", "����", "�׮�", "�`�D", "�ˤ�", "�W��", "�a��", "�˪F", "��s","�E�g�Y","�X��", "�n�e", 
  "���W", "���u", "�B��", "�s�u", "����", "����", "���L"
};


static char *siteno[MAX_SITE] =	/* �����N�X */
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
static char fr_no[4], to_no[4], tr_no[4];	/* �_���� �N�X */
static char fr_tm[3], to_tm[3];			/* �_���� �ɨ� */
static char ttime[6];				/* �ҵ{/��F�ɶ� */
static char typer[5];				/* �︹/�D�︹ */
static char transit[9];				/* �ਮ/���F */


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
  move(b_lines, 30);	/* �m�� */
  prints("\033[%dm %s \033[m", color, msg);
}


static void
site_drawrow(x)		/* ��ø x �C */
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
  outs("\033[1;37;44m�� �x�K�����ɨ��d�ߨt�� ��\033[m");

  for (i = 0; i < MAX_SITE; i++)
  {
    if (i % 13 == 0)
      move(i / 13 + RAILWAY_XPOS, RAILWAY_YPOS);
    outs_site(i);
  }

  move(b_lines - 4, 0);
  outs("�ҵ{�����G         ��F�����G         �ਮ�����G");
  move(b_lines - 3, 0);
  outs("�d�߮ɶ��G00:00 �� 24:00 1)�}�� 2)��F ");
  move(b_lines - 2, 0);
  outs("�d�ߨ��ءG1)�︹�֨� 2)�D�︹�� 3)��������      1)���F 2)�ਮ");

  cx = cy = 0;
  fr_pos = to_pos = tr_pos = -1;
  tr_no[0] = '\0';
}


static int		/* 1:���\ */
site_choose(flag)
  int flag;		/* 1:��_��  2:�勤��  4: ���௸ */
{
  int pos;

  if (cx == 16 && cy >= 7)	/* �o�X��S�����x */
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
  return 1;	/* ��U�@�� */
}


static int		/* 1: �w�M�w�_���� */
site_menu(flag)		/* �ثe�b�� 1:�_ 4:�� 2:�� �� */
  int flag;
{
  for (;;)
  {
    /* itoc.031209: �Y�J��T�Ӧr�����W�A�S�������ήɡA����|�h���@���A�Ȯɤ��ޤF :p */
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

    case KEY_ENTER:
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

  outs_info(46, "�п�J�d�߮ɶ�");
  if (!vget(b_lines - 3, 0, "�d�߮ɶ��G", fr_tm, 3, DOECHO))
  {
    strcpy(fr_tm, "0");
    move(b_lines - 3, 10);
    outs("00");
  }
  move(b_lines - 3, 12);
  outs(":00 ");

  if (!vget(b_lines - 3, 16, "�� ", to_tm, 3, DOECHO))
  {
    strcpy(to_tm, "24");
    move(b_lines - 3, 19);
    outs("24");
  }
  move(b_lines - 3, 21);
  outs(":00 ");

  if (vget(b_lines - 3, 25, "���� 1)�}�� 2)��F�G[1] ", ans, 3, DOECHO) != '2')
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
    outs("2)��F");
  }
}


static int		/* 1:�ਮ  0:���F */
rail_query()
{
  char ans[3], *msg;

  outs_info(45, "�п�ܬd�ߨ���");

  switch (vget(b_lines - 2, 0, "�d�ߨ��ءG1)�︹�֨� 2)�D�︹�� 3)�������ءG[3] ", ans, 3, DOECHO))
  {
  case '1':
    strcpy(typer, "fast");
    msg = "1)�︹�֨�";
    break;

  case '2':
    strcpy(typer, "slow");
    msg = "2)�D�︹��";
    break;

  default:
    strcpy(typer, "all");
    msg = "3)��������";
    break;
  }

  move(b_lines - 2, 10);
  outs(msg);
  clrtoeol();

  refresh();	/* �A�� vget() �|�X�{�e��������X�A�n refresh */

  if (vget(b_lines - 2, 48, "1)���F 2)�ਮ�G[1] ", ans, 3, DOECHO) != '2')
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
  outs("2)�ਮ");
  return 1;
}


static char
Tcolor(str)		/* �̨����ӨM�w�C�� */
  char *str;
{
  if (!strncmp(str, "�۱j", 4))
    return '1';
  if (!strncmp(str, "����", 4))
    return '6';
  if (!strncmp(str, "�_��", 4))
    return '4';
  return '0';
}


static void
html_parser(src, dst, undirect)
  char *src, *dst;
  int undirect;		/* 1: �ਮ */
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
	fprintf(fpw, "�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w\n   %s", data + 1);

      if (tag_count == 4 || tag_count == 13 || tag_count == 15)		/* �y���B�����ɶ� */
	fprintf(fpw, "\033[45m%s\033[m", data + 1);

      if (tag_count == 6 || tag_count == 10)				/* ���� */
	fprintf(fpw, "\033[44m%s\033[m", data + 1);

      if (tag_count == 5 || tag_count == 8 || tag_count == 12 || tag_count == 14)
	fprintf(fpw, "%s", data + 1);

      if (tag_count == 16)
	fprintf(fpw, "%s\n�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w", data + 1);
    }
    else if (in_table && (table_num == 2 || (table_num == 3 && undirect)))
    {
      if (!str_ncmp(tag, "img", 3))
	continue;

      tag_count++;

      if (flag < table_num) /* �Ĥ@���i table �~�L�C�p�G���ਮ�A�A�L�@�� */
      {
	flag++;

	if (undirect)
	{
	  if (table_num == 3)
	  {
	    fprintf(fpw, "\n�|�w�w�w�r�w�w�r�w�r�w�w�w�r�w�r�w�w�w�r�w�w�w�r�w�w�w�r�w�w�r�w�w�w�w�w�w�w�}");
	    fprintf(fpw, "\n\n            ���U�C�୼�Z�� (\033[42m%s\033[m-\033[44m%s\033[m) �i�ѱz���\n",
	      siteid[tr_pos], siteid[to_pos]);
	  }
	  else
	  {
	    fprintf(fpw, "\n\n            ���U�C�୼�Z�� (\033[41m%s\033[m-\033[42m%s\033[m) �i�ѱz���\n",
	      siteid[fr_pos], siteid[tr_pos]);
	  }
	}

	fprintf(fpw, "\n"
	  "�z�w�w�w�s�w�w�s�w�s�w�w�w�s�w�s�w�w�w�s�w�w�w�s�w�w�w�s�w�w�s�w�w�w�w�w�w�w�{\n"
	  "�x ���� �x�����x�s�x �o�� �x�}�x ���I �x �ҵ{ �x ��F �x�����x�ɥR�x\n"
	  "�x ���� �x�s���x���x ���� �x���x ���� �x �ɶ� �x �ɶ� �x�����x�Ƶ��x");
      }


      if (!str_ncmp(tag, "a href", 2))	/* �����A6 bytes */
      {
	fprintf(fpw, "\n�u�w�w�w�q�w�w�q�w�q�w�w�w�q�w�q�w�w�w�q�w�w�w�q�w�w�w�q�w�w�q�w�w�w�w�w�w�w�t");
	if (strlen(data) == 1 + 1 + 4)	/* '<' + ' ' + "���W" */
	  strcat(data, " ");
	fprintf(fpw, "\n�x\033[1;4%c;37m%6.6s\033[m", Tcolor(data + 2), data + 2);	/* �N���ȷ|�W�L 6 bytes */
	tag_count = 0;
      }

      if (tag_count == 4)	/* �����N�X�A4 bytes */
	fprintf(fpw, "�x%4.4s", data + 1);		/* �N���ȷ|�W�L 4 bytes */

      if (tag_count == 7)	/* �s�u�ή��u�ΪŪ��A2 bytes */
	fprintf(fpw, "�x%2s", data + 1);

      if (tag_count == 10 || tag_count == 14)		/* �ҵ{���B��F���A6 bytes */
      {
	if (strlen(data) == 1 + 4)
	  strcat(data, " ");
	fprintf(fpw, "�x%6s", data + 1);
      }

      if (tag_count == 12)	/* �� */
	fprintf(fpw, "�x��");

      if (tag_count == 16 || tag_count == 18)	/* �}���B��F�ɶ� */
	fprintf(fpw, "�x%6s", data + 1);

      if (tag_count == 20)	/* �������� 3 bytes */
	fprintf(fpw, "�x%4s", data + 1);

      if (tag_count == 22)	/* �Ƶ� */
	fprintf(fpw, "�x%-14.14s�x", data + 1);

    }		/* end of if (table_num) */
  }		/* end of while */

  if (table_num == 1)
    fprintf(fpw, "\n\n\n\t\t\t��p�I�S���z���f�����Z��");
  else
    fprintf(fpw, "\n�|�w�w�w�r�w�w�r�w�r�w�w�w�r�w�r�w�w�w�r�w�w�w�r�w�w�w�r�w�w�r�w�w�w�w�w�w�w�}\n\n");

  fclose(fpr);
  fclose(fpw);
}


static inline int
data_ready(sockfd)
  int sockfd;
{
  static struct timeval tv = {1, 100};	/* Thor.980806: man page ���] timeval struct�O�|���ܪ� */
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

  mouts(b_lines - 1, 0, "�s�����A�����A�еy��.............");
  refresh();

  if ((sockfd = dns_open(server, HTTP_PORT)) < 0)
  {
    vmsg("�L�k�P���A�����o�s���A�d�ߥ���");
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
      /* Thor.990401: �L��@�U */
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
    if (vans("�z�n�N�d�ߵ��G�H�^�H�c�ܡH[N] ") == 'y')
      mail_self(dst, cuser.userid, "[�� �� ��] �����ɨ�d�ߵ��G", MAIL_READ);
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
  sprintf(day, "%d%%20%02d/%d/%d�P��%.2s", (delay + 1),
    ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday, "��@�G�T�|����" + (ptime->tm_wday << 1));

  sprintf(atrn, FORM_railway, day, fr_no, to_no, fr_tm, to_tm, ttime, typer, transit, tr_no);
  sprintf(sendform, "GET %s?%s", CGI_railway, atrn);
  http_conn(SERVER_railway, sendform);
}


int
main_railway()
{
  int ch;

  site_show();

  outs_info(41, "�п�ܱҵ{����");
  if (!site_menu(1))
    return 0;

  outs_info(44, "�п�ܨ�F����");
  if (!site_menu(4))
    return 0;

  time_query();		/* �M�w�ҵ{/��F�ɶ� */

  if (rail_query())	/* �M�w�︹/���F/�ਮ */
  {
    outs_info(42, "�п���ਮ����");
    if (!site_menu(2))
      return 0;
  }

  ch = vans("�d�� 0)���� 1)���� 2)��� 3456789)�ѫ᪺�ɨ�� Q)���}�H[0] ");
  if (ch == 'q')
  {
    vmsg("hmm.....���Q�d�o...^o^");
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
