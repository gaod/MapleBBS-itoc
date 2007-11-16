/* ----------------------------------------------------- */
/* hdr_stamp - create unique HDR based on timestamp	 */
/* ----------------------------------------------------- */
/* fpath - directory					 */
/* token - (A / F / 0) | [HDR_LINK / HDR_COPY]		 */
/* ----------------------------------------------------- */
/* return : open() fd (not close yet) or link() result	 */
/* ----------------------------------------------------- */


#include "dao.h"
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#if 0	/* itoc.030303.����: ²������ */

  hdr_stamp() �|���X�@�ӷs�� HDR�A�̶ǤJ�� token ���P�Ӧ��t���G

   0 : �s�W�@�g�H��(family �O @)�A�^�Ǫ� fpath �O hdr �ҫ��V
       �M hdr_fpath(fpath, folder, hdr); �Ҳ��ͪ� fpath �ۦP

  'A': �s�W�@�g�峹(family �O A)�A�^�Ǫ� fpath �O hdr �ҫ��V
       �M hdr_fpath(fpath, folder, hdr); �Ҳ��ͪ� fpath �ۦP

  'F': �s�W�@�Ө��v(family �O F)�A�^�Ǫ� fpath �O hdr �ҫ��V
       �M hdr_fpath(fpath, folder, hdr); �Ҳ��ͪ� fpath �ۦP

  HDR_LINK      : fpath �w�����ɮ׮ɡA�n�ƻs���ɮר�s�H��(family �O @) �h
                  �ñN hdr ���V�o�g�s�H��A�^�Ǫ� fpath �O������ɮ�
                  ���ɮשM�s�H��O hard link�A��F�䤤�@�g�A�t�@�g�]�|�@�_�Q��
                  �R�����ɮסA�s�H��ä��|�Q�R��

  HDR_LINK | 'A': fpath �w�����ɮ׮ɡA�n�ƻs���ɮר�s�峹(family �O A) �h
                  �ñN hdr ���V�o�g�s�峹�A�^�Ǫ� fpath �O������ɮ�
                  ���ɮשM�s�峹�O hard link�A��F�䤤�@�g�A�t�@�g�]�|�@�_�Q��
                  �R�����ɮסA�s�峹�ä��|�Q�R��

  HDR_COPY      : fpath �w�����ɮ׮ɡA�n�ƻs���ɮר�s�H��(family �O @) �h
                  �ñN hdr ���V�o�g�s�H��A�^�Ǫ� fpath �O������ɮ�
                  ���ɮשM�s�H��O copy�A��F�䤤�@�g�A�t�@�g�ä��|�Q��
                  ���ɮ׻P�s�H��O�����W�ߤ��������G���ɮ�

  HDR_COPY | 'A': fpath �w�����ɮ׮ɡA�n�ƻs���ɮר�s�峹(family �O A) �h
                  �ñN hdr ���V�o�g�s�峹�A�^�Ǫ� fpath �O������ɮ�
                  ���ɮשM�s�峹�O copy�A��F�䤤�@�g�A�t�@�g�ä��|�Q��
                  ���ɮ׻P�s�峹�O�����W�ߤ��������G���ɮ�

#endif

int
hdr_stamp(folder, token, hdr, fpath)
  char *folder;
  int token;
  HDR *hdr;
  char *fpath;
{
  char *fname, *family;
  int rc, chrono;
  char *flink, buf[128];

  flink = NULL;
  if (token & (HDR_LINK | HDR_COPY))
  {
    flink = fpath;
    fpath = buf;
  }

  fname = fpath;
  while (rc = *folder++)
  {
    *fname++ = rc;
    if (rc == '/')
      family = fname;
  }
  if (*family != '.')
  {
    fname = family;
    family -= 2;
  }
  else
  {
    fname = family + 1;
    *fname++ = '/';
  }

  if (rc = token & 0xdf)	/* �ܤj�g */
  {
    *fname++ = rc;
  }
  else
  {
    *fname = *family = '@';
    family = ++fname;
  }

  chrono = time(0);

  for (;;)
  {
    *family = radix32[chrono & 31];
    archiv32(chrono, fname);

    if (flink)
    {
      if (token & HDR_LINK)
	rc = f_ln(flink, fpath);
      else
        rc = f_cp(flink, fpath, O_EXCL);
    }
    else
    {
      rc = open(fpath, O_WRONLY | O_CREAT | O_EXCL, 0600);
    }

    if (rc >= 0)
    {
      memset(hdr, 0, sizeof(HDR));
      hdr->chrono = chrono;
      str_stamp(hdr->date, &hdr->chrono);
      strcpy(hdr->xname, --fname);
      break;
    }

    if (errno != EEXIST)
      break;

    chrono++;
  }

  return rc;
}
