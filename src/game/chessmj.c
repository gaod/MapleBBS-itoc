/*-------------------------------------------------------*/
/* chessmj.c      ( NTHU CS MapleBBS Ver 3.10 )          */
/*-------------------------------------------------------*/
/* target : 象棋麻將遊戲                                 */
/* create : 98/07/29                                     */
/* update : 01/04/25                                     */
/* author : weiren@mail.eki.com.tw                       */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_GAME


static int host_card[5];	/* 電腦(莊家) 的牌 */
static int guest_card[5];	/* 玩家的牌 */
static int throw[32];		/* 被丟棄的牌 */

static int flag;
static int tflag;
static int tflagA;
static int tflagB;
static int selftouch;		/* 自摸 */

static int cnum[32] = 
{
  1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
  7, 7, 7, 7, 7, 8, 9, 9, 10, 10,
  11, 11, 12, 12, 13, 13,
  14, 14, 14, 14, 14
};

static int group[32] = 
{
  1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 4, 4, 4, 4, 4,
  5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6
};


static void
out_song()
{
  static int count = 0;

  /* 蘇慧倫˙不要再說愛我 */
  uschar *msg[8] = 
  {
    "忘掉吧我的老情人  別在意也別說出來",
    " 時間已沖淡一切  心情也沒有季節  把過去放回你心中",
    "不要問我誰對誰錯  你的愛在我回憶中  這些年習慣自由",
    "也沒有太多煩憂  那過去就像一陣風",
    "不要再說愛我  不要再說愛我",
    "現在我們之間  只能當當朋友",
    "落下太多眼淚\  等過太多黑夜",
    "現在我一個人  愛情我不想問"
  };
  move(b_lines - 2, 0);
  prints("\033[1;3%dm%s\033[m  籌碼還有 %d 元", time(0) % 7, msg[count], cuser.money);
  clrtoeol();
  if (++count == 8)
    count = 0;
}


static void
print_Sign(x, y)
  int x, y;
{
  move(x, y);
  outs("╭●╮");
}


static void
print_Schess(card, x, y)
  int card, x, y;
{
  char *chess[33] = 
  {
    "帥", "仕", "仕", "相", "相", "硨", "硨", "傌", "傌", "炮", "炮",
    "兵", "兵", "兵", "兵", "兵",
    "將", "士", "士", "象", "象", "車", "車", "馬", "馬", "包", "包",
    "卒", "卒", "卒", "卒", "卒", "Ｘ"
  };

  move(x, y);
  outs("╭─╮");
  move(x + 1, y);
  prints("│%2s│", chess[card]);
  move(x + 2, y);
  outs("╰─╯");
}


static inline void
clear_5card()
{
  move(15, 24);
  outs("      ");
  move(16, 24);
  outs("      ");
  move(17, 24);
  outs("      ");
}


static inline void
Phost5()
{
  move(4, 24);
  outs("╭─╮");
  move(5, 24);
  outs("│  │");
  move(6, 24);
  outs("╰─╯");
}


static inline void
Chost5()
{
  move(4, 24);
  outs("      ");
  move(5, 24);
  outs("      ");
  move(6, 24);
  outs("      ");
}


static inline void
P_allchess()
{
  char *chess[33] = {
    "帥", "仕", "仕", "相", "相", "硨", "硨", "傌", "傌", "炮", "炮",
    "兵", "兵", "兵", "兵", "兵",
    "將", "士", "士", "象", "象", "車", "車", "馬", "馬", "包", "包",
    "卒", "卒", "卒", "卒", "卒", ""
  };

  int i;

  for (i = 0; i < 4; i++)
  {
    move(16, 2 + 6 * i);
    outs(chess[guest_card[i]]);
  }
}


static inline void
sortchess()
{
  int i, j, x;
  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < (3 - i); j++)
    {
      if (guest_card[j] > guest_card[j + 1])
      {
	x = guest_card[j];
	guest_card[j] = guest_card[j + 1];
	guest_card[j + 1] = x;
      }
      if (host_card[j] > host_card[j + 1])
      {
	x = host_card[j];
	host_card[j] = host_card[j + 1];
	host_card[j + 1] = x;
      }
    }
  }
}


static int
testpair(a, b)
  int a, b;
{
  if (cnum[a] == cnum[b])
    return 1;
  if (cnum[a] == 1 && cnum[b] == 8)
    return 1;
  if (cnum[a] == 8 && cnum[b] == 1)
    return 1;
  return 0;
}


static int
testthree(a, b, c)
  int a, b, c;
{
  int tmp;
  if (a > b)
  {
    tmp = a;
    a = b;
    b = tmp;
  }
  if (b > c)
  {
    tmp = b;
    b = c;
    c = tmp;
  }
  if (a > b)
  {
    tmp = a;
    a = b;
    b = tmp;
  }
  if (cnum[a] == 1 && cnum[b] == 2 && cnum[c] == 3)
    return 1;			/* 帥仕相 */
  if (cnum[a] == 4 && cnum[b] == 5 && cnum[c] == 6)
    return 1;			/* 硨傌炮 */
  if (cnum[a] == 8 && cnum[b] == 9 && cnum[c] == 10)
    return 1;			/* 將士象 */
  if (cnum[a] == 11 && cnum[b] == 12 && cnum[c] == 13)
    return 1;			/* 車馬包 */
  if (cnum[a] == 7 && cnum[b] == 7 && cnum[c] == 7)
    return 1;			/* 兵兵兵 */
  if (cnum[a] == 14 && cnum[b] == 14 && cnum[c] == 14)
    return 1;			/* 卒卒卒 */
  return 0;
}


static int
testall(set)
  int set[5];
{
  int i, j, k, m, p[3];
  for (i = 0; i < 4; i++)
  {
    for (j = i + 1; j < 5; j++)
    {
      m = 0;
      for (k = 0; k < 5; k++)
      {
	if (k != i && k != j)
	{
	  p[m] = set[k];
	  m++;
	}
      }
      if (testpair(set[i], set[j]) != 0 && testthree(p[0], p[1], p[2]) != 0)
	return 1;
    }
  }
  return 0;
}


static void
printhostall()
{
  int i;
  for (i = 0; i < 5; i++)
    print_Schess(host_card[i], 4, 6 * i);
}


static void
printhostfour()
{
  int i;
  for (i = 0; i < 4; i++)
    print_Schess(host_card[i], 4, 6 * i);
}


static int
testlisten(set)
  int set[4];
{
  int i, j, k, p[2] = {0}, m = 0, mm = 0;

  j = 0;
  for (i = 0; i < 4; i++)
  {
    if (group[set[i]] != 3)
      j++;
  } if (j == 0)
    return 1;			/* 四支兵 */
  j = 0;
  for (i = 0; i < 4; i++)
  {
    if (group[set[i]] != 6)
      j++;
  } if (j == 0)
    return 1;			/* 四支卒 */

  if (testthree(set[1], set[2], set[3]) != 0)
    return 1;
  if (testthree(set[0], set[2], set[3]) != 0)
    return 1;
  if (testthree(set[0], set[1], set[3]) != 0)
    return 1;
  if (testthree(set[0], set[1], set[2]) != 0)
    return 1;			/* 三支成形則聽 */
  for (i = 0; i < 3; i++)
  {
    for (j = i + 1; j < 4; j++)
    {
      if (testpair(set[i], set[j]))
      {				/* 兩支有胚看另兩支有沒有聽 */
	m = 0;
	for (k = 0; k < 4; k++)
	{
	  if (k != i && k != j)
	  {
	    p[m] = set[k];
	    m++;
	  }
	  if (group[set[i]] == 3 || group[set[i]] == 6)
	    mm = 1;		/* 有胚的是兵或卒 */
	}
      }
    }
  }
  if (m != 0)
  {
    if ((group[p[0]] == group[p[1]]) && (cnum[p[0]] != cnum[p[1]]))
      return 1;			/* 兩支是 pair 另兩支有聽 */
    if ((group[p[0]] == group[p[1]] == 3) || (group[p[0]] == group[p[1]] == 6))
      return 1;
    if (testpair(p[0], p[1]) && mm == 1)
      return 1;
  }
  return 0;
}


static inline void
host_hula()
{
  print_Sign(11, (tflagB - 1) * 4);	/* 印撿牌符號 */
  printhostall();
}


static inline void
host_self()
{
  printhostall();
}


static int
diecard(a)			/* 傳進一張牌, 看是否絕張 */
  int a;
{
  int i, k = 0;
  for (i = 0; i < tflag; i++)
  {
    if (cnum[throw[i]] == cnum[a])
      k++;
    if (cnum[throw[i]] == 1 && cnum[a] == 8)
      return 1;
    if (cnum[throw[i]] == 8 && cnum[a] == 1)
      return 1;
  }
  if ((cnum[a] == 7 || cnum[a] == 14) && k == 4)
    return 1;			/* 兵卒絕張 */
  if (k == 1 && (cnum[a] != 7 && cnum[a] != 14))
    return 1;
  return 0;
}


static inline int
any_throw()
{
  int i, j, k, set[5] = {0}, tmp[4] = {0};
  int point[5] = {0};	/* point[5] 為評分系統, 看丟哪張牌比較好, 分數高的優先丟 */

  /* 測試將手上五支拿掉一支 */
  for (i = 0; i < 5; i++)
  {
    k = 0;
    for (j = 0; j < 5; j++)
    {
      if (i != j)
      {
	tmp[k] = host_card[j];
	k++;
      }
    }
    if (testlisten(tmp))	/* 若剩餘的四支已聽牌, 丟多的那張 */
    {
      point[i] += 10;		/* 有聽就加 10 分 */
      if (diecard(host_card[i]))
	point[i] += 5;		/* 絕張更該丟 */
      for (k = 0; k < 4; k++)
      {
	if (((cnum[host_card[i]] == cnum[tmp[k]]) || 
	  (cnum[tmp[k]] == 1 && cnum[host_card[i]] == 8) || 
	  (cnum[tmp[k]] == 8 && cnum[host_card[i]] == 1)) && 
	  cnum[host_card[i]] != 7 && cnum[host_card[i]] != 14)
	  point[i] += 10;
      }
      /* 車馬包包, 包該丟 */
    }
  }

  k = 0;
  for (i = 0; i < 5; i++)
  {
    if (cnum[host_card[i]] == 7)
      k++;			/* 算有幾支兵 */
  }
  if (k == 3)		/* 有三支兵: 剩下二支不是兵的各加 5 分 */
  {
    for (i = 0; i < 5; i++)
    {
      if (cnum[host_card[i]] != 7)
	point[i] += 5;
    }
  }
  else if (k == 4)	/* 有四支兵 */
  {
    if (diecard(12))	/* 但最後一支兵已絕張: 丟兵 */
    {
      for (i = 0; i < 5; i++)
      {
	if (cnum[host_card[i]] == 7)
	  point[i] += 9999;
      }
    }
    else		/* 最後一支兵尚未絕張: 丟不是兵的那支 */
    {
      for (i = 0; i < 5; i++)
      {
	if (cnum[host_card[i]] != 7)
	  point[i] += 9999;
      }
    }
  }

  k = 0;
  for (i = 0; i < 5; i++)
  {
    if (cnum[host_card[i]] == 14)
      k++;			/* 算有幾支卒 */
  }
  if (k == 3)		/* 有三支卒: 剩下二支不是卒的各加 5 分 */
  {
    for (i = 0; i < 5; i++)
    {
      if (cnum[host_card[i]] != 14)
	point[i] += 5;
    }
  }
  else if (k == 4)	/* 有四支卒 */
  {
    if (diecard(28))	/* 但最後一支卒已絕張: 丟卒 */
    {
      for (i = 0; i < 5; i++)
      {
	if (cnum[host_card[i]] == 14)
	  point[i] += 9999;
      }
    }
    else		/* 最後一支卒尚未絕張: 丟不是卒的那支 */
    {
      for (i = 0; i < 5; i++)
      {
	if (cnum[host_card[i]] != 14)
	  point[i] += 9999;
      }
    }
  }

  for (i = 0; i < 4; i++)
  {
    for (j = i + 1; j < 5; j++)
    {
      if (group[host_card[i]] == group[host_card[j]])
      {
	point[i] -= 2;
	point[j] -= 2;		/* 差一支成三的不丟 */
      }
      if (testpair(host_card[i], host_card[j]))
      {
	point[i] -= 2;
	point[j] -= 2;		/* 有胚的不丟 */
      }
    }
  }

#if 1	/* 耍賤, 如果丟了會被胡就死都不丟, 耍賤機率 5/6 */
  for (i = 0; i < 4; i++)
    set[i] = guest_card[i];
  for (i = 0; i < 5; i++)
  {
    set[4] = host_card[i];
    if (testall(set) && rnd(6))
      point[i] = -9999;
  }
#endif

  /* 找出分數最高的 */
  j = 0;
  k = point[0];
  for (i = 1; i < 5; i++)
  {
    if (point[i] >= k)
    {
      k = point[i];
      j = i;
    }
  }
  return j;
}


static int
count_tai(set)
  int set[5];		/* 贏的五張牌 */
{
  char *name[10] = 
  {
    "將帥對", "將士象", "帥仕相",
    "五兵合縱", "五卒連橫", "三兵入列", "三卒入列",
    "海底", "天胡", "自摸"
  };

  int tai[10] = 	/* 台數對應上面的敘述 */
  {
    2, 1, 1,
    5, 5, 2, 2,
    3, 5, 1
  };

  int yes[10] = {0};
  int i, j, k, sum;

  if (selftouch)
    yes[9] = 1;			/* 自摸 */

  if (flag == 32)
    yes[7] = 1;			/* 海底 */
  else if (tflag <= 1)
    yes[8] = 1;			/* 天胡 */

  for (i = 0, j = 0, k = 0; i < 5; i++)
  {
    /* 算 帥/將 的支數 */
    if (cnum[set[i]] == 1)
      j++;
    if (cnum[set[i]] == 8)
      k++;
  }
  if (j)
  {
    if (k)
      yes[0] = 1;		/* 有帥又有將就是 將帥對 */
    else
      yes[2] = 1;		/* 有帥沒有將就是 帥仕相 */
  }
  else if (k)
  {
    yes[1] = 1;			/* 有將沒有帥就是 將士象 */
  }

  for (i = 0, j = 0; i < 5; i++)
  {
    /* 算 兵 的支數 */
    if (cnum[set[i]] == 7)
      j++;
  }
  if (j == 5)
    yes[3] = 1;			/* 五兵合縱 */
  else if (j == 3)
    yes[5] = 1;			/* 三兵入列 */

  for (i = 0, j = 0; i < 5; i++)
  {
    /* 算 卒 的支數 */
    if (cnum[set[i]] == 14)
      j++;
  }
  if (j == 5)
    yes[4] = 1;			/* 五卒連橫 */
  else if (j == 3)
    yes[6] = 1;			/* 三卒入列 */

  /* 算台數 */
  sum = 0;
  for (i = 0; i < 10; i++)
  {
    if (yes[i])
      sum += tai[i];
  }

  /* 列印出獎項 */
  move(b_lines - 5, 0);
  outs("┌──────────────────────────┐\n");
  for (i = 0; i < 5; i++)
  {
    if (yes[i])
      prints("    %8s [%d 台]", name[i], tai[i]);
  }
  move(b_lines - 3, 0);
  for (i = 5; i < 10; i++)
  {
    if (yes[i])
      prints("    %8s [%d 台]", name[i], tai[i]);
  }
  move(b_lines - 2, 0);
  clrtoeol();		/* 清除 out_song() */
  prints("          底 [2 台]          合計 [%d 台]\n", sum += 2);
  outs("└──────────────────────────┘");

  return sum;
}


int
main_chessmj()
{
  int money;			/* 押金 */
  int mo;			/* 1:已摸牌 0:未摸牌 */
  int picky;			/* 1:撿別人的牌 0:自己摸的 */
  int pickup;
  int listen;			/* 1:聽牌 0:沒有聽牌 */
  int chesslist[32];		/* 32 張牌組 */

  int i, j, k, m;
  int jp, x, xx, ch, z;

  char ans[10], *msg;
  int tmp[4];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  while (1)
  {
    vs_bar("象棋麻將");

    out_song();

    vget(2, 0, "請問要下注多少呢？(1 ~ 50000) ", ans, 6, DOECHO);
    money = atoi(ans);
    if (money < 1 || money > 50000 || money > cuser.money)
      break;			/* 離開賭場 */
    cuser.money -= money;	/* 扣一份賭金，玩家如果中途離開將拿不回賭金 */

    move(2, 0);
    clrtoeol();		/* 清掉「請問要下注多少」 */
    outs("(按 ←→選牌, ↑丟牌, 按 ENTER 胡牌)");

    for (i = 0; i < 32; i++)		/* 牌先一張一張排好，準備洗牌 */
      chesslist[i] = i;

    for (i = 0; i < 31; i++)
    {
      j = rnd(32 - i) + i;

      /* chesslist[j] 和 chesslist[i] 交換 */
      m = chesslist[i];
      chesslist[i] = chesslist[j];
      chesslist[j] = m;
    }

    for (i = 0; i < 32; i++)		/* 還沒有牌被丟棄 */
      throw[i] = 32;

    selftouch = 0;			/* 歸零 */
    mo = 0;
    pickup = 0;
    picky = 0;
    listen = 0;
    flag = 0;
    tflag = 0;
    tflagA = 0;
    tflagB = 0;

    for (i = 0; i < 4; i++)		/* 發前四張牌 */
    {
      host_card[i] = chesslist[flag];
      flag++;
      guest_card[i] = chesslist[flag];
      flag++;
    }

    for (i = 0; i < 4; i++)			/* 排序 */
    {
      for (j = 0; j < (3 - i); j++)
      {
	if (guest_card[j] > guest_card[j + 1])
	{
	  m = guest_card[j];
	  guest_card[j] = guest_card[j + 1];
	  guest_card[j + 1] = m;
	}
	if (host_card[j] > host_card[j + 1])
	{
	  m = host_card[j];
	  host_card[j] = host_card[j + 1];
	  host_card[j + 1] = m;
	}
      }
    }

    move(4, 0);
    outs("╭─╮╭─╮╭─╮╭─╮");
    move(5, 0);
    outs("│  ││  ││  ││  │");
    move(6, 0);
    outs("╰─╯╰─╯╰─╯╰─╯");

    for (i = 0; i < 4; i++)
      print_Schess(guest_card[i], 15, 6 * i);	/* 印出前四張牌 */

    for (;;)
    {
      jp = 5;
      x = 0;
      z = 1;
      move(18, 26);
      do
      {
	if (!mo)
	{
	  move(14, 24);
	  outs("按任一鍵摸牌(或 ↓ 撿牌)");
	}
	else
	{
	  move(14, 0);
	  clrtoeol();
	}
	move(18, 2 + (jp - 1) * 6);
	outs("▲");
	move(18, 3 + (jp - 1) * 6);		/* 避免全形偵測 */

	ch = vkey();

	if (ch != '\n' && flag == 32)
	{
	  msg = "流局";
	  goto next_game;
	}
	if (!mo && ch != KEY_DOWN && ch != '\n')
	{
	  ch = 'p';		/* 四張牌則強制摸牌 */
	}

	switch (ch)
	{
	case KEY_RIGHT:
	  move(18, 2 + (jp - 1) * 6);
	  outs("  ");
	  jp += 1;
	  if (jp > 5)
	    jp = 5;
	  move(18, 2 + (jp - 1) * 6);
	  outs("▲");
	  move(18, 3 + (jp - 1) * 6);	/* 避免全形偵測 */
	  break;

	case KEY_LEFT:
	  move(18, 2 + (jp - 1) * 6);
	  outs("  ");
	  jp -= 1;
	  if (jp < 1)
	    jp = 1;
	  move(18, 2 + (jp - 1) * 6);
	  outs("▲");
	  move(18, 3 + (jp - 1) * 6);	/* 避免全形偵測 */
	  break;

	case KEY_UP:		/* 出牌 */
	  move(18, 2 + (jp - 1) * 6);
	  outs("  ");
	  throw[tflag] = guest_card[jp - 1];
	  tflag++;
	  tflagB++;
	  z = 0;
	  mo = 0;
	  guest_card[jp - 1] = guest_card[4];
	  guest_card[4] = 0;
	  sortchess();
	  clear_5card();
	  P_allchess();
	  print_Schess(throw[tflag - 1], 11, (tflagB - 1) * 4);
	  picky = 0;
	  break;

	case 'p':		/* 摸牌 */
	  if (!mo)
	  {
	    move(18, 2 + (jp - 1) * 6);
	    outs("  ");
	    guest_card[4] = chesslist[flag];
	    flag++;
	    print_Schess(guest_card[4], 15, 24);
	    mo = 1;
	  }
	  break;

	case KEY_DOWN:
	  if (tflag > 0 && !mo)
	  {
	    guest_card[4] = throw[tflag - 1];
	    print_Sign(8, (tflagA - 1) * 4);
	    print_Schess(guest_card[4], 15, 24);
	    mo = 1;
	    picky = 1;
	  }
	  break;

	case 'q':
	  return 0;
	  goto abort_game;
	  break;

	case '\n':
	  if (testall(guest_card) && mo && !picky)
	  {
	    printhostfour();
	    msg = "哇咧自摸啦！";
	    addmoney(money * count_tai(guest_card));
	    goto next_game;
	  }
	  else if (picky && testall(guest_card))
	  {
	    printhostfour();
	    msg = "看我的厲害，胡啦！";
	    addmoney(money * count_tai(guest_card));
	    goto next_game;
	  }

	  if (tflag > 0 && !mo)
	  {
	    i = guest_card[4];
	    guest_card[4] = throw[tflag - 1];
	    if (testall(guest_card) == 1)
	    {
	      print_Sign(8, (tflagA - 1) * 4);
	      print_Schess(guest_card[4], 15, 24);
	      printhostfour();
	      msg = "胡！";
	      addmoney(money * count_tai(guest_card));
	      goto next_game;
	    }
	    guest_card[4] = i;
	  }
	  break;

	default:
	  break;
	}
      } while (z == 1);

      host_card[4] = throw[tflag - 1];
      if (testall(host_card))
      {
	host_hula();
	msg = "電腦胡啦！";
	cuser.money -= money * count_tai(host_card);	/* 電腦台數越多就賠越多，且押金沒收 */
	if (cuser.money < 0)
	  cuser.money = 0;
	goto next_game;
      }

      if (tflag == 32)
      {
        msg = "流局";
        goto next_game;
      }

      host_card[4] = chesslist[flag];
      if (testall(host_card))
      {
	host_self();
	msg = "電腦自摸！";
	cuser.money -= money * count_tai(host_card);	/* 電腦台數越多就賠越多，且押金沒收 */
	if (cuser.money < 0)
	  cuser.money = 0;
	goto next_game;
      }

      for (i = 0; i < 4; i++)
	tmp[i] = host_card[i];

      if (!testlisten(tmp))
      {				/* 沒聽的話 */
	for (i = 0; i < 4; i++)
	{
	  k = 0;
	  for (j = 0; j < 4; j++)
	  {
	    if (i != j)
	    {
	      tmp[k] = host_card[j];
	      k++;
	    }
	  }
	  tmp[3] = throw[tflag - 1];	/* 把撿起那張跟手上的牌比對 */
	  if (testlisten(tmp))
	  {			/* 撿牌有聽的話 */
	    listen = 1;
	    host_card[4] = throw[tflag - 1];
	    tflag--;
	    print_Sign(11, (tflagB - 1) * 4);	/* 印撿牌符號 */
	    xx = i;		/* 紀錄下要丟的那張牌 */
	    pickup = 1;
	    break;		/* 跳出 i loop */
	  }
	}
      }

      for (i = 0; i < 4; i++)
	tmp[i] = host_card[i];
      if (testlisten(tmp) && !pickup)
      {				/* 有聽且剛剛沒撿 */
	m = 0;
	for (i = 0; i < 4; i++)
	  if (cnum[tmp[i]] == 7)
	    m++;
	if (m == 2 && cnum[throw[tflag - 1]] == 7)
	  pickup = 1;
	if (m == 3 && cnum[throw[tflag - 1]] == 7)
	{
	  pickup = 1;
	  for (i = 0; i < tflag - 1; i++)
	    if (cnum[throw[i]] == 7)
	      pickup = 0;
	}
	m = 0;
	for (i = 0; i < 4; i++)
	  if (cnum[tmp[i]] == 14)
	    m++;
	if (m == 2 && cnum[throw[tflag - 1]] == 14)
	  pickup = 1;
	if (m == 3 && cnum[throw[tflag - 1]] == 14)
	{
	  pickup = 1;
	  for (i = 0; i < tflag - 1; i++)
	    if (cnum[throw[i]] == 14)
	      pickup = 0;
	}
	if (pickup)
	{
	  host_card[4] = throw[tflag - 1];
	  tflag--;
	  print_Sign(11, (tflagB - 1) * 4);	/* 印撿牌符號 */
	}
      }


      if (!pickup)
      {
	host_card[4] = chesslist[flag];
	flag++;
      }
      /* 剛剛沒撿牌現在就摸牌 */
      Phost5();
      Chost5();

      if (!pickup)
      {
	for (i = 0; i < 4; i++)
	{
	  k = 0;
	  for (j = 0; j < 4; j++)
	  {
	    if (i != j)
	    {
	      tmp[k] = host_card[j];
	      k++;
	    }
	  }
	  tmp[3] = host_card[4];
	  if (testlisten(tmp))
	  {			/* 摸牌有聽的話 */
	    listen = 1;
	    xx = i;		/* 紀錄下要丟的那張牌 */
	    break;		/* 跳出 i loop */
	  }
	}
      }

      for (i = 0; i < 4; i++)
	tmp[i] = host_card[i];

      xx = any_throw();

      throw[tflag] = host_card[xx];
      tflag++;
      tflagA++;
      host_card[xx] = host_card[4];	/* 丟出沒聽那張 */
      print_Schess(throw[tflag - 1], 8, (tflagA - 1) * 4);

      pickup = 0;
      listen = 0;

    }		/* for 迴圈結束 */

next_game:
    vmsg(msg);

  }		/* while 迴圈結束 */

abort_game:
  return 0;
}
#endif				/* HAVE_GAME */
