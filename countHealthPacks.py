#!/usr/bin/env python3

import sys
import os
import shutil
import json
import fnmatch

def getMatchData(matchFolder) :
    playerAData = ''
    playerBData = ''
    #get player stats in existing csvs
    for file_name in os.listdir(matchFolder):
        if fnmatch.fnmatch(file_name, 'A*.csv'):
            file = open(matchFolder + "/" + file_name, "r")
            playerAData = file.readlines()
            file.close()
        if fnmatch.fnmatch(file_name, 'B*.csv'):
            file = open(matchFolder + "/" + file_name, "r")
            playerBData = file.readlines()
            file.close()

    return playerAData, playerBData

#returns A, B, or X for tie
def getMatchWinner(matchFolder) :
    for dir_name in os.listdir(matchFolder):
        dir = matchFolder + "/" + dir_name
        if os.path.isdir(dir):
            for file_name in os.listdir(dir):
                if fnmatch.fnmatch(file_name, 'endGameState.txt'):
                    resultFile = dir + "/" + file_name
                    file = open(resultFile, "r")
                    result = file.readlines()
                    for line in result:
                        if line.startswith("The winner is:") :
                            winner = line[15]
                            file.close()
                            return winner
                    file.close()
                    return 'X'

def countHealthPacks(playerData) :
    worm1H_prev, worm2H_prev, worm3H_prev = 9999, 9999, 9999
    healthPacks = 0
    for line in playerData[1:]:
        csvs = line.split(',')
        worm1H, worm2H, worm3H = int(csvs[6]), int(csvs[9]), int(csvs[12])
        if worm1H > worm1H_prev or worm2H > worm2H_prev or worm3H > worm3H_prev:
            healthPacks += 1
        worm1H_prev, worm2H_prev, worm3H_prev = worm1H, worm2H, worm3H

    return healthPacks

############################################################################

if len(sys.argv) < 2:
    print("Give me a folder with matches: \"matchAnalysis.py [MATCH_FOLDER]\"")
    exit()

_matchesFolder = sys.argv[1]
print("Scraping ", _matchesFolder, "...")

if not os.path.exists(_matchesFolder):
    print("can't find that folder.")
    exit()

player1HP = 0
player2HP = 0

for dir_name in os.listdir(_matchesFolder):
    _matchFolder = _matchesFolder + "/" + dir_name
    print("getting match ", _matchFolder, "...")
    if os.path.isdir(_matchFolder):
            playerAData, playerBData = getMatchData(_matchFolder)
            player1HP += countHealthPacks(playerAData)
            player2HP += countHealthPacks(playerBData)

print("Player 1 health packs got: ", player1HP)
print("Player 2 health packs got: ", player2HP)


#each row:
