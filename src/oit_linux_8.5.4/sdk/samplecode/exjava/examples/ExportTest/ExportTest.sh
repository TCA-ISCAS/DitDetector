#!/bin/sh
#

show_usage()
{
	echo '-----------------------------------------'
	echo '. Usage is'
	echo '.    ExportTest.sh inputfile outputfile configfile [timeout(ms)]'
	echo '.'
	echo '-----------------------------------------'
}

if test $# -lt 3 ; then
	show_usage
	exit 1
fi
if test $# -gt 4 ; then
	show_usage
	exit 1
fi

if test $# -eq 4 ; then
    echo java -classpath ExportTest.jar:Export.jar ExportTest $1 $2 $3 $4
    java -Dsun.jnu.encoding=UTF-8 -Dfile.encoding=UTF-8 -classpath ExportTest.jar:Export.jar ExportTest $1 $2 $3 $4
else
    echo java -classpath ExportTest.jar:Export.jar ExportTest $1 $2 $3
    java  -Dsun.jnu.encoding=UTF-8 -Dfile.encoding=UTF-8 -classpath ExportTest.jar:Export.jar ExportTest $1 $2 $3
fi

