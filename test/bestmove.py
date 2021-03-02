#!/usr/bin/env python3
""" test/bestmove.py - Tester to print the best move in a position

Examples:

```
$ test/bestmove.py test/startpos.fen
```

@author: Cade Brown <cade@cade.site>
"""

import os
import signal
import subprocess
import argparse
import time

parser = argparse.ArgumentParser(description='Find best move in a given position')

parser.add_argument('pos', help='Position or file to use')
parser.add_argument('--engine', default='./cce', help='Chess engine to use')
parser.add_argument('--time', default=1.0, type=float, help='Time to think')
parser.add_argument('--debug', action='store_true', help='Debug switch, which causes all output to be printed')

args = parser.parse_args()


# Get FEN string
fen = None
if args.pos.endswith('.fen'):
    # .fen file, read it in
    with open(args.pos) as fp:
        fen = fp.read()

else:
    # Assume it is FEN itself
    fen = args.pos

# Launch chess engine
proc = subprocess.Popen([args.engine], stdout=subprocess.PIPE, stdin=subprocess.PIPE, encoding='utf-8', bufsize=0)

# Run UCI command
def run(cmd):
    print('>', cmd)
    proc.stdin.write(cmd)
    proc.stdin.write('\n')


# Now, initialize the engine
run('uci')

# Set up a position
run('position fen ' + fen)

# Let the engine think
run('go')
time.sleep(args.time)
run('stop')

# Iterate over output
for line in proc.stdout:
    line = line.replace('\n', '')
    
    # Print all output in debug mode
    if args.debug:
        print(line)

    # Check for the 'bestmove' output
    if line.startswith('bestmove'):
        # If given, print the best move (in long algebraic notation)
        print ('best:', line.split(' ')[1])
        break

os.kill(proc.pid, signal.SIGTERM)
