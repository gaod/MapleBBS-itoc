#!/bin/sh
#���{���ΨӤ��R�s�� bbs �� smtp �s���ӷ��ƶq
cat /dev/null | awk 'BEGIN {printf("%10s    %-20s\n", "�s�u����", "�s�u�ӷ�")} {} END{}'

cat /home/bbs/run/bmta.log.* | grep CONN | sort -k 3 -r | awk '{print $3}'| awk 'BEGIN {} {Number[$1]++} END {
  for(course in Number)
     printf("%10d    %-20s\n", Number[course], course)
}' | sort -n -r
