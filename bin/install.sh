#!/bin/sh
# ¨Ï¥Î¦¹ script «e¡A½Ð¥ý½T»{¤w¸g­×§ï¤F¤U­±ªº¬ÛÃö³]©w

# ½Ð¦b³oÃä­×§ï¦¨¦Û¤vªº³]©w
# ¥H¤U³]©w¨ä¤¤¬Ò¤£¯à¦³ªÅ¥Õ©M [] µ¥²Å¸¹¡A¦p "TNFSH [Wolf] BBS" ³o¼Ë¬O¤£¦X®æªº

schoolname="¥x«n¤@¤¤"
bbsname="»P«n¦@»R"
bbsname2="TNFSH.Wolf.BBS"
sysopnick="¯T¤Hªø¦Ñ"
tag_valid="WolfBBS"
myipaddr="210.70.137.5"
myhostname="bbs.tnfsh.tn.edu.tw"
msg_bmw="¤ô²y"

# ½Ð­×§ï±zªº§@·~¨t²Î
# sun linux solaris sol-x86 freebsd bsd

ostype="freebsd"

echo "±z©Ò³]©wªº SCHOOLNAME ¬O $schoolname"
echo "±z©Ò³]©wªº BBSNAME    ¬O $bbsname"
echo "±z©Ò³]©wªº BBSNAME2   ¬O $bbsname2"
echo "±z©Ò³]©wªº SYSOPNICK  ¬O $sysopnick"
echo "±z©Ò³]©wªº TAG_VALID  ¬O $tag_valid"
echo "±z©Ò³]©wªº MYIPADDR   ¬O $myipaddr"
echo "±z©Ò³]©wªº MYHOSTNAME ¬O $myhostname"
echo "±z©Ò³]©wªº MSG_BMW    ¬O $msg_bmw"
echo "­Y±z¦³¦h­Ó FQDN (Eg: twbbs)¡A«hÁÙ»Ý¤â°Ê­×§ï src/include/config.h ªº HOST_ALIASES"

echo "±zªº§@·~¨t²Î          ¬O $ostype"

echo "±z¥i¥H¦b http://home.pchome.com.tw/soho/itoc/ §ä¨ì³Ì·sªºµ{¦¡¤Î¦w¸Ë¤å¥ó"

# ¦^ BBSHOME
cd
echo "[1;36m¶i¦æÂà´«¤¤ [0;5m...[m"


# ´«¦WºÙ ip addr

filelist_1="etc/valid etc/tip src/include/config.h";

for i in $filelist_1
do
  cat $i | sed 's/¥x«n¤@¤¤/'$schoolname'/g' > $i.sed;
  mv -f $i.sed $i

  cat $i | sed 's/»P«n¦@»R/'$bbsname'/g' > $i.sed;
  mv -f $i.sed $i

  cat $i | sed 's/TNFSH.Wolf.BBS/'$bbsname2'/g' > $i.sed;
  mv -f $i.sed $i

  cat $i | sed 's/¯T¤Hªø¦Ñ/'$sysopnick'/g' > $i.sed;
  mv -f $i.sed $i

  cat $i | sed 's/WolfBBS/'$tag_valid'/g' > $i.sed;
  mv -f $i.sed $i

  cat $i | sed 's/210.70.137.5/'$myipaddr'/g' > $i.sed;
  mv -f $i.sed $i

  cat $i | sed 's/bbs.tnfsh.tn.edu.tw/'$myhostname'/g' > $i.sed; 
  mv -f $i.sed $i
done


# ´« ¤ô²y

filelist_2="etc/tip \
src/include/config.h src/include/global.h src/include/modes.h \
src/include/struct.h src/include/theme.h src/include/ufo.h src/maple/CHANGE \
src/maple/acct.c src/maple/bbsd.c src/maple/bmw.c src/maple/menu.c \
src/maple/pal.c src/maple/post.c src/maple/talk.c src/maple/visio.c \
src/maple/xover.c";

for i in $filelist_2
do
  cat $i | sed 's/¤ô²y/'$msg_bmw'/g' > $i.sed;
  mv -f $i.sed $i
done


# ¦w¸Ë Maple 3.10
echo "[1;36m¦w¸Ë BBS ¤¤ [0;5m...[m"
cd src
make clean $ostype install

# ±Ò°Ê
# °²³]¶}¦b port 9987
cd
bin/bbsd 9987
bin/camera
bin/account

# telnet ´ú¸Õ
telnet 0 9987
