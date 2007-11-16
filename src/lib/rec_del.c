#include "dao.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>


int
rec_del(fpath, size, pos, fchk)
  char *fpath;
  int size;
  int pos;
  int (*fchk) ();
{
  int fd;
  off_t off, len;
  char pool[REC_SIZ];
  struct stat st;

  if ((fd = open(fpath, O_RDWR)) < 0)
    return -1;

  /* flock(fd, LOCK_EX); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_exlock(fd);

  fstat(fd, &st);
  len = st.st_size;

  fpath = pool;
  off = size * pos;

  /* ���� pos ��m��ƪ����T�� */

  if (len > off)
  {
    lseek(fd, off, SEEK_SET);
    read(fd, fpath, size);

    pos = fchk ? (*fchk) (fpath) : 1;
  }
  else
  {
    pos = 0;
  }

  /* ���諸�ܡA���Y��_ */

  if (!pos)
  {
    off = 0;
    lseek(fd, off, SEEK_SET);
    while (read(fd, fpath, size) == size)
    {
      if (pos = (*fchk) (fpath))
	break;

      off += size;
    }
  }

  /* ��줧��A�R����� */

  if (pos)
  {
    len -= (off + size);
    fpath = (char *) malloc(len);
    read(fd, fpath, len);

    lseek(fd, off, SEEK_SET);
    write(fd, fpath, len);

    ftruncate(fd, off + len);
    free(fpath);
  }

  /* flock(fd, LOCK_UN); */
  /* Thor.981205: �� fcntl ���Nflock, POSIX�зǥΪk */
  f_unlock(fd);

  close(fd);

  return !pos;
}
