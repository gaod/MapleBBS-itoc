/*-------------------------------------------------------*/
/* util/transman.c					 */
/*-------------------------------------------------------*/
/* target : Magic �� Maple 3.02 ��ذ��ഫ		 */
/* create : 01/10/03					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

  0. �A�� Magic/Napoleon �� Maple ��ذ�
  1. �{�����}�ؿ��A�ϥΫe���T�w gem/target_board/? �ؿ��s�b
    if not�A���}�s�O or transbrd

  ps. User on ur own risk.

#endif


#include "mag.h"


/* ----------------------------------------------------- */
/* basic functions					 */
/* ----------------------------------------------------- */


static void
merge_msg(msg)		/* fget() �r��̫ᦳ '\n' �n�B�z�� */
  char *msg;
{
  int end;

  end = 0;
  while (end < 80)
  {
    if (msg[end] == '\n')
    {
      msg[end] = '\0';
      return;
    }

    end++;
  }
  msg[end] = '\0';	/* �j���_�b 80 �r */
}


static int			/* 'A':�峹  'F':�ؿ� */
get_record(src, src_folder, title, path, num)	/* Ū�X .Name */
  char *src;
  char *src_folder;
  char *title, *path;
  int num;
{
  FILE *fp;
  char Name[128], Path[128], buf[128];
  int i, j;

  if (fp = fopen(src_folder, "r"))
  {
    /* �e�T��S�� */
    j = num * 4 + 3;
    for (i = 0; i < j; i++)
    {
      fgets(buf, 80, fp);
    }

    /* �C�ӯ��ަ��|�� */
    fgets(Name, 80, fp);
    fgets(Path, 80, fp);
    fgets(buf, 80, fp);
    fgets(buf, 80, fp);

    fclose(fp);

    if (Name[0] != '#' && Path[0] != '#')
    {
      merge_msg(Name);
      merge_msg(Path);

      strcpy(title, Name + 5);
      strcpy(path, Path + 7);

      sprintf(buf, "%s/%s", src, path);

      if (dashf(buf))
	return 'A';
      else if (dashd(buf))
	return 'F';
    }
  }

  return 0;
}


/* ----------------------------------------------------- */
/* �ഫ�{��						 */
/* ----------------------------------------------------- */


static void
transman(brdname, src, dst_folder)
  char *brdname;	/* �ݪO�O�W */
  char *src;		/* �¯����ؿ� */
  char *dst_folder;	/* �s���� .DIR ��  FXXXXXXX */
{
  int num;
  int type;
  char src_folder[80];		/* �¯��� .Names */
  char sub_src[80];		
  char sub_dst_folder[80];
  char title[80], path[80];
  char cmd[256];
  HDR hdr;

  num = 0;
  sprintf(src_folder, "%s/.Names", src);

  while ((type = get_record(src, src_folder, title, path, num)))
  {
    if (type == 'A')		/* ��� */
    {
      close(hdr_stamp(dst_folder, 'A', &hdr, cmd));
      str_ncpy(hdr.title, title, sizeof(hdr.title));
      rec_add(dst_folder, &hdr, sizeof(HDR));

      sprintf(cmd, "cp %s/%s gem/brd/%s/%c/%s", src, path, brdname, hdr.xname[7], hdr.xname);
      system(cmd);
    }
    else if (type == 'F')	/* �ؿ� */
    {
      close(hdr_stamp(dst_folder, 'F', &hdr, cmd));
      hdr.xmode = GEM_FOLDER;
      str_ncpy(hdr.title, title, sizeof(hdr.title));
      rec_add(dst_folder, &hdr, sizeof(HDR));

      sprintf(sub_src, "%s/%s", src, path);
      sprintf(sub_dst_folder, "gem/brd/%s/%c/%s", brdname, hdr.xname[7], hdr.xname);
      transman(brdname, sub_src, sub_dst_folder);
    }
    num++;
  }
}


int
main()
{
  int fd;
  char src[64], dst_folder[64];
  BRD brd;

  chdir(BBSHOME);

  if ((fd = open(FN_BRD, O_RDONLY)) >= 0)
  {
    while (read(fd, &brd, sizeof(BRD)) == sizeof(BRD))
    {
      sprintf(src, OLD_MANPATH "/%s", brd.brdname);

      if (dashd(src))
      {
	sprintf(dst_folder, "gem/brd/%s", brd.brdname);

	if (dashd(dst_folder))
	{
	  sprintf(dst_folder, "gem/brd/%s/.DIR", brd.brdname);
	  transman(brd.brdname, src, dst_folder);	/* �s�¯����n���o�ӬݪO�~�� */
	}
      }
    }
    close(fd);
  }

  exit(0);
}
