/*-------------------------------------------------------*/
/* util/brd2gem.c	( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : �ݪO�������ذ�			         */
/* create : 01/09/09					 */
/* update : 03/02/13                                     */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/
/* syntax : brd2gem ���� Classname			 */
/*-------------------------------------------------------*/


#include "bbs.h"


#if 0	/* �ϥΤ�k */

  �o�ӵ{���O���ӵ� bbs �����ഫ�ɡA�۰ʫإ� @Class �Ϊ��C
  �άO�b�~������ɡA�]�i�H���X�ӭ��� @Class �ΡC

  ���]�n��Ҧ��������u�t�Ρv�M�u�����v���ݪO����b�uBBS�v�o�Ӥ����̭��A
  �H�Χ�Ҧ��������u�ӤH�v���ݪO����b�uPersonal�v�o�Ӥ����̭��C
  
  1. �W BBS ���A�b (A)nnounce/Class �̭� Ctrl+P �� (C)�A�إ��ɦW�� BBS ������ (���D���N)�C
  2. �W BBS ���A�b (A)nnounce/Class �̭� Ctrl+P �� (C)�A�إ��ɦW�� Personal ������ (���D���N)�C
  3. �b�u�@�����H bbs ��������
     % ~bbs/src/util/tran/brd2gem �t�� BBS
     % ~bbs/src/util/tran/brd2gem ���� BBS
     % ~bbs/src/util/tran/brd2gem �ӤH Personal

#endif


static void
brd_2_gem(brd, gem)
  BRD *brd;
  HDR *gem;
{
  memset(gem, 0, sizeof(HDR));
  time(&gem->chrono);
  strcpy(gem->xname, brd->brdname);
  sprintf(gem->title, "%-13s%-5s%s", brd->brdname, brd->class, brd->title);
  gem->xmode = GEM_BOARD | GEM_FOLDER;

#ifdef HAVE_MODERATED_BOARD
  /* ���K�O�B�n�ͪO */
  if (brd->readlevel == PERM_SYSOP || brd->readlevel == PERM_BOARD)
    gem->xmode |= GEM_RESTRICT;
#endif
}


static int
hdr_cmp(a, b)
  HDR *a;
  HDR *b;
{
  /* itoc.010413: ����/�O�W��e��� */
  int k = strncmp(a->title + BNLEN + 1, b->title + BNLEN + 1, BCLEN);
  return k ? k : str_cmp(a->xname, b->xname);
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int fd;
  char folder[64];
  BRD brd;
  HDR hdr;

  chdir(BBSHOME);

  if (argc != 3)
  {
    printf("Usage: %s ���� Classname\n", argv[0]);
    exit(-1);
  }

  if (strlen(argv[1]) > BCLEN || strlen(argv[2]) > BNLEN)
  {
    printf("�u�����v�n�u�� %d�AClassname �n�u�� %d\n", BCLEN, BNLEN);
    exit(-1);
  }

  sprintf(folder, "gem/@/@%s", argv[2]);

  if ((fd = open(FN_BRD, O_RDONLY)) >= 0)
  {
    while (read(fd, &brd, sizeof(BRD)) == sizeof(BRD))
    {
      if (!strcmp(brd.class, argv[1]))
      {
	brd_2_gem(&brd, &hdr);
	rec_add(folder, &hdr, sizeof(HDR));
      }
    }
    close(fd);
  }

  rec_sync(folder, sizeof(HDR), hdr_cmp, NULL);
}
