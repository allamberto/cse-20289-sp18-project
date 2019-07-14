#!/bin/bash

COMMAND=$1

if [ $COMMAND = 1 ]; then
    ./thor.py -r 10 -p 4 http://student02.cse.nd.edu:9300 >> sd.txt
    ./thor.py -r 10 -p 4 http://student02.cse.nd.edu:9300/html/index.html >> ss.txt
    ./thor.py -r 10 -p 4 "http://student02.cse.nd.edu:9300/scripts/cowsay.sh?message=hi&template=default" >> sc.txt
elif [ $COMMAND = 2 ]; then
    ./thor.py -r 10 -p 4 http://student02.cse.nd.edu:9200 >> fd.txt
    ./thor.py -r 10 -p 4 http://student02.cse.nd.edu:9200/html/index.html >> fs.txt
    ./thor.py -r 10 -p 4 "http://student02.cse.nd.edu:9200/scripts/cowsay.sh?message=hi&template=default" >> fc.txt
elif [ $COMMAND  = 3 ]; then
    ./thor.py -r 10 -p 1 http://student02.cse.nd.edu:9200/text/k.txt >> stk.txt
    ./thor.py -r 10 -p 1 http://student02.cse.nd.edu:9200/text/m.txt >> stm.txt
    ./thor.py -r 10 -p 1 http://student02.cse.nd.edu:9200/text/g.txt >> stg.txt
elif [ $COMMAND = 4 ]; then
    ./thor.py -r 10 -p 1 http://student02.cse.nd.edu:9300/text/k.txt >> ftk.txt
    ./thor.py -r 10 -p 1 http://student02.cse.nd.edu:9300/text/m.txt >> ftm.txt
    ./thor.py -r 10 -p 1 http://student02.cse.nd.edu:9300/text/g.txt >> ftg.txt
fi

exit 0
