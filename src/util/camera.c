/*-------------------------------------------------------*/
/* util/camera.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : �إ� [�ʺA�ݪO] cache			 */
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


static int		/* 1:���\ 0:�w�W�L�e�q */
play(data, movie, size)
  char *data;
  int movie;
  int size;
{
  int line, ch;
  char *head;

  head = data;

  if (movie)		/* �ʺA�ݪO�A�n������ */
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

  ch = size + 1;	/* Thor.980804: +1 �N������ '\0' �]��J */

  line = tail + ch;
  if (line >= MOVIE_SIZE)	/* �Y�[�J�o�g�|�W�L�����e�q�A�h�� mirror */
    return 0;

  data = image.film + tail;
  memcpy(data, head, ch);
  image.shot[++number] = tail = line;
  return 1;
}


static int		/* 1:���\ 0:�w�W�L�g�� */
mirror(fpath, movie)
  char *fpath;
  int movie;		/* 0:�t�Τ��A������C��  !=0:�ʺA�ݪO�A�n����C�� */
{
  int fd, size;
  char buf[FILM_SIZ + 1];

  /* �Y�w�g�W�L�̤j�g�ơA�h���A�~�� mirror */
  if (number >= MOVIE_MAX - 1)
    return 0;

  size = 0;
  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    /* Ū�J�ɮ� */
    size = read(fd, buf, FILM_SIZ);
    close(fd);
  }

  if (size <= 0)
  {
    if (movie)		/* �p�G�O �ʺA�ݪO/�I�q ���ɮסA�N�� mirror */
      return 1;

    /* �p�G�O�t�Τ����ɮת��ܡA�n�ɤW�h */
    sprintf(buf, "�Чi�D�������ɮ� %s", fpath);
    size = strlen(buf);
  }

  buf[size] = '\0';
  return play(buf, movie, size);
}


static void
do_gem(folder)		/* itoc.011105: ��ݪO/��ذϪ��峹���i movie */
  char *folder;		/* index ���| */
{
  char fpath[64];
  FILE *fp;
  HDR hdr;

  if (fp = fopen(folder, "r"))
  {
    while (fread(&hdr, sizeof(HDR), 1, fp) == 1)
    {
      if (hdr.xmode & (GEM_RESTRICT | GEM_RESERVED | GEM_BOARD))	/* ����šB�ݪO ����J movie �� */
	continue;

      hdr_fpath(fpath, folder, &hdr);

      if (hdr.xmode & GEM_FOLDER)	/* �J����v�h�j��i�h�� movie */
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
lunar_calendar(key, now, ptime)	/* itoc.050528: �Ѷ����A���� */
  char *key;
  time_t *now;
  struct tm *ptime;
{
#if 0	/* Table ���N�q */

  (1) "," �u�O���j�A��K���H�\Ū�Ӥw
  (2) �e 12 �� byte ���O�N��A�� 1-12 �묰�j��άO�p��C"L":�j��T�Q�ѡA"-":�p��G�Q�E��
  (3) �� 14 �� byte �N���~�A��|��C"X":�L�|��A"123456789:;<" ���O�N��A�� 1-12 �O�|��
  (4) �� 16-20 �� bytes �N���~�A��s�~�O���ѡC�Ҧp "02:15" ��ܶ���G��Q����O�A��s�~

#endif

  #define TABLE_INITAIL_YEAR	2005
  #define TABLE_FINAL_YEAR	2011

  char Table[TABLE_FINAL_YEAR - TABLE_INITAIL_YEAR + 1][21] = 
  {
    "-L-L-LL-L-L-,X,02:09",	/* 2005 ���~ */
    "L-L-L-L-LL-L,7,01:29",	/* 2006 ���~ */
    "--L--L-LLL-L,X,02:18",	/* 2007 �ަ~ */
    "L--L--L-LL-L,X,02:07",	/* 2008 ���~ */
    "LL--L--L-L-L,5,01:26",	/* 2009 ���~ */
    "L-L-L--L-L-L,X,02:14",	/* 2010 ��~ */
    "L-LL-L--L-L-,X,02:14",	/* 2011 �ߦ~ */
  };

  char year[21];

  time_t nyd;	/* ���~���A��s�~ */
  struct tm ntime;

  int i;
  int Mon, Day;
  int leap;	/* 0:���~�L�|�� */

  /* ����X���ѬO�A����@�~ */

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

  /* �A�q�A�䥿���@�}�l�ƨ줵�� */

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
	year[Mon - 1] = '-';	/* �|�륲�p�� */
      }
      else if (year[13] == Mon + '0')
      {
	if (leap == 1)		/* �U��O�|�� */
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
  char key1[6];			/* mm/dd: ���� mm��dd�� */
  char key2[6];			/* mm/#A: ���� mm�몺��#�ӬP��A */
  char key3[6];			/* MM\DD: �A�� MM��DD�� */
  time_t now;
  struct tm *ptime;

  time(&now);
  ptime = localtime(&now);
  sprintf(key1, "%02d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
  sprintf(key2, "%02d/%d%c", ptime->tm_mon + 1, (ptime->tm_mday - 1) / 7 + 1, ptime->tm_wday + 'A');
  lunar_calendar(key3, &now, ptime);

  today = fshm->today;
  sprintf(today, "%s %.2s", key1, "��@�G�T�|����" + (ptime->tm_wday << 1));

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
	  /* ���L�ťդ��j */
	  for (str++; *str && isspace(*str); str++)
	    ;

	  str_ncpy(today, str, sizeof(fshm->today));
	  if (str = (char *) strchr(today, '\n'))	/* �̫᪺ '\n' ���n */
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

  number = 0;		/* �p�⦳�X�g�ʺA�ݪO�Ϊ� */
  tail = 0;

  /* --------------------------------------------------- */
  /* ���J�`�Ϊ����� help			 	 */
  /* --------------------------------------------------- */

  strcpy(fpath, "gem/@/@");
  fname = fpath + 7;

  for (i = 0; str = list[i]; i++)
  {
    strcpy(fname, str);
    mirror(fpath, 0);
  }

  /* itoc.����: ��o�̥H��A���Ӥw�� FILM_MOVIE �g */

  /* --------------------------------------------------- */
  /* ���J�ʺA�ݪO					 */
  /* --------------------------------------------------- */

  /* itoc.����: �ʺA�ݪO���I�q���X�p�u�� MOVIE_MAX - FILM_MOVIE - 1 �g�~�|�Q���i movie */

  sprintf(fpath, "gem/brd/%s/@/@note", BN_CAMERA);	/* �ʺA�ݪO���s���ɮצW�����R�W�� @note */
  do_gem(fpath);					/* �� [note] ��ذϦ��i movie */

#ifdef HAVE_SONG_CAMERA
  brd_fpath(fpath, BN_KTV, FN_DIR);
  do_gem(fpath);					/* ��Q�I���q���i movie */
#endif

  /* --------------------------------------------------- */
  /* resolve shared memory				 */
  /* --------------------------------------------------- */

  fshm = (FCACHE *) shm_new(FILMSHM_KEY, sizeof(FCACHE));
  memcpy(fshm, &image, sizeof(image.shot) + tail);
  /* Thor.980805: �A�[�W shot������ */
  fshm->shot[0] = number;	/* �`�@���X�� */

  /* printf("%d/%d films, %d/%d bytes\n", number, MOVIE_MAX, tail, MOVIE_SIZE); */

  do_today(fshm);

  exit(0);
}
