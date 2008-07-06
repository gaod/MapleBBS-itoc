/*-------------------------------------------------------*/
/* menu.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : menu/help/movie routines		 	 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern UCACHE *ushm;
extern FCACHE *fshm;


#ifndef ENHANCED_VISIT
extern time_t brd_visit[];
#endif


/* ----------------------------------------------------- */
/* ���} BBS ��						 */
/* ----------------------------------------------------- */


#define	FN_RUN_NOTE_PAD	"run/note.pad"
#define	FN_RUN_NOTE_TMP	"run/note.tmp"


typedef struct
{
  time_t tpad;
  char msg[400];
}      Pad;


int
pad_view()
{
  int fd, count;
  Pad *pad;

  if ((fd = open(FN_RUN_NOTE_PAD, O_RDONLY)) < 0)
    return XEASY;

  count = 0;
  mgets(-1);

  for (;;)
  {
    pad = mread(fd, sizeof(Pad));
    if (!pad)
    {
      vmsg(NULL);
      break;
    }
    else if (!(count % 5))	/* itoc.020122: �� pad �~�L */
    {
      clear();
      move(0, 23);
      prints("�i �� �� �W �� �d �� �O �j                �� %d ��\n\n", count / 5 + 1);
    }

    outs(pad->msg);
    count++;

    if (!(count % 5))
    {
      move(b_lines, 0);
      outs("�Ы� [SPACE] �~���[��A�Ϋ���L�䵲���G ");
      /* itoc.010127: �ץ��b�������k����ΤU�A������|�����G�h��檺���D */

      if (vkey() != ' ')
	break;
    }
  }

  close(fd);
  return 0;
}


static int
pad_draw()
{
  int i, cc, fdr, color;
  FILE *fpw;
  Pad pad;
  char *str, buf[3][71];

  /* itoc.����: ���Q�ΰ��m�סA�Ӫ� */
  static char pcolors[6] = {31, 32, 33, 34, 35, 36};

  /* itoc.010309: �d���O���Ѥ��P���C�� */
  color = vans("�߱��C�� 1) \033[41m  \033[m 2) \033[42m  \033[m 3) \033[43m  \033[m "
    "4) \033[44m  \033[m 5) \033[45m  \033[m 6) \033[46m  \033[m [Q] ");

  if (color < '1' || color > '6')
    return XEASY;
  else
    color -= '1';

  do
  {
    buf[0][0] = buf[1][0] = buf[2][0] = '\0';
    move(MENU_XPOS, 0);
    clrtobot();
    outs("\n�Яd�� (�ܦh�T��)�A��[Enter]����");
    for (i = 0; (i < 3) &&
      vget(16 + i, 0, "�G", buf[i], 71, DOECHO); i++);
    cc = vans("(S)�s���[�� (E)���s�ӹL (Q)��F�H[S] ");
    if (cc == 'q' || i == 0)
      return 0;
  } while (cc == 'e');

  time(&pad.tpad);

  /* itoc.020812.����: �睊�����ɭԭn�`�N struct Pad.msg[] �O�_���j */
  str = pad.msg;
  sprintf(str, "�~�t\033[1;46m %s �� %s \033[m�u", cuser.userid, cuser.username);

  for (cc = strlen(str); cc < 60; cc += 2)
    strcpy(str + cc, "�w");
  if (cc == 60)
    str[cc++] = ' ';

  sprintf(str + cc,
    "\033[1;44m %s \033[m��\n"
    "�x  \033[1;%dm%-70s\033[m  �x\n"
    "�x  \033[1;%dm%-70s\033[m  �x\n"
    "��  \033[1;%dm%-70s\033[m  ��\n",
    Btime(&pad.tpad),
    pcolors[color], buf[0],
    pcolors[color], buf[1],
    pcolors[color], buf[2]);

  f_cat(FN_RUN_NOTE_ALL, str);

  if (!(fpw = fopen(FN_RUN_NOTE_TMP, "w")))
    return 0;

  fwrite(&pad, sizeof(pad), 1, fpw);

  if ((fdr = open(FN_RUN_NOTE_PAD, O_RDONLY)) >= 0)
  {
    Pad *pp;

    i = 0;
    cc = pad.tpad - NOTE_DUE * 60 * 60;
    mgets(-1);
    while (pp = mread(fdr, sizeof(Pad)))
    {
      fwrite(pp, sizeof(Pad), 1, fpw);
      if ((++i > NOTE_MAX) || (pp->tpad < cc))
	break;
    }
    close(fdr);
  }

  fclose(fpw);

  rename(FN_RUN_NOTE_TMP, FN_RUN_NOTE_PAD);
  pad_view();
  return 0;
}


static int
goodbye()
{
  /* itoc.010803: �q�i�������� */
  clear();
  film_out(FILM_GOODBYE, 0);

  switch (vans("G)�H���ӳu M)���i���� N)�d���O Q)�����H[Q] "))
  {
  /* lkchu.990428: ���w�אּ������ */
  case 'g':
  case 'y':
    break;    

  case 'm':
    m_sysop();
    break;

  case 'n':
    /* if (cuser.userlevel) */
    if (HAS_PERM(PERM_POST)) /* Thor.990118: �n��post�~��d��, �������e */
      pad_draw();
    break;

  case 'q':
  default:
    /* return XEASY; */
    return 0;	/* itoc.010803: �q�F FILM_GOODBYE �n��ø */
  }

#ifdef LOG_BMW
  bmw_log();			/* lkchu.981201: ���y�O���B�z */
#endif

  if (!(cuser.ufo & UFO_MOTD))	/* itoc.000407: �����e���@��²�� */
  {  
    clear();
    prints("�˷R�� \033[32m%s(%s)\033[m�A�O�ѤF�A�ץ��{�i %s �j\n"
      "�H�U�O�z�b���������U��ơG\n",
      cuser.userid, cuser.username, str_site);
    acct_show(&cuser, 0);
    vmsg(NULL);
  }
  
  u_exit("EXIT ");
  exit(0);
}


/* ----------------------------------------------------- */
/* help & menu processring				 */
/* ----------------------------------------------------- */


void
vs_head(title, mid)
  char *title, *mid;
{
  char buf[(T_COLS - 1) - 79 + 69 + 1];		/* d_cols �̤j�i��O (T_COLS - 1) */
  char ttl[(T_COLS - 1) - 79 + 69 + 1];
  int spc, len;

  if (mid)	/* xxxx_head() ���O�� vs_head(title, str_site); */
  {
    clear();
  }
  else		/* menu() ���~�� vs_head(title, NULL); ��椤�L�� clear() */
  {
    move(0, 0);
    clrtoeol();
    mid = str_site;
  }

  len = d_cols + 69 - strlen(title) - strlen(currboard);	/* len: �����ٳѤU�h�����Ŷ� */

  if (HAS_STATUS(STATUS_BIFF))
  {
    mid = "\033[5;41m �l�t�ӫ��a�F \033[m";
    spc = 14;
  }
  else
  {
    if ((spc = strlen(mid)) > len)	/* �Ŷ������\�U�쥻�n�\�� mid�A�u�n�� mid �I�_ */
    {
      spc = len;
      memcpy(ttl, mid, spc);
      mid = ttl;
      mid[spc] = '\0';
    }
  }

  spc = 2 + len - spc;		/* �\�� mid �H��A�����٦� spc ��Ŷ��A�b mid ���k�U�� spc/2 �����ť� */
  len = 1 - spc & 1;
  memset(buf, ' ', spc >>= 1);
  buf[spc] = '\0';

#ifdef COLOR_HEADER
  spc = (time(0) % 7) + '1';
  prints("\033[1;4%cm�i%s�j%s\033[33m%s\033[1;37;4%cm%s�m%s�n\033[m\n",
    spc, title, buf, mid, spc, buf + len, currboard);
#else
  prints("\033[1;44m�i%s�j%s\033[33m%s\033[1;37;44m%s�m%s�n\033[m\n",
    title, buf, mid, buf + len, currboard);
#endif
}


/* ------------------------------------- */
/* �ʵe�B�z				 */
/* ------------------------------------- */


static char feeter[160];


/* itoc.010403: �� feeter �� status �W�ߥX�ӡA�w�ƨѨ�L function �s�� */

static void
status_foot()
{
  static int orig_flag = -1;
  static time_t uptime = -1;
  static int orig_money = -1;
  static int orig_gold = -1;
  static char flagmsg[16];
  static char coinmsg[20];

  int ufo;
  time_t now;

  ufo = cuser.ufo;
  time(&now);

  /* Thor: �P�� ��� �I�s�� �W���q�� ���� */

#ifdef HAVE_ALOHA
  ufo &= UFO_PAGER | UFO_ALOHA | UFO_CLOAK | UFO_QUIET;
  if (orig_flag != ufo)
  {
    orig_flag = ufo;
    sprintf(flagmsg,
      "%s%s%s%s",
      (ufo & UFO_PAGER) ? "��" : "�}",
      (ufo & UFO_ALOHA) ? "�W" : "  ",
      (ufo & UFO_QUIET) ? "�R" : "  ",
      (ufo & UFO_CLOAK) ? "��" : "  ");
  }
#else
  ufo &= UFO_PAGER | UFO_CLOAK | UFO_QUIET;
  if (orig_flag != ufo)
  {
    orig_flag = ufo;
    sprintf(flagmsg,
      "%s%s%s  ",
      (ufo & UFO_PAGER) ? "��" : "�}",
      (ufo & UFO_QUIET) ? "�R" : "  ",
      (ufo & UFO_CLOAK) ? "��" : "  ");
  }
#endif

  if (now > uptime)	/* �L�F�l�]�n��s�ͤ�X�� */
  {
    struct tm *ptime;

    ptime = localtime(&now);

    if (cuser.day == ptime->tm_mday && cuser.month == ptime->tm_mon + 1)
      cutmp->status |= STATUS_BIRTHDAY;
    else
      cutmp->status &= ~STATUS_BIRTHDAY;

    uptime = now + 86400 - ptime->tm_hour * 3600 - ptime->tm_min * 60 - ptime->tm_sec;
  }

  if (cuser.money != orig_money)
  {
    orig_money = cuser.money;
    sprintf(coinmsg, "��%4d%c", 
      (orig_money & 0x7FF00000) ? (orig_money >> 20) : (orig_money & 0x7FFFFC00) ? (orig_money >> 10) : orig_money, 
      (orig_money & 0x7FF00000) ? 'M' : (orig_money & 0x7FFFFC00) ? 'K' : ' ');
    coinmsg[7] = ' ';
  }
  if (cuser.gold != orig_gold)
  {
    orig_gold = cuser.gold;
    sprintf(coinmsg + 8, "��%4d%c ", 
      (orig_gold & 0x7FF00000) ? (orig_gold >> 20) : (orig_gold & 0x7FFFFC00) ? (orig_gold >> 10) : orig_gold, 
      (orig_gold & 0x7FF00000) ? 'M' : (orig_gold & 0x7FFFFC00) ? 'K' : ' ');
  }

  /* Thor.980913.����: �̱`���I�s status_foot() ���ɾ��O�C����s film�A�b 60 ��H�W�A
                       �G���ݰw�� hh:mm �ӯS�O�@�@�r���x�s�H�[�t */

  ufo = (now - (uptime - 86400)) / 60;	/* �ɥ� ufo �Ӱ��ɶ�(��) */

  /* itoc.010717: ��@�U feeter �Ϫ��שM FEETER_XXX �@�P */
  sprintf(feeter, COLOR1 " %8.8s %02d:%02d " COLOR2 " �H�� %-4d �ڬO %-12s %s [�I�s]%-9s ",
    fshm->today, ufo / 60, ufo % 60, total_user, cuser.userid, coinmsg, flagmsg);
  outf(feeter);
}


void
movie()
{
  /* Thor: it is depend on which state */

  if ((bbsmode <= M_XMENU) && (cuser.ufo & UFO_MOVIE))
    film_out(FILM_MOVIE, MENU_XNOTE);

  /* itoc.010403: �� feeter �� status �W�ߥX�� */
  status_foot();
}


typedef struct
{
  void *func;
  /* int (*func) (); */
  usint level;
  int umode;
  char *desc;
}      MENU;


#define	MENU_LOAD	1
#define	MENU_DRAW	2
#define	MENU_FILM	4


#define	PERM_MENU	PERM_PURGE


static MENU menu_main[];


/* ----------------------------------------------------- */
/* administrator's maintain menu			 */
/* ----------------------------------------------------- */


static MENU menu_admin[] =
{
  "bin/admutil.so:a_user", PERM_ALLACCT, - M_SYSTEM,
  "User       �� �U�ȸ�� ��",

  "bin/admutil.so:a_search", PERM_ALLACCT, - M_SYSTEM,
  "Hunt       �� �j�M���� ��",

  "bin/admutil.so:a_editbrd", PERM_ALLBOARD, - M_SYSTEM,
  "QSetBoard  �� �]�w�ݪO ��",

  "bin/innbbs.so:a_innbbs", PERM_ALLBOARD, - M_SYSTEM,
  "InnBBS     �� ��H�]�w ��",

#ifdef HAVE_REGISTER_FORM
  "bin/admutil.so:a_register", PERM_ALLREG, - M_SYSTEM,
  "Register   �� �f���U�� ��",

  "bin/admutil.so:a_regmerge", PERM_ALLREG, - M_SYSTEM,
  "Merge      �� �_��f�� ��",
#endif

  "bin/admutil.so:a_xfile", PERM_ALLADMIN, - M_XFILES,
  "Xfile      �� �t���ɮ� ��",
  
  "bin/admutil.so:a_resetsys", PERM_ALLADMIN, - M_SYSTEM,
  "BBSreset   �� ���m�t�� ��",

  "bin/admutil.so:a_restore", PERM_SYSOP, - M_SYSTEM,
  "TRestore   �� �٭�ƥ� ��",

  menu_main, PERM_MENU + Ctrl('A'), M_AMENU,
  "�t�κ��@"
};


/* ----------------------------------------------------- */
/* mail menu						 */
/* ----------------------------------------------------- */


static int
XoMbox()
{
  xover(XZ_MBOX);
  return 0;
}


static MENU menu_mail[] =
{
  XoMbox, PERM_BASIC, M_RMAIL,
  "Read       �u �\\Ū�H�� �t",

  m_send, PERM_LOCAL, M_SMAIL,
  "Mail       �u �����H�H �t",

#ifdef MULTI_MAIL  /* Thor.981009: ����R�����B�H */
  m_list, PERM_LOCAL, M_SMAIL,
  "List       �u �s�ձH�H �t",
#endif

  m_internet, PERM_INTERNET, M_SMAIL,
  "Internet   �u �H�̩f�� �t",

#ifdef HAVE_SIGNED_MAIL
  m_verify, 0, M_XMODE,
  "Verify     �u ���ҫH�� �t",
#endif

#ifdef HAVE_MAIL_ZIP
  m_zip, PERM_INTERNET, M_SMAIL,
  "Zip        �u ���]��� �t",
#endif

  m_sysop, 0, M_SMAIL,
  "Yes Sir!   �u ��ѯ��� �t",
  
  "bin/admutil.so:m_bm", PERM_ALLADMIN, - M_SMAIL,
  "BM All     �u �O�D�q�i �t",	/* itoc.000512: �s�W m_bm */
  
  "bin/admutil.so:m_all", PERM_ALLADMIN, - M_SMAIL,
  "User All   �u �����q�i �t",	/* itoc.000512: �s�W m_all */

  menu_main, PERM_MENU + Ctrl('A'), M_MMENU,	/* itoc.020829: �� guest �S�ﶵ */
  "�q�l�l��"
};


/* ----------------------------------------------------- */
/* talk menu						 */
/* ----------------------------------------------------- */


static int
XoUlist()
{
  xover(XZ_ULIST);
  return 0;
}


static MENU menu_talk[];


  /* --------------------------------------------------- */
  /* list menu						 */
  /* --------------------------------------------------- */

static MENU menu_list[] =
{
  t_pal, PERM_BASIC, M_PAL,
  "Pal        �� �B�ͦW�� ��",

#ifdef HAVE_LIST
  t_list, PERM_BASIC, M_PAL,
  "List       �� �S�O�W�� ��",
#endif

#ifdef HAVE_ALOHA
  "bin/aloha.so:t_aloha", PERM_PAGE, - M_PAL,
  "Aloha      �� �W���q�� ��",
#endif

#ifdef LOGIN_NOTIFY
  t_loginNotify, PERM_PAGE, M_PAL,
  "Notify     �� �t�Ψ�M ��",
#endif

  menu_talk, PERM_MENU + 'P', M_TMENU,
  "�U���W��"
};


static MENU menu_talk[] =
{
  XoUlist, 0, M_LUSERS,
  "Users      �� �C�ȦW�� ��",

  menu_list, PERM_BASIC, M_TMENU,
  "ListMenu   �� �]�w�W�� ��",

  t_pager, PERM_BASIC, M_XMODE,
  "Pager      �� �����I�s ��",

  t_cloak, PERM_CLOAK, M_XMODE,
  "Invis      �� �����K�k ��",

  t_query, 0, M_QUERY,
  "Query      �� �d�ߺ��� ��",

  t_talk, PERM_PAGE, M_PAGE,
  "Talk       �� ���ܺ��� ��",

  /* Thor.990220: ��ĥ~�� */
  "bin/chat.so:t_chat", PERM_CHAT, - M_CHAT,
  "ChatRoom   �� ���f��� ��",

  t_display, PERM_BASIC, M_BMW,
  "Display    �� �s�����y ��",

  t_bmw, PERM_BASIC, M_BMW,
  "Write      �� �^�U���y ��",

  menu_main, PERM_MENU + 'U', M_TMENU,
  "�𶢲��"
};


/* ----------------------------------------------------- */
/* user menu						 */
/* ----------------------------------------------------- */


static MENU menu_user[];


  /* --------------------------------------------------- */
  /* register menu                                      */
  /* --------------------------------------------------- */

static MENU menu_register[] =
{
  u_addr, PERM_BASIC, M_XMODE,
  "Address    �m �q�l�H�c �n",

#ifdef HAVE_REGISTER_FORM
  u_register, PERM_BASIC, M_UFILES,
  "Register   �m ����U�� �n",
#endif

#ifdef HAVE_REGKEY_CHECK
  u_verify, PERM_BASIC, M_UFILES,
  "Verify     �m ��{�ҽX �n",
#endif

  u_deny, PERM_BASIC, M_XMODE,
  "Perm       �m ��_�v�� �n",

  menu_user, PERM_MENU + 'A', M_UMENU,
  "���U���"
};


static MENU menu_user[] =
{
  u_info, PERM_BASIC, M_XMODE,
  "Info       �m �ӤH��� �n",

  u_setup, 0, M_UFILES,
  "Habit      �m �ߦn�Ҧ� �n",

  menu_register, PERM_BASIC, M_UMENU,
  "Register   �m ���U��� �n",

  pad_view, 0, M_READA,
  "Note       �m �[�ݯd�� �n",

  /* itoc.010309: ���������i�H�g�d���O */
  pad_draw, PERM_POST, M_POST,
  "Pad        �m �߱���~ �n",

  u_lock, PERM_BASIC, M_IDLE,
  "Lock       �m ��w�ù� �n",

  u_xfile, PERM_BASIC, M_UFILES,
  "Xfile      �m �ӤH�ɮ� �n",

  u_log, PERM_BASIC, M_UFILES,
  "ViewLog    �m �W���O�� �n",

  menu_main, PERM_MENU + 'H', M_UMENU,
  "�ӤH�]�w"
};


#ifdef HAVE_EXTERNAL

/* ----------------------------------------------------- */
/* tool menu						 */
/* ----------------------------------------------------- */


static MENU menu_tool[];


#ifdef HAVE_SONG
  /* --------------------------------------------------- */
  /* song menu						 */
  /* --------------------------------------------------- */

static MENU menu_song[] =
{
  "bin/song.so:XoSongLog", 0, - M_XMODE,
  "KTV        �� �I�q���� ��",

  "bin/song.so:XoSongMain", 0, - M_XMODE,
  "Book       �� �۩ұ��� ��",

  "bin/song.so:XoSongSub", 0, - M_XMODE,
  "Note       �� �q����Z ��",

  menu_tool, PERM_MENU + 'K', M_XMENU,
  "���I�q��"
};
#endif


#ifdef HAVE_GAME

#if 0

  itoc.010426.����:
  �q���C�����ν����סA�����a���n�����A�u�[���A������C

  itoc.010714.����:
  (a) �C�����C�����`��������b 1.01�A�@�ӱߤW���i�� 100 ���C���A
      �Y�N�`�]����J�h���C���A�h 1.01^100 = 2.7 ��/�C���@�ӱߤW�C
  (b) �Y�U�����v�������A�]�������b 1.0 ~ 1.02 �����A�����a�@�w���ȿ��A
      �B�Y�@����̰�����Ȫ����@���A�]���|�ȱo�L�����СC
  (c) ��h�W�A���v�V�C�̨��������� 1.02�A���v�����̨��������� 1.01�C

  itoc.011011.����:
  ���F�קK user multi-login ���Ӭ~���A
  �ҥH�b���C�����}�l�N�n�ˬd�O�_���� login �Y if (HAS_STATUS(STATUS_COINLOCK))�C

#endif

  /* --------------------------------------------------- */
  /* game menu						 */
  /* --------------------------------------------------- */

static MENU menu_game[];

static MENU menu_game1[] =
{
  "bin/liteon.so:main_liteon", 0, - M_GAME,
  "0LightOn   �� �ж��}�O ��",

  "bin/guessnum.so:guessNum", 0, - M_GAME,
  "1GuessNum  �� ���q�Ʀr ��",

  "bin/guessnum.so:fightNum", 0, - M_GAME,
  "2FightNum  �� ���q�Ʀr ��",

  "bin/km.so:main_km", 0, - M_GAME,
  "3KongMing  �� �թ����� ��",

  "bin/recall.so:main_recall", 0, - M_GAME,
  "4Recall    �� �^�Ф��Z ��",

  "bin/mine.so:main_mine", 0, - M_GAME,
  "5Mine      �� �ý�a�p ��",

  "bin/fantan.so:main_fantan", 0, - M_GAME, 
  "6Fantan    �� �f�u���s ��",

  "bin/dragon.so:main_dragon", 0, - M_GAME,
  "7Dragon    �� ���s�C�� ��",

  "bin/nine.so:main_nine", 0, - M_GAME,
  "8Nine      �� �Ѧa�E�E ��",

  menu_game, PERM_MENU + '0', M_XMENU,
  "�q���Ŷ�"
};

static MENU menu_game2[] =
{
  "bin/dice.so:main_dice", 0, - M_GAME,
  "0Dice      �� �g�Y��l ��",

  "bin/gp.so:main_gp", 0, - M_GAME,
  "1GoldPoker �� ���P���J ��",

  "bin/bj.so:main_bj", 0, - M_GAME,
  "2BlackJack �� �G�Q�@�I ��",

  "bin/chessmj.so:main_chessmj", 0, - M_GAME,
  "3ChessMJ   �� �H�ѳ±N ��",

  "bin/seven.so:main_seven", 0, - M_GAME,
  "4Seven     �� �䫰�C�i ��",
 
  "bin/race.so:main_race", 0, - M_GAME,
  "5Race      �� �i�ɰ��� ��",

  "bin/bingo.so:main_bingo", 0, - M_GAME,
  "6Bingo     �� ���G�j�� ��",

  "bin/marie.so:main_marie", 0, - M_GAME,
  "7Marie     �� �j�p���� ��",

  "bin/bar.so:main_bar", 0, - M_GAME,
  "8Bar       �� �a�x���� ��",

  menu_game, PERM_MENU + '0', M_XMENU,
  "�C���ֶ�"
};

static MENU menu_game3[] =
{
  "bin/pip.so:main_pip", PERM_BASIC, - M_GAME,
  "0Chicken   �� �q�l�p�� ��",

  "bin/pushbox.so:main_pushbox", 0, - M_GAME,
  "1PushBox   �� �ܮw�f�f ��",

  "bin/tetris.so:main_tetris", 0, - M_GAME,
  "2Tetris    �� �Xù���� ��",

  "bin/reversi.so:main_reversi", 0, - M_GAME,
  "3Reversi   �� �L�Ǥj�� ��",

  menu_game, PERM_MENU + '0', M_XMENU,
  "�Ϥ�S��"
};

static MENU menu_game[] =
{
  menu_game1, PERM_BASIC, M_XMENU,
  "1Game      �i �q���Ѱ� �j",

  menu_game2, PERM_BASIC, M_XMENU,
  "2Game      �i �C���ֶ� �j",

  menu_game3, PERM_BASIC, M_XMENU,
  "3Game      �i �Ϥ�S�� �j",

  menu_tool, PERM_MENU + '1', M_XMENU,
  "�C���H��"
};
#endif


#ifdef HAVE_BUY
  /* --------------------------------------------------- */
  /* buy menu						 */
  /* --------------------------------------------------- */

static MENU menu_buy[] =
{
  "bin/bank.so:x_bank", PERM_BASIC, - M_GAME,
  "Bank       �� �H�U�Ȧ� ��",

  "bin/bank.so:b_invis", PERM_BASIC, - M_GAME,
  "Invis      �� ���β{�� ��",

  "bin/bank.so:b_cloak", PERM_BASIC, - M_GAME,
  "Cloak      �� �L������ ��",

  "bin/bank.so:b_mbox", PERM_BASIC, - M_GAME,
  "Mbox       �� �H�c�L�� ��",

  "bin/bank.so:b_xempt", PERM_BASIC, - M_GAME,
  "Xempt      �� �ä[�O�d ��",

  menu_tool, PERM_MENU + 'B', M_XMENU,
  "���ĥ���"
};
#endif


  /* --------------------------------------------------- */
  /* other tools menu					 */
  /* --------------------------------------------------- */

static MENU menu_other[] =
{
  "bin/vote.so:vote_all", PERM_BASIC, - M_VOTE,	/* itoc.010414: �벼���� */
  "VoteAll    �� �벼���� ��",

#ifdef HAVE_TIP
  "bin/xyz.so:x_tip", 0, - M_READA,
  "Tip        �� �оǺ��F ��",
#endif

#ifdef HAVE_LOVELETTER
  "bin/xyz.so:x_loveletter", 0, - M_READA,
  "LoveLetter �� ���Ѽ��g ��",
#endif

  "bin/xyz.so:x_password", PERM_VALID, - M_XMODE,
  "Password   �� �ѰO�K�X ��",

#ifdef HAVE_CLASSTABLE
  "bin/classtable.so:main_classtable", PERM_BASIC, - M_XMODE,
  "ClassTable �� �\\�Үɬq ��",
#endif

#ifdef HAVE_CREDIT
  "bin/credit.so:main_credit", PERM_BASIC, - M_XMODE,
  "MoneyNote  �� �O�b�⥾ ��",
#endif

#ifdef HAVE_CALENDAR
  "bin/todo.so:main_todo", PERM_BASIC, - M_XMODE,
  "XTodo      �� �ӤH��{ ��",

  "bin/calendar.so:main_calendar", 0, - M_XMODE,
  "YCalendar  �� �U�~��� ��",
#endif

  menu_tool, PERM_MENU + Ctrl('A'), M_XMENU,	/* itoc.020829: �� guest �S�ﶵ */
  "��L�\\��"
};


static MENU menu_tool[] =
{
#ifdef HAVE_SONG
  menu_song, 0, M_XMENU,
  "KTV        �i �u���I�q �j",
#endif

#ifdef HAVE_COSIGN
  "bin/newbrd.so:XoNewBoard", PERM_VALID, - M_XMODE,
  "Join       �i �ݪO�s�p �j",
#endif

#ifdef HAVE_GAME
  menu_game, PERM_BASIC, M_XMENU,
  "Game       �i �C���H�� �j",
#endif

#ifdef HAVE_BUY
  menu_buy, PERM_BASIC, M_XMENU,
  "Market     �i ���ĥ��� �j",
#endif

  menu_other, 0, M_XMENU,
  "Other      �i ���C���K �j",

  menu_main, PERM_MENU + Ctrl('A'), M_XMENU,	/* itoc.020829: �� guest �S�ﶵ */
  "�ӤH�u��"
};

#endif	/* HAVE_EXTERNAL */


/* ----------------------------------------------------- */
/* main menu						 */
/* ----------------------------------------------------- */


static int
Gem()
{
  /* itoc.001109: �ݪO�`�ަb (A)nnounce �U�� GEM_X_BIT�A��K�}�O */
  XoGem("gem/"FN_DIR, "��اG�i��", (HAS_PERM(PERM_ALLBOARD) ? (GEM_W_BIT | GEM_X_BIT | GEM_M_BIT) : 0));
  return 0;
}


static MENU menu_main[] =
{
  menu_admin, PERM_ALLADMIN, M_AMENU,
  "0Admin    �X �t�κ��@�� �X",

  Gem, 0, M_GEM,
  "Announce  �i ��ؤ��G�� �i",

  Boards, 0, M_BOARD,
  "Boards    �[ �G�i�Q�װ� �[",

  Class, 0, M_BOARD,
  "Class     �p ���հQ�׶� �p",

#ifdef MY_FAVORITE
  MyFavorite, PERM_BASIC, M_MF,
  "Favorite  �b �ڪ��̷R�s �b",
#endif

  menu_mail, 0, M_MMENU, 
  "Mail      �g �H����ò� �g",

  menu_talk, 0, M_TMENU,
  "Talk      �s �𶢲�Ѧa �s",

  menu_user, 0, M_UMENU,
  "User      �k �ӤH�u��{ �k",

#ifdef HAVE_EXTERNAL
  menu_tool, 0, M_XMENU,
  "Xyz       �c �S��۫ݩ� �c",
#endif

#if 0	/* itoc.010209: ���� s �����i�J Select() ��ֿ����� */
  Select, 0, M_BOARD,
  "Select    �m ��ܥD�ݪO �m",
#endif

  goodbye, 0, M_XMODE,
  "Goodbye   �_ �U���A�|�a �_",

  NULL, PERM_MENU + 'B', M_0MENU,
  "�D�\\���"
};


void
menu()
{
  MENU *menu, *mptr, *table[12];
  usint level, mode;
  int cc, cx;			/* current / previous cursor position */
  int max, mmx;			/* current / previous menu max */
  int cmd, depth;
  char *str;

  mode = MENU_LOAD | MENU_DRAW | MENU_FILM;
  menu = menu_main;
  level = cuser.userlevel;
  depth = mmx = 0;

  for (;;)
  {
    if (mode & MENU_LOAD)
    {
      for (max = -1;; menu++)
      {
	cc = menu->level;
	if (cc & PERM_MENU)
	{

#ifdef	MENU_VERBOSE
	  if (max < 0)		/* �䤣��A�X�v�����\��A�^�W�@�h�\��� */
	  {
	    menu = (MENU *) menu->func;
	    continue;
	  }
#endif

	  break;
	}
	if (cc && !(cc & level))	/* �����v���~�q�X */
	  continue;

	table[++max] = menu;
      }

      if (mmx < max)
	mmx = max;

      if ((depth == 0) && HAS_STATUS(STATUS_BIFF))	/* �Ĥ@���W���Y���s�H�A�i�J Mail ��� */
	cmd = 'M';
      else
	cmd = cc ^ PERM_MENU;	/* default command */
      utmp_mode(menu->umode);
    }

    if (mode & MENU_DRAW)
    {
      if (mode & MENU_FILM)
      {
	clear();
	movie();
	cx = -1;
      }

      vs_head(menu->desc, NULL);

      mode = 0;
      do
      {
	move(MENU_XPOS + mode, MENU_YPOS + 2);
	if (mode <= max)
	{
	  mptr = table[mode];
	  str = mptr->desc;
	  prints("(\033[1;36m%c\033[m)", *str++);
	  outs(str);
	}
	clrtoeol();
      } while (++mode <= mmx);

      mmx = max;
      mode = 0;
    }

    switch (cmd)
    {
    case KEY_DOWN:
      cc = (cc == max) ? 0 : cc + 1;
      break;

    case KEY_UP:
      cc = (cc == 0) ? max : cc - 1;
      break;

    case Ctrl('A'):	/* itoc.020829: �w�]�ﶵ�Ĥ@�� */
    case KEY_HOME:
      cc = 0;
      break;

    case KEY_END:
      cc = max;
      break;

    case KEY_PGUP:
      cc = (cc == 0) ? max : 0;
      break;

    case KEY_PGDN:
      cc = (cc == max) ? 0 : max;
      break;

    case '\n':
    case KEY_RIGHT:
      mptr = table[cc];
      cmd = mptr->umode;
#if 1
     /* Thor.990212: dynamic load , with negative umode */
      if (cmd < 0)
      {
	void *p = DL_get(mptr->func);
	if (!p)
	  break;
	mptr->func = p;
	cmd = -cmd;
	mptr->umode = cmd;
      }
#endif
      utmp_mode(cmd);

      if (cmd <= M_XMENU)	/* �l�ؿ��� mode �n <= M_XMENU */
      {
	menu->level = PERM_MENU + mptr->desc[0];
	menu = (MENU *) mptr->func;

	mode = MENU_LOAD | MENU_DRAW;
	/* mode = MENU_LOAD | MENU_DRAW | MENU_FILM;	/* itoc.010304: �i�J�l��歫�� movie */

	depth++;
	continue;
      }

      {
	int (*func) ();

	func = mptr->func;
	mode = (*func) ();
      }

      utmp_mode(menu->umode);

      if (mode == XEASY)
      {
	outf(feeter);
	mode = 0;
      }
      else
      {
	mode = MENU_DRAW | MENU_FILM;
      }

      cmd = mptr->desc[0];
      continue;

#ifdef EVERY_Z
    case Ctrl('Z'):
      every_Z(0);
      goto every_key;

    case Ctrl('U'):
      every_U(0);
      goto every_key;
#endif

    /* itoc.010911: Select everywhere�A���A����O�b M_0MENU */
    case 's':
    case Ctrl('S'):
      utmp_mode(M_BOARD);
      Select();
      goto every_key;

#ifdef MY_FAVORITE
    /* itoc.010911: Favorite everywhere�A���A����O�b M_0MENU */
    case 'f':
    case Ctrl('F'):
      if (cuser.userlevel)	/* itoc.010407: �n�ˬd�v�� */
      {
	utmp_mode(M_MF);
	MyFavorite();
      }
      goto every_key;
#endif

    /* itoc.020301: Read currboard in M_0MENU */
    case 'r':
      if (bbsmode == M_0MENU)
      {
	if (currbno >= 0)
	{
	  utmp_mode(M_BOARD);
	  XoPost(currbno);
	  xover(XZ_POST);
#ifndef ENHANCED_VISIT
	  time(&brd_visit[currbno]);
#endif
	}
	goto every_key;
      }
      goto default_key;	/* �Y���b M_0MENU ���� r ���ܡA�n�����@����� */

every_key:	/* �S����B�z���� */
      utmp_mode(menu->umode);
      mode = MENU_DRAW | MENU_FILM;
      cmd = table[cc]->desc[0];
      continue;

    case KEY_LEFT:
    case 'e':
      if (depth > 0)
      {
	menu->level = PERM_MENU + table[cc]->desc[0];
	menu = (MENU *) menu->func;
	mode = MENU_LOAD | MENU_DRAW;
	/* mode = MENU_LOAD | MENU_DRAW | MENU_FILM;	/* itoc.010304: �h�X�l��歫�� movie */
	depth--;
	continue;
      }
      cmd = 'G';

default_key:
    default:

      if (cmd >= 'a' && cmd <= 'z')
	cmd ^= 0x20;			/* �ܤj�g */

      cc = 0;
      for (;;)
      {
	if (table[cc]->desc[0] == cmd)
	  break;
	if (++cc > max)
	{
	  cc = cx;
	  goto menu_key;
	}
      }
    }

    if (cc != cx)	/* �Y��в��ʦ�m */
    {
#ifdef CURSOR_BAR
      if (cx >= 0)
      {
	move(MENU_XPOS + cx, MENU_YPOS);
	if (cx <= max)
	{
	  mptr = table[cx];
	  str = mptr->desc;
	  prints("  (\033[1;36m%c\033[m)%s ", *str, str + 1);
	}
	else
	{
	  outs("  ");
	}
      }
      move(MENU_XPOS + cc, MENU_YPOS);
      mptr = table[cc];
      str = mptr->desc;
      prints(COLOR4 "> (%c)%s \033[m", *str, str + 1);
      cx = cc;
#else		/* �S�� CURSOR_BAR */
      if (cx >= 0)
      {
	move(MENU_XPOS + cx, MENU_YPOS);
	outc(' ');
      }
      move(MENU_XPOS + cc, MENU_YPOS);
      outc('>');
      cx = cc;
#endif
    }
    else		/* �Y��Ъ���m�S���� */
    {
#ifdef CURSOR_BAR
      move(MENU_XPOS + cc, MENU_YPOS);
      mptr = table[cc];
      str = mptr->desc;
      prints(COLOR4 "> (%c)%s \033[m", *str, str + 1);
#else
      move(MENU_XPOS + cc, MENU_YPOS + 1);
#endif
    }

menu_key:

    cmd = vkey();
  }
}
