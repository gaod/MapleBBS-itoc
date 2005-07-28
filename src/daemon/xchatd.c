/*-------------------------------------------------------*/
/* xchatd.c     ( NTHU CS MapleBBS Ver 3.00 )            */
/*-------------------------------------------------------*/
/* target : super KTV daemon for chat server             */
/* create : 95/03/29                                     */
/* update : 97/10/20                                     */
/*-------------------------------------------------------*/


#include "bbs.h"
#include "xchat.h"


#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <sys/resource.h>


#define	SERVER_USAGE
#define WATCH_DOG
#undef	DEBUG			/* �{���������� */
#undef	MONITOR			/* �ʷ� chatroom ���ʥH�ѨM�ȯ� */
#undef	STAND_ALONE		/* ���f�t BBS �W�߰��� */


#ifdef	DEBUG
#define	MONITOR
#endif

static int gline;

#ifdef  WATCH_DOG
#define MYDOG  gline = __LINE__
#else
#define MYDOG			/* NOOP */
#endif


#define CHAT_PIDFILE    "run/chat.pid"
#define CHAT_LOGFILE    "run/chat.log"
#define	CHAT_INTERVAL	(60 * 30)
#define SOCK_QLEN	3


/* name of the main room (always exists) */


#define	MAIN_NAME	"main"
#define	MAIN_TOPIC	"���t�d���Ӭ۷|"


#define ROOM_LOCKED	1
#define ROOM_SECRET	2
#define ROOM_OPENTOPIC  4
#define ROOM_ALL	(NULL)


#define LOCKED(room)	(room->rflag & ROOM_LOCKED)
#define SECRET(room)	(room->rflag & ROOM_SECRET)
#define OPENTOPIC(room) (room->rflag & ROOM_OPENTOPIC)


#define RESTRICTED(usr)	(usr->uflag == 0)	/* guest */
#define CHATSYSOP(usr)	(usr->uflag & PERM_ALLCHAT)
#define	PERM_ROOMOP	PERM_CHAT	/* Thor: �� PERM_CHAT �� PERM_ROOMOP */
#define	PERM_CHATOP	PERM_DENYCHAT	/* Thor: �� PERM_DENYCHAT �� PERM_CHATOP */
/* #define ROOMOP(usr)  (usr->uflag & (PERM_ROOMOP | PERM_ALLCHAT)) */
/* Thor.980603: PERM_ALLCHAT �אּ default �S�� roomop, ���i�H�ۤv���o chatop */
#define ROOMOP(usr)	(usr->uflag & (PERM_ROOMOP | PERM_CHATOP))
#define CLOAK(usr)	(usr->uflag & PERM_CLOAK)


/* ----------------------------------------------------- */
/* ChatRoom data structure                               */
/* ----------------------------------------------------- */


typedef struct ChatRoom ChatRoom;
typedef struct ChatUser ChatUser;
typedef struct UserList UserList;
typedef struct ChatCmd ChatCmd;
typedef struct ChatAction ChatAction;


struct ChatUser
{
  ChatUser *unext;
  ChatRoom *room;
  UserList *ignore;
  int sock;			/* user socket */
  int userno;
  int uflag;
  int clitype;			/* Xshadow: client type. 1 for common client,
				 * 0 for bbs only client */
  time_t tbegin;
  time_t uptime;
  int sno;
  int xdata;
  int retry;

  int isize;			/* current size of ibuf */
  char ibuf[80];		/* buffer for non-blocking receiving */
  char userid[IDLEN + 1];	/* real userid */
  char chatid[9];		/* chat id */
  char rhost[30];		/* host address */
};


struct ChatRoom
{
  ChatRoom *next, *prev;
  UserList *invite;
  char name[IDLEN + 1];
  char topic[48];		/* Let the room op to define room topic */
  int rflag;			/* ROOM_LOCKED, ROOM_SECRET, ROOM_OPENTOPIC */
  int occupants;		/* number of users in room */
};


struct UserList
{
  UserList *next;
  int userno;
  char userid[0];
};


struct ChatCmd
{
  char *cmdstr;
  void (*cmdfunc) ();
  int exact;
};


static ChatRoom mainroom, *roompool;
static ChatUser *mainuser, *userpool;
static fd_set mainfset;
static int totaluser;		/* current number of connections */
static struct timeval zerotv;	/* timeval for selecting */
static int common_client_command;


#ifdef STAND_ALONE
static int userno_inc = 0;	/* userno auto-incrementer */
#endif


static char msg_not_op[] = "�� �z���O�o����ѫǪ� Op";
static char msg_no_such_id[] = "�� �ثe�S���H�ϥ� [%s] �o�Ӳ�ѥN��";
static char msg_not_here[] = "�� [%s] ���b�o����ѫ�";


#define	FUZZY_USER	((ChatUser *) -1)


/* ----------------------------------------------------- */
/* operation log and debug information                   */
/* ----------------------------------------------------- */


static FILE *flog;


static void
logit(key, msg)
  char *key;
  char *msg;
{
  time_t now;
  struct tm *p;

  time(&now);
  p = localtime(&now);
  fprintf(flog, "%02d/%02d %02d:%02d:%02d %-13s%s\n",
    p->tm_mon + 1, p->tm_mday,
    p->tm_hour, p->tm_min, p->tm_sec, key, msg);
}


static inline void
log_init()
{
  FILE *fp;

  /* --------------------------------------------------- */
  /* log daemon's PID					 */
  /* --------------------------------------------------- */

  if (fp = fopen(CHAT_PIDFILE, "w"))
  {
    fprintf(fp, "%d\n", getpid());
    fclose(fp);
  }

  flog = fopen(CHAT_LOGFILE, "a");
  logit("START", "chat daemon");
}


#ifdef	DEBUG
static char chatbuf[256];	/* general purpose buffer */


static void
debug_list(list)
  UserList *list;
{
  char buf[80];
  int i = 0;

  if (!list)
  {
    logit("DEBUG_L", "NULL");
    return;
  }
  while (list)
  {
    sprintf(buf, "%d) list: %p userno: %d next: %p", i++, list, list->userno, list->next);
    logit("DEBUG_L", buf);

    list = list->next;
  }
  logit("DEBUG_L", "end");
}


static void
debug_user()
{
  ChatUser *user;
  int i;
  char buf[80];

  sprintf(buf, "mainuser: %p userpool: %p", mainuser, userpool);
  logit("DEBUG_U", buf);
  for (i = 0, user = mainuser; user; user = user->unext)
  {
    /* MYDOG; */
    sprintf(buf, "%d) %p %-6d %s %s", ++i, user, user->userno, user->userid, user->chatid);
    logit("DEBUG_U", buf);
  }
}


static void
debug_room()
{
  ChatRoom *room;
  int i;
  char buf[80];

  i = 0;
  room = &mainroom;

  sprintf(buf, "mainroom: %p roompool: %p", mainroom, roompool);
  logit("DEBUG_R", buf);
  do
  {
    MYDOG;
    sprintf(buf, "%d) %p %s %d", ++i, room, room->name, room->occupants);
    logit("DEBUG_R", buf);
  } while (room = room->next);
}


static void
log_user(cu)
  ChatUser *cu;
{
  static int log_num;

  if (cu)
  {
    if (log_num > 100 && log_num < 150)
    {
      sprintf(chatbuf, "%d: %p <%d>", log_num, cu, gline);
      logit("travese user ", chatbuf);
    }
    else if (log_num == 100)
    {
      sprintf(chatbuf, "BOOM !! at line %d", gline);
      logit("travese user ", chatbuf);
    }
    log_num++;
  }
  else
    log_num = 0;
}
#endif				/* DEBUG */


/* ----------------------------------------------------- */
/* string routines                                       */
/* ----------------------------------------------------- */


static int
valid_chatid(id)
  char *id;
{
  int ch, len;

  for (len = 0; ch = *id; id++)
  { /* Thor.980921: �ťլ����X�zchatid, ��getnext�P�_���~���� */
    if (ch == '/' || ch == '*' || ch == ':' || ch ==' ')
      return 0;
    if (++len > 8)
      return 0;
  }
  return len;
}


/* itoc.����: �ѩ��� MUD-like ������ match �Y�i */
/* �ҥH MUD-like �� action �ɶq���n�έ^���Y�g�A�ä��n�����Ъ� */

static int		/* 0: fit */
str_belong(s1, s2)	/* itoc.010321: �� mud-like ���O���� match �Y�i�A�M mud �@�� */
  uschar *s1;		/* ChatAction �̪��p�g verb */
  uschar *s2;		/* user input command �j�p�g���i */
{
  int c1, c2;
  int num = 0;

  for (;;)
  {
    c1 = *s1;
    c2 = *s2;

    if (c2 >= 'A' && c2 <= 'Z')
      c2 |= 0x20;	/* ���p�g */

    if (num >= 2)	/* �ܤ֭n���G�r���ۦP */
    {
      if (!c1 || !c2)	/* ���� match �γ��� match �ҥi (s1�]�ts2 �� s2�]�ts1����) */
        return 0;
    }

    if (c1 > c2)	/* itoc.010927: ���P���^�ǭ� */
      return 1;
    else if (c1 < c2)
      return -1;

    s1++;
    s2++;
    num++;
  }
}


/* ----------------------------------------------------- */
/* match strings' similarity case-insensitively          */
/* ----------------------------------------------------- */
/* str_match(keyword, string)				 */
/* ----------------------------------------------------- */
/* 0 : equal            ("foo", "foo")                   */
/* -1 : mismatch        ("abc", "xyz")                   */
/* ow : similar         ("goo", "good")                  */
/* ----------------------------------------------------- */


static int
str_match(s1, s2)
  uschar *s1;		/* lower-case (sub)string */
  uschar *s2;
{
  int c1, c2;

  for (;;)
  {
    c1 = *s1;
    c2 = *s2;

    if (!c1)
      return c2;

    if (c2 >= 'A' && c2 <= 'Z')
      c2 |= 0x20;	/* ���p�g */

    if (c1 != c2)
      return -1;

    s1++;
    s2++;
  }
}


/* ----------------------------------------------------- */
/* search user/room by its ID                            */
/* ----------------------------------------------------- */


static ChatUser *
cuser_by_userid(userid)
  char *userid;
{
  ChatUser *cu;
  char buf[80]; /* Thor.980727: �@���̪��~80 */

  str_lower(buf, userid);
  for (cu = mainuser; cu; cu = cu->unext)
  {
    if (!cu->userno)
      continue;
    if (!str_cmp(buf, cu->userid))
      break;
  }
  return cu;
}


static ChatUser *
cuser_by_chatid(chatid)
  char *chatid;
{
  ChatUser *cu;
  char buf[80]; /* Thor.980727: �@���̪��~80 */

  str_lower(buf, chatid);

  for (cu = mainuser; cu; cu = cu->unext)
  {
    if (!cu->userno)
      continue;
    if (!str_cmp(buf, cu->chatid))
      break;
  }
  return cu;
}


static ChatUser *
fuzzy_cuser_by_chatid(chatid)
  char *chatid;
{
  ChatUser *cu, *xuser;
  int mode;
  char buf[80]; /* Thor.980727: �@���̪��~80 */

  str_lower(buf, chatid);
  xuser = NULL;

  for (cu = mainuser; cu; cu = cu->unext)
  {
    if (!cu->userno)
      continue;

    mode = str_match(buf, cu->chatid);
    if (mode == 0)
      return cu;

    if (mode > 0)
    {
      if (xuser)
	return FUZZY_USER;	/* �ŦX�̤j�� 2 �H */

      xuser = cu;
    }
  }
  return xuser;
}


static ChatRoom *
croom_by_roomid(roomid)
  char *roomid;
{
  ChatRoom *room;
  char buf[80]; /* Thor.980727: �@���̪��~80 */

  str_lower(buf, roomid);
  room = &mainroom;
  do
  {
    if (!str_cmp(buf, room->name))
      break;
  } while (room = room->next);
  return room;
}


/* ----------------------------------------------------- */
/* UserList routines                                     */
/* ----------------------------------------------------- */


static void
list_free(list)
  UserList **list;
{
  UserList *user, *next;

  for (user = *list, *list = NULL; user; user = next)
  {
    next = user->next;
    free(user);
  }
}


static void
list_add(list, user)
  UserList **list;
  ChatUser *user;
{
  UserList *node;
  char *userid;
  int len;

  len = strlen(userid = user->userid) + 1;
  if (node = (UserList *) malloc(sizeof(UserList) + len))
  {
    node->next = *list;
    node->userno = user->userno;
    memcpy(node->userid, userid, len);
    *list = node;
  }
}


static int
list_delete(list, userid)
  UserList **list;
  char *userid;
{
  UserList *node;
  char buf[80]; /* Thor.980727: ��J�@���̪��~ 80 */

  str_lower(buf, userid);

  while (node = *list)
  {
    if (!str_cmp(buf, node->userid))
    {
      *list = node->next;
      free(node);
      return 1;
    }
    list = &node->next;
  }

  return 0;
}


static int
list_belong(list, userno)
  UserList *list;
  int userno;
{
  while (list)
  {
    if (userno == list->userno)
      return 1;
    list = list->next;
  }
  return 0;
}


/* ------------------------------------------------------ */
/* non-blocking socket routines : send message to users   */
/* ------------------------------------------------------ */


static void
do_send(nfds, wset, msg)
  int nfds;
  fd_set *wset;
  char *msg;
{
  int len, sr;

#if 1
  /* Thor: for future reservation bug */
  zerotv.tv_sec = 0;
  zerotv.tv_usec = 0;
#endif

  sr = select(nfds + 1, NULL, wset, NULL, &zerotv);

  if (sr > 0)
  {
    len = strlen(msg) + 1;
    do
    {
      if (FD_ISSET(nfds, wset))
      {
	send(nfds, msg, len, 0);
	if (--sr <= 0)
	  return;
      }
    } while (--nfds > 0);
  }
}


static void
send_to_room(room, msg, userno, number)
  ChatRoom *room;
  char *msg;
  int userno;
  int number;
{
  ChatUser *cu;
  fd_set wset;
  int sock, max;
  int clitype;			/* ���� bbs client �� common client �⦸�B�z */
  char *str, buf[256];

  for (clitype = (number == MSG_MESSAGE || !number) ? 0 : 1;
    clitype < 2; clitype++)
  {
    FD_ZERO(&wset);
    max = -1;

    for (cu = mainuser; cu; cu = cu->unext)
    {
      if (cu->userno && (cu->clitype == clitype) &&
	(room == ROOM_ALL || room == cu->room) &&
	(!userno || !list_belong(cu->ignore, userno)))
      {
	sock = cu->sock;

	FD_SET(sock, &wset);

	if (max < sock)
	  max = sock;
      }
    }

    if (max <= 0)
      continue;

    if (clitype)
    {
      str = buf;

      if (*msg)
	sprintf(str, "%3d %s", number, msg);
      else
	sprintf(str, "%3d", number);
    }
    else
    {
      str = msg;
    }

    do_send(max, &wset, str);
  }
}


static void
send_to_user(user, msg, userno, number)
  ChatUser *user;
  char *msg;
  int userno;
  int number;
{
  int sock;

#if 0
  if (!user->userno || (!user->clitype && number && number != MSG_MESSAGE))
#endif
  /* Thor.980911: �p�G�duser->userno�h�blogin_user��error message�|�L�k�e�^ */
  if (!user->clitype && number != MSG_MESSAGE)
    return;

  if ((sock = user->sock) <= 0)
    return;

  if (!userno || !list_belong(user->ignore, userno))
  {
    fd_set wset;
    char buf[256];

    FD_ZERO(&wset);
    FD_SET(sock, &wset);

    if (user->clitype)
    {
      if (*msg)
	sprintf(buf, "%3d %s", number, msg);
      else
	sprintf(buf, "%3d", number);
      msg = buf;
    }

    do_send(sock, &wset, msg);
  }
}


/* ----------------------------------------------------- */


static void
room_changed(room)
  ChatRoom *room;
{
  if (room)
  {
    char buf[256];

    sprintf(buf, "= %s %d %d %s",
      room->name, room->occupants, room->rflag, room->topic);
    send_to_room(ROOM_ALL, buf, 0, MSG_ROOMNOTIFY);
  }
}


static void
user_changed(cu)
  ChatUser *cu;
{
  if (cu)
  {
    ChatRoom *room;
    char buf[256];

    room = cu->room;
    sprintf(buf, "= %s %s %s %s%s",
      cu->userid, cu->chatid, room->name, cu->rhost,
      ROOMOP(cu) ? " Op" : "");
    send_to_room(room, buf, 0, MSG_USERNOTIFY);
  }
}


static void
exit_room(user, mode, msg)
  ChatUser *user;
  int mode;
  char *msg;
{
  ChatRoom *room;
  char buf[128];

  if (!(room = user->room))
    return;

  user->room = NULL;
  /* user->uflag &= ~(PERM_ROOMOP | PERM_ALLCHAT); */
  user->uflag &= ~PERM_ROOMOP;
  /* Thor.980601: ���}�ж��ɥu�M room op, ���M sysop, chatroom �]�ѥͨ㦳 */

  if (--room->occupants > 0)
  {
    char *chatid;

    chatid = user->chatid;
    switch (mode)
    {
    case EXIT_LOGOUT:

      sprintf(buf, "�� %s ���}�F ... %.50s", chatid, (msg && *msg) ? msg : "");
      break;

    case EXIT_LOSTCONN:

      sprintf(buf, "�� %s ���F�_�u�������o", chatid);
      break;

    case EXIT_KICK:

      sprintf(buf, "�� �����I%s �Q��X�h�F", chatid);
      break;
    }

    if (!CLOAK(user))
      send_to_room(room, buf, 0, MSG_MESSAGE);

    sprintf(buf, "- %s", user->userid);
    send_to_room(room, buf, 0, MSG_USERNOTIFY);
    room_changed(room);
  }
  else if (room != &mainroom)
  {
    ChatRoom *next;

    fprintf(flog, "room-\t[%d] %s\n", user->sno, room->name);
    sprintf(buf, "- %s", room->name);

    room->prev->next = next = room->next;
    if (next)
      next->prev = room->prev;

    list_free(&room->invite);

    /* free(room); */

    /* �^�� */
    room->next = roompool;
    roompool = room;

    send_to_room(ROOM_ALL, buf, 0, MSG_ROOMNOTIFY);
  }
}


/* ----------------------------------------------------- */
/* chat commands                                         */
/* ----------------------------------------------------- */


#ifndef STAND_ALONE
/* ----------------------------------------------------- */
/* BBS server side routines                              */
/* ----------------------------------------------------- */


/* static */
int
acct_load(acct, userid)
  ACCT *acct;
  char *userid;
{
  int fd;

  usr_fpath((char *) acct, userid, FN_ACCT);
  fd = open((char *) acct, O_RDONLY);
  if (fd >= 0)
  {
    read(fd, acct, sizeof(ACCT));
    close(fd);
  }
  return fd;
}


static void
chat_query(cu, msg)
  ChatUser *cu;
  char *msg;
{
  FILE *fp;
  ACCT acct;
  char buf[256];

  /* Thor.980617: �i���d�O�_���Ŧr�� */
  if (*msg && acct_load(&acct, msg) >= 0)
  {
    sprintf(buf, "%s(%s) �@�W�� %d ���A�峹 %d �g",
      acct.userid, acct.username, acct.numlogins, acct.numposts);
    send_to_user(cu, buf, 0, MSG_MESSAGE);

    sprintf(buf, "�̪�(%s)�q(%s)�W��", Btime(&acct.lastlogin),
      (acct.lasthost[0] ? acct.lasthost : "�~�Ӫ�"));
    send_to_user(cu, buf, 0, MSG_MESSAGE);

    usr_fpath(buf, acct.userid, FN_PLANS);
    if (fp = fopen(buf, "r"))
    {
      int i;

      i = 0;
      while (fgets(buf, sizeof(buf), fp))
      {
	buf[strlen(buf) - 1] = 0;
	send_to_user(cu, buf, 0, MSG_MESSAGE);
	if (++i >= MAXQUERYLINES)
	  break;
      }
      fclose(fp);
    }
  }
  else
  {
    sprintf(buf, msg_no_such_id, msg);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }
}
#endif


static void
chat_clear(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (cu->clitype)
    send_to_user(cu, "", 0, MSG_CLRSCR);
  else
    send_to_user(cu, "/c", 0, MSG_MESSAGE);
}


static void
chat_date(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char buf[128];

  sprintf(buf, "�� �зǮɶ�: %s", Now());
  send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_topic(cu, msg)
  ChatUser *cu;
  char *msg;
{
  ChatRoom *room;
  char *topic, buf[128];

  room = cu->room;

  if (!ROOMOP(cu) && !OPENTOPIC(room))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  if (*msg == '\0')
  {
    send_to_user(cu, "�� �Ы��w���D", 0, MSG_MESSAGE);
    return;
  }

  topic = room->topic;
  str_ncpy(topic, msg, sizeof(room->topic));

  if (cu->clitype)
  {
    send_to_room(room, topic, 0, MSG_TOPIC);
  }
  else
  {
    sprintf(buf, "/t%s", topic);
    send_to_room(room, buf, 0, MSG_MESSAGE);
  }

  room_changed(room);

  if (!CLOAK(cu))
  {
    sprintf(buf, "�� %s �N���D�אּ \033[1;32m%s\033[m", cu->chatid, topic);
    send_to_room(room, buf, 0, MSG_MESSAGE);
  }
}


static void
chat_version(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char buf[80];

  sprintf(buf, "[Version] MapleBBS-3.10-20040726-PACK.itoc + Xchat-%d.%d", 
    XCHAT_VERSION_MAJOR, XCHAT_VERSION_MINOR);
  send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_nick(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *chatid, *str, buf[128];
  ChatUser *xuser;

  chatid = nextword(&msg);
  chatid[8] = '\0';
  if (!valid_chatid(chatid))
  {
    send_to_user(cu, "�� �o�Ӳ�ѥN���O�����T��", 0, MSG_MESSAGE);
    return;
  }

  xuser = cuser_by_chatid(chatid);
  if (xuser != NULL && xuser != cu)
  {
    send_to_user(cu, "�� �w�g���H�������n�o", 0, MSG_MESSAGE);
    return;
  }

  /* itoc.010528: ���i�H�ΧO�H�� id ������ѥN�� */
  usr_fpath(buf, chatid, NULL);
  if (dashd(buf) && str_cmp(chatid, cu->userid))
  {
    send_to_user(cu, "�� ��p�o�ӥN�����H���U�� id�A�ҥH�z�������ѥN��", 0, MSG_MESSAGE);
    return;
  }

  str = cu->chatid;

  if (!CLOAK(cu))
  {
    sprintf(buf, "�� %s �N��ѥN���אּ \033[1;33m%s\033[m", str, chatid);
    send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
  }

  strcpy(str, chatid);

  user_changed(cu);

  if (cu->clitype)
  {
    send_to_user(cu, chatid, 0, MSG_NICK);
  }
  else
  {
    sprintf(buf, "/n%s", chatid);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }
}


static void
chat_list_rooms(cuser, msg)
  ChatUser *cuser;
  char *msg;
{
  ChatRoom *cr, *room;
  char buf[128];
  int mode;

  if (RESTRICTED(cuser))
  {
    send_to_user(cuser, "�� �z�S���v���C�X�{������ѫ�", 0, MSG_MESSAGE);
    return;
  }

  mode = common_client_command;

  if (mode)
    send_to_user(cuser, "", 0, MSG_ROOMLISTSTART);
  else
    send_to_user(cuser, "\033[7m �ͤѫǦW��  �x�H�Ƣx���D        \033[m", 0,
      MSG_MESSAGE);

  room = cuser->room;
  cr = &mainroom;

  do
  {
    if ((cr == room) || !SECRET(cr) || CHATSYSOP(cuser))
    {
      if (mode)
      {
	sprintf(buf, "%s %d %d %s",
	  cr->name, cr->occupants, cr->rflag, cr->topic);
	send_to_user(cuser, buf, 0, MSG_ROOMLIST);
      }
      else
      {
	sprintf(buf, " %-12s�x%4d�x%s", cr->name, cr->occupants, cr->topic);
	if (LOCKED(cr))
	  strcat(buf, " [���]");
	if (SECRET(cr))
	  strcat(buf, " [���K]");
	if (OPENTOPIC(cr))
	  strcat(buf, " [���D]");
	send_to_user(cuser, buf, 0, MSG_MESSAGE);
      }
    }
  } while (cr = cr->next);

  if (mode)
    send_to_user(cuser, "", 0, MSG_ROOMLISTEND);
}


static void
chat_do_user_list(cu, msg, theroom)
  ChatUser *cu;
  char *msg;
  ChatRoom *theroom;
{
  ChatRoom *myroom, *room;
  ChatUser *user;
  int start, stop, curr, mode; /* , uflag; */
  char buf[128];

  curr = 0; /* Thor.980619: initialize curr */
  start = atoi(nextword(&msg));
  stop = atoi(nextword(&msg));

  mode = common_client_command;

  if (mode)
    send_to_user(cu, "", 0, MSG_USERLISTSTART);
  else
    send_to_user(cu, "\033[7m ��ѥN���x�ϥΪ̥N��  �x��ѫ� \033[m", 0,
      MSG_MESSAGE);

  myroom = cu->room;

  /* Thor.980717: �ݭn���ư� cu->userno == 0 �����p��? */
  for (user = mainuser; user; user = user->unext)
  {
#if 0	/* Thor.980717: �J�M cu ���ŤF���ٶi�ӷF��? */
    if (!cu->userno)
      continue;
#endif

    if (!user->userno) 
      continue;

    room = user->room;
    if ((theroom != ROOM_ALL) && (theroom != room))
      continue;

    /* Thor.980717: viewer check */
    if ((myroom != room) && (RESTRICTED(cu) || (room && SECRET(room) && !CHATSYSOP(cu))))
      continue;

    /* Thor.980717: viewee check */
    if (CLOAK(user) && (user != cu) && !CHATSYSOP(cu))
      continue;

    curr++;
    if (start && curr < start)
      continue;
    else if (stop && (curr > stop))
      break;

    if (mode)
    {
      if (!room)
	continue;		/* Xshadow: �٨S�i�J����ж����N���C�X */

      sprintf(buf, "%s %s %s %s",
	user->chatid, user->userid, room->name, user->rhost);

      /* Thor.980603: PERM_ALLCHAT �אּ default �S�� roomop, ���i�H�ۤv���o */
      /* if (uflag & (PERM_ROOMOP | PERM_ALLCHAT)) */
      if (ROOMOP(user))
	strcat(buf, " Op");
    }
    else
    {
      sprintf(buf, " %-8s�x%-12s�x%s",
	user->chatid, user->userid, room ? room->name : "[�b���f�r��]");
      /* Thor.980603: PERM_ALLCHAT �אּ default �S�� roomop, ���i�H�ۤv���o */
      /* if (uflag & (PERM_ROOMOP | PERM_ALLCHAT)) */
      /* if (uflag & (PERM_ROOMOP | PERM_CHATOP)) */
      if (ROOMOP(user))  /* Thor.980602: �Τ@�Ϊk */
	strcat(buf, " [Op]");
    }

    send_to_user(cu, buf, 0, mode ? MSG_USERLIST : MSG_MESSAGE);
  }

  if (mode)
    send_to_user(cu, "", 0, MSG_USERLISTEND);
}


static void
chat_list_by_room(cu, msg)
  ChatUser *cu;
  char *msg;
{
  ChatRoom *whichroom;
  char *roomstr, buf[128];

  roomstr = nextword(&msg);
  if (!*roomstr)
  {
    whichroom = cu->room;
  }
  else
  {
    if (!(whichroom = croom_by_roomid(roomstr)))
    {
      sprintf(buf, "�� �S�� [%s] �o�Ӳ�ѫ�", roomstr);
      send_to_user(cu, buf, 0, MSG_MESSAGE);
      return;
    }

    if (whichroom != cu->room && SECRET(whichroom) && !CHATSYSOP(cu))
    {
      send_to_user(cu, "�� �L�k�C�X�b���K��ѫǪ��ϥΪ�", 0, MSG_MESSAGE);
      return;
    }
  }
  chat_do_user_list(cu, msg, whichroom);
}


static void
chat_list_users(cu, msg)
  ChatUser *cu;
  char *msg;
{
  chat_do_user_list(cu, msg, ROOM_ALL);
}


static void
chat_chatroom(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (common_client_command)
    send_to_user(cu, "��ѫ�", 0, MSG_CHATROOM);
}


static void
chat_map_chatids(cu, whichroom)
  ChatUser *cu;			/* Thor: �٨S���@���P���� */
  ChatRoom *whichroom;
{
  int c;
  ChatRoom *myroom, *room;
  ChatUser *user;
  char buf[128];

  myroom = cu->room;

  send_to_user(cu,
    "\033[7m ��ѥN�� �ϥΪ̥N��  �x ��ѥN�� �ϥΪ̥N��  �x ��ѥN�� �ϥΪ̥N�� \033[m", 0, MSG_MESSAGE);

  for (c = 0, user = mainuser; user; user = user->unext)
  {
    if (!cu->userno)
      continue;

    room = user->room;
    if (whichroom != ROOM_ALL && whichroom != room)
      continue;

    if (myroom != room)
    {
      if (RESTRICTED(cu) ||	/* Thor: �n��check room �O���O�Ū� */
	(room && SECRET(room) && !CHATSYSOP(cu)))
	continue;
    }

    if (CLOAK(user) && (user != cu) && !CHATSYSOP(cu))	/* Thor:�����N */
      continue;

    sprintf(buf + (c * 24), " %-8s%c%-12s%s",
      user->chatid, ROOMOP(user) ? '*' : ' ',
      user->userid, (c < 2 ? "�x" : "  "));

    if (++c == 3)
    {
      send_to_user(cu, buf, 0, MSG_MESSAGE);
      c = 0;
    }
  }

  if (c > 0)
    send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_map_chatids_thisroom(cu, msg)
  ChatUser *cu;
  char *msg;
{
  chat_map_chatids(cu, cu->room);
}


static void
chat_setroom(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *modestr;
  ChatRoom *room;
  char *chatid;
  int sign, flag;
  char *fstr, buf[128];

  if (!ROOMOP(cu))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  modestr = nextword(&msg);
  sign = 1;
  if (*modestr == '+')
  {
    modestr++;
  }
  else if (*modestr == '-')
  {
    modestr++;
    sign = 0;
  }

  if (*modestr == '\0')
  {
    send_to_user(cu,
      "�� �Ы��w���A: {[+(�]�w)][-(����)]}{[L(���)][s(���K)][t(�}����D)}", 0, MSG_MESSAGE);
    return;
  }

  room = cu->room;
  chatid = cu->chatid;

  while (*modestr)
  {
    flag = 0;
    switch (*modestr)
    {
    case 'l':
    case 'L':
      flag = ROOM_LOCKED;
      fstr = "���";
      break;

    case 's':
    case 'S':
      flag = ROOM_SECRET;
      fstr = "���K";
      break;

    case 't':
    case 'T':
      flag = ROOM_OPENTOPIC;
      fstr = "�}����D";
      break;

    default:
      sprintf(buf, "�� ���A���~�G[%c]", *modestr);
      send_to_user(cu, buf, 0, MSG_MESSAGE);
    }

    /* Thor: check room �O���O�Ū�, ���Ӥ��O�Ū� */

    if (flag && (room->rflag & flag) != sign * flag)
    {
      room->rflag ^= flag;

      if (!CLOAK(cu))
      {
	sprintf(buf, "�� ����ѫǳQ %s %s [%s] ���A",
	  chatid, sign ? "�]�w��" : "����", fstr);
	send_to_room(room, buf, 0, MSG_MESSAGE);
      }
    }
    modestr++;
  }

  /* Thor.980602: ���� Main room ��_ or ���K�A�_�h���}���N�i���ӡA�n�ݤ]�ݤ���C
     �Q�n��H�]�𤣶i main room�A���|�ܩ_�ǶܡH */

  if (!str_cmp(MAIN_NAME, room->name))
  {
    if (room->rflag & (ROOM_LOCKED | ROOM_SECRET))
    {
      send_to_room(room, "�� ���ѨϬI�F�y�_��z���]�k", 0, MSG_MESSAGE);
      room->rflag &= ~(ROOM_LOCKED | ROOM_SECRET);
    }
  }

  room_changed(room);
}


static char *chat_msg[] =
{
  "[//]help", "MUD-like ����ʵ�",
  "[/h]elp op", "�ͤѫǺ޲z���M�Ϋ��O",
  "[/a]ct <msg>", "���@�Ӱʧ@",
  "[/b]ye [msg]", "�D�O",
  "[/c]lear  [/d]ate", "�M���ù�  �ثe�ɶ�",
  "[/i]gnore [user]", "�����ϥΪ�",
  "[/j]oin <room>", "�إߩΥ[�J�ͤѫ�",
  "[/l]ist [start [stop]]", "�C�X�ͤѫǨϥΪ�",
  "[/m]sg <id|user> <msg>", "�� <id> ��������",
  "[/n]ick <id>", "�N�ͤѥN������ <id>",
  "[/p]ager", "�����I�s��",
  "[/q]uery <user>", "�d�ߺ���",
  "[/qui]t [msg]", "�D�O",  
  "[/r]oom", "�C�X�@��ͤѫ�",
  "[/t]ape", "�}��������",
  "[/u]nignore <user>", "��������",
  "[/w]ho", "�C�X���ͤѫǨϥΪ�",
  "[/w]hoin <room>", "�C�X�ͤѫ�<room> ���ϥΪ�",
  NULL
};


static char *room_msg[] =
{
  "[/f]lag [+-][lst]", "�]�w��w�B���K�B�}����D",
  "[/i]nvite <id>", "�ܽ� <id> �[�J�ͤѫ�",
  "[/kick] <id>", "�N <id> ��X�ͤѫ�",
  "[/o]p [<id>]", "�N Op ���v�O�ಾ�� <id>",
  "[/topic] <text>", "���Ӹ��D",
  "[/w]all", "�s�� (�����M��)",
  NULL
};


static void
chat_help(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char **table, *str, buf[128];

  if (!str_cmp("op", nextword(&msg)))
  {
    send_to_user(cu, "�ͤѫǺ޲z���M�Ϋ��O", 0, MSG_MESSAGE);
    table = room_msg;
  }
  else
  {
    table = chat_msg;
  }

  while (str = *table++)
  {
    sprintf(buf, "  %-20s- %s", str, *table++);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }
}


static void
chat_private(cu, msg)
  ChatUser *cu;
  char *msg;
{
  ChatUser *xuser;
  char *recipient, buf[128];

  recipient = nextword(&msg);
  xuser = (ChatUser *) fuzzy_cuser_by_chatid(recipient);
  if (xuser == NULL)		/* Thor.980724: �� userid�]�i�Ǯ����� */
  {
    xuser = cuser_by_userid(recipient);
  }

  if (xuser == NULL)
  {
    sprintf(buf, msg_no_such_id, recipient);
  }
  else if (xuser == FUZZY_USER)
  {				/* ambiguous */
    strcpy(buf, "�� �Ы�����ѥN��");
  }
  else if (*msg)
  {
    int userno;

    userno = cu->userno;

    sprintf(buf, "\033[1m*%s*\033[m %.50s", cu->chatid, msg);
    send_to_user(xuser, buf, userno, MSG_MESSAGE);

    if (xuser->clitype)		/* Xshadow: �p�G���O�� client �W�Ӫ� */
    {
      sprintf(buf, "%s %s %.50s", cu->userid, cu->chatid, msg);
      send_to_user(xuser, buf, userno, MSG_PRIVMSG);
    }

    if (cu->clitype)
    {
      sprintf(buf, "%s %s %.50s", xuser->userid, xuser->chatid, msg);
      send_to_user(cu, buf, 0, MSG_MYPRIVMSG);
    }

    sprintf(buf, "%s> %.50s", xuser->chatid, msg);
  }
  else
  {
    sprintf(buf, "�� �z�Q�� %s ������ܩO�H", xuser->chatid);
  }

  send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_cloak(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (CHATSYSOP(cu))
  {
    char buf[128];

    cu->uflag ^= PERM_CLOAK;
    sprintf(buf, "�� %s", CLOAK(cu) ? MSG_CLOAKED : MSG_UNCLOAK);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }
}


/* ----------------------------------------------------- */


static void
arrive_room(cuser, room)
  ChatUser *cuser;
  ChatRoom *room;
{
  char *rname, buf[256];

  /* Xshadow: �����e���ۤv, �ϥ����ж��N�|���s build user list */

  sprintf(buf, "+ %s %s %s %s",
    cuser->userid, cuser->chatid, room->name, cuser->rhost);
  if (ROOMOP(cuser))
    strcat(buf, " Op");
  send_to_room(room, buf, 0, MSG_USERNOTIFY);

  room->occupants++;
  room_changed(room);

  cuser->room = room;
  rname = room->name;

  if (cuser->clitype)
  {
    send_to_user(cuser, rname, 0, MSG_ROOM);
    send_to_user(cuser, room->topic, 0, MSG_TOPIC);
  }
  else
  {
    sprintf(buf, "/r%s", rname);
    send_to_user(cuser, buf, 0, MSG_MESSAGE);
    sprintf(buf, "/t%s", room->topic);
    send_to_user(cuser, buf, 0, MSG_MESSAGE);
  }

  sprintf(buf, "�� \033[32;1m%s\033[m �i�J \033[33;1m[%s]\033[m �]�[",
    cuser->chatid, rname);

  if (!CLOAK(cuser))
    send_to_room(room, buf, cuser->userno, MSG_MESSAGE);
  else
    send_to_user(cuser, buf, 0, MSG_MESSAGE);
}


static int
enter_room(cuser, rname, msg)
  ChatUser *cuser;
  char *rname;
  char *msg;
{
  ChatRoom *room;
  int create;
  char buf[256];

  create = 0;
  room = croom_by_roomid(rname);

  if (room == NULL)
  {
    /* new room */

#ifdef DEBUG
    logit(cuser->userid, "create new room");
    debug_room();
#endif

    if (room = roompool)
    {
      roompool = room->next;
    }
    else
    {
      room = (ChatRoom *) malloc(sizeof(ChatRoom));
    }

    if (room == NULL)
    {
      send_to_user(cuser, "�� �L�k�A�s�P�]�[�F", 0, MSG_MESSAGE);
      return 0;
    }

    memset(room, 0, sizeof(ChatRoom));
    str_ncpy(room->name, rname, sizeof(room->name));
    strcpy(room->topic, "�o�O�@�ӷs�Ѧa");

    sprintf(buf, "+ %s 1 0 %s", room->name, room->topic);
    send_to_room(ROOM_ALL, buf, 0, MSG_ROOMNOTIFY);

    if (mainroom.next)
      mainroom.next->prev = room;
    room->next = mainroom.next;

    mainroom.next = room;
    room->prev = &mainroom;

#ifdef DEBUG
    logit(cuser->userid, "create room succeed");
    debug_room();
#endif

    create = 1;
    fprintf(flog, "room+\t[%d] %s\n", cuser->sno, rname);
  }
  else
  {
    if (cuser->room == room)
    {
      sprintf(buf, "�� �z���ӴN�b [%s] ��ѫ��o :)", rname);
      send_to_user(cuser, buf, 0, MSG_MESSAGE);
      return 0;
    }

    if (!CHATSYSOP(cuser) && LOCKED(room) &&
      !list_belong(room->invite, cuser->userno))
    {
      send_to_user(cuser, "�� �����c���A�D�в��J", 0, MSG_MESSAGE);
      return 0;
    }
  }

  exit_room(cuser, EXIT_LOGOUT, msg);
  arrive_room(cuser, room);

  if (create)
    cuser->uflag |= PERM_ROOMOP;

  return 0;
}


static void
cuser_free(cuser)
  ChatUser *cuser;
{
  int sock;

  sock = cuser->sock;
  shutdown(sock, 2);
  close(sock);

  FD_CLR(sock, &mainfset);

  list_free(&cuser->ignore);
  totaluser--;

  if (cuser->room)
  {
    exit_room(cuser, EXIT_LOSTCONN, NULL);
  }

  fprintf(flog, "BYE\t[%d] T%d X%d\n",
    cuser->sno, time(0) - cuser->tbegin, cuser->xdata);
}


static void
print_user_counts(cuser)
  ChatUser *cuser;
{
  ChatRoom *room;
  int num, userc, suserc, roomc, number;
  char buf[256];

  userc = suserc = roomc = 0;

  room = &mainroom;
  do
  {
    num = room->occupants;
    if (SECRET(room))
    {
      suserc += num;
      if (CHATSYSOP(cuser))
	roomc++;
    }
    else
    {
      userc += num;
      roomc++;
    }
  } while (room = room->next);

  number = (cuser->clitype) ? MSG_MOTD : MSG_MESSAGE;

  sprintf(buf,
    "�� �w����{�i��ѫǡj�A�ثe�}�F \033[1;31m%d\033[m ���]�[", roomc);
  send_to_user(cuser, buf, 0, number);

  sprintf(buf, "�� �@�� \033[1;36m%d\033[m �H���\\�s���}", userc);
  if (suserc)
    sprintf(buf + strlen(buf), " [%d �H�b���K��ѫ�]", suserc);

  send_to_user(cuser, buf, 0, number);
}


static int
login_user(cu, msg)
  ChatUser *cu;
  char *msg;
{
  int utent;

  char *userid;
  char *chatid, *passwd;
  ChatUser *xuser;
  int level;
  /* struct hostent *hp; */

#ifndef STAND_ALONE
  ACCT acct;
#endif

  /* Xshadow.0915: common client support : /-! userid chatid password */
  /* client/server �����̾� userid �� .PASSWDS �P�_ userlevel */

  userid = nextword(&msg);
  chatid = nextword(&msg);

#ifdef	DEBUG
  logit("ENTER", userid);
#endif

#ifndef STAND_ALONE
  /* Thor.980730: parse space before passwd */

  passwd = msg;

  /* Thor.980813: ���L�@�Ů�Y�i, �]���ϥ��p�Gchatid���Ů�, �K�X�]���� */
  /* �N��K�X��, �]���|����:p */
  /* �i�O�p�G�K�X�Ĥ@�Ӧr�O�Ů�, �����Ӧh�Ů�|�i����... */
  /* Thor.980910: �ѩ� nextword�קאּ�ᱵ�Ů��0, �ǤJ�ȫh�����Ჾ��0��,
                  �ҥH���ݧ@���ʧ@ */
#if 0
  if (*passwd == ' ')
    passwd++;
#endif

  /* Thor.980729: load acct */

  if (!*userid || (acct_load(&acct, userid) < 0))
  {

#ifdef	DEBUG
    logit("noexist", userid);
#endif

    if (cu->clitype)
      send_to_user(cu, "���~���ϥΪ̥N��", 0, ERR_LOGIN_NOSUCHUSER);
    else
      send_to_user(cu, CHAT_LOGIN_INVALID, 0, MSG_MESSAGE);

    return -1;
  }

  /* Thor.980813: ��ίu�� password check, for C/S bbs */

  /* Thor.990214: �`�N daolib �� �D 0 �N���� */
  /* if (!chkpasswd(acct.passwd, passwd)) */
  if (chkpasswd(acct.passwd, passwd))
  {

#ifdef	DEBUG
    logit("fake", userid);
#endif

    if (cu->clitype)
      send_to_user(cu, "�K�X���~", 0, ERR_LOGIN_PASSERROR);
    else
      send_to_user(cu, CHAT_LOGIN_INVALID, 0, MSG_MESSAGE);

    return -1;
  }

  level = acct.userlevel;
  utent = acct.userno;

#else				/* STAND_ALONE */
  level = 1;
  utent = ++userno_inc;
#endif				/* STAND_ALONE */

  /* Thor.980819: for client/server bbs */

#ifdef DEBUG
  log_user(NULL);
#endif

  for (xuser = mainuser; xuser; xuser = xuser->unext)
  {

#ifdef DEBUG
    log_user(xuser);
#endif

    if (xuser->userno == utent)
    {

#ifdef	DEBUG
      logit("enter", "bogus");
#endif

      if (cu->clitype)
	send_to_user(cu, "�ФŬ��������i�J��ѫǡI", 0,
	  ERR_LOGIN_USERONLINE);
      else
	send_to_user(cu, CHAT_LOGIN_BOGUS, 0, MSG_MESSAGE);
      return -1;		/* Thor: �άO0�����ۤv�F�_? */
    }
  }


#ifndef STAND_ALONE
  /* Thor.980629: �Ȯɭɥ� invalid_chatid �o�� �S��PERM_CHAT���H */
               
  if (!valid_chatid(chatid) || !(level & PERM_CHAT) || (level & PERM_DENYCHAT))
  { /* Thor.981012: �����@��, �s denychat�]BAN��, �K�o client�@�� */

#ifdef	DEBUG
    logit("enter", chatid);
#endif

    if (cu->clitype)
      send_to_user(cu, "���X�k����ѫǥN���I", 0, ERR_LOGIN_NICKERROR);
    else
      send_to_user(cu, CHAT_LOGIN_INVALID, 0, MSG_MESSAGE);
    return 0;
  }
#endif

#ifdef	DEBUG
  debug_user();
#endif

  if (cuser_by_chatid(chatid) != NULL)
  {
    /* chatid in use */

#ifdef	DEBUG
    logit("enter", "duplicate");
#endif

    if (cu->clitype)
      send_to_user(cu, "�o�ӥN���w�g���H�ϥ�", 0, ERR_LOGIN_NICKINUSE);
    else
      send_to_user(cu, CHAT_LOGIN_EXISTS, 0, MSG_MESSAGE);
    return 0;
  }

#ifdef DEBUG			/* CHATSYSOP �@�i�ӴN���� */
  cu->uflag = level & ~(PERM_ROOMOP | PERM_CHATOP | (CHATSYSOP(cu) ? 0 : PERM_CLOAK));
#else
  cu->uflag = level & ~(PERM_ROOMOP | PERM_CHATOP | PERM_CLOAK);
#endif

  /* Thor: �i�ӥ��M�� ROOMOP (�PPERM_CHAT) */

  strcpy(cu->userid, userid);
  str_ncpy(cu->chatid, chatid, sizeof(cu->chatid));
  /* Thor.980921: str_ncpy�P�@�� strncpy���Ҥ��P, �S�O�`�N */

  fprintf(flog, "ENTER\t[%d] %s\n", cu->sno, userid);

  /* Xshadow: ���o client ���ӷ� */

  dns_name(cu->rhost, cu->ibuf);
  str_ncpy(cu->rhost, cu->ibuf, sizeof(cu->rhost));
#if 0
  hp = gethostbyaddr(cu->rhost, sizeof(struct in_addr), AF_INET);
  str_ncpy(cu->rhost, hp ? hp->h_name : inet_ntoa((struct in_addr *) cu->rhost), sizeof(cu->rhost));
#endif

  cu->userno = utent;

  if (cu->clitype)
    send_to_user(cu, "���Q", 0, MSG_LOGINOK);
  else
    send_to_user(cu, CHAT_LOGIN_OK, 0, MSG_MESSAGE);

  arrive_room(cu, &mainroom);

  send_to_user(cu, "", 0, MSG_MOTDSTART);
  print_user_counts(cu);
  send_to_user(cu, "", 0, MSG_MOTDEND);

#ifdef	DEBUG
  logit("enter", "OK");
#endif

  return 0;
}


static void
chat_act(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (*msg)
  {
    char buf[256];

    sprintf(buf, "%s \033[36m%s\033[m", cu->chatid, msg);
    send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
  }
}


static void
chat_ignore(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *str, buf[256];

  if (RESTRICTED(cu))
  {
    str = "�� �z�S�� ignore �O�H���v�Q";
  }
  else
  {
    char *ignoree;

    str = buf;
    ignoree = nextword(&msg);
    if (*ignoree)
    {
      ChatUser *xuser;

      xuser = cuser_by_userid(ignoree);

      if (xuser == NULL)
      {
	sprintf(str, msg_no_such_id, ignoree);
      }
      else if (xuser == cu || CHATSYSOP(xuser) ||
	(ROOMOP(xuser) && (xuser->room == cu->room)))
      {
	sprintf(str, "�� ���i�H ignore [%s]", ignoree);
      }
      else
      {
	if (list_belong(cu->ignore, xuser->userno))
	{
	  sprintf(str, "�� %s �w�g�Q�ᵲ�F", xuser->chatid);
	}
	else
	{
	  list_add(&(cu->ignore), xuser);
	  sprintf(str, "�� �N [%s] ���J�N�c�F :p", xuser->chatid);
	}
      }
    }
    else
    {
      UserList *list;

      if (list = cu->ignore)
      {
	int len;
	char userid[16];

	send_to_user(cu, "�� �o�ǤH�Q���J�N�c�F�G", 0, MSG_MESSAGE);
	len = 0;
	do
	{
	  sprintf(userid, "%-13s", list->userid);
	  strcpy(str + len, userid);
	  len += 13;
	  if (len >= 78)
	  {
	    send_to_user(cu, str, 0, MSG_MESSAGE);
	    len = 0;
	  }
	} while (list = list->next);

	if (len == 0)
	  return;
      }
      else
      {
	str = "�� �z�ثe�èS�� ignore ����H";
      }
    }
  }

  send_to_user(cu, str, 0, MSG_MESSAGE);
}


static void
chat_unignore(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *ignoree, *str, buf[80];

  ignoree = nextword(&msg);

  if (*ignoree)
  {
    sprintf(str = buf, (list_delete(&(cu->ignore), ignoree)) ?
      "�� [%s] ���A�Q�z�N���F" :
      "�� �z�å� ignore [%s] �o���H��", ignoree);
  }
  else
  {
    str = "�� �Ы��� user ID";
  }
  send_to_user(cu, str, 0, MSG_MESSAGE);
}


static void
chat_join(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (RESTRICTED(cu))
  {
    send_to_user(cu, "�� �z�S���[�J��L��ѫǪ��v��", 0, MSG_MESSAGE);
  }
  else
  {
    char *roomid = nextword(&msg);

    if (*roomid)
      enter_room(cu, roomid, msg);
    else
      send_to_user(cu, "�� �Ы��w��ѫ�", 0, MSG_MESSAGE);
  }
}


static void
chat_kick(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *twit, buf[80];
  ChatUser *xuser;
  ChatRoom *room;

  if (!ROOMOP(cu))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  twit = nextword(&msg);
  xuser = cuser_by_chatid(twit);

  if (xuser == NULL)
  {                       /* Thor.980604: �� userid�]���q */
    xuser = cuser_by_userid(twit);
  }
               
  if (xuser == NULL)
  {
    sprintf(buf, msg_no_such_id, twit);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  room = cu->room;
  if (room != xuser->room || CLOAK(xuser))
  {
    sprintf(buf, msg_not_here, twit);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  if (CHATSYSOP(xuser))
  {
    sprintf(buf, "�� ���i�H kick [%s]", twit);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  exit_room(xuser, EXIT_KICK, (char *) NULL);

  if (room == &mainroom)
    xuser->uptime = 0;		/* logout_user(xuser); */
  else
    enter_room(xuser, MAIN_NAME, (char *) NULL);  
    /* Thor.980602: ����N��,���nshow�Xxxx���}�F���T������n */
}


static void
chat_makeop(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *newop, buf[80];
  ChatUser *xuser;
  ChatRoom *room;

  /* Thor.980603: PERM_ALLCHAT �אּ default �S�� roomop, ���i�H�ۤv���o */

  newop = nextword(&msg);

  room = cu->room;

  if (!*newop && CHATSYSOP(cu))
  {
    /* Thor.980603: PERM_ALLCHAT �אּ default �S�� roomop, ���i�H�ۤv���o */
    cu->uflag ^= PERM_CHATOP;

    user_changed(cu);
    if (!CLOAK(cu))
    {
      sprintf(buf,ROOMOP(cu) ? "�� �Ѩ� �N Op �v�O�¤� %s"
                             : "�� �Ѩ� �N %s �� Op �v�O���^", cu->chatid);
      send_to_room(room, buf, 0, MSG_MESSAGE);
    }
    
    return;
  }

  /* if (!ROOMOP(cu)) */
  if (!(cu->uflag & PERM_ROOMOP)) /* Thor.980603: chat room�`�ޤ����ಾ Op �v�O */
  {
    send_to_user(cu, "�� �z�����ಾ Op ���v�O" /* msg_not_op */, 0, MSG_MESSAGE);
    return;
  }

  xuser = cuser_by_chatid(newop);

#if 0
  if (xuser == NULL)
  {                       /* Thor.980604: �� userid ���]�q */
    xuser = cuser_by_userid(newop);
  }
#endif

  if (xuser == NULL)
  {
    sprintf(buf, msg_no_such_id, newop);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  if (cu == xuser)
  {
    send_to_user(cu, "�� �z���N�w�g�O Op �F��", 0, MSG_MESSAGE);
    return;
  }

  /* room = cu->room; */

  if (room != xuser->room || CLOAK(xuser))
  {
    sprintf(buf, msg_not_here, xuser->chatid);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  cu->uflag &= ~PERM_ROOMOP;
  xuser->uflag |= PERM_ROOMOP;

  user_changed(cu);
  user_changed(xuser);

  if (!CLOAK(cu))
  {
    sprintf(buf, "�� %s �N Op �v�O�ಾ�� %s",
      cu->chatid, xuser->chatid);
    send_to_room(room, buf, 0, MSG_MESSAGE);
  }
}


static void
chat_invite(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char *invitee, buf[80];
  ChatUser *xuser;
  ChatRoom *room;
  UserList **list;

  if (!ROOMOP(cu))
  {
    send_to_user(cu, msg_not_op, 0, MSG_MESSAGE);
    return;
  }

  invitee = nextword(&msg);
  xuser = cuser_by_chatid(invitee);

#if 0
  if (xuser == NULL)
  {                       /* Thor.980604: �� userid ���]�q */
    xuser = cuser_by_userid(invitee);
  }
#endif
               
  if (xuser == NULL)
  {
    sprintf(buf, msg_no_such_id, invitee);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }

  room = cu->room;		/* Thor: �O�_�n check room �O�_ NULL ? */
  list = &(room->invite);

  if (list_belong(*list, xuser->userno))
  {
    sprintf(buf, "�� %s �w�g�����L�ܽФF", xuser->chatid);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
    return;
  }
  list_add(list, xuser);

  sprintf(buf, "�� %s �ܽбz�� [%s] ��ѫ�",
    cu->chatid, room->name);
  send_to_user(xuser, buf, 0, MSG_MESSAGE);
  sprintf(buf, "�� %s ����z���ܽФF", xuser->chatid);
  send_to_user(cu, buf, 0, MSG_MESSAGE);
}


static void
chat_broadcast(cu, msg)
  ChatUser *cu;
  char *msg;
{
  char buf[80];

  if (!CHATSYSOP(cu))
  {
    send_to_user(cu, "�� �z�S���b��ѫǼs�����v�O!", 0, MSG_MESSAGE);
    return;
  }

  if (*msg == '\0')
  {
    send_to_user(cu, "�� �Ы��w�s�����e", 0, MSG_MESSAGE);
    return;
  }

  sprintf(buf, "\033[1m�� " BBSNAME "�ͤѫǼs���� [%s].....\033[m",
    cu->chatid);
  send_to_room(ROOM_ALL, buf, 0, MSG_MESSAGE);
  sprintf(buf, "�� %s", msg);
  send_to_room(ROOM_ALL, buf, 0, MSG_MESSAGE);
}


static void
chat_bye(cu, msg)
  ChatUser *cu;
  char *msg;
{
  exit_room(cu, EXIT_LOGOUT, msg);
  cu->uptime = 0;
  /* logout_user(cu); */
}


/* --------------------------------------------- */
/* MUD-like social commands : action             */
/* --------------------------------------------- */


#if 0	/* itoc.010816: ���s½�פ@�Ǥ��ӾA�� action �ԭz */
  1. �`�N���r���ƦC�C
  2. �зR�Υ��μ��I�Ÿ��C
  3. �T�� action ���঳���СC
  4. �ѩ� action �ĥΡu�������v�A�G�̦n���n�����O�]�t�t�@���O�Ҧ�����r�����p�C
     �]�Ҧp fire/fireball�Akiss/kissbye�Ano/nod�Atea/tear/tease�Adrive/drivel�Alove/lover�^
     �]���o�˪����Τ]���|���ˡA�u�O�ϥΪ̮e���d�V�^
  5. �ѩ� action �������ܤ� 2 bytes�A�G���n�� //1 //2 �o���u���@�Ӧr�� action�C
  6. �ѩ� action �ĥγ������A�G���O�������Y�g�C
  7. �Τ@ action message �̫ᤣ�n�[�y�I�C
  8. �ץ����r�C�]�O adore�A���O aodre �� :p�^
  9. ��֭��Ъ��r���C�]���n�ѬO�u���h���ӡv�� :p�^
#endif


struct ChatAction
{
  char *verb;			/* �ʵ� */
  char *chinese;		/* ����½Ķ */
  char *part1_msg;		/* ���� */
  char *part2_msg;		/* �ʧ@ */
};


static ChatAction *
action_fit(action, actnum, cmd)		/* ��ݬݬO���� ChatAction */
  ChatAction *action;
  int actnum;
  char *cmd;
{
  ChatAction *pos, *locus, *mid;	/* locus:������ mid:������ pos:�k���� */
  int cmp;

  /* itoc.010927: �ѩ� ChatAction ���O���r���ƧǪ��A�ҥH�i�H�� binary search */
  /* itoc.010928.����:�ѩ�O binary search �ҥH���M recline �Ʀb recycle �e��
    ���O�� //rec �ɫo�i��X�{ //recycle ���ĪG�A�P�w�u�����Ǻݿ� binary ������ */

  locus = action;
  pos = action + actnum - 1;		/* �̫�@�ӬO NULL�A�����i��Q�ˬd�� */

  while (1)
  {
    if (pos <= locus + 1)
      break;

    mid = locus + ((pos - locus) >> 1);

    if (!(cmp = str_belong(mid->verb, cmd)))	/* itoc.010321: MUD-like match */
      return mid;
    else if (cmp < 0)
      locus = mid;
    else
      pos = mid;
  }

  /* �S��: �p�G�k���а��d�b 1�A�n�ˬd�� 0 �� */
  if (pos == action + 1)
  {
    if (!str_belong(action->verb, cmd))		/* itoc.010321: MUD-like match */
      return action;
  }

  return NULL;
}


/* itoc.010805.����:  //adore sysop   itoc �� sysop ���������p�ʷʦ����A�s�������K */

#define ACTNUM_PARTY	110

static ChatAction party_data[ACTNUM_PARTY] =
{
  {
    "adore", "����", "��", "���������p�ʷʦ����A�s�������K"
  },
  {
    "aluba", "���|��", "��", "�[�W�W�l���즺�I"
  },
  {
    "aruba", "���|��", "��", "�[�W�W�l���즺�I"
  },
  {
    "bark", "�p�s", "�L�L�I��", "�j�n�p�s"
  },
  {
    "bite", "�٫r", "��", "�r�o���h����"
  },
  {
    "blade", "�@�M", "�@�M��", "�e�W���"
  },
  {
    "bless", "����", "����", "�߷Q�Ʀ�"
  },
  {
    "blink", "�w��", "���", "�w�w���A�����t�ܵۤ���"
  },
  {
    "board", "�D���O", "��", "��h���D���O"
  },
  {
    "bokan", "��\\", "���x�L�X�A�W�իݵo�K�K��M���A�q���E�{�A��", "�ϥX�F�Т�--�٢��"
  },
  {
    "bow", "���`", "���`���q���V", "���`"
  },
  {
    "box", "������", "�}�l���\\������A��", "�@�xŦ����"
  },
  {
    "bye", "�T�T", "�V", "���T�T"
  },
  {
    "cake", "��J�|", "���X�@�ӳJ�|�A��", "���y�W�{�h"
  },
  {
    "call", "�I��", "�j�n���I��A�ڡ�",	"�ڡ�H�b���̰ڰڡ��"
  },
  {
    "caress", "����", "���������N��", ""
  },
  {
    "clap", "���x", "�V", "���P���x"
  },
  {
    "claw", "���", "�q�߫}�ֶ�ɤF���ߤ��A��",	"��o���ѷt�a"
  },
  {
    "clock", "���x��", "����", "���x���A�ְ_�ɰ�"
  },
  {
    "cola", "��i��", "��", "��F�@�[�ڪ��i��"
  },
  {
    "comfort", "�w��", "�Ũ��w��", ""
  },
  {
    "congratulate", "����", "�q�I�᮳�X�F�Ԭ��A��I��I����", ""
  },
  {
    "cowhide", "�@��","���@�l��", "�����a�⥴"
  },
  {
    "cpr", "�f��f", "���", "���f��f�H�u�I�l"
  },
  {
    "crime", "�D�w", "���G", "���D�w���Ƥ����A���y�Ѯ�"
  },
  {
    "cringe", "�^��", "�V", "���`�}���A�n���^��"
  },
  {
    "cry", "�j��", "�V", "�z�ޤj��"
  },
  {
    "curtsy", "���j§", "�u���a���", "�椤�j�@�����}��§�C"
  },
  {
    "dance", "���R", "�ԤF", "���⽡���_�R"
  },
  {
    "destroy", "����", "���_�F�y���j�����G��z�A�F�V", ""
  },
  {
    "dogleg", "���L", "��", "���۩^�ӡA�j�j���L�F�@�f"
  },
  {
    "drivel", "�y�f��",	"���",	"�y�f��"
  },
  {
    "envy", "�r�}", "�V", "�y�S�X�r�}������"
  },
  {
    "evening", "�ߦw", "��", "���y�ߦw�z"
  },
  {
    "eye", "�e��i", "��", "�W�e��i"
  },
  {
    "fire", "�R��", "���ۤ������K�Ψ��V", ""
  },
  {
    "forgive", "���", "����", "���D�p�A��̤F�L"
  },
  {
    "french", "�k���k",	"����Y����", "���V�̡���z�I�@�Ӯ������k�ꦡ�`�k"
  },
  {
    "fuzzy", "����", "���X�����@���V", "�ĹL�h"
  },
  {
    "gag", "�_�L��", "��", " ���L�ڥΰw�_�_��"
  },
  {
    "giggle", "�̯�", "���", "�̶̪��b��"
  },
  {
    "glare", "���H", "�N�N�a����", ""
  },
  {
    "glue", "�ɤ�", "�Χְ���", "�����H�F�_��"
  },
  {
    "goodbye", "�i�O", "�\\���L�L���V",	"�i�O"
  },
  {
    "grin", "�l��", "��", "�S�X���c�����e"
  },
  {
    "growl", "�H��", "��", "�H�����w"
  },
  {
    "hand", "����", "��", "����"
  },
  {
    "hide", "��", "���b", "�I��"
  },
  {
    "hospital", "�e��|", "��", "�e�i��|"
  },
  {
    "hrk", "�@�s��", "�Ií�F���ΡA�׻E�F���l�A��", "�ϥX�F�@�O�֢�--��B��--�٢��"
  },
  {
    "hug", "����", "�������֩�", ""
  },
  {
    "hypnoze", "�ʯv", "���۱����̧r�̪��A��", "�i�}�ʯv"
  },
  {
    "jab", "Ѷ�H", "�ΤO�aѶ��", "�A���G��L�ܬO����"
  },
  {
    "judo", "�X�D", "���F", "�����̡A�ਭ�K�K�ڡA�O�@�O�L�ӺL"
  },
  {
    "kick", "��H", "��", "��o�h���y��"
  },
  {
    "kill", "��H", "��", "�äM�妺���"
  },
  {
    "kiss", "���k", "���k", "���y�U"
  },
  {
    "laugh", "�J��", "�j�n�J��", ""
  },
  {
    "levis", "����", "���G����", "�I��l�K�͡I"
  },
  {
    "lick", "�Q", "�g�Q", ""
  },
  {
    "listen", "ť", "�s", "���L�J��ť"
  },
  {
    "lobster", "����", "�I�i�f���ΩT�w�A��", "����b�a�O�W"
  },
  {
    "love", "���", "��", "�`�������"
  },
  {
    "mail", "���]", "��", "���]���e��j��"
  },
  {
    "marry", "�D�B", "���ۤE�ʤE�Q�E�������V", "�D�B"
  },
  {
    "morning", "���w", "��", "���y���w�z"
  },
  {
    "noon", "�Ȧw", "��", "���y�Ȧw�z"
  },
  {
    "nod", "�I�Y", "�V", "�I�Y�٬O"
  },
  {
    "nudge", "���{�l", "�Τ�y��", "���Ψ{�l"
  },
  {
    "pad", "��ӻH", "����", "���ӻH"
  },
  {
    "pan", "������", "�q�I�᮳�X�F������A��", "�V���F"
  },
  {
    "pat", "���Y", "���", "���Y"
  },
  {
    "pettish", "���b", "��", "���n�ݮ�a���b"
  },
  {
    "pili", "�R�E", "�ϥX �g�l�� �Ѧa��	��Y�b �T���X�@���V", "���"
  },
  {
    "pinch", "���H", "�ΤO����", "���o�«C"
  },
  {
    "poke", "�W��", "�W�F�W", "�Q�n�ް_�L���`�N"
  },
  {
    "puding", "�饬�B",	"��", "��F�@�d�����B"
  },
  {
    "roll", "���u", "��X�h���O�����֡A", "�b�a�W�u�Ӻu�h"
  },
  {
    "protect", "�O�@", "�}���O�@��", ""
  },
  {
    "pull", "��", "���R�a�Ԧ�",	"����"
  },
  {
    "punch", "�~�H", "�����~�F", "�@�y"
  },
  {
    "rascal", "�A��", "��", "�A��"
  },
  {
    "recline", "�J�h", "�p��", "���h�̺εۤF�K�K"
  },
  {
    "recycle", "�^����", "��", "���귽�^����"
  },
  {
    "respond", "�t�d", "�w��", "���G�y���n���A�ڷ|�t�d���z"
  },
  {
    "scratch", "�i��", "�߰_", "���䪺�ۤl�i�i�ۤv���Q��"
  },
  {
    "sex", "�����Z", "��", "�����Z"
  },
  {
    "shit", "���S", "��", "�|�F�@�n�y���S�z"
  },
  {
    "shrug", "�q��", "�L�`�a�V", "�q�F�q�ӻH"
  },
  {
    "sigh", "�ۮ�", "��", "�ۤF�@�f��"
  },
  {
    "slap", "���ե�", "�԰Ԫ��ڤF", "�@�y�ե�"
  },
  {
    "smooch", "�֧k", "�֧k��",	""
  },
  {
    "snicker", "�ѯ�", "�K�K�K�a��", "�ѯ�"
  },
  {
    "sniff", "���h", "��", "�ᤧ�H��"
  },
  {
    "sorry", "�藍�_", "�V", "���藍�_�I�ڹ藍�_�j�a�A�ڹ藍�_��a���|"
  },
  {
    "spank", "������", "�Τڴx��", "���v��"
  },
  {
    "squeeze", "���", "���a�֩��", ""
  },
  {
    "thank", "�P��", "�V", "�P�±o�����a"
  },
  {
    "throw", "���Y", "���F�}�U�@���j���Y��", "����F�L�h"
  },
  {
    "tickle", "�k�o", "�B�T�B�T�A�k", "���o"
  },
  {
    "wait", "���@�U", "�s", "���@�U�@�I"
  },
  {
    "wake", "�n��", "�����a��",	"�n��"
  },
  {
    "wave", "����", "���", "������A��ܧi�O���N"
  },
  {
    "welcome", "�w��", "�w��", "�i�ӤK���@�U"
  },
  {
    "what", "����", "���G�y", "�����M�K�z��ť�Y?�H?�S?�z"
  },
  {
    "whip", "�@��", "��W��������A���@�l�h��",	""
  },
  {
    "wiggle", "�᧾��",	"���",	"�᧾��"
  },
  {
    "wink", "�w��", "��", "�������w�w����"
  },
  {
    "zap", "�r��", "��", "�ƨg������"
  },
  {
    NULL, NULL, NULL, NULL
  }
};


static int
party_action(cu, cmd, party)
  ChatUser *cu;
  char *cmd;
  char *party;
{
  ChatAction *cap;
  char buf[256];

  if ((cap = action_fit(party_data, ACTNUM_PARTY, cmd)))
  {
    if (*party == '\0')
    {
      party = "�j�a";
    }
    else
    {
      ChatUser *xuser;

      xuser = fuzzy_cuser_by_chatid(party);
      if (xuser == NULL)
      {			/* Thor.980724: �� userid�]���q */
	xuser = cuser_by_userid(party);
      }

      if (xuser == NULL)
      {
	sprintf(buf, msg_no_such_id, party);
	send_to_user(cu, buf, 0, MSG_MESSAGE);
	return 0;
      }
      else if (xuser == FUZZY_USER)
      {
	send_to_user(cu, "�� �Ы�����ѥN��", 0, MSG_MESSAGE);
	return 0;
      }
      else if (cu->room != xuser->room || CLOAK(xuser))
      {
	sprintf(buf, msg_not_here, party);
	send_to_user(cu, buf, 0, MSG_MESSAGE);
	return 0;
      }
      else
      {
	party = xuser->chatid;
      }
    }
    sprintf(buf, "\033[1;32m%s \033[31m%s\033[33m %s \033[31m%s\033[m",
      cu->chatid, cap->part1_msg, party, cap->part2_msg);
    send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
    return 0;			/* Thor: cu->room �O�_�� NULL? */
  }
  return 1;
}


/* --------------------------------------------- */
/* MUD-like social commands : speak              */
/* --------------------------------------------- */


/* itoc.010805.����:  //ask �j�a���ѹL�o�n�ܡH	 itoc �ݤj�a���ѹL�o�n�ܡH*/

#define ACTNUM_SPEAK	29

static ChatAction speak_data[ACTNUM_SPEAK] =
{
  {
    "ask", "�߰�", "��", NULL
  },
  {
    "broadcast", "�s��", "�s��", NULL
  },
  {
    "chant", "�q�|", "���n�q�|", NULL
  },
  {
    "cheer", "�ܪ�", "�ܪ�", NULL
  },
  {
    "chuckle", "����", "����", NULL
  },
  {
    "curse", "�A�G", "�t�F", NULL
  },
  {
    "demand", "�n�D", "�n�D", NULL
  },
  {
    "fuck", "���F", "���F", NULL
  },
  {
    "groan", "�D�u", "�D�u", NULL
  },
  {
    "grumble", "�o�c��", "�o�c��", NULL
  },
  {
    "guitar", "�u��", "��u�ۦN�L�A��۵�", NULL
  },
  {
    "hum", "���", "���ۻy", NULL
  },
  {
    "moan", "���", "���", NULL
  },
  {
    "notice", "�j��", "�j��", NULL
  },
  {
    "order", "�R�O", "�R�O", NULL
  },
  {
    "ponder", "�H��", "�H��", NULL
  },
  {
    "pout", "���L", "���ۼL��",	NULL
  },
  {
    "pray", "��ë", "��ë", NULL
  },
  {
    "request", "���D", "���D", NULL
  },
  {
    "shout", "�j�|", "�j�|", NULL
  },
  {
    "sing", "�ۺq", "�ۺq", NULL
  },
  {
    "smile", "�L��", "�L��", NULL
  },
  {
    "smirk", "����", "����", NULL
  },
  {
    "swear", "�o�}", "�o�}", NULL
  },
  {
    "tease", "�J��", "�J��", NULL
  },
  {
    "whimper", "��|", "��|����", NULL
  },
  {
    "yawn", "����", "�䥴�����仡", NULL
  },
  {
    "yell", "�j��", "�j��", NULL
  },
  {
    NULL, NULL, NULL, NULL
  }
};


static int
speak_action(cu, cmd, msg)
  ChatUser *cu;
  char *cmd;
  char *msg;
{
  ChatAction *cap;
  char buf[256];

  if ((cap = action_fit(speak_data, ACTNUM_SPEAK, cmd)))
  {
    sprintf(buf, "\033[1;32m%s \033[31m%s�G\033[33m %s\033[m",
      cu->chatid, cap->part1_msg, msg);
    send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
    return 0;
  }
  return 1;
}


/* ----------------------------------------------------- */
/* MUD-like social commands : condition			 */
/* ----------------------------------------------------- */


/* itoc.010805.����:  //agree	itoc �`��P�N */

#define ACTNUM_CONDITION	73

static ChatAction condition_data[ACTNUM_CONDITION] =
{
  {
    "agree", "�P�N", "�`��P�N", NULL
  },
  {
    "aha", "�F��", "�W��}�[�A���M�F���@�{�A���T�r�����@�n", NULL
  },
  {
    "akimbo", "���y", "�S��S�L�`����ⴡ�y", NULL
  },
  {
    "alas", "�u�r", "�u�r�r��", NULL
  },
  {
    "applaud", "���", "�԰԰԰԰ԡK�K�԰�", NULL
  },
  {
    "avert", "�`��", "�`�ۦa��}���u", NULL
  },
  {
    "ayo", "�����", "����ޡ�", NULL
  },
  {
    "back", "���^��", "�^�ӧ����~��ľ�", NULL
  },
  {
    "blood", "�b�夤", "�˦b��y����", NULL
  },
  {
    "blush", "�y��", "�y�����F", NULL
  },
  {
    "broke", "�߸H", "���߯}�H���@���@����", NULL
  },
  {
    "bug", "����", "�o�{�o�t�Φ��Т����", NULL
  },
  {
    "careles", "�S�H�z", "���㳣�S���H�z�� �G��", NULL
  },
  {
    "chew", "�ߥʤl", "�ܱy�����߰_�ʤl�ӤF", NULL
  },
  {
    "climb", "���s", "�ۤv�C�C���W�s�ӡK�K", NULL
  },
  {
    "cold", "�P�_", "�P�_�F�A���������ڥX�h�� �G�]", NULL
  },
  {
    "cough", "�y��", "�y�F�X�n", NULL
  },
  {
    "crash", "���", "��K" BBSNAME "����F", NULL
  },
  {
    "die", "����", "�������", NULL
  },
  {
    "dive", "���", "������̸��_��", NULL
  },
  {
    "faint", "����", "�������", NULL
  },
  {
    "fart", "��", "���O�b�񧾡A�J��@�q�I", NULL
  },
  {
    "flop", "������", "��쭻���֡K�ƭˡI", NULL
  },
  {
    "fly", "���ƵM", "���ƵM�a�A�n�����F�_��", NULL
  },
  {
    "frown", "�٬�", "�٬ܡA�������F����", NULL
  },
  {
    "gold", "�����P", "�۵ۡG�y���|�������|�����X����ɡA�o�a�x�A�����P�A���a�˾H�ӡI�z", NULL
  },
  {
    "gulu", "�{�l�j", "���{�l�o�X�B�P�B�P�㪺�n��", NULL
  },
  {
    "haha", "����", "�z�������K�j���F�_��", NULL
  },
  {
    "happy", "����", "�����o�b�a�W���u", NULL
  },
  {
    "hiccup", "����", "���ЭӤ���", NULL
  },
  {
    "hoho", "����", "���������Ӥ���", NULL
  },
  {
    "hypnzed", "�Q�ʯv", "�����b���A�Q�ʯv�F�K�K�C��Czzz", NULL
  },
  {
    "idle", "�b��", "�����b��F", NULL
  },
  {
    "jacky", "�l�l", "�l�l�몺�̨Ӯ̥h", NULL
  },
  {
    "jealous", "�Y�L", "�𹪹��a�ܤF�@���L", NULL
  },
  {
    "jump", "����", "���Ӧ۱�",	NULL
  },
  {
    "luck", "���B", "�z�I�֮�աI", NULL
  },
  {
    "macarn", "�@�ػR",	"�}�l���_�F�ۢ�Ѣ���ܢ�����", NULL
  },
  {
    "miou", "�p�p", "�p�p�f�]�f�]������", NULL
  },
  {
    "money", "�ȿ�", "�I����s����Ȥj��", NULL
  },
  {
    "mouth", "��L", "��L���I", NULL
  },
  {
    "mutter", "�C�B", "�C�n�B���۬Y�ǨơC", NULL
  },
  {
    "nani", "���|", "�G�`���ڮ�??", NULL
  },
  {
    "nose", "�y���", "�y���",	NULL
  },
  {
    "puke", "�æR", "�æR��", NULL
  },
  {
    "rest", "��", "�𮧤��A�Фť��Z",	NULL
  },
  {
    "reverse", "½�{", "½�{", NULL
  },
  {
    "room", "�}�ж�", "r-o-O-m-r-O-��-Mmm-rR��........", NULL
  },
  {
    "scream", "�y�s", "�j�n�y�s�I ��~~~~~~~", NULL
  },
  {
    "shake", "�n�Y", "�n�F�n�Y", NULL
  },
  {
    "sleep", "�ε�", "�w�b��L�W�εۤF�A�f���y�i��L�A�y������I", NULL
  },
  {
    "snore", "���M��", "���M���K", NULL
  },
  {
    "sob", "��F", "����� �ݢ� �Т�����I�I", NULL
  },
  {
    "stare", "����", "�R�R�a�����ۤѪ�", NULL
  },
  {
    "stretch", "�h��", "�����i�y�S���F�Ө���ܯh�¦����C", NULL
  },
  {
    "story", "���j", "�}�l���j�F", NULL
  },
  {
    "strut", "�n�\\��",	"�j�n�j�\\�a��", NULL
  },
  {
    "suicide", "�۱�", "�۱�", NULL
  },
  {
    "sweat", "�y��", "�����p�B�I", NULL
  },
  {
    "tear", "�y�\\", "�h���y����.....",	NULL
  },
  {
    "think", "���", "�n���Y�Q�F�@�U", NULL
  },
  {
    "tongue", "�R��", "�R�F�R���Y", NULL
  },
  {
    "wall", "����", "�]�h����",	NULL
  },
  {
    "wawa", "�z�z", "�z�z�z~~~~~!!!!!  ~~~>_<~~~", NULL
  },
  {
    "wc", "�~�ⶡ", "���~�ⶡ�@�U :>", NULL
  },
  {
    "whine", "�{�l�j", "�{�l�j!	:(", NULL
  },
  {
    "whistle", "�j�f��", "�j�f��", NULL
  },
  {
    "wolf", "�T�z", "���������K���������K", NULL
  },
  {
    "www", "�L�L", "�L�L�L�I", NULL
  },
  {
    "ya", "���C", "�����ϡI *^_^*", NULL
  },
  {
    "zzz", "���I", "�I�P��ZZzZz�C��ZZzzZzzzZZ", NULL
  },
  {
    NULL, NULL, NULL, NULL
  }
};


static int
condition_action(cu, cmd)
  ChatUser *cu;
  char *cmd;
{
  ChatAction *cap;
  char buf[256];

  if ((cap = action_fit(condition_data, ACTNUM_CONDITION, cmd)))
  {
    sprintf(buf, "\033[1;32m%s \033[31m%s\033[m",
      cu->chatid, cap->part1_msg);
    send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
    return 1;
  }
  return 0;
}


/* --------------------------------------------- */
/* MUD-like social commands : help               */
/* --------------------------------------------- */


static char *dscrb[] =
{
  "\033[1;37m�i Verb + Nick�G   �ʵ� + ���W�r �j\033[36m  �ҡG//kick piggy\033[m",
  "\033[1;37m�i Verb + Message�G�ʵ� + �n������ �j\033[36m  �ҡG//sing �ѤѤ���\033[m",
  "\033[1;37m�i Verb�G�ʵ� �j   �����G�¸ܭ���\033[m", NULL
};



static ChatAction *catbl[] =
{
  party_data, speak_data, condition_data, NULL
};


static void
chat_partyinfo(cu, msg)
  ChatUser *cu;
  char *msg;
{
  if (common_client_command)
  {
    send_to_user(cu, "3 �ʧ@  ���  ���A", 0, MSG_PARTYINFO);
  }
}


static void
chat_party(cu, msg)
  ChatUser *cu;
  char *msg;
{
  int kind, i;
  ChatAction *cap;
  char buf[80];

  if (!common_client_command)
    return;

  kind = atoi(nextword(&msg));
  if (kind < 0 || kind > 2)
    return;

  sprintf(buf, "%d\t%s", kind, kind == 2 ? "I" : "");

  /* Xshadow: �u�� condition �~�O immediate mode */
  send_to_user(cu, buf, 0, MSG_PARTYLISTSTART);

  cap = catbl[kind];
  for (i = 0; cap[i].verb; i++)
  {
    sprintf(buf, "%-10s %-20s", cap[i].verb, cap[i].chinese);
    send_to_user(cu, buf, 0, MSG_PARTYLIST);
  }

  sprintf(buf, "%d", kind);
  send_to_user(cu, buf, 0, MSG_PARTYLISTEND);
}


#define	MAX_VERB_LEN	8
#define VERB_NO		10


static void
view_action_verb(cu, cmd)	/* Thor.980726: �s�[�ʵ�������� */
  ChatUser *cu;
  int cmd;
{
  int i;
  char *p, *q, *data, *expn, buf[256];
  ChatAction *cap;

  send_to_user(cu, "/c", 0, MSG_CLRSCR);

  data = buf;

  if (cmd < '1' || cmd > '3')
  {				/* Thor.980726: �g�o���n, �Q��k��i... */
    for (i = 0; p = dscrb[i]; i++)
    {
      sprintf(data, "  [//]help %d          - MUD-like ����ʵ�   �� %d ��", i + 1, i + 1);
      send_to_user(cu, data, 0, MSG_MESSAGE);
      send_to_user(cu, p, 0, MSG_MESSAGE);
      send_to_user(cu, " ", 0, MSG_MESSAGE);	/* Thor.980726: ���� */
    }
  }
  else
  {
    i = cmd - '1';

    send_to_user(cu, dscrb[i], 0, MSG_MESSAGE);

    expn = buf + 100;		/* Thor.980726: ���Ӥ��|overlap�a? */

    *data = '\0';
    *expn = '\0';

    cap = catbl[i];

    for (i = 0; p = cap[i].verb; i++)
    {
      q = cap[i].chinese;

      strcat(data, p);
      strcat(expn, q);

      if (((i + 1) % VERB_NO) == 0)
      {
	send_to_user(cu, data, 0, MSG_MESSAGE);
	send_to_user(cu, expn, 0, MSG_MESSAGE);	/* Thor.980726: ��ܤ������ */
	*data = '\0';
	*expn = '\0';
      }
      else
      {
	strncat(data, "        ", MAX_VERB_LEN - strlen(p));
	strncat(expn, "        ", MAX_VERB_LEN - strlen(q));
      }
    }

    if (i % VERB_NO)
    {
      send_to_user(cu, data, 0, MSG_MESSAGE);
      send_to_user(cu, expn, 0, MSG_MESSAGE);	/* Thor.980726: ��ܤ������ */
    }
  }
  /* send_to_user(cu, " ", 0); *//* Thor.980726: ����, �ݭn " " ��? */
}


/* ----------------------------------------------------- */
/* chat user service routines                            */
/* ----------------------------------------------------- */


static ChatCmd chatcmdlist[] =
{
  "act", chat_act, 0,
  "bye", chat_bye, 0,
  "chatroom", chat_chatroom, 1,		/* Xshadow: for common client */
  "clear", chat_clear, 0,
  "cloak", chat_cloak, 2,
  "date", chat_date, 0,
  "flags", chat_setroom, 0,
  "help", chat_help, 0,
  "ignore", chat_ignore, 1,
  "invite", chat_invite, 0,
  "join", chat_join, 0,
  "kick", chat_kick, 1,
  "msg", chat_private, 0,
  "nick", chat_nick, 0,
  "operator", chat_makeop, 0,
  "party", chat_party, 1,		/* Xshadow: party data for common client */
  "partyinfo", chat_partyinfo, 1,	/* Xshadow: party info for common client */

#ifndef STAND_ALONE
  "query", chat_query, 0,
#endif

  "quit", chat_bye, 0,

  "room", chat_list_rooms, 0,
  "unignore", chat_unignore, 1,
  "whoin", chat_list_by_room, 1,
  "wall", chat_broadcast, 2,

  "who", chat_map_chatids_thisroom, 0,
  "list", chat_list_users, 0,
  "topic", chat_topic, 1,
  "version", chat_version, 1,

  NULL, NULL, 0
};


/* Thor: 0 ���� exact, 1 �n exactly equal, 2 ���K���O */


static int
command_execute(cu)
  ChatUser *cu;
{
  char *cmd, *msg, buf[128];
  /* Thor.981108: lkchu patch: chatid + msg �u�� 80 bytes ����, �אּ 128 */
  ChatCmd *cmdrec;
  int match, ch;

  msg = cu->ibuf;
  match = *msg;

  /* Validation routine */

  if (cu->room == NULL)
  {
    /* MUST give special /! or /-! command if not in the room yet */

    if (match == '/' && ((ch = msg[1]) == '!' || (ch == '-' && msg[2] == '!')))
    {
      if (ch == '-')
	fprintf(flog, "cli\t[%d] S%d\n", cu->sno, cu->sock);

      cu->clitype = (ch == '-') ? 1 : 0;
      return (login_user(cu, msg + 2 + cu->clitype));
    }
    else
    {
      return -1;
    }
  }

  /* If not a /-command, it goes to the room. */

  if (match != '/')
  {
    if (match)
    {
      if (cu->room && !CLOAK(cu))	/* �������H�]���໡�ܮ@ */
      {
	char chatid[16];

	sprintf(chatid, "%s:", cu->chatid);
	sprintf(buf, "%-10s%s", chatid, msg);
	send_to_room(cu->room, buf, cu->userno, MSG_MESSAGE);
      }
    }
    return 0;
  }

  msg++;
  cmd = nextword(&msg);
  match = 0;

  if (*cmd == '/')
  {
    cmd++;
    /* if (!*cmd || !str_cmp("help", cmd)) */
    if (!*cmd || str_match(cmd, "help") >= 0)	/* itoc.010321: ���� match �N�� */
    {
      cmd = nextword(&msg);	/* Thor.980726: �ʵ����� */
      view_action_verb(cu, *cmd);
      match = 1;
    }
    else if (!party_action(cu, cmd, msg))
      match = 1;
    else if (!speak_action(cu, cmd, msg))
      match = 1;
    else
      match = condition_action(cu, cmd);
  }
  else
  {
    char *str;

    common_client_command = 0;
    if (*cmd == '-')
    {
      if (cu->clitype)
      {
	cmd++;			/* Xshadow: ���O�q�U�@�Ӧr���~�}�l */
	common_client_command = 1;
      }
      else
      {
	/* ���O common client ���e�X common client ���O -> ���˨S�ݨ� */
      }
    }

    str_lower(buf, cmd);

    for (cmdrec = chatcmdlist; str = cmdrec->cmdstr; cmdrec++)
    {
      switch (cmdrec->exact)
      {
      case 1:			/* exactly equal */
	match = !str_cmp(str, buf);
	break;

      case 2:			/* Thor: secret command */
	if (CHATSYSOP(cu))
	  match = !str_cmp(str, buf);
	break;

      default:			/* not necessary equal */
	match = str_match(buf, str) >= 0;
	break;
      }

      if (match)
      {
	cmdrec->cmdfunc(cu, msg);
	break;
      }
    }
  }

  if (!match)
  {
    sprintf(buf, "�� ���O���~�G/%s", cmd);
    send_to_user(cu, buf, 0, MSG_MESSAGE);
  }

  return 0;
}


/* ----------------------------------------------------- */
/* serve chat_user's connection                          */
/* ----------------------------------------------------- */


static int
cuser_serve(cu)
  ChatUser *cu;
{
  int ch, len, isize;
  char *str, *cmd, buf[256];

  str = buf;
  len = recv(cu->sock, str, sizeof(buf) - 1, 0);
  if (len < 0)
  {
    ch = errno;

    exit_room(cu, EXIT_LOSTCONN, NULL);
    logit("recv", strerror(ch));
    return -1;
  }

  if (len == 0)
  {
    if (++cu->retry > 100)
      return -1;
    return 0;
  }

#if 0
  /* Xshadow: �N�e�F����Ʃ�������U�� */
  memcpy(logbuf, buf, sizeof(buf));
  for (ch = 0; ch < sizeof(buf); ch++)
  {
    if (!logbuf[ch])
      logbuf[ch] = '$';
  }

  logbuf[len + 1] = '\0';
  logit("recv: ", logbuf);
#endif

#if 0
  logit(cu->userid, str);
#endif

  cu->xdata += len;

  isize = cu->isize;
  cmd = cu->ibuf + isize;
  while (len--)
  {
    ch = *str++;

    if (ch == '\r' || !ch)
      continue;

    if (ch == '\n')
    {
      *cmd = '\0';

      if (command_execute(cu) < 0)
	return -1;

      isize = 0;
      cmd = cu->ibuf;

      continue;
    }

    if (isize < SCR_WIDTH)
    {
      *cmd++ = ch;
      isize++;
    }
  }
  cu->isize = isize;
  return 1;
}


/* ----------------------------------------------------- */
/* chatroom server core routines                         */
/* ----------------------------------------------------- */


static int
/* start_daemon(mode) 
  int mode; */
servo_daemon(inetd)
  int inetd;
{
  int fd, value;
  char buf[80];
  struct sockaddr_in sin;
  struct linger ld;
#ifdef HAVE_RLIMIT
  struct rlimit limit;
#endif

  /*
   * More idiot speed-hacking --- the first time conversion makes the C
   * library open the files containing the locale definition and time zone.
   * If this hasn't happened in the parent process, it happens in the
   * children, once per connection --- and it does add up.
   */

  time((time_t *) &value);
  gmtime((time_t *) &value);
  strftime(buf, 80, "%d/%b/%Y:%H:%M:%S", localtime((time_t *) &value));

  /* --------------------------------------------------- */
  /* speed-hacking DNS resolve                           */
  /* --------------------------------------------------- */

  dns_init();
#if 0
  gethostname(buf, sizeof(buf));
  gethostbyname(buf);
#endif

#ifdef HAVE_RLIMIT
  /* --------------------------------------------------- */
  /* adjust the resource limit                           */
  /* --------------------------------------------------- */

  getrlimit(RLIMIT_NOFILE, &limit);
  limit.rlim_cur = limit.rlim_max;
  setrlimit(RLIMIT_NOFILE, &limit);

  limit.rlim_cur = limit.rlim_max = 4 * 1024 * 1024;
  setrlimit(RLIMIT_DATA, &limit);

#ifdef SOLARIS
#define RLIMIT_RSS RLIMIT_AS	/* Thor.981206: port for solaris 2.6 */
#endif  

  setrlimit(RLIMIT_RSS, &limit);

  limit.rlim_cur = limit.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &limit);

#if 0
  limit.rlim_cur = limit.rlim_max = 60 * 20;
  setrlimit(RLIMIT_CPU, &limit);
#endif
#endif

  /* --------------------------------------------------- */
  /* detach daemon process                               */
  /* --------------------------------------------------- */

  close(2);
  close(1);

  /* if (mode > 1) */
  if (inetd)
    return 0;

  close(0);

  if (fork())
    exit(0);

  setsid();

  if (fork())
    exit(0);

  /* --------------------------------------------------- */
  /* bind the service port				 */
  /* --------------------------------------------------- */

  fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  /*
   * timeout �譱, �N socket �令 O_NDELAY (no delay, non-blocking),
   * �p�G�බ�Q�e�X��ƴN�e�X, ����e�X�N��F, ���A���� TCP_TIMEOUT �ɶ��C
   * (default �O 120 ��, �åB�� 3-way handshaking ����, ���i��@���A��)�C
   */

#if 1
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NDELAY);
#endif

  value = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &value, sizeof(value));

  value = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *) &value, sizeof(value));

  ld.l_onoff = ld.l_linger = 0;
  setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

  sin.sin_family = AF_INET;
  sin.sin_port = htons(CHAT_PORT);
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

  if ((bind(fd, (struct sockaddr *) & sin, sizeof(sin)) < 0) ||
    (listen(fd, SOCK_QLEN) < 0))
    exit(1);

  return fd;
}


#ifdef	SERVER_USAGE
static void
server_usage()
{
  struct rusage ru;

  if (getrusage(RUSAGE_SELF, &ru))
    return;

  fprintf(flog, "\n[Server Usage]\n\n"
    "user time: %.6f\n"
    "system time: %.6f\n"
    "maximum resident set size: %lu P\n"
    "integral resident set size: %lu\n"
    "page faults not requiring physical I/O: %d\n"
    "page faults requiring physical I/O: %d\n"
    "swaps: %d\n"
    "block input operations: %d\n"
    "block output operations: %d\n"
    "messages sent: %d\n"
    "messages received: %d\n"
    "signals received: %d\n"
    "voluntary context switches: %d\n"
    "involuntary context switches: %d\n"
    "gline: %d\n\n",

    (double) ru.ru_utime.tv_sec + (double) ru.ru_utime.tv_usec / 1000000.0,
    (double) ru.ru_stime.tv_sec + (double) ru.ru_stime.tv_usec / 1000000.0,
    ru.ru_maxrss,
    ru.ru_idrss,
    ru.ru_minflt,
    ru.ru_majflt,
    ru.ru_nswap,
    ru.ru_inblock,
    ru.ru_oublock,
    ru.ru_msgsnd,
    ru.ru_msgrcv,
    ru.ru_nsignals,
    ru.ru_nvcsw,
    ru.ru_nivcsw,
    gline);

  fflush(flog);
}
#endif


static void
reaper()
{
  while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0)
    ;
}


static void
sig_trap(sig)
  int sig;
{
  char buf[80];

  sprintf(buf, "signal [%d] at line %d (errno: %d)", sig, gline, errno);
  logit("EXIT", buf);
  fclose(flog);
  exit(1);
}


static void
sig_over()
{
  int fd;

  server_usage();
  logit("OVER", "");
  fclose(flog);
  for (fd = 0; fd < 64; fd++)
    close(fd);
  execl("bin/xchatd", NULL);
}


static void
main_signals()
{
  struct sigaction act;

  /* sigblock(sigmask(SIGPIPE)); */
  /* Thor.981206: �Τ@ POSIX �зǥΪk  */

  /* act.sa_mask = 0; */ /* Thor.981105: �зǥΪk */
  sigemptyset(&act.sa_mask);      
  act.sa_flags = 0;

  act.sa_handler = sig_trap;
  sigaction(SIGBUS, &act, NULL);
  sigaction(SIGSEGV, &act, NULL);
  sigaction(SIGTERM, &act, NULL);

  act.sa_handler = sig_over;
  sigaction(SIGXCPU, &act, NULL);

  act.sa_handler = reaper;
  sigaction(SIGCHLD, &act, NULL);

#ifdef  SERVER_USAGE
  act.sa_handler = server_usage;
  sigaction(SIGPROF, &act, NULL);
#endif

  /* Thor.981206: lkchu patch: �Τ@ POSIX �зǥΪk  */
  /* �b���ɥ� sigset_t act.sa_mask */
  sigaddset(&act.sa_mask, SIGPIPE);
  sigprocmask(SIG_BLOCK, &act.sa_mask, NULL);

}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int sock, nfds, maxfds, servo_sno;
  ChatUser *cu,/* *userpool,*/ **FBI;
  time_t uptime, tcheck;
  fd_set rset, xset;
  static struct timeval tv = {CHAT_INTERVAL, 0};
  struct timeval tv_tmp; /* Thor.981206: for future reservation bug */

  sock = 0;

  while ((nfds = getopt(argc, argv, "hid")) != -1)
  {
    switch (nfds)
    {
    case 'i':
      sock = 1;
      break;

    case 'd':
      break;

    case 'h':
    default:

      fprintf(stderr, "Usage: %s [options]\n"
        "\t-i  start from inetd with wait option\n"
        "\t-d  debug mode\n"
        "\t-h  help\n",
        argv[0]);
      exit(0);
    }       
  }

  servo_daemon(sock); 
  /* start_daemon(argc); */

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);
  umask(077);

  log_init();

  main_signals();

  /* --------------------------------------------------- */
  /* init variable : rooms & users			 */
  /* --------------------------------------------------- */

  userpool = NULL;
  strcpy(mainroom.name, MAIN_NAME);
  strcpy(mainroom.topic, MAIN_TOPIC);

  /* --------------------------------------------------- */
  /* main loop						 */
  /* --------------------------------------------------- */

  tcheck = 0;
  servo_sno = 0;

  for (;;)
  {
    uptime = time(0);
    if (tcheck < uptime)
    {
      nfds = maxfds = 0;
      FD_ZERO(&mainfset);
      FD_SET(0, &mainfset);

      tcheck = uptime - CHAT_INTERVAL;

      for (FBI = &mainuser; cu = *FBI;)
      {
	if (cu->uptime < tcheck)
	{
	  cuser_free(cu);

	  *FBI = cu->unext;

	  cu->unext = userpool;
	  userpool = cu;
	}
	else
	{
	  nfds++;
	  sock = cu->sock;
	  FD_SET(sock, &mainfset);
	  if (maxfds < sock)
	    maxfds = sock;

	  FBI = &(cu->unext);
	}
      }

      totaluser = nfds;
      fprintf(flog, "MAINTAIN %d user (%d)\n", nfds, maxfds++);
      fflush(flog);

      tcheck = uptime + CHAT_INTERVAL;
    }

    /* ------------------------------------------------- */
    /* Set up the fdsets				 */
    /* ------------------------------------------------- */

    rset = mainfset;
    xset = mainfset;

    /* Thor.981206: for future reservation bug */   
    tv_tmp = tv;
    nfds = select(maxfds, &rset, NULL, &xset, &tv_tmp);

#if 0
    {
      char buf[32];
      static int xxx;

      if ((++xxx & 8191) == 0)
      {
	sprintf(buf, "%d/%d", nfds, maxfds);
	logit("MAIN", buf);
      }
    }
#endif

    if (nfds == 0)
    {
      continue;
    }

    if (nfds < 0)
    {
      sock = errno;
      if (sock != EINTR)
      {
	logit("select", strerror(sock));
      }
      continue;
    }

    /* ------------------------------------------------- */
    /* serve active agents				 */
    /* ------------------------------------------------- */

    uptime = time(0);

    for (FBI = &mainuser; cu = *FBI;)
    {
      sock = cu->sock;

      if (FD_ISSET(sock, &rset))
      {
	static int xxx, xno;

	nfds = cuser_serve(cu);

	if ((++xxx & 511) == 0)
	{
	  int sno;

	  sno = cu->sno;
	  fprintf(flog, "rset\t[%d] S%d R%d %d\n", sno, sock, nfds, xxx);
	  if (sno == xno)
	    nfds = -1;
	  else
	    xno = sno;
	}
      }
      else if (FD_ISSET(sock, &xset))
      {
	nfds = -1;
      }
      else
      {
	nfds = 0;
      }

      if (nfds < 0 || cu->uptime <= 0)	/* free this client */
      {
	cuser_free(cu);

	*FBI = cu->unext;

	cu->unext = userpool;
	userpool = cu;

	continue;
      }

      if (nfds > 0)
      {
	cu->uptime = uptime;
      }

      FBI = &(cu->unext);
    }

    /* ------------------------------------------------- */
    /* accept new connection				 */
    /* ------------------------------------------------- */

    if (FD_ISSET(0, &rset))
    {

      {
	static int yyy;

	if ((++yyy & 2047) == 0)
	  fprintf(flog, "conn\t%d\n", yyy);
      }

      for (;;)
      {
	int value;
	struct sockaddr_in sin;

	value = sizeof(sin);
	sock = accept(0, (struct sockaddr *) &sin, &value);
	if (sock > 0)
	{
	  if (cu = userpool)
	  {
	    userpool = cu->unext;
	  }
	  else
	  {
	    cu = (ChatUser *) malloc(sizeof(ChatUser));
	  }

	  *FBI = cu;

	  /* variable initialization */

	  memset(cu, 0, sizeof(ChatUser));
	  cu->sock = sock;
	  cu->tbegin = uptime;
	  cu->uptime = uptime;
	  cu->sno = ++servo_sno;
	  cu->xdata = 0;
	  cu->retry = 0;
	  memcpy(cu->rhost, &sin.sin_addr, sizeof(struct in_addr));

	  totaluser++;

	  FD_SET(sock, &mainfset);
	  if (sock >= maxfds)
	    maxfds = sock + 1;

	  {
	    int value;

	    value = 1;
	    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
	      (char *) &value, sizeof(value));
	  }

#if 1
	  fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NDELAY);
#endif

	  fprintf(flog, "CONN\t[%d] %d %s\n",
	    servo_sno, sock, Btime(&cu->tbegin));
	  break;
	}

	nfds = errno;
	if (nfds != EINTR)
	{
	  logit("accept", strerror(nfds));
	  break;
	}

#if 0
	while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
#endif
      }
    }

    /* ------------------------------------------------- */
    /* tail of main loop				 */
    /* ------------------------------------------------- */

  }
}
