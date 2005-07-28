/*-------------------------------------------------------*/
/* util/transbrd.c					 */
/*-------------------------------------------------------*/
/* target : Maple Sob 2.36 �� Maple 3.02 �ݪO�ഫ	 */
/*          .BOARDS => .BRD				 */
/* create :   /  /  					 */
/* update : 98/06/14					 */
/* author : ernie@micro8.ee.nthu.edu.tw			 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/
/* syntax : transbrd [target_board]			 */
/*-------------------------------------------------------*/

#if 0

   1. �ק� struct boardheader �� transbrd()
       (boardheader �⪩�w�q���r����פ��@�A�Цۦ洫���Ʀr)
   2. �벼���ഫ
   3. �i�O�e�� copy
   4. �p���ݭn�� chmod 644 `find PATH -perm 600`
   5. �} gem �ؿ� gem/target_board/? �����ഫ gem
   6. ���|��s bshm�A�ϥΫ�Цۦ��s
   7. �ഫ��Ф�ʳ]�ݪO�v��

   ps. Use on ur own risk.

#endif


#include "sob.h"


static time_t
trans_hdr_chrono(filename)
  char *filename;
{
  char time_str[11];

  /* M.1087654321.A �� M.987654321.A */
  str_ncpy(time_str, filename + 2, filename[2] == '1' ? 11 : 10);

  return (time_t) atoi(time_str);
}


static void
trans_hdr_stamp(folder, t, hdr, fpath)
  char *folder;
  time_t t;
  HDR *hdr;
  char *fpath;
{
  FILE *fp;
  char *fname, *family;
  int rc;

  fname = fpath;
  while (rc = *folder++)
  {
    *fname++ = rc;
    if (rc == '/')
      family = fname;
  }
  fname = family + 1;
  *fname++ = '/';
  *fname++ = 'A';

  for (;;)
  {
    *family = radix32[t & 31];
    archiv32(t, fname);

    if (fp = fopen(fpath, "r"))
    {
      fclose(fp);
      t++;
    }
    else
    {
      memset(hdr, 0, sizeof(HDR));
      hdr->chrono = t;
      str_stamp(hdr->date, &hdr->chrono);
      strcpy(hdr->xname, --fname);
      break;
    }
  }
}


/* ----------------------------------------------------- */
/* �ഫ�D�{��						 */
/* ----------------------------------------------------- */


static void
transbrd(bh)
  boardheader *bh;
{
  static time_t stamp = 0;

  int fd;
  char index[64], folder[64], buf[64], fpath[64];
  fileheader fh;
  HDR hdr;
  BRD newboard;
  time_t chrono;

  printf("�ഫ %s �ݪO\n", bh->brdname);

  brd_fpath(buf, bh->brdname, NULL);
  if (dashd(buf))
  {
    printf("%s �w�g�����ݪO\n", bh->brdname);
    return;
  }

  if (!stamp)
    time(&stamp);

  /* �ഫ .BRD */

  memset(&newboard, 0, sizeof(newboard));
  str_ncpy(newboard.brdname, bh->brdname, sizeof(newboard.brdname));
  str_ncpy(newboard.class, bh->title, sizeof(newboard.class));
  str_ncpy(newboard.title, bh->title + 4, sizeof(newboard.title));
  str_ncpy(newboard.BM, bh->BM, sizeof(newboard.BM));
  newboard.bstamp = stamp++;
  newboard.battr = BRD_NOTRAN;				/* �w�]����H */
  newboard.readlevel = 0;
  newboard.postlevel = PERM_POST;

  rec_add(FN_BRD, &newboard, sizeof(newboard));		/* �O�ѤF�� brd2gem.c ���ഫ Class */

  /* �}�s�ؿ� */

  sprintf(fpath, "gem/brd/%s", newboard.brdname);
  mak_dirs(fpath);
  mak_dirs(fpath + 4);

  /* �ഫ�i�O�e�� */

  sprintf(buf, OLD_BBSHOME "/boards/%s/notes", bh->brdname);

  if (dashf(buf))
  {
    brd_fpath(fpath, newboard.brdname, FN_NOTE);
    f_cp(buf, fpath, O_TRUNC);
  }

  /* �ഫ�峹 */

  sprintf(index, OLD_BBSHOME "/boards/%s/.DIR", bh->brdname);	/* �ª� .DIR */
  brd_fpath(folder, newboard.brdname, ".DIR");			/* �s�� .DIR */

  if ((fd = open(index, O_RDONLY)) >= 0)
  {
    while (read(fd, &fh, sizeof(fh)) == sizeof(fh))
    {
      sprintf(buf, OLD_BBSHOME "/boards/%s/%s", bh->brdname, fh.filename);
      if (dashf(buf))	/* �峹�ɮצb�~���ഫ */
      {
	/* �ഫ�峹 .DIR */
	memset(&hdr, 0, sizeof(HDR));
	chrono = trans_hdr_chrono(fh.filename);
	trans_hdr_stamp(folder, chrono, &hdr, fpath);
	str_ncpy(hdr.owner, fh.owner, sizeof(hdr.owner));
	str_ansi(hdr.title, fh.title, sizeof(hdr.title));
	hdr.xmode = (fh.filemode & 0x2) ? POST_MARKED : 0;
	rec_add(folder, &hdr, sizeof(HDR));

	/* �����ɮ� */
	f_cp(buf, fpath, O_TRUNC);
      }
    }
    close(fd);
  }
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int fd;
  boardheader bh;

  /* argc == 1 ������O */
  /* argc == 2 ��Y�S�w�O */

  if (argc > 2)
  {
    printf("Usage: %s [target_board]\n", argv[0]);
    exit(-1);
  }

  chdir(BBSHOME);

  if (!dashf(FN_BOARD))
  {
    printf("ERROR! Can't open " FN_BOARD "\n");
    exit(-1);
  }
  if (!dashd(OLD_BBSHOME "/boards"))
  {
    printf("ERROR! Can't open " OLD_BBSHOME "/boards\n");
    exit(-1);
  }

  if ((fd = open(FN_BOARD, O_RDONLY)) >= 0)
  {
    while (read(fd, &bh, sizeof(bh)) == sizeof(bh))
    {
      if (argc == 1)
      {
	transbrd(&bh);
      }
      else if (!strcmp(bh.brdname, argv[1]))
      {
	transbrd(&bh);
	exit(1);
      }
    }
    close(fd);
  }

  exit(0);
}
