import sys
import os
import shutil
import json
#import plotly.express as px
import matplotlib.pyplot as plt
import numpy as np

#script scrapes match logs for interesting metrics, and draws graphs

def getData(filename):
    file = open(filename, "r")
    lines = file.readlines()
    file.close()

    for i, line in enumerate(lines):
        if(line == "MY_LIBS_SRC_DIR := ../../src\n"):
            print("making change to makefile...")
            lines[i] = "MY_LIBS_SRC_DIR := ./src\n"

def getRoundFolder(roundNumber) :
    ret = "Round "
    if(roundNumber < 100) :
        ret = ret + "0"
    if(roundNumber < 10) :
        ret = ret + "0"

    ret = ret + str(roundNumber)

    return ret

def getJsonMapData(JsonMapFilePath) :
    ret = {}
    json_map = json.load(open(JsonMapFilePath, 'r'))
    ret["score"] = json_map["myPlayer"]["score"]
    return ret

def getPlayerData(playerFolder) :
    return getJsonMapData(playerFolder + "/JsonMap.json")

def getRoundData(roundFolder) :
    playerFolders = [d for d in os.listdir(roundFolder)]
    for playerFolder in playerFolders:
        if("A - " in playerFolder):
            playerA = playerFolder[4:]
            playerAData = getPlayerData(roundFolder + "/" + playerFolder)
        if("B - " in playerFolder):
            playerB = playerFolder[4:]
            playerBData = getPlayerData(roundFolder + "/" + playerFolder)

    return playerA, playerB, playerAData, playerBData

############################################################################

if len(sys.argv) < 2:
    print("Give me a match folder: \"matchAnalysis.py [MATCH_FOLDER]\"")

matchFolder = sys.argv[1]
print("Scraping ", matchFolder, "...")

if not os.path.exists(matchFolder):
    print("can't find that folder.")
    exit()

playerA = ""
playerB = ""
playerAScores = []
playerBScores = []
rounds = []

roundNumber = 1
roundFolder = matchFolder + "/" + getRoundFolder(roundNumber)
while os.path.exists(roundFolder):
    playerA, playerB, playerAData, playerBData = getRoundData(roundFolder)
    rounds.append(roundNumber)
    playerAScores.append(playerAData["score"])
    playerBScores.append(playerBData["score"])
    roundNumber += 1
    roundFolder = matchFolder + "/" + getRoundFolder(roundNumber)

print("playerA: ", playerA, ", playerB: ", playerB)


fig, ax = plt.subplots()
ax.plot(rounds, playerAScores, label=playerA, color='red')
ax.plot(rounds, playerBScores, label=playerB, color='blue')
ax.set(title='Player scores')
ax.grid()
ax.legend()
#fig.savefig("test.png")
plt.show()

#graph of score over time
#graph of health over time
#mark when banana bombs were used
#mark when selects were used
#get time to make command


print("done.")
 