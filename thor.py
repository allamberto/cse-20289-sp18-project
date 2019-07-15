#!/usr/bin/env python3

import multiprocessing
import os
import requests
import sys
import time

# Globals

PROCESSES = 1
REQUESTS  = 1
VERBOSE   = False
URL       = None
ARGUMENTS   = sys.argv[1:]
pNum = 0

# Functions

def usage(status=0):
    print('''Usage: {} [-p PROCESSES -r REQUESTS -v] URL
    -h              Display help message
    -v              Display verbose output

    -p  PROCESSES   Number of processes to utilize (1)
    -r  REQUESTS    Number of requests per process (1)
    '''.format(os.path.basename(sys.argv[0])))
    sys.exit(status)

def do_request(pid):
    total = []
    for i in range(REQUESTS):
        bTime = time.time()
        r = requests.get(URL)
        eTime = time.time()
        if(VERBOSE):
            print(r.text)
        print("Process: {}, Request: {}, Elapsed Time: {}".format(pid, i, round((eTime-bTime), 2)))
        total.append(eTime - bTime)

    print("Process: {}, AVERAGE   , Elapsed Time: {}".format(pid, round((sum(total) / len(total)), 2)))
    return sum(total) / len(total)

# Main execution

if __name__ == '__main__':
    while ARGUMENTS and ARGUMENTS[0].startswith('-') and len(ARGUMENTS[0]) > 1:
        arg = ARGUMENTS.pop(0)
        if arg == '-h':
            usage(0)
        elif arg == '-v':
            VERBOSE = True
        elif arg == '-p':
            PROCESSES = int(ARGUMENTS.pop(0))
        elif arg == '-r':
            REQUESTS = int(ARGUMENTS.pop(0))
        else:
            usage(1)

    if len(ARGUMENTS) == 0:
        usage(1)
    else:
        URL = ARGUMENTS.pop(0)

    # Create pool of workers and perform requests
    pool = multiprocessing.Pool(PROCESSES)
    times = pool.map(do_request, range(PROCESSES))
    print("TOTAL AVERAGE ELAPSED TIME: {}".format(round(sum(times) / len(times), 2)))
    # pass

# vim: set sts=4 sw=4 ts=8 expandtab ft=python:
