int			/* >=1:�b�W�檺���@�� 0:���b�W�椺 */
str_has(list, tag, len)
  char *list;
  char *tag;
  int len;		/* strlen(tag) */
{
  int cc, priority;
  char *str;

  priority = 1;
  str = tag;
  do
  {
    cc = list[len];	/* itoc.030730.����: �i��|����W�L list �����ץH�~�h�F�A���L�S�t */
    if ((!cc || cc == '/') && !str_ncmp(list, str, len))
    {
      return priority;
    }
    while (cc = *list++)
    {
      if (cc == '/')
      {
	priority++;
	break;
      }
    }
  } while (cc);

  return 0;
}
