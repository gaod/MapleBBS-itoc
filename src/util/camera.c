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


static void
str_strip(str, size)		/* itoc.060417: �N�ʺA�ݪO�C�C���e���t�b SCR_WIDTH */
  char *str;
  int size;
{
  int ch, ansi, len;
  char *ptr;
  const char *strip = "\033[m\n";

  /* �Y�ʺA�ݪO���@�C���e�׶W�L SCR_WIDTH�A�|��ܤG�C�A�y���ƪ����~ (�D�n�O�I�q������)
     �ҥH�N���ܧ�W�L SCR_WIDTH �������R�� (�b�����Ҽ{�e�ù�) */
  /* �Y���C���� \033*s �� \033*n�A��ܥX�ӷ|����A�ҥH�n�S�O�B�z */

  ansi = len = 0;
  ptr = str;
  while (ch = *ptr++)
  {
    if (ch == '\n')
    {
      break;
    }
    else if (ch == '\033')
    {
      ansi = 1;
    }
    else if (ansi)
    {
      if (ch == '*')	/* KEY_ESC + * + s/n �q�X ID/username�A�Ҽ{�̤j���� */
      {
	ansi = 0;
	len += BMAX(IDLEN, UNLEN) - 1;
	if (len > SCR_WIDTH)
	{
	  if (ptr - 1 + strlen(strip) < str + size)	/* �קK overflow */
	    strcpy(ptr - 1, strip);
	  else
	    strcpy(ptr - 1, "\n");
	  break;
	}
      }
      else if ((ch < '0' || ch > '9') && ch != ';' && ch != '[')
	ansi = 0;
    }
    else
    {
      if (++len > SCR_WIDTH)
      {
	if (ptr - 1 + strlen(strip) < str + size)	/* �קK overflow */
	  strcpy(ptr - 1, strip);
	else
	  strcpy(ptr - 1, "\n");
	break;
      }
    }
  }
}


static FCACHE image;
static int number;	/* �ثe�w mirror �X�g�F */
static int total;	/* �ثe�w mirror �X byte �F */


static int		/* 1:���\ 0:�w�W�L�g�Ʃήe�q */
mirror(fpath, line)
  char *fpath;
  int line;		/* 0:�t�Τ��A������C��  !=0:�ʺA�ݪO�Aline �C */
{
  int size, i;
  char buf[FILM_SIZ];
  char tmp[ANSILINELEN];
  struct stat st;
  FILE *fp;

  /* �Y�w�g�W�L�̤j�g�ơA�h���A�~�� mirror */
  if (number >= MOVIE_MAX - 1)
    return 0;

  if (stat(fpath, &st))
    size = -1;
  else
    size = st.st_size;

  if (size > 0 && size < FILM_SIZ && (fp = fopen(fpath, "r")))
  {
    size = i = 0;
    while (fgets(tmp, ANSILINELEN, fp))
    {
      str_strip(tmp, ANSILINELEN);

      strcpy(buf + size, tmp);
      size += strlen(tmp);
  
      if (line)
      {
	/* �ʺA�ݪO�A�̦h line �C */
	if (++i >= line)
	  break;
      }
    }
    fclose(fp);

    if (i != line)	
    {
      /* �ʺA�ݪO�A�Y���� line �C�A�n�� line �C */
      for (; i < line; i++)
      {
	buf[size] = '\n';
	size++;
      }
      buf[size] = '\0';
    }
  }

  if (size <= 0 || size >= FILM_SIZ)
  {
    if (line)		/* �p�G�O �ʺA�ݪO/�I�q ���ɮסA�N�� mirror */
      return 1;

    /* �p�G�O�t�Τ��X�����ܡA�n�ɤW�h */
    sprintf(buf, "�Чi�D�����ɮ� %s �򥢩άO�L�j", fpath);
    size = strlen(buf);
  }

  size++;	/* Thor.980804: +1 �N������ '\0' �]��J */

  i = total + size;
  if (i >= MOVIE_SIZE)	/* �Y�[�J�o�g�|�W�L�����e�q�A�h�� mirror */
    return 0;

  memcpy(image.film + total, buf, size);
  image.shot[++number] = total = i;

  return 1;
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
      if (hdr.xmode & (GEM_RESTRICT | GEM_RESERVED | GEM_BOARD | GEM_LINE))	/* ����šB�ݪO�B���j�u ����J movie �� */
	continue;

      hdr_fpath(fpath, folder, &hdr);

      if (hdr.xmode & GEM_FOLDER)	/* �J����v�h�j��i�h�� movie */
      {
	do_gem(fpath);
      }
      else				/* plain text */
      {
	if (!mirror(fpath, MOVIE_LINES))
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
  (4) �� 16-20 �� bytes �N���~�A��s�~�O���ѡC�Ҧp "02/15" ��ܶ���G��Q����O�A��s�~

#endif

  #define TABLE_INITAIL_YEAR	2005
  #define TABLE_FINAL_YEAR	2016

  /* �Ѧ� http://sean.tw.googlepages.com/calendar.htm �ӱo */
  char Table[TABLE_FINAL_YEAR - TABLE_INITAIL_YEAR + 1][21] = 
  {
    "-L-L-LL-L-L-,X,02/09",	/* 2005 ���~ */
    "L-L-L-L-LL-L,7,01/29",	/* 2006 ���~ */
    "--L--L-LLL-L,X,02/18",	/* 2007 �ަ~ */
    "L--L--L-LL-L,X,02/07",	/* 2008 ���~ */
    "LL--L-L-L-LL,5,01/26",	/* 2009 ���~ */
    "L-L-L--L-L-L,X,02/14",	/* 2010 ��~ */
    "L-LL-L--L-L-,X,02/03",	/* 2011 �ߦ~ */
    "L-LLL-L-L-L-,4,01/23",	/* 2012 �s�~ */
    "L-L-LL-L-L-L,X,02/10",	/* 2013 �D�~ */
    "-L-L-L-LLL-L,9,01/31",	/* 2014 ���~ */
    "-L--L-LLL-L-,X,02/19",	/* 2015 �Ϧ~ */
    "L-L--L-LL-LL,X,02/08",	/* 2016 �U�~ */
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


static char *
do_today()
{
  FILE *fp;
  char buf[80], *ptr1, *ptr2, *ptr3, *today;
  char key1[6];			/* mm/dd: ���� mm��dd�� */
  char key2[6];			/* mm/#A: ���� mm�몺��#�ӬP��A */
  char key3[6];			/* MM\DD: �A�� MM��DD�� */
  time_t now;
  struct tm *ptime;
  static char feast[64];

  time(&now);
  ptime = localtime(&now);
  sprintf(key1, "%02d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
  sprintf(key2, "%02d/%d%c", ptime->tm_mon + 1, (ptime->tm_mday - 1) / 7 + 1, ptime->tm_wday + 'A');
  lunar_calendar(key3, &now, ptime);

  today = image.today;
  sprintf(today, "%s %.2s", key1, "��@�G�T�|����" + (ptime->tm_wday << 1));

  if (fp = fopen(FN_ETC_FEAST, "r"))
  {
    while (fgets(buf, 80, fp))
    {
      if (buf[0] == '#')
	continue;

      if ((ptr1 = strtok(buf, " \t\n")) && (ptr2 = strtok(NULL, " \t\n")))
      {
	if (!strcmp(ptr1, key1) || !strcmp(ptr1, key2) || !strcmp(ptr1, key3))
	{
	  str_ncpy(today, ptr2, sizeof(image.today));

	  if (ptr3 = strtok(NULL, " \t\n"))
	    sprintf(feast, "etc/feasts/%s", ptr3);
	  if (!dashf(feast))
	    feast[0] = '\0';

	  break;
	}
      }
    }
    fclose(fp);
  }

  return feast;
}


int
main()
{
  int i;
  char *fname, *str, *feast, fpath[64];
  FCACHE *fshm;

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  number = total = 0;

  /* --------------------------------------------------- */
  /* ���Ѹ`��					 	 */
  /* --------------------------------------------------- */

  feast = do_today();

  /* --------------------------------------------------- */
  /* ���J�`�Ϊ����� help			 	 */
  /* --------------------------------------------------- */

  strcpy(fpath, "gem/@/@");
  fname = fpath + 7;

  for (i = 0; str = list[i]; i++)
  {
    if (i >= FILM_OPENING0 && i <= FILM_OPENING2 && feast[0])	/* �Y�O�`��A�}�Y�e���θӸ`�骺�e�� */
    {
      mirror(feast, 0);
    }
    else
    {
      strcpy(fname, str);
      mirror(fpath, 0);
    }
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

  image.shot[0] = number;	/* �`�@���X�� */

  fshm = (FCACHE *) shm_new(FILMSHM_KEY, sizeof(FCACHE));
  memcpy(fshm, &image, sizeof(FCACHE));

  /* printf("%d/%d films, %d/%d bytes\n", number, MOVIE_MAX, total, MOVIE_SIZE); */

  exit(0);
}
