#!/bin/sh
# �ϥΦ� script �e�A�Х��T�{�w�g�ק�F�U���������]�w

# �Цb�o��ק令�ۤv���]�w
# �H�U�]�w�䤤�Ҥ��঳�ťթM [] ���Ÿ��A�p "TNFSH [Wolf] BBS" �o�ˬO���X�檺

schoolname="�x�n�@��"
bbsname="�P�n�@�R"
bbsname2="TNFSH.Wolf.BBS"
sysopnick="�T�H����"
tag_valid="WolfBBS"
myipaddr="210.70.137.5"
myhostname="bbs.tnfsh.tn.edu.tw"
msg_bmw="���y"

# �Эק�z���@�~�t��
# sun linux solaris sol-x86 freebsd bsd

ostype="freebsd"

echo "�z�ҳ]�w�� SCHOOLNAME �O $schoolname"
echo "�z�ҳ]�w�� BBSNAME    �O $bbsname"
echo "�z�ҳ]�w�� BBSNAME2   �O $bbsname2"
echo "�z�ҳ]�w�� SYSOPNICK  �O $sysopnick"
echo "�z�ҳ]�w�� TAG_VALID  �O $tag_valid"
echo "�z�ҳ]�w�� MYIPADDR   �O $myipaddr"
echo "�z�ҳ]�w�� MYHOSTNAME �O $myhostname"
echo "�z�ҳ]�w�� MSG_BMW    �O $msg_bmw"
echo "�Y�z���h�� FQDN (Eg: twbbs)�A�h�ٻݤ�ʭק� src/include/config.h �� HOST_ALIASES"

echo "�z���@�~�t��          �O $ostype"

echo "�z�i�H�b http://home.pchome.com.tw/soho/itoc/ ���̷s���{���Φw�ˤ��"

# �^ BBSHOME
cd
echo "[1;36m�i���ഫ�� [0;5m...[m"


# ���W�� ip addr

filelist_1="etc/valid etc/tip src/include/config.h";

for i in $filelist_1
do
  cat $i | sed 's/�x�n�@��/'$schoolname'/g' > $i.sed;
  mv -f $i.sed $i

  cat $i | sed 's/�P�n�@�R/'$bbsname'/g' > $i.sed;
  mv -f $i.sed $i

  cat $i | sed 's/TNFSH.Wolf.BBS/'$bbsname2'/g' > $i.sed;
  mv -f $i.sed $i

  cat $i | sed 's/�T�H����/'$sysopnick'/g' > $i.sed;
  mv -f $i.sed $i

  cat $i | sed 's/WolfBBS/'$tag_valid'/g' > $i.sed;
  mv -f $i.sed $i

  cat $i | sed 's/210.70.137.5/'$myipaddr'/g' > $i.sed;
  mv -f $i.sed $i

  cat $i | sed 's/bbs.tnfsh.tn.edu.tw/'$myhostname'/g' > $i.sed; 
  mv -f $i.sed $i
done


# �� ���y

filelist_2="etc/tip \
src/include/config.h src/include/global.h src/include/modes.h \
src/include/struct.h src/include/theme.h src/include/ufo.h src/maple/CHANGE \
src/maple/acct.c src/maple/bbsd.c src/maple/bmw.c src/maple/menu.c \
src/maple/pal.c src/maple/post.c src/maple/talk.c src/maple/visio.c \
src/maple/xover.c";

for i in $filelist_2
do
  cat $i | sed 's/���y/'$msg_bmw'/g' > $i.sed;
  mv -f $i.sed $i
done


# �w�� Maple 3.10
echo "[1;36m�w�� BBS �� [0;5m...[m"
cd src
make clean $ostype install

# �Ұ�
# ���]�}�b port 9987
cd
bin/bbsd 9987
bin/camera
bin/account

# telnet ����
telnet 0 9987
