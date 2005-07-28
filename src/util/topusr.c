/*-------------------------------------------------------*/
/* util/topusr.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : �ϥΪ̸�Ʋέp�αƦW			 */
/* create : 99/03/29					 */
/* update : 01/10/01					 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


/* itoc.����: �N �W������/�������/�ȹ�/����/�~��/�P�y/�ʧO/�جP �@�ֲέp�A�K�o�`�`�b�} .ACCT */

#include "bbs.h"


#define OUTFILE_TOPLOGIN BBSHOME"/gem/@/@-toplogin"
#define OUTFILE_TOPPOST	 BBSHOME"/gem/@/@-toppost"
#define OUTFILE_TOPMONEY BBSHOME"/gem/@/@-topmoney"
#define OUTFILE_TOPGOLD	 BBSHOME"/gem/@/@-topgold"
#define OUTFILE_AGE	 BBSHOME"/gem/@/@-age"
#define OUTFILE_STAR	 BBSHOME"/gem/@/@-star"
#define OUTFILE_SEX	 BBSHOME"/gem/@/@-sex"
#define OUTFILE_BIRTHDAY BBSHOME"/gem/@/@-birthday"
#define TMPFILE_BIRTHMON BBSHOME"/tmp/birthmon"


/*-------------------------------------------------------*/
/* author : mat.bbs@fall.twbbs.org			 */
/* modify : gslin@abpe.org				 */
/* target : �W�����ơB������ơB���ȹ��Ʀ�]		 */
/*-------------------------------------------------------*/


#define	TOPNUM		36
#define	TOPNUM_HALF	(TOPNUM / 2)


typedef struct
{
  char userid[IDLEN + 1];
  char username[UNLEN + 1];
  int num;			/* �ƭ� */
  int rank;			/* �W�� */
}      DATA;

static DATA toplogins[TOPNUM], topposts[TOPNUM], topmoney[TOPNUM], topgold[TOPNUM];


static int
sort_compare(p1, p2)
  const void *p1;
  const void *p2;
{
  int k;
  DATA *a1, *a2;

  a1 = (DATA *) p1;
  a2 = (DATA *) p2;

  k = a2->num - a1->num;
  return k ? k : str_cmp(a1->userid, a2->userid);
}


static DATA *
findmin(src)
  DATA *src;
{
  int i;
  DATA *p;

  p = src;
  for (i = 0; i < TOPNUM; i++)
  {
    if (src[i].num < p->num)
      p = src + i;
  }
  return p;
}


static void
sort_rank(data)		/* ���w�W�� */
  DATA *data;		/* �w�� data->num �Ƨ� */
{
  int i, rank;

  data[0].rank = rank = 1;
  for (i = 1; i < TOPNUM; i++)
  {
    if (data[i].num != data[i - 1].num)
      rank = i + 1;
    data[i].rank = rank;
  }
}


static void
merge_id_nick(dst, userid, nick)
  char *dst;
  char *userid;
  char *nick;
{
  if (*userid)
  {
    sprintf(dst, "%s (%s)", userid, nick);

    if (strlen(dst) > 25)
      dst[25] = '\0';
  }
  else
    dst[0] = '\0';
}


static void
write_data(fpath, title, data)
  char *fpath;
  char *title;
  DATA *data;
{
  FILE *fp;
  char buf[256];
  int i, num1, num2;

  sort_rank(data);

  if (!(fp = fopen(fpath, "w")))
    return;

  i = 12 - (strlen(title) >> 1);
  sprintf(buf, " \033[1;33m�� �w�w�w�w�w�w�w�w�w�w�� \033[41m%%%ds%%s%%%ds\033[40m ���w�w�w�w�w�w�w�w�w�w ��\033[m\n\n", i, i);
  fprintf(fp, buf, "", title, "");

  fprintf(fp, 
    "\033[1;37m�W��  \033[33m�N��(�ʺ�)                \033[36m�ƶq\033[m   "
    "\033[1;37m�W��  \033[33m�N��(�ʺ�)                \033[36m�ƶq\033[m\n"
    "\033[1;32m%s\033[m\n", MSG_SEPERATOR);

  for (i = 0; i < TOPNUM_HALF; i++)
  {
    char buf1[80], buf2[80];

    merge_id_nick(buf1, data[i].userid, data[i].username);
    merge_id_nick(buf2, data[i + TOPNUM_HALF].userid,
      data[i + TOPNUM_HALF].username);

    /* itoc.010408: �ѨM���Ӧh�A�e���z�������D */
    num1 = data[i].num / 1000000;
    num2 = data[i + TOPNUM_HALF].num / 1000000;
    if (num2)			/* ���� data[i].num �]���w > 10^6 */
    {
      fprintf(fp, "[%2d] %-25s %5dM  [%2d] %-25s %5dM\n", 
	data[i].rank, buf1, num1,
	data[i + TOPNUM_HALF].rank, buf2, num2);
    }
    else if (num1)
    {
      fprintf(fp, "[%2d] %-25s %5dM  [%2d] %-25s %6d\n", 
	data[i].rank, buf1, num1,
	data[i + TOPNUM_HALF].rank, buf2, data[i + TOPNUM_HALF].num);
    }
    else
    {
      fprintf(fp, "[%2d] %-25s %6d  [%2d] %-25s %6d\n", 
	data[i].rank, buf1, data[i].num,
	data[i + TOPNUM_HALF].rank, buf2, data[i + TOPNUM_HALF].num);
    }
  }

  fprintf(fp, "\n");
  fclose(fp);
}


static inline void
topusr(acct)
  ACCT *acct;
{
  DATA *p;

  if ((p = findmin(&toplogins))->num < acct->numlogins)
  {
    str_ncpy(p->userid, acct->userid, sizeof(p->userid));
    str_ncpy(p->username, acct->username, sizeof(p->username));
    p->num = acct->numlogins;
  }

  if ((p = findmin(&topposts))->num < acct->numposts)
  {
    str_ncpy(p->userid, acct->userid, sizeof(p->userid));
    str_ncpy(p->username, acct->username, sizeof(p->username));
    p->num = acct->numposts;
  }

  if ((p = findmin(&topmoney))->num < acct->money)
  {
    str_ncpy(p->userid, acct->userid, sizeof(p->userid));
    str_ncpy(p->username, acct->username, sizeof(p->username));
    p->num = acct->money;
  }

  if ((p = findmin(&topgold))->num < acct->gold)
  {
    str_ncpy(p->userid, acct->userid, sizeof(p->userid));
    str_ncpy(p->username, acct->username, sizeof(p->username));
    p->num = acct->gold;
  }
}


/*-------------------------------------------------------*/
/* author : wsyfish.bbs@fpg.m4.ntu.edu.tw		 */
/* target : ���W�~�ֲέp				 */
/*-------------------------------------------------------*/


#define MAX_LINE	16

static int act_age[25];		/* 24�ئ~�֡B�`�X */


static inline void
count_age(acct, year)
  ACCT *acct;
  int year;
{
  int age;

  age = year - 11 - acct->year;	/* acct->year �O����~�� */

  /* �u�έp 10~33 �� */
  if (age >= 10 && age <= 10 + 24 - 1)
  {
    act_age[age - 10]++;
    act_age[24]++;
  }
}


static void
fouts(fp, buf, mode)
  FILE *fp;
  char buf[], mode;
{
  static char state = '0';

  if (state != mode)
    fprintf(fp, "\033[3%cm", state = mode);
  if (buf[0])
  {
    fprintf(fp, buf);
    buf[0] = 0;
  }
}


static inline void
write_age(fpath, year, month, day)
  char *fpath;
  int year, month, day;
{
  char buf[256];
  FILE *fp;
  int i, j;
  int age, max, item, maxage, totalage;

  if ((fp = fopen(fpath, "w")) == NULL)
  {
    printf("cann't open %s\n", fpath);
    return;
  }

  max = maxage = totalage = 0;
  for (i = 0; i < 24; i++)
  {
    totalage += act_age[i] * (i + 10);
    if (act_age[i] > max)
    {
      max = act_age[i];
      maxage = i;
    }
  }

  item = max / MAX_LINE + 1;

  fprintf(fp, "\t\t\t \033[1;33;45m " BBSNAME " �~�ֲέp [%02d/%02d/%02d] \033[m\n\n",
    year % 100, month, day);

  for (i = MAX_LINE + 1; i > 0; i--)
  {
    strcpy(buf, "   ");
    for (j = 0; j < 24; j++)
    {
      max = item * i;
      age = act_age[j];
      if (age && (max > age) && (max - item <= age))
      {
	fouts(fp, buf, '7');
	fprintf(fp, "%-3d", age);
      }
      else if (max <= age)
      {
	fouts(fp, buf, '4');
	fprintf(fp, "�i ");
      }
      else
	strcat(buf, "   ");
    }
    fprintf(fp, "\n");
  }

  fprintf(fp, "  \033[1;35m�������������������������������������������������������������������������� \n"
    "   \033[1;32m10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33\n"
    "\t\t      \033[36m���Ĳέp�H���G\033[37m%-9d\033[36m�����~�֡G\033[37m%d\033[40;0m\n",
    act_age[24], (int) totalage / (act_age[24] ? act_age[24] : 1));

  fclose(fp);
}


/*-------------------------------------------------------*/
/* author : wsyfish.bbs@fpg.m4.ntu.edu.tw		 */
/* target : ���W�P�y�έp				 */
/*-------------------------------------------------------*/


static int act_star[13];	/* �Q�G�P�y�B��L */


static inline void
count_star(acct)
  ACCT *acct;
{
  switch (acct->month)
  {
  case 1:
    if (acct->day <= 19)
      act_star[9]++;
    else
      act_star[10]++;
    break;
  case 2:
    if (acct->day <= 18)
      act_star[10]++;
    else
      act_star[11]++;
    break;
  case 3:
    if (acct->day <= 20)
      act_star[11]++;
    else
      act_star[0]++;
    break;
  case 4:
    if (acct->day <= 19)
      act_star[0]++;
    else
      act_star[1]++;
    break;
  case 5:
    if (acct->day <= 20)
      act_star[1]++;
    else
      act_star[2]++;
    break;
  case 6:
    if (acct->day <= 21)
      act_star[2]++;
    else
      act_star[3]++;
    break;
  case 7:
    if (acct->day <= 22)
      act_star[3]++;
    else
      act_star[4]++;
    break;
  case 8:
    if (acct->day <= 22)
      act_star[4]++;
    else
      act_star[5]++;
    break;
  case 9:
    if (acct->day <= 22)
      act_star[5]++;
    else
      act_star[6]++;
    break;
  case 10:
    if (acct->day <= 23)
      act_star[6]++;
    else
      act_star[7]++;
    break;
  case 11:
    if (acct->day <= 22)
      act_star[7]++;
    else
      act_star[8]++;
    break;
  case 12:
    if (acct->day <= 21)
      act_star[8]++;
    else
      act_star[9]++;
    break;
  default:
    act_star[12]++;
  }
}


static inline void
write_star(fpath, year, month, day)
  char *fpath;
  int year, month, day;
{
  FILE *fp;
  int i, j;
  int max, item, maxstar;

  char name[13][7] = 
  {
    "�d�Ϯy",
    "�����y",
    "���l�y",
    "���ɮy",
    "��l�y",
    "�B�k�y",
    "�ѯ��y",
    "���Ȯy",
    "�g��y",
    "���~�y",
    "���~�y",
    "�����y",
    "���i��"
  };

  char blk[10][3] =
  {
    "  ", "�j", "�k", "�l", "�m",
    "�n", "�o", "�p", "�i", "�i",
  };


  if ((fp = fopen(fpath, "w")) == NULL)
  {
    printf("cann't open %s\n", fpath);
    return;
  }

  max = maxstar = 0;

  for (i = 0; i < 13; i++)
  {
    if (act_star[i] > max)
    {
      max = act_star[i];
      maxstar = i;
    }
  }

  item = max / 30 + 1;

  fprintf(fp, "\t\t\t \033[1;33;45m " BBSNAME " �P�y�έp [%02d/%02d/%02d] \033[m\n\n",
    year % 100, month, day);

  for (i = 0; i < 13; i++)
  {
    fprintf(fp, " \033[1;37m%s \033[0;36m", name[i]);
    for (j = 0; j < act_star[i] / item; j++)
    {
      fprintf(fp, "%2s", blk[9]);
    }
    fprintf(fp, "%2s \033[1;37m%d\033[m\n", blk[(act_star[i] % item) * 10 / item],
      act_star[i]);
  }

  fclose(fp);
}


/*-------------------------------------------------------*/
/* author : BioStar.bbs@micro.bio.ncue.edu.tw		 */
/* target : ���W�ʧO�έp				 */
/*-------------------------------------------------------*/


static int act_sex[3];		/* ���ʡB�k�ʡB�k�� */


static inline void
count_sex(acct)
  ACCT *acct;
{
  switch (acct->sex)
  {
  case 1:
  case 2:
    act_sex[acct->sex]++;
    break;
  default:
    act_sex[0]++;
  }
}


static inline void
write_sex(fpath, year, month, day)
  char *fpath;
  int year, month, day;
{
  FILE *fp;
  int i, j;
  int max, item, maxsex;

  char name[3][7] = 
  {
    "  ����",
    "  �k��",
    "  �k��"
  };

  char blk[10][3] =
  {
    "  ", "�j", "�k", "�l", "�m",
    "�n", "�o", "�p", "�i", "�i",
  };


  if ((fp = fopen(fpath, "w")) == NULL)
  {
    printf("cann't open %s\n", fpath);
    return;
  }

  max = maxsex = 0;

  for (i = 0; i < 3; i++)
  {
    if (act_sex[i] > max)
    {
      max = act_sex[i];
      maxsex = i;
    }
  }

  item = max / 30 + 1;

  fprintf(fp, "\t\t\t \033[1;33;45m " BBSNAME " �ʧO�έp [%02d/%02d/%02d] \033[m\n\n",
    year % 100, month, day);

  for (i = 0; i < 3; i++)
  {
    fprintf(fp, " \033[1;37m%s \033[0;36m", name[i]);
    for (j = 0; j < act_sex[i] / item; j++)
    {
      fprintf(fp, "%2s", blk[9]);
    }
    fprintf(fp, "%2s \033[1;37m%d\033[m\n", blk[(act_sex[i] % item) * 10 / item],
      act_sex[i]);
  }

  fclose(fp);
}

/*-------------------------------------------------------*/
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/* target : ���W�جP�έp				 */
/*-------------------------------------------------------*/


static inline void
write_birthday(fpath, year, month, day)
  char *fpath;
  int year, month, day;
{
  FILE *fp;

  if (!(fp = fopen(fpath, "w")))
    return;

  fprintf(fp, "\t\t\t \033[1;33;45m " BBSNAME " ����جP [%02d/%02d/%02d] \033[m\n\n",
    year % 100, month, day);

  fclose(fp);
}


static inline void
write_birthmon(fpath, year, month, day)
  char *fpath;
  int year, month, day;
{
  FILE *fp;

  /* �⥻��جP���b OUTFILE_BIRTHDAY �᭱ */

  if (!(fp = fopen(fpath, "a")))
    return;

  fprintf(fp, "\n\n\t\t\t \033[1;33;45m " BBSNAME " ����جP [%02d/%02d/%02d] \033[m\n\n",
    year % 100, month, day);

  f_suck(fp, TMPFILE_BIRTHMON);
  unlink(TMPFILE_BIRTHMON);

  fclose(fp);
}


static inline void
check_birth(fpath, acct, year, month, day)
  char *fpath;
  ACCT *acct;
  int year, month, day;		/* ���ѬO�X�~�X��X�� */
{
  static int birthday_num = 0;	/* ���X�ӤH���ѥͤ� */
  static int birthmon_num = 0;	/* ���X�ӤH����ͤ� */

  if (acct->month == month)
  {
    FILE *fp;
    char buf[50];

    if (acct->day == day)	/* ����جP */
    {
      if (!(fp = fopen(fpath, "a")))
	return;

      birthday_num++;
      merge_id_nick(buf, acct->userid, acct->username);

      fprintf(fp, "[%3d]  %-25s  �W������: %-5d  �峹�g��: %-5d  %2d ���ͤ�\n", 
	birthday_num, buf, acct->numlogins, acct->numposts, year - 11 - acct->year);

      fclose(fp);    
    }
    else			/* ����جP */
    {
      if (!(fp = fopen(TMPFILE_BIRTHMON, "a")))
	return;

      birthmon_num++;
      merge_id_nick(buf, acct->userid, acct->username);

      fprintf(fp, "[%3d]  %-25s  �W������: %-5d  �峹�g��: %-5d  ��: %02d/%02d\n", 
	birthmon_num, buf, acct->numlogins, acct->numposts, month, acct->day);

      fclose(fp);
    }
  }
}


/*-------------------------------------------------------*/
/* �D�{��						 */
/*-------------------------------------------------------*/


int
main()
{
  char c;
  int year, month, day;
  time_t now;
  struct tm *ptime;

  memset(&toplogins, 0, sizeof(toplogins));
  memset(&topposts, 0, sizeof(topposts));
  memset(&topmoney, 0, sizeof(topmoney));
  memset(&topgold, 0, sizeof(topgold));

  memset(act_age, 0, sizeof(act_age));
  memset(act_star, 0, sizeof(act_star));
  memset(act_sex, 0, sizeof(act_sex));

  now = time(NULL);
  ptime = localtime(&now);

  year = ptime->tm_year;
  month = ptime->tm_mon + 1;
  day = ptime->tm_mday;

  write_birthday(OUTFILE_BIRTHDAY, year, month, day);

  for (c = 'a'; c <= 'z'; c++)
  {
    char buf[64];
    struct dirent *de;
    DIR *dirp;

    sprintf(buf, BBSHOME "/usr/%c", c);
    chdir(buf);

    if (!(dirp = opendir(".")))
      continue;

    while (de = readdir(dirp))
    {
      ACCT acct;
      int fd;
      char *fname;

      fname = de->d_name;
      if (*fname <= ' ' || *fname == '.')
	continue;

      sprintf(buf, "%s/.ACCT", fname);
      if ((fd = open(buf, O_RDONLY)) < 0)
	continue;

      read(fd, &acct, sizeof(ACCT));
      close(fd);

      topusr(&acct);
      count_age(&acct, year);
      count_star(&acct);
      count_sex(&acct);
      check_birth(OUTFILE_BIRTHDAY, &acct, year, month, day);
    }

    closedir(dirp);
  }

  write_birthmon(OUTFILE_BIRTHDAY, year, month, day);

  qsort(toplogins, TOPNUM, sizeof(DATA), sort_compare);
  write_data(OUTFILE_TOPLOGIN, "�W�����ƱƦ�]", &toplogins);

  qsort(topposts, TOPNUM, sizeof(DATA), sort_compare);
  write_data(OUTFILE_TOPPOST, "����g�ƭ^���]", &topposts);

  qsort(topmoney, TOPNUM, sizeof(DATA), sort_compare);
  write_data(OUTFILE_TOPMONEY, "�ȹ��I�Ϋʯ��]", &topmoney);

  qsort(topgold, TOPNUM, sizeof(DATA), sort_compare);
  write_data(OUTFILE_TOPGOLD, "�����I����g�]", &topgold);

  write_age(OUTFILE_AGE, year, month, day);

  write_star(OUTFILE_STAR, year, month, day);

  write_sex(OUTFILE_SEX, year, month, day);

  return 0;
}
