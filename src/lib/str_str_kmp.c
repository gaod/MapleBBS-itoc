/* a alternative str_str() using KMP algorithm. */
/* English case independent and BIG5 Chinese supported */

#include <stdlib.h>


void
str_expand(dst, src)	/* �N char �ର short�A�ñN�^���ܤp�g */
  char *dst, *src;
{
  int ch;
  int in_chi = 0;	/* 1: �e�@�X�O����r */

  do
  {
    ch = *src++;

    if (in_chi || ch & 0x80)
    {
      in_chi ^= 1;
    }
    else
    {    
      if (ch >= 'A' && ch <= 'Z')
        ch |= 0x20;
      *dst++ = 0;
    }
    *dst++ = ch;
  } while (ch);
}


void
str_str_kmp_tbl(pat, tbl)
  const short *pat;
  int *tbl;
{
  register short c;
  register int i, j;

  tbl[0] = -1;
  for (j = 1; c = pat[j]; j++)
  {
    i = tbl[j - 1];
    while (i >= 0 && c != pat[i + 1])
      i = tbl[i];
    tbl[j] = (c == pat[i + 1]) ? i + 1 : -1;
  }
}


const int
str_str_kmp(str, pat, tbl)
  const short *str;
  const short *pat;
  const int *tbl;
{
  register const short *i;
  register int j;

  for (i = str, j = 0; *i && pat[j];)
  {
    if (*i == pat[j])
    {
      j++;
    }
    else if (j)
    {
      j = tbl[j - 1] + 1;
      continue;		/* ���ݭn i++ */
    }
    i++;
  }

  /* match */
  if (!pat[j])
    return 1;

  return 0;
}


#undef	TEST

#ifdef TEST
static void
try_match(str, key)
  char *str, *key;
{
  short a[256], b[256];		/* ���] 256 �w���� */
  int tbl[256];

  str_expand(a, str);
  str_expand(b, key);

  str_str_kmp_tbl(key, tbl);
  printf("�u%s�v %s�]�A �u%s�v\n", 
    str, str_str_kmp(a, b, tbl) ? "" : "��", key);
}


int
main()
{
  try_match("�n���q�v", "��");
  try_match("�n���q�v", "N");
  try_match("�n���q�v", "n");
  try_match("�n���q�v", "�n��");

  try_match("x�n��x�q�v", "�x�q");
  try_match("x�n��x�q�v", "��x");
  try_match("x�n��x�q�v", "��X");
  try_match("x�n��X�q�v", "��x");

  try_match("abx�n��x�q�v", "x�q");

  return 0;
}
#endif
