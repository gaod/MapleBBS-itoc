/*-------------------------------------------------------*/
/* util/gem-expire.c	( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : �p�⥼�s��ذ� & ���s�Ѽƺ�ذϱƦ�]	 */
/* create : 99/11/26                                     */
/* update : 01/08/27					 */
/* author : Jimmy.bbs@whshs.twbbs.org			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/
/* syntax : gem-expire					 */
/*-------------------------------------------------------*/


#include "bbs.h"

#define OUTFILE_GEMEMPTY	"gem/@/@-gem_empty"
#define OUTFILE_GEMOVERDUE	"gem/@/@-gem_overdue"


/*-------------------------------------------------------*/
/* BRD shm �������P cache.c �ۮe			 */
/*-------------------------------------------------------*/


static BCACHE *bshm;


static void
init_bshm()
{
  /* itoc.030727: �b�}�� bbsd ���e�A���ӴN�n����L account�A
     �ҥH bshm ���Ӥw�]�w�n */

  bshm = shm_new(BRDSHM_KEY, sizeof(BCACHE));

  if (bshm->uptime <= 0)	/* bshm ���]�w���� */
    exit(0);
}


/*-------------------------------------------------------*/
/* �D�{��						 */
/*-------------------------------------------------------*/


typedef struct
{
  int day;			/* �X�ѨS��z��ذ� */
  char brdname[BNLEN + 1];	/* �O�W */
  char BM[BMLEN + 1];		/* �O�D */
}	BRDDATA;


static int
int_cmp(a, b)
  BRDDATA *a, *b;
{
  return (b->day - a->day);	/* �Ѥj�ƨ�p */
}


static BRDDATA board[MAXBOARD];
static int locus = 0;			/* �`�@�O���F�X�ӪO */


static void
topgem()
{
  time_t now;
  struct stat st;
  BRD *bhdr, *tail;
  char fpath[64], *brdname;

  time(&now);

  bhdr = bshm->bcache;
  tail = bhdr + bshm->number;
  do
  {
    /* ���L���C�J�Ʀ�]���ݪO */
    if ((bhdr->readlevel | bhdr->postlevel) >= (PERM_VALID << 1))	/* (BASIC + ... + VALID) < (VALID << 1) */
      continue;

    brdname = bhdr->brdname;
    sprintf(fpath, "gem/brd/%s/@/@log", brdname);

    if (stat(fpath, &st) != -1)	/* ����ذϪ��ˬd�X�ѥ��s */
      board[locus].day = (now - st.st_mtime) / 86400;
    else			/* �L��ذϪ� */
      board[locus].day = 999;
    strcpy(board[locus].brdname, brdname);
    strcpy(board[locus].BM, bhdr->BM);

    locus++;
  } while (++bhdr < tail);

  qsort(board, locus, sizeof(BRDDATA), int_cmp);
}


static void
write_data()
{
  time_t now;
  struct tm *ptime;
  FILE *fpe, *fpo;
  int i, m, n;  

  time(&now);
  ptime = localtime(&now);

  fpe = fopen(OUTFILE_GEMEMPTY, "w");
  fpo = fopen(OUTFILE_GEMOVERDUE, "w");

  fprintf(fpe,
    "         \033[1;34m-----\033[37m=====\033[41m �ݪO��ذϥ��s���ݪO (�� %d �� %d ���) \033[;1;37m=====\033[34m-----\033[m\n"
    "           \033[1;42m �W�� \033[44m   �ݪO�W��   \033[42m      ��ذϥ��s      \033[44m   �O   �D    \033[m\n",
    ptime->tm_mon + 1, ptime->tm_mday);

  fprintf(fpo,
    "        \033[1;34m-----\033[37m=====\033[41m �ݪO��ذϥ��s�ѼƤ��ݪO (�� %d �� %d ���) \033[;1;37m=====\033[34m-----\033[m\n"
    "              \033[1;42m �W�� \033[44m    �ݪO�W��    \033[42m ��ذϥ��s�Ѽ� \033[44m   �O   �D    \033[m\n",
    ptime->tm_mon + 1, ptime->tm_mday);

  m = 1;
  n = 1;

  for (i = 0; i < locus; i++)
  {
    if (board[i].day == 999)
    {
      fprintf(fpe, "            %3d   %12s     %s      %.20s\n",
	m, board[i].brdname, "�|���s���ذ�", board[i].BM);
      m++;
    }
    else
    {
      fprintf(fpo, "                %s%3d    %12s        %4d       %.20s\033[m\n",
	n <= 3 ? "\033[1m" : (n <= 10 ? "\033[1;31m" : "\033[m"),
	n, board[i].brdname, board[i].day, board[i].BM);
      n++;
    }
  }

  fclose(fpe);
  fclose(fpo);
}


int 
main()
{
  chdir(BBSHOME);

  init_bshm();

  topgem();
  write_data();

  return 0;
}
