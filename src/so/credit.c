/*-------------------------------------------------------*/
/* credit.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : �O�b���A�O���ͬ��������J��X		 */
/* create : 99/12/18                                     */
/* update : 02/01/26					 */
/* author : wildcat@wd.twbbs.org			 */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_CREDIT

/* ----------------------------------------------------- */
/* credit.c ���B�Ϊ���Ƶ��c                             */
/* ----------------------------------------------------- */

typedef struct
{
  int year;			/* �~ */
  char month;			/* �� */
  char day;			/* �� */

  char flag;			/* ��X/���J */
  int money;			/* ���B */
  char useway;			/* ���O(������|��) */
  char desc[112];		/* ���� */		/* �o�Ӫ��F�A�O�d����L���ϥ� */
}      CREDIT;


#define CREDIT_OUT	0x1	/* ��X */
#define CREDIT_IN	0x2	/* ���J */

#define CREDIT_OTHER	0	/* ��L */
#define CREDIT_EAT	1	/* �� */
#define CREDIT_WEAR	2	/* �� */
#define CREDIT_LIVE	3	/* �� */
#define CREDIT_MOVE	4	/* �� */
#define CREDIT_EDU	5	/* �| */
#define CREDIT_PLAY	6	/* �� */

static char fpath[64];		/* FN_CREDIT �ɮ׸��| */


static void
credit_head()
{
  vs_head("�O�b�⥾", str_site);
  prints(NECKER_CREDIT, d_cols, "");
}


static void
credit_body(page)
  int page;
{
  CREDIT credit;
  char *way[] = {"��L", "[��]", "[��]", "[��]", "[��]", "[�|]", "[��]"};
  int fd;

  move(1, 65);
  prints("�� %2d ��", page + 1);

  move(3, 0);
  clrtobot();

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    int pos, n;

    pos = page * XO_TALL;	/* �@���� XO_TALL �� */
    n = XO_TALL;

    while (n)
    {
      lseek(fd, (off_t) (sizeof(CREDIT) * pos), SEEK_SET);
      if (read(fd, &credit, sizeof(CREDIT)) == sizeof(CREDIT))
      {
	n--;
	pos++;
	prints("%6d %04d/%02d/%02d %s %8d %4s %.*s\n", 
	  pos, credit.year, credit.month, credit.day, 
	  credit.flag == CREDIT_OUT ? "\033[1;32m��X\033[m" : "\033[1;31m���J\033[m",
	  credit.money, 
	  credit.flag == CREDIT_OUT ? way[credit.useway] : "    ",
	  d_cols + 46, credit.desc);
      }
      else
      {
        break;
      }
    }

    close(fd);
  }
}


static int
credit_add()
{
  CREDIT credit;
  char buf[80];

  move(3, 0);
  clrtobot();

  memset(&credit, 0, sizeof(CREDIT));

  if (vget(5, 0, "���� (1)���J (2)��X [2] ", buf, 3, DOECHO) == '1')
    credit.flag = CREDIT_IN;
  else
    credit.flag = CREDIT_OUT;
    
  vget(6, 0, "�ɶ� (�~��) ", buf, 5, DOECHO);
  credit.year = atoi(buf);

  vget(7, 0, "�ɶ� (���) ", buf, 3, DOECHO);
  credit.month = atoi(buf);

  vget(8, 0, "�ɶ� (���) ", buf, 3, DOECHO);
  credit.day = atoi(buf);

  vget(9, 0, "���� (��) ", buf, 9, DOECHO);
  credit.money = atoi(buf);

  if (credit.flag == CREDIT_OUT)	/* ��X�~���O���γ~ */
  {
    int useway;

    useway = vget(10, 0, "�γ~ 0)��L 1)�� 2)�� 3)�� 4)�� 5)�| 6)�� [0] ", buf, 3, DOECHO) - '0';
    if (useway > 6 || useway < 0)
      useway = 0;
    credit.useway = useway;
  }

  vget(11, 0, "�����G", credit.desc, 51, DOECHO);

  rec_add(fpath, &credit, sizeof(CREDIT));
  return 1;
}


static int
credit_delete()
{
  int pos;
  char buf[4];

  vget(b_lines, 0, "�n�R���ĴX����ơG", buf, 4, DOECHO);
  pos = atoi(buf);
  
  if (rec_num(fpath, sizeof(CREDIT)) < pos)
  {
    vmsg("�z�d���o�A�S���o�����");
    return 0;
  }

  rec_del(fpath, sizeof(CREDIT), pos - 1, NULL);
  return 1;
}


static int
credit_count()
{
  CREDIT *credit;
  struct stat st;
  int fd;
  int way[7], moneyin, moneyout;

  if ((fd = open(fpath, O_RDONLY)) >= 0 && !fstat(fd, &st) && st.st_size > 0)
  {
    memset(way, 0, sizeof(way));
    moneyin = 0;

    mgets(-1);
    while (credit = mread(fd, sizeof(CREDIT)))
    {
      if (credit->flag == CREDIT_OUT)	/* ��X�~���O���γ~ */
	way[credit->useway] += credit->money;
      else
        moneyin += credit->money;
    }
    close(fd);

    moneyout = 0;
    for (fd = 0; fd <= 6; fd++)
      moneyout += way[fd];

    move(3, 0);
    clrtobot();

    move(7, 0);
    prints("      \033[1;31m�`���J  %12d ��\033[m\n", moneyin);
    prints("      \033[1;32m�`��X  %12d ��\033[m\n\n", moneyout);

    prints("��b  \033[1;36m [��]   %12d ��    \033[32m [��]   %12d ��\033[m\n", way[CREDIT_EAT], way[CREDIT_WEAR]);
    prints("      \033[1;31m [��]   %12d ��    \033[33m [��]   %12d ��\033[m\n", way[CREDIT_LIVE], way[CREDIT_MOVE]);
    prints("      \033[1;35m [�|]   %12d ��    \033[37m [��]   %12d ��\033[m\n", way[CREDIT_EDU], way[CREDIT_PLAY]);
    prints("      \033[1;34m ��L   %12d ��\033[m", way[CREDIT_OTHER]);

    vmsg(NULL);
    return 1;
  }

  vmsg("�z�S���O�b�O��");
  return 0;
}


int
main_credit()
{
  int page, redraw;
  char buf[3];

  credit_head();

  usr_fpath(fpath, cuser.userid, FN_CREDIT);
  page = 0;
  redraw = 1;

  for (;;)
  {
    if (redraw)
      credit_body(page);

    switch (vans("�O�b�⥾ C)���� 1)�s�W 2)�R�� 3)���R 4)�`�p Q)���} [Q] "))
    {
    case 'c':
      vget(b_lines, 0, "����ĴX���G", buf, 3, DOECHO);
      redraw = atoi(buf) - 1;
      
      if (page != redraw && redraw >= 0 && 
        redraw <= (rec_num(fpath, sizeof(CREDIT)) - 1) / XO_TALL)
      {
        page = redraw;
        redraw = 1;
      }
      else
      {
        redraw = 0;
      }
      break;

    case '1':
      redraw = credit_add();
      break;

    case '2':
      redraw = credit_delete();
      break;

    case '3':
      if (vans(MSG_SURE_NY) == 'y')
      {
        unlink(fpath);
        return 0;
      }
      break;

    case '4':
      redraw = credit_count();
      break;

    default:
      return 0;
    }
  }
}
#endif				/* HAVE_CREDIT */
