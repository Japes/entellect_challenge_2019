import sys
import os
import shutil
import json
#import plotly.express as px
import matplotlib.pyplot as plt
import numpy as np

#script scrapes match logs for interesting metrics, and draws graphs

def getRoundFolder(roundNumber) :
    ret = "Round "
    if(roundNumber < 100) :
        ret = ret + "0"
    if(roundNumber < 10) :
        ret = ret + "0"

    ret = ret + str(roundNumber)

    return ret

def getPlayerCommandData(filename) :
    file = open(filename, "r")
    lines = file.readlines()
    file.close()

    command = ""
    for i, line in enumerate(lines):
        if "Command:" in line:
           command =  line[9:12]

    return command

def getJsonMapData(JsonMapFilePath) :
    ret = {}
    json_map = json.load(open(JsonMapFilePath, 'r'))
    ret["score"] = json_map["myPlayer"]["score"]
    return ret

def getPlayerData(playerFolder) :
    ret = getJsonMapData(playerFolder + "/JsonMap.json")
    ret["move"] = getPlayerCommandData(playerFolder + "/PlayerCommand.txt")
    return ret

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
playerAMoves = []
playerBScores = []
playerBMoves = []
rounds = []

roundNumber = 1
roundFolder = matchFolder + "/" + getRoundFolder(roundNumber)
while os.path.exists(roundFolder):
    playerA, playerB, playerAData, playerBData = getRoundData(roundFolder)
    rounds.append(roundNumber)

    playerAScores.append(playerAData["score"])
    playerAMoves.append(playerAData["move"])

    playerBScores.append(playerBData["score"])
    playerBMoves.append(playerBData["move"])

    roundNumber += 1
    roundFolder = matchFolder + "/" + getRoundFolder(roundNumber)

print("playerA: ", playerA, ", playerB: ", playerB)

fig, ax = plt.subplots()

playerASelects = [i for i, m in enumerate(playerAMoves) if m == "sel"]
playerABananas = [i for i, m in enumerate(playerAMoves) if m == "ban"]
ax.plot(rounds, playerAScores, ">", ls='-', label=playerA + " selects", color='red', markevery=playerASelects)
ax.plot(rounds, playerAScores, "o", ls='-', label=playerA + " bananas", color='red', markevery=playerABananas)

playerBSelects = [i for i, m in enumerate(playerBMoves) if m == "sel"]
playerBBananas = [i for i, m in enumerate(playerBMoves) if m == "ban"]
ax.plot(rounds, playerBScores, '<', ls='-', label=playerB + " selects", color='blue', markevery=playerBSelects)
ax.plot(rounds, playerBScores, 'o', ls='-', label=playerB + " bananas", color='blue', markevery=playerBBananas)


print("playerASelects: ", playerASelects, ", playerBSelects: ", playerBSelects)

ax.set(title='Player scores')
ax.grid()
ax.legend()
#fig.savefig("test.png")
plt.show()

#TODO
#graph of health over time
#mark when banana bombs were used
#mark when selects were used
#get time to make command


print("done.")
 