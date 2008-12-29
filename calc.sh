#!/bin/sh
tmp=/tmp/test_$$

cat >$tmp.c << EOS
#include <stdio.h>
#include "r600_reg.h"
int main (int argc, char *argv[]) {
long val =
EOS

if [ "x$1" = x ] ; then
  cat >>$tmp.c
else
  echo "$@" >>$tmp.c
fi
if [ "x`sed -e '$!d;s/.*\(.\)$/\1/' $tmp.c`" != "x;" ] ; then
  echo ";" >>$tmp.c
fi

cat >>$tmp.c <<EOE
printf (" = %ld = 0x%lx\n", val, val);
return 0;
}
EOE

if gcc -I. -o $tmp.out $tmp.c ; then
  $tmp.out
else
  cat $tmp.c
fi

rm -f $tmp.c $tmp.out
