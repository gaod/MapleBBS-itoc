/*-------------------------------------------------------*/
/* util/transacct.c	( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : M3 BRD �ഫ�{��				 */
/* create : 05/05/19					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

  �ϥΤ�k�G

  0. For Maple 3.X To Maple 3.X

  1. �ϥΫe�Х��ƥ� .BRD

  2. �Цۦ�� struct NEW �M struct OLD

#endif


#include "bbs.h"


/* ----------------------------------------------------- */
/* (�s��) �ݪO .BRD struct				 */
/* ----------------------------------------------------- */


typedef struct			/* �n�M�s���{�� struct �@�� */
{
  char brdname[BNLEN + 1];	/* board name */
  char class[BCLEN + 1];
  char title[BTLEN + 1];
  char BM[BMLEN + 1];		/* BMs' uid, token '/' */

  char bvote;			/* 0:�L�벼 -1:����L(�i�঳�벼) 1:���벼 */

  time_t bstamp;		/* �إ߬ݪO���ɶ�, unique */
  usint readlevel;		/* �\Ū�峹���v�� */
  usint postlevel;		/* �o��峹���v�� */
  usint battr;			/* �ݪO�ݩ� */
  time_t btime;			/* -1:bpost/blast �ݭn��s */
  int bpost;			/* �@���X�g post */
  time_t blast;			/* �̫�@�g post ���ɶ� */
}	NEW;


/* ----------------------------------------------------- */
/* (�ª�) �ݪO .BRD struct				 */
/* ----------------------------------------------------- */


typedef struct			/* �n�M�ª��{�� struct �@�� */
{
  char brdname[BNLEN + 1];	/* board name */
  char class[BCLEN + 1];
  char title[BTLEN + 1];
  char BM[BMLEN + 1];		/* BMs' uid, token '/' */

  char bvote;			/* 0:�L�벼 -1:����L(�i�঳�벼) 1:���벼 */

  time_t bstamp;		/* �إ߬ݪO���ɶ�, unique */
  usint readlevel;		/* �\Ū�峹���v�� */
  usint postlevel;		/* �o��峹���v�� */
  usint battr;			/* �ݪO�ݩ� */
  time_t btime;			/* -1:bpost/blast �ݭn��s */
  int bpost;			/* �@���X�g post */
  time_t blast;			/* �̫�@�g post ���ɶ� */
}	OLD;


/* ----------------------------------------------------- */
/* �ഫ�D�{��						 */
/* ----------------------------------------------------- */


#define FN_BRD_TMP	".BRD.tmp"


int
main()
{
  int fd;
  OLD old;
  NEW new;

  chdir(BBSHOME);

  if ((fd = open(FN_BRD, O_RDONLY)) >= 0)
  {
    while (read(fd, &old, sizeof(OLD)) == sizeof(OLD))
    {
      if (!*old.brdname)	/* ���O�w�Q�R�� */
	continue;

      memset(&new, 0, sizeof(NEW));

      /* �ഫ���ʧ@�b�� */
      str_ncpy(new.brdname, old.brdname, sizeof(new.brdname));
      str_ncpy(new.class, old.class, sizeof(new.class));
      str_ncpy(new.title, old.title, sizeof(new.title));
      str_ncpy(new.BM, old.BM, sizeof(new.BM));
      new.bvote = old.bvote;
      new.bstamp = old.bstamp;
      new.readlevel = old.readlevel;
      new.postlevel = old.postlevel;
      new.battr = old.battr;
      new.btime = old.btime;
      new.bpost = old.bpost;
      new.blast = old.blast;

      rec_add(FN_BRD_TMP, &new, sizeof(NEW));
    }
    close(fd);
  }

  /* �R���ª��A��s����W */
  unlink(FN_BRD);
  rename(FN_BRD_TMP, FN_BRD);

  return 0;
}
