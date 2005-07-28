/*-------------------------------------------------------*/
/* util/fb/fb2brd.c					 */
/*-------------------------------------------------------*/
/* target : firebird 3.0 �� Maple 3.x �ݪO		 */
/*          .BOARDS => .BRD				 */
/* create : 00/11/22					 */
/* update :   /  /					 */
/* author : hightman@263.net				 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#define TRANS_BITS_BRD
#define TRANS_BITS_PERM


#include "fb.h"


static inline void
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
  char *ptr, index[64], folder[64], buf[64], fpath[64];
  fileheader fh;
  HDR hdr;
  BRD newboard;
  BITS *p;
  time_t chrono;

  printf("�ഫ %s �ݪO\n", bh->filename);

  brd_fpath(buf, bh->filename, NULL);
  if (dashd(buf))
  {
    printf("%s �w�g�����ݪO\n", bh->filename);
    return;
  }

  if (!stamp)
    time(&stamp);

  /* �ഫ .BRD */

  memset(&newboard, 0, sizeof(BRD));
  str_ncpy(newboard.brdname, bh->filename, sizeof(newboard.brdname));
  str_ncpy(newboard.class, (bh->flag |= 0x4) ? "����" : "����", sizeof(newboard.class));
  str_ncpy(newboard.title, bh->title + 10, sizeof(newboard.title));
  newboard.bstamp = stamp++;
  if (bh->BM[0] > ' ')
  {
    /* �N�Ҧ��� ',' ���� '/' */
    for (ptr = bh->BM; *ptr; ptr++)
    {
       if (*ptr == ',')
	*ptr = '/';
    }
    str_ncpy(newboard.BM, bh->BM, sizeof(newboard.BM));
  }
  for (p = flag; p->old; p++)
  {
    if (bh->flag & p->old)
      newboard.battr |= p->new;
  }
  for (p = perm; p->old; p++)
  {
    if (bh->level & p->old)
      newboard.readlevel |= p->new;
  }
  newboard.postlevel = newboard.readlevel;
  rec_add(FN_BRD, &newboard, sizeof(BRD));

  /* �}�s�ؿ� */

  sprintf(fpath, "gem/brd/%s", newboard.brdname);
  mak_dirs(fpath);
  mak_dirs(fpath + 4);

  /* �ഫ�i�O�e�� */

  sprintf(buf, OLD_BBSHOME "/boards/%s/notes", bh->filename);

  if (dashf(buf))
  {
    brd_fpath(fpath, newboard.brdname, FN_NOTE);
    f_cp(buf, fpath, O_TRUNC);
  }

  /* �ഫ�벼���G */

  sprintf(buf, OLD_BBSHOME "/boards/%s/results", bh->filename);

  if (dashf(buf))
  {
    brd_fpath(fpath, newboard.brdname, "@/@vote");
    f_cp(buf, fpath, O_TRUNC);
  }

  /* �ഫ�峹 */

  sprintf(index, OLD_BBSHOME "/boards/%s/.DIR", bh->filename);	/* �ª� .DIR */
  brd_fpath(folder, newboard.brdname, ".DIR");			/* �s�� .DIR */

  if ((fd = open(index, O_RDONLY)) >= 0)
  {
    while (read(fd, &fh, sizeof(fh)) == sizeof(fh))
    {
      sprintf(buf, OLD_BBSHOME "/boards/%s/%s", bh->filename, fh.filename);
      if (dashf(buf))	/* �峹�ɮצb�~���ഫ */
      {
	struct stat st;

	/* �ഫ�峹 .DIR */
	memset(&hdr, 0, sizeof(HDR));
	stat(buf, &st);
	chrono = st.st_mtime;
	trans_hdr_stamp(folder, chrono, &hdr, fpath);
	str_ncpy(hdr.owner, fh.owner, sizeof(hdr.owner));
	str_ansi(hdr.title, fh.title, sizeof(hdr.title));
	hdr.xmode = 0;
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
      else if (!strcmp(bh.filename, argv[1]))
      {
	transbrd(&bh);
	exit(1);
      }
    }
    close(fd);
  }

  exit(0);
}
