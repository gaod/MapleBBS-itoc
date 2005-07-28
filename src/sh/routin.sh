#!/bin/bash
####################################################################
#�ݪO�Ʀ�]�έp
basedir=/home/bbs/manage
logfile=/home/bbs/gem/@/@hotboard
hotweek=/home/bbs/gem/@/@hotweek
hotmonth=/home/bbs/gem/@/@hotmonth
yestarday=`date --date='1 day ago' +%y%m%d`
if [ ! -d "$basedir/brd" ]; then
	mkdir -p $basedir/brd
fi
cp /home/bbs/run/brd_usies.log $basedir/brd/$yestarday.log
cat /home/bbs/run/brd_usies.log|awk '{print $1}'|uniq -c|awk '{printf "%-15s%s\n" ,$2,$1}'|sort -rn -k 2 > $basedir/board.log
echo "�ݪO�W�� ���[�H��"|awk '{printf("%-15s%s\n",$1,$2)}'	> $logfile
echo "�W�� �ݪO�W�� ���[�H��"|awk '{printf("%-8s%-15s%s\n",$1,$2,$3)}'	> $hotweek
echo "�W�� �ݪO�W�� ���[�H��"|awk '{printf("%-8s%-15s%s\n",$1,$2,$3)}'	> $hotmonth
echo "-----------------------------"			>> $logfile
echo "-----------------------------"			>> $hotweek
echo "-----------------------------"			>> $hotmonth
ls -l /home/bbs/brd|awk '$1 !~ /total/ {print $9}'|sed 's/\///' > $basedir/boardlist
echo "�ثe�ݪO�Ӽ�:`cat $basedir/boardlist|wc -l`"	>> $logfile
cat $basedir/board.log                          	>> $logfile
today=`date +%y%m%d`
find $basedir/brd -mtime -7 -type f -exec cat '{}' \;|awk '{print $1}'|sort|uniq -c|awk '{printf "%-15s%s\n" ,$2,$1}'|sort -rn -k 2 > $basedir/week
cat -b $basedir/week					>> $hotweek
find $basedir/brd -mtime -30 -type f -exec cat '{}' \;|awk '{print $1}'|sort|uniq -c|awk '{printf "%-15s%s\n" ,$2,$1}'|sort -rn -k 2 > $basedir/month
cat -b $basedir/month                                       >> $hotmonth
boards=`cat $basedir/boardlist`
for board in $boards
do
	judge=`cat $basedir/board.log|grep "^$board"`
	if [ "$judge" == "" ]; then
		echo "$board 0"|awk '{printf("%-15s%s\n",$1,$2)}'	>> $logfile
	fi
	judge=`cat $basedir/week|grep "^$board"`
	if [ "$judge" == "" ]; then
		echo "$board 0"|awk '{printf("%-15s%s\n",$1,$2)}'       >> $hotweek
	fi
	judge=`cat $basedir/month|grep "^$board"`
	if [ "$judge" == "" ]; then
		echo "$board 0"|awk '{printf("%-15s%s\n",$1,$2)}'       >> $hotmonth
	fi
done
}
#####################################################################
