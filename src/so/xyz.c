/*-------------------------------------------------------*/
/* xyz.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : ���C���K���~��				 */
/* create : 01/03/01					 */
/* update :   /  /  					 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_TIP

/* ----------------------------------------------------- */
/* �C��p���Z						 */
/* ----------------------------------------------------- */

int
x_tip()
{
  int i, j;
  char msg[128];
  FILE *fp;

  if (!(fp = fopen(FN_ETC_TIP, "r")))
    return XEASY;

  fgets(msg, 128, fp);
  j = atoi(msg);		/* �Ĥ@��O���`�g�� */
  i = time(0) % j + 1;
  j = 0;

  while (j < i)			/* ���� i �� tip */
  {
    fgets(msg, 128, fp);
    if (msg[0] == '#')
      j++;
  }

  move(12, 0);
  clrtobot();
  fgets(msg, 128, fp);
  prints("\033[1;36m�C��p���Z�G\033[m\n");
  prints("            %s", msg);
  fgets(msg, 128, fp);
  prints("            %s", msg);
  vmsg(NULL);
  fclose(fp);
  return 0;
}
#endif	/* HAVE_TIP */


#ifdef HAVE_LOVELETTER 

/* ----------------------------------------------------- */
/* ���Ѳ��;�						 */
/* ----------------------------------------------------- */

int
x_loveletter()
{
  FILE *fp;
  int start_show;	/* 1:�}�l�q */
  int style;		/* 0:�}�Y 1:���� 2:���� */
  int line;
  char buf[128];
  char header[3][5] = {"head", "body", "foot"};	/* �}�Y�B����B���� */
  int num[3];

  /* etc/loveletter �e�q�O#head ���q�O#body ��q�O#foot */
  /* ��ƤW���G#head����  #body�K��  #foot���� */

  if (!(fp = fopen(FN_ETC_LOVELETTER, "r")))
    return XEASY;

  /* �e�T��O���g�� */
  fgets(buf, 128, fp);
  num[0] = atoi(buf + 5);
  num[1] = atoi(buf + 5);
  num[2] = atoi(buf + 5);

  /* �M�w�n��ĴX�g */
  line = time(0);
  num[0] = line % num[0];
  num[1] = (line >> 1) % num[1];
  num[2] = (line >> 2) % num[2];

  vs_bar("���Ѳ��;�");

  start_show = style = line = 0;

  while (fgets(buf, 128, fp))
  {
    if (*buf == '#')
    {
      if (!strncmp(buf + 1, header[style], 4))  /* header[] ���׳��O 5 bytes */
	num[style]--;

      if (num[style] < 0)	/* �w�g fget ��n�諸�o�g�F */
      {
	outc('\n');
	start_show = 1;
	style++;
      }
      else
      {
	start_show = 0;
      }
      continue;
    }

    if (start_show)
    {
      if (line >= (b_lines - 5))	/* �W�L�ù��j�p�F */
	break;

      outs(buf);
      line++;
    }
  }

  fclose(fp);
  vmsg(NULL);

  return 0;
}
#endif	/* HAVE_LOVELETTER */


/* ----------------------------------------------------- */
/* �K�X�ѰO�A���]�K�X					 */
/* ----------------------------------------------------- */


int
x_password()
{
  int i;
  ACCT acct;
  FILE *fp;
  char fpath[80], email[60], passwd[PSWDLEN + 1];
  time_t now;

  vmsg("���L�ϥΪ̧ѰO�K�X�ɡA���e�s�K�X�ܸӨϥΪ̪��H�c");

  if (acct_get(msg_uid, &acct) > 0)
  {
    time(&now);

    if (acct.lastlogin > now - 86400 * 10)
    {
      vmsg("�ӨϥΪ̥����Q�ѥH�W���W����i���e�K�X");
      return 0;
    }

    vget(b_lines - 2, 0, "�п�J�{�Үɪ� Email�G", email, 40, DOECHO);

    if (str_cmp(acct.email, email))
    {
      vmsg("�o���O�ӨϥΪ̻{�ҮɥΪ� Email");
      return 0;
    }

    if (not_addr(email) || !mail_external(email))
    {
      vmsg(err_email);
      return 0;
    }

    vget(b_lines - 1, 0, "�п�J�u��m�W�G", fpath, RNLEN + 1, DOECHO);
    if (strcmp(acct.realname, fpath))
    {
      vmsg("�o���O�ӨϥΪ̪��u��m�W");
      return 0;
    }

    if (vans("��ƥ��T�A�нT�{�O�_���ͷs�K�X(Y/N)�H[N] ") != 'y')
      return 0;

    sprintf(fpath, "%s ��F %s ���K�X", cuser.userid, acct.userid);
    blog("PASSWD", fpath);

    /* �üƲ��� A~Z �զX���K�X�K�X */
    for (i = 0; i < PSWDLEN; i++)
      passwd[i] = rnd(26) + 'A';
    passwd[PSWDLEN] = '\0';

    /* ���s acct_load ���J�@���A�קK���b vans() �ɵn�J�|���~�����ĪG */
    if (acct_load(&acct, acct.userid) >= 0)
    {
      str_ncpy(acct.passwd, genpasswd(passwd), PASSLEN + 1);
      acct_save(&acct);
    }

    sprintf(fpath, "tmp/sendpass.%s", cuser.userid);
    if (fp = fopen(fpath, "w"))
    {
      fprintf(fp, "%s ���z�ӽФF�s�K�X\n\n", cuser.userid);
      fprintf(fp, BBSNAME "ID : %s\n\n", acct.userid);
      fprintf(fp, BBSNAME "�s�K�X : %s\n", passwd);
      fclose(fp);

      bsmtp(fpath, BBSNAME "�s�K�X", email, 0);
      unlink(fpath);

      vmsg("�s�K�X�w�H��ӻ{�ҫH�c");
    }
  }

  return 0;
}
