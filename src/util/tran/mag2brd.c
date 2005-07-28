/*-------------------------------------------------------*/
/* util/transbrd.c					 */
/*-------------------------------------------------------*/
/* target : Magic �� Maple 3.02 �ݪO�ഫ		 */
/*          .BOARDS => .BRD				 */
/* create : 02/09/09					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

  0. �A�� Magic/FireBird �� Maple ��ذ�
  1. �{�����}�ؿ��A�ϥΫe���T�w gem/target_board/? �ؿ��s�b
     if not�A���}�s�O or transbrd

  ps. User on ur own risk.

#endif


#include "mag.h"


/*-------------------------------------------------------*/
/* basic function					 */
/*-------------------------------------------------------*/


static inline time_t
trans_hdr_chrono(filename)
  char *filename;
{
  char time_str[11];

  /* M.1087654321.A �� M.987654321.A */
  str_ncpy(time_str, filename + 2, filename[2] == '1' ? 11 : 10);

  return (time_t) atoi(time_str);
}


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


static void
trans_owner(hdr, old)
  HDR *hdr;
  char *old;
{
  char *left, *right;
  char owner[128];

  str_ncpy(owner, old, sizeof(owner));

  if (strchr(owner, '.'))	/* innbbsd ==> bbs */
  {
    /* �榡: itoc.bbs@bbs.tnfsh.tn.edu.tw (�ڪ��ʺ�) */

    hdr->xmode = POST_INCOME;

    left = strchr(owner, '(');
    right = strrchr(owner, ')');

    if (!left || !right)
    {
      str_ncpy(hdr->owner, "�H�W", sizeof(hdr->owner));
    }
    else
    {
      *(left - 1) = '\0';
      str_ncpy(hdr->owner, owner, sizeof(hdr->owner));
      *right = '\0';
      str_ncpy(hdr->nick, left + 1, sizeof(hdr->nick));
    }
  }
  else if (left = strchr(owner, '('))	/* local post */
  {
    /* �榡: itoc (�ڪ��ʺ�) */

    *(left - 1) = '\0';
    str_ncpy(hdr->owner, owner, sizeof(hdr->owner));
    if (right = strchr(owner, ')'))
    {
      *right = '\0';
      str_ncpy(hdr->nick, left + 1, sizeof(hdr->nick));
    }
  }
  else
  {
    /* �榡: itoc */

    str_ncpy(hdr->owner, owner, sizeof(hdr->owner));
  }
}


/*-------------------------------------------------------*/
/* �ഫ�{��						 */
/*-------------------------------------------------------*/


static void
trans_brd(bh)
  boardheader *bh;
{
  static time_t stamp = 0;

  int fd;
  char *brdname, index[64], folder[64], buf[64], fpath[64];
  fileheader fh;
  HDR hdr;
  BRD brd;
  time_t chrono;

  brdname = bh->filename;
  if (strlen(brdname) > BNLEN)
  {
    printf("%s name is too long!\n", brdname);
    return;
  }

  if (!stamp)
    time(&stamp);

  /* �ഫ .BRD */
  memset(&brd, 0, sizeof(BRD));
  str_ncpy(brd.brdname, brdname, sizeof(brd.brdname));
  str_ncpy(brd.class, bh->title + 2, sizeof(brd.class));
  str_ncpy(brd.title, bh->title + 13, sizeof(brd.title));
  /* str_ncpy(brd.BM, bh->BM, sizeof(brd.BM)); */	/* �S�� bh->BM? ������ʧ� */
  brd.bstamp = stamp++;
  brd.readlevel = 0;			/* ���w�] read/post level�A�����A�ۤv��ʧ� */
  brd.postlevel = PERM_POST;
  brd.battr = BRD_NOTRAN;
  rec_add(FN_BRD, &brd, sizeof(BRD));

  /* �إؿ� */
  sprintf(buf, "gem/brd/%s", brdname);
  mak_dirs(buf);
  mak_dirs(buf + 4);

  /* �ഫ�i�O�e�� */
  sprintf(index, "%s/%s/notes", OLD_BOARDPATH, brdname);
  brd_fpath(folder, brdname, FN_NOTE);
  f_cp(index, folder, O_TRUNC);

  /* �ഫ�벼�O�� */
  sprintf(index, "%s/%s/results", OLD_BOARDPATH, brdname);
  sprintf(folder, "brd/%s/@/@vote", brdname);
  f_cp(index, folder, O_TRUNC);

  /* �ഫ .DIR */
  sprintf(index, "%s/%s/.DIR", OLD_BOARDPATH, brdname);	/* �ª� .DIR */
  brd_fpath(folder, brdname, FN_DIR);			/* �s�� .DIR */

  if ((fd = open(index, O_RDONLY)) >= 0)
  {
    while (read(fd, &fh, sizeof(fh)) == sizeof(fh))
    {
      sprintf(buf, OLD_BOARDPATH "/%s/%s", brdname, fh.filename);

      chrono = trans_hdr_chrono(fh.filename);
      trans_hdr_stamp(folder, chrono, &hdr, fpath);
      trans_owner(&hdr, fh.owner);
      str_ansi(hdr.title, fh.title, sizeof(hdr.title));
      if (fh.accessed[0] & 0x8)	/* FILE_MARDKED */
	hdr.xmode |= POST_MARKED;
      rec_add(folder, &hdr, sizeof(HDR));

      /* �����ɮ� */
      f_cp(buf, fpath, O_TRUNC);
    }
    close(fd);
  }

  printf("%s transfer ok!\n", brdname);
}


int
main()
{
  int fd;
  boardheader bh;

  chdir(BBSHOME);

  if ((fd = open(FN_BOARDS, O_RDONLY)) >= 0)
  {
    while (read(fd, &bh, sizeof(bh)) == sizeof(bh))
      trans_brd(&bh);
    close(fd);
  }

  return 0;
}
