/*-------------------------------------------------------*/
/* race.c         ( NTHU CS MapleBBS Ver 3.10 )          */
/*-------------------------------------------------------*/
/* target : �ɰ����C��                                   */
/* create : 98/12/17                                     */
/* update : 01/04/21                                     */
/* author : SugarII (u861838@Oz.nthu.edu.tw)             */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_GAME


static int pace[5];		/* ���ǰ��]�F�h�� */


static void
out_song()
{
  static int count = 0;  

  /* �i�B���@������ */
  uschar *msg[13] = 
  {
    "�p���ڤ@����  �p�ݵۧڤJ�g",
    "�Q�p�q�߸̭鸨���P��  �h���������˥h",
    "��𤣯�۫H�o�Pı  ���ۤv�M�ۤv����",
    "�ӫH�}�������R��  �b����",
    "�ڤ@������  �Ԥ���ˤ�",
    "�Ŷq���X�R�Τ��R�������Z��",
    "�p���p����  ���A�ż��p��",
    "�q���̶}�l  �q���̥��h",
    "�ڤ@������  �Ԥ���ˤ�",
    "�Ŷq���X�R�Τ��R�������Z��",
    "����������  ���թp���M�w",
    "�����j�j�p  �u�n�����ۤv",
    "�ڬ����ڦۤv  �ڬ����ڦۤv"
  };
  move(b_lines - 2, 0);
  prints("\033[1;3%dm%s\033[m  �w�X�٦� %d ��", time(0) % 7, msg[count], cuser.money);
  clrtoeol();
  if (++count == 13)
    count = 0;
}


static int			/* -1: �٨S���X�ӭt 0~4:Ĺ�����ǰ� */
race_path(run, j, step)
  int run, j, step;
{
  int i;

  if (!step)
  {
    return -1;
  }
  else if (step < 0)
  {
    if (j + step < 0)
      j = -step;
    move(run + 9, (j + step) * 2 + 8);
    clrtoeol();
    pace[run] += step * 100;
    if (pace[run] < 1)
      pace[run] = 1;
    return -1;
  }

  /* step > 0 */
  move(run + 9, j * 2 + 8);
  for (i = 0; i < step; i++)
    outs("��");

  if (pace[run] + step * 100 > 3000)
    return run;
  return -1;
}


int
main_race()
{
  int money[5];			/* ���ǰ������ */
  int speed[5];			/* ���ǰ����t�� */
  int stop[5];			/* ���ǰ����Ȱ� */
  int bomb;			/* �O�_�ϥά��u */
  int run;			/* �ثe�b�p�⪺���� */
  int win;			/* ���ǰ�Ĺ�F */
  int flag;			/* �ƥ�o�ͦ��� */
  int i, j, ch;
  char buf[60];
  char *racename[5] = {"����", "���c", "����", "���q", "����"};

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  while (1)
  {
    vs_bar("�ɰ���");
    out_song();
    bomb = 0;
    win = -1;
    flag = 0;
    for (i = 0; i < 5; i++)
    {
      pace[i] = 1;
      stop[i] = money[i] = 0;
      speed[i] = 100;
    }
    move(5, 0);
    outs("  \033[1m���W�G\033[m");
    for (i = 0; i < 5; i++)
      prints("     %d. \033[1;3%dm%s\033[m", i + 1, i + 1, racename[i]);
    outs("\n  \033[1m�t�סG\033[m\n  \033[1m����G\033[m\n\n");

    for (i = 0; i < 5; i++)
      prints("%d.\033[1;3%dm%s\033[m��\n", i + 1, i + 1, racename[i]);
    outs("�w�w�w���w�w�r�w�w�r�w�w�r�w�w�r�w�w�r�w�w�r�w�w�r�w�w�r�w�w�r�w�w��");

    while (1)
    {
      /* �M�w�U�ǰ���` */
      ch = vget(2, 0, "�z�n����ǰ�(1-5)�H[S]�}�l [Q]���}�G", buf, 3, DOECHO);
      i = buf[0];
      if (!ch || i == 's')
      {
	if (money[0] || money[1] || money[2] || money[3] || money[4])
	  break;
	addmoney(money[0] + money[1] + money[2] + money[3] + money[4]);	/* �ٿ� */
	goto abort_game;
      }
      else if (i < '1' || i > '5')
      {
	addmoney(money[0] + money[1] + money[2] + money[3] + money[4]);	/* �ٿ� */
	goto abort_game;
      }

      ch = vget(3, 0, "�n��h�ֽ���H", buf, 6, DOECHO);
      j = atoi(buf);
      if (!ch)
      {
	if (money[0] || money[1] || money[2] || money[3] || money[4])
	  break;
	addmoney(money[0] + money[1] + money[2] + money[3] + money[4]);	/* �ٿ� */
	goto abort_game;
      }
      if (j < 1 || j > cuser.money)
      {
	addmoney(money[0] + money[1] + money[2] + money[3] + money[4]);	/* �ٿ� */
	goto abort_game;
      }

      money[i - '1'] += j;
      cuser.money -= j;

      move(7, 15);
      clrtoeol();
      outs("\033[1m");
      for (i = 0; i < 5; i++)
	prints("     \033[3%dm%7d", i + 1, money[i]);
      outs("\033[m");
      out_song();
    }

    /* �}�l�C�� */
    move(3, 0);
    clrtoeol();
    move(2, 0);
    clrtoeol();
    outs("-== �Ы� \033[1;36mk\033[m ���z�諸�l�s�[�o�A�� \033[1;36mz\033[m �i��X���u(�u���@�����|) ==-");

    while (win < 0)
    {
      move(6, 15);
      clrtoeol();
      outs("\033[1m");
      for (i = 0; i < 5; i++)
      {
	if (stop[i] < 1)
	  speed[i] += rnd(20) - (speed[i] + 170) / 30;
	if (speed[i] < 0)
	  speed[i] = 0;
	prints("     \033[3%dm%7d", i + 1, speed[i]);
      }
      outs("\033[m");

      do
      {
	ch = igetch();
      } while (ch != 'k' && (ch != 'z' || bomb));

      run = rnd(5);		/* ��ܨƥ�o�͹�H */
      flag %= 5;		/* �C�L�ƥ��ù��W */
      move(15 + flag, 0);
      clrtoeol();

      if (ch == 'z')		/* �ᬵ�u */
      {
	stop[run] = 3;
	prints("\033[1m�O�ֲy�{��\033[3%dm%s\033[37m����e�i�T���A�t�� = 0\033[m",
	  run + 1, racename[run]);
	speed[run] = 0;
	flag++;
	bomb = 1;
      }
      else if (rnd(12) == 0)	/* �S��ƥ� */
      {
	prints("\033[1;3%dm%s\033[36m", run + 1, racename[run]);

	switch (rnd(14))
	{
	case 0:
	  outs("�A�U�¦ӭ�A�t�� x1.5\033[m");
	  speed[run] *= 1.5;
	  break;
	case 1:
	  outs("�ϥX�����z�o�O�A�e�i����\033[m");
	  win = race_path(run, pace[run] / 100, 5);
	  pace[run] += 500;
	  break;
	case 2:
	  outs("���a�p�A�t�״�b\033[m");
	  speed[run] /= 2;
	  break;
	case 3:
	  outs("��쭻���ַƭˡA�Ȱ��G��\033[m");
	  stop[run] += 2;
	  break;
	case 4:
	  outs("�Я��W���A�Ȱ��|���A�t�ץ[��\033[m");
	  stop[run] += 4;
	  speed[run] *= 2;
	  break;
	case 5:
	  outs("�ۥX�j�]�k�G�A�Ϩ�L�H�Ȱ��T��\033[m");
	  for (i = 0; i < 5 && i != run; i++)
	    stop[i] += 3;
	  break;
	case 6:
	  outs("ť�� badboy ���[�o�n�A�t�� +100\033[m");
	  speed[run] += 100;
	  break;
	case 7:
	  outs("�ϥX���K�ܨ��A�e�i�T��A�t�� +30\033[m");
	  win = race_path(run, pace[run] / 100, 3);
	  speed[run] += 30;
	  break;
	case 8:
	  outs("�I���W���t�״�b�A����Ȱ��G��\033[m");
	  speed[run] /= 2;
	  if (run > 0)
	    stop[run - 1] += 2;
	  if (run < 4)
	    stop[run + 1] += 2;
	  break;
	case 9:
	  outs("�Q�A�G�A�^��_�I\033[m");
	  win = race_path(run, pace[run] / 100, -30);
	  break;
	case 10:
	  if (pace[0] + pace[1] + pace[2] + pace[3] + pace[4] > 6000)
	  {
	    outs("\033[5m�ϥX�ͤ��ȳ��A�Ҧ��H�^��_�I\033[m");
	    for (i = 0; i < 5; i++)
	      win = race_path(i, pace[i] / 100, -30);
	  }
	  else
	  {
	    outs("�ϥX���Z�u���A�t�� x1.3�A��L�H��b\033[m");
	    for (i = 0; i < 5 && i != run; i++)
	      speed[i] /= 2;
	    speed[run] *= 1.3;
	  }
	  break;
	case 11:
	  if (money[run])
	  {
	    outs("�ߨ�ܦh���A�Ȱ��@��\033[m");
	    addmoney(money[run]);
	    out_song();
	    stop[run]++;
	  }
	  else
	  {
	    outs("��ǰ��n�_�ӤF�A�t�� +50\033[m");
	    speed[run] += 50;
	  }
	  break;
	case 12:
	  j = rnd(5);
	  prints("�R�W�F[3%dm%s[36m�A�t�׸�e�@��", j + 1, racename[j]);
	  speed[run] = speed[j];
	  break;
	case 13:
	  if (money[run] > 0)
	  {
	    outs("����� x1.5�A�ȰաI\033[m");
	    money[run] *= 1.5;
	    move(7, 15);
	    clrtoeol();
	    outs("\033[1m");
	    for (i = 0; i < 5; i++)
	      prints("     \033[3%dm%7d ", i + 1, money[i]);
	    outs("\033[m");
	  }
	  else
	  {
	    outs("�c�l���F�A�h��T��\033[m");
	    race_path(run, pace[run] / 100, -3);
	  }
	  break;
	}
      }
      else		/* ���e�] */
      {
	if (stop[run])
	{
	  prints("\033[1;3%dm%s\033[37m �����_��\033[m", run + 1, racename[run]);
	  stop[run]--;
	}
	else
	{
	  prints("\033[1;3%dm%s\033[37m ��R�b�]\033[m", run + 1, racename[run]);
	  i = pace[run] / 100;
	  win = race_path(run, i, (pace[run] + speed[run]) / 100 - i);
	  pace[run] += speed[run];
	}
      }
      flag++;
    }
    move(b_lines - 1, 0);
    prints("\033[1;35m�� \033[37m�C������ \033[35m�� \033[37m��Ӫ��O\033[3%dm %s \033[m",
      win + 1, racename[win]);
    if (money[win])
    {
      money[win] += money[win] * (pace[win] - (pace[0] + pace[1] + pace[2] + pace[3] + pace[4]) / 5) / 500;
      sprintf(buf, "���߱z�㤤�F�A��o���� %d ��", money[win]);
      addmoney(money[win]);
    }
    else
    {
      strcpy(buf, "��p...�z�S�㤤��~~~");
    }
    vmsg(buf);
  }

abort_game:
  return 0;
}
#endif				/* HAVE_GAME */
