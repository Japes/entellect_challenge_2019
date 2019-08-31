#!/usr/bin/env python3

from collections import defaultdict
import os
import sys

def Count_moves(filename):
    if not os.path.exists(filename):
        return
        
    mv_count = defaultdict(int)
    with open(filename) as f:
        for line in f.readlines()[2:]:
            mv = line.split(',')[1]
            mv_count[mv] += 1
    return mv_count

def Compare_move_counts(amvs, bmvs):
    for k in ['health', 'score', 'dig', 'move', 'shoot', 'banana', 'snowball', 'select', 'invalid', 'nothing']:
        if amvs[k] < bmvs[k]:
            print(' X ', end='')
        else:
            print('   ', end='')
        if amvs[k] != 0 or bmvs[k] != 0:
            print(f'{k}: {amvs[k]:3}-{bmvs[k]:3}', end=' ')
        amvs.pop(k)
        bmvs.pop(k)
    if not len(amvs) == 0:
        print(amvs)
    if not len(bmvs) == 0:
        print(bmvs)

def Output_game_result(res):
    #print(f'seed:{res[0][11:-1]: >6}', end=' ')
    print(f'win: {res[2][15]}', end=' ')

def Count_selects_file(filename, mvs):
    if not os.path.exists(filename):
        return
    with open(filename) as f:
        lines = f.readlines()
        if(len(lines) == 0) :
            return
        if 'select' in lines[0]:
            mvs['select'] += 1

def Count_selects(root, amvs, bmvs):
    for subdir, dirs, files in os.walk(root):
        for path in dirs:
            if path[0] == 'A':
                Count_selects_file(os.path.join(subdir, path, 'PlayerCommand.txt'), amvs)
            elif path[0] == 'B':
                Count_selects_file(os.path.join(subdir, path, 'PlayerCommand.txt'), bmvs)

def Summarize_match(root):
    game_result = None
    for subdir, dirs, files in os.walk(root):
        for filename in files:
            if filename == 'endGameState.txt':
                with open(os.path.join(subdir, filename)) as f:
                    game_result = f.readlines()[:7]
            elif filename[0] == 'A':
                amvs = Count_moves(os.path.join(subdir, filename))
            elif filename[0] == 'B':
                bmvs = Count_moves(os.path.join(subdir, filename))
        for path in dirs:
            Count_selects(os.path.join(subdir, path), amvs, bmvs)

    if game_result == None :
        return

    Output_game_result(game_result)
    af = game_result[4].split(':')
    bf = game_result[5].split(':')
    amvs['health'] = af[2][:-1]
    bmvs['health'] = bf[2][:-1]
    amvs['score'] = af[1].split()[0]
    bmvs['score'] = bf[1].split()[0]
    if game_result[2][15] == 'A':
        Compare_move_counts(amvs, bmvs)
    else:
        Compare_move_counts(bmvs, amvs)
    print(f"  {game_result[4].split('-')[1]} vs {game_result[5].split('-')[1]}")

def For_each_play_in(root, fn):
    for subdir, dirs, files in os.walk(root):
        for dirname in dirs:
            print(dirname, end=' ')
            fn(os.path.join(subdir, dirname));
        break

if __name__ == '__main__':
    For_each_play_in(sys.argv[1], Summarize_match)
