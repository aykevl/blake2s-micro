#!/usr/bin/python3

import json
import subprocess

def test(path):
    data = json.load(open(path))
    success = 0
    fail    = 0
    skip    = 0
    for case in data:
        if case['hash'] != 'blake2s':
            continue
        if case['key']:
            continue
        call = subprocess.run(['build/test', case['in']], stdout=subprocess.PIPE)
        out = call.stdout.decode('utf-8').strip()
        if out.startswith('skip:'):
            skip += 1
        elif out != case['out']:
            print('Fail for input:', case['in'])
            print('  expected:', case['out'])
            print('  actual:  ', out)
            fail += 1
        else:
            success += 1
    print('Test run: %d, success: %d, skip: %d, fail: %d' % (success + fail + skip, success, skip, fail))

def main():
    import sys, os
    if len(sys.argv) >= 2:
        path = sys.argv[1]
    elif os.path.isfile('blake2-kat.json'):
        path = 'blake2-kat.json'
    else:
        print('Provide a JSON test file, see:\nhttps://github.com/BLAKE2/BLAKE2/blob/master/testvectors/blake2-kat.json')
        return
    test(path)

if __name__ == '__main__':
    main()
