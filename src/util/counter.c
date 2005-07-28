/*-------------------------------------------------------*/
/* util/counter.c	( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : ���v�y��					 */
/* create : 03/03/03					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"


#define OUTFILE_COUNTER		"gem/@/@-counter"
#define FN_RUN_COUNTER		"run/var/counter"


typedef struct
{
  time_t uptime;		/* ��s�ɶ� */

  int total_acct;		/* ���W�`�@���h�֨ϥΪ̵��U */
  int online_usr;		/* �u�W�P�ɦ��h�֨ϥΪ� */
  int today_usr;		/* �����`�@���X�ӤH���W�� */
  int online_usr_every_hour[24];/* ����U���I�u�W���h�֨ϥΪ� */
  int total_brd;		/* ���W�`�@���h�֬ݪO */

  char ident[12];		/* �d�եH�X�R�� */
}      COUNTER;


#define break_record(new, old)	(new > old + old / 20)	/* ��s�����W�L�¬��� 5% �H�W�ɡA�N��g���� */


int
main()
{
  UCACHE *ushm;
  COUNTER counter;

  int num;
  char *fname, date[20];
  FILE *fp;
  time_t now;
  struct tm *ptime;

  chdir(BBSHOME);

  if (!(fp = fopen(OUTFILE_COUNTER, "a+")))
    return -1;

  fname = FN_RUN_COUNTER;

  memset(&counter, 0, sizeof(COUNTER));
  rec_get(fname, &counter, sizeof(COUNTER), 0);

  counter.uptime = time(&now);
  ptime = localtime(&now);
  sprintf(date, "�i%02d/%02d/%02d %02d:%02d�j", 
    ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_hour, ptime->tm_min);


  /* ���U�H�� */
  num = rec_num(FN_SCHEMA, sizeof(SCHEMA));
  if (break_record(num, counter.total_acct))
  {
    fprintf(fp, "�� %s \033[31m�������U�H��\033[m�g�P�W�L \033[1;31m%d\033[m �H\n", date, num);
    counter.total_acct = num;
  }

  /* �u�W�H�� */
  ushm = shm_new(UTMPSHM_KEY, sizeof(UCACHE));
  num = ushm->count;
  if (break_record(num, counter.online_usr))
  {
    fprintf(fp, "�� %s \033[32m�P�ɽu�W�H��\033[m�����F�� \033[1;32m%d\033[m �H\n", date, num);
    counter.online_usr = num;
  }
  counter.online_usr_every_hour[ptime->tm_hour] = num;	/* ���p�ɪ��u�W�H�ơA�N���o�� sample �� */

  /* ����W���H�� */
  if (ptime->tm_hour == 23)	/* �C�ѭp��@������W���H�� */
  {
    int i;

    /* itoc.����: �o�ӭȬO�����T���A�]���u�O���C�Ӥp�� sample ���M�A
       �ӥB�ٰ��]�C�ӤH�����b���W�ɶ��O 60 ���� */
    num = 0;
    for (i = 0; i < 24; i++)
      num += counter.online_usr_every_hour[i];

    if (break_record(num, counter.today_usr))
    {
      fprintf(fp, "�� %s \033[33m���W���H��\033[m������} \033[1;33m%d\033[m �H\n", date, num);
      counter.today_usr = num;
    }
  }

  /* �ݪO�Ӽ� */
  num = rec_num(FN_BRD, sizeof(BRD));
  if (break_record(num, counter.total_brd))
  {
    fprintf(fp, "�� %s \033[34m�����ݪO�Ӽ�\033[m�ŧG���F \033[1;34m%d\033[m ��\n", date, num);
    counter.total_brd = num;
  }


  rec_put(fname, &counter, sizeof(COUNTER), 0, NULL);

  fclose(fp);
  return 0;
}
