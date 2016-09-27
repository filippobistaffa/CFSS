#!/usr/bin/env bash

red='\033[0;31m'			# Red
nc='\033[0m'				# No color
re='^[0-9]+$'				# Regular expression to detect natural numbers
s=0					# Default s = 0
m=2					# Default m = 2
basename="twitter/net/twitter-2010"	# Twitter files must be in "twitter/net" subdir and have twitter-2010.* filenames
wg="twitter/wg"				# Place WebGraph libraries in "twitter/wg" subdir
arg=""

usage() { echo -e "Usage: $0 -t <scalefree|twitter|filename> -n <#agents> [-s <seed>] [-m <barabasi_m>]\n-t\tNetwork topology (either scalefree, twitter, or the input filename)\n-n\tNumber of agents\n-s\tSeed (optional, default s = 0)\n-m\tParameter m of the Barabasi-Albert model (optional, default m = 2)" 1>&2; exit 1; }

while getopts ":t:n:s:d:m:p:" o; do
	case "${o}" in
	t)
		t=${OPTARG}
		if ! [[ $t == "scalefree" || $t == "twitter" ]] ; then
			if [ ! -f "$1" ]
			then
				echo -e "${red}Input file \"$t\" not found!${nc}\n"
				usage
			fi
		fi
		;;
	n)
		n=${OPTARG}
		if ! [[ $n =~ $re ]] ; then
			echo -e "${red}Number of agents must be a number!${nc}\n"
			usage
		fi
		;;
	s)
		s=${OPTARG}
		if ! [[ $s =~ $re ]] ; then
			echo -e "${red}Seed must be a number!${nc}\n"
			usage
		fi
		;;
	m)
		m=${OPTARG}
		if ! [[ $m =~ $re ]] ; then
			echo -e "${red}Parameter m must be a number!${nc}\n"
			usage
		fi
		;;
	\?)
		echo -e "${red}-$OPTARG is not a valid option!${nc}\n"
		usage
		;;
	esac
done
shift $((OPTIND-1))

if [ -z "${t}" ] || [ -z "${n}" ] || [ -z "${s}" ]; then
	echo -e "${red}Missing one or more required options!${nc}\n"
	usage
fi

tmp=`mktemp`
echo "#define N $n" > $tmp

case "$t" in
scalefree)
	arg=$s
	echo "#define M $m" >> $tmp
	;;
twitter)
	arg=`mktemp`
	java -Xmx4000m -cp .:$wg/* ReduceGraph $basename $n $s | grep -v WARN > $arg
	e=`cat $arg | wc -l`
	echo "#define E $e" >> $tmp
	cat $arg
	;;
*)
	e=`cat $arg | wc -l $t`
	echo "#define E $e" >> $tmp
	arg=$t
	;;
esac

if [ ! -f instance.h ]
then
	mv $tmp "instance.h"
else
	md5a=`md5sum instance.h | cut -d\  -f 1`
	md5b=`md5sum $tmp | cut -d\  -f 1`

	if [ $md5a != $md5b ]
	then
		mv $tmp "instance.h"
	else
		rm $tmp
	fi
fi

make -j
if [[ $? == 0 ]]
then
	./cfss $arg
fi

if [[ $t == "twitter" ]]
then
	rm $arg
fi
