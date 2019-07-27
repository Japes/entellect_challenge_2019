import sys
import os
import shutil
import json
#import plotly.express as px
import matplotlib.pyplot as plt
import numpy as np

#script scrapes match logs for interesting metrics, and draws graphs

class PlayerData :
    def __init__(self):
        self.name = ""
        self.scores = []
        self.moves = []
        self.moveTimes = []

    def AppendData(self, data):
        self.scores.append(data["score"])
        self.moves.append(data["move"])
        self.moveTimes.append(data["executionTime"])

    def PlotData(self, axes, xValues, playerA):
        selects = [i for i, m in enumerate(self.moves) if m == "sel"]
        bananas = [i for i, m in enumerate(self.moves) if m == "ban"]
        teleports = [i for i, m in enumerate(self.moves) if m == "mov"]
        digs = [i for i, m in enumerate(self.moves) if m == "dig"]
        shoots = [i for i, m in enumerate(self.moves) if m == "sho"]

        color = 'red'
        timeColor = '#FFbbbb'
        playerThatIAm = "Player A"
        selectIndicator = ">"

        if not playerA:
            color = 'blue'
            timeColor = '#bbbbFF'
            playerThatIAm = "Player B"
            selectIndicator = "<"

        axes.plot(xValues, self.scores, selectIndicator, ls='-', label=self.name + " selects", color=color, markevery=selects)
        axes.plot(xValues, self.scores, "o", ls='-', label=self.name + " bananas", color=color, markevery=bananas)
        axes.plot(xValues, self.moveTimes, ls='-', label=self.name + " execution time", color=timeColor)

        print(playerThatIAm, " selects: ",  selects, "(" + str(len(selects)) + ")")
        totalOtherMoves = len(xValues) - (len(teleports) + len(digs) + len(bananas) + len(selects) + len(shoots))
        print(playerThatIAm, " moves: ",  len(teleports), " digs: ", len(digs), " INVALIDS/NO_MOVES: ", totalOtherMoves)


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
    exeTime = 0
    for i, line in enumerate(lines):
        if "Command:" in line:
           command =  line[9:12]

        if "Execution time:" in line:
            exeTime = int(line[16:-3])

    return command, exeTime

def getJsonMapData(JsonMapFilePath) :
    ret = {}
    json_map = json.load(open(JsonMapFilePath, 'r'))
    ret["score"] = json_map["myPlayer"]["score"]
    return ret

def getPlayerData(playerFolder) :
    ret = getJsonMapData(playerFolder + "/JsonMap.json")
    ret["move"], ret["executionTime"] = getPlayerCommandData(playerFolder + "/PlayerCommand.txt")
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

playerA = PlayerData()
playerB = PlayerData()
rounds = []

roundNumber = 1
roundFolder = matchFolder + "/" + getRoundFolder(roundNumber)
while os.path.exists(roundFolder):
    playerA.name, playerB.name, playerAData, playerBData = getRoundData(roundFolder)
    rounds.append(roundNumber)

    playerA.AppendData(playerAData)
    playerB.AppendData(playerBData)

    roundNumber += 1
    roundFolder = matchFolder + "/" + getRoundFolder(roundNumber)

print("playerA: ", playerA.name, ", playerB: ", playerB.name)

fig, ax = plt.subplots()

playerA.PlotData(ax, rounds, True)
playerB.PlotData(ax, rounds, False)

ax.set(title='Player scores')
ax.grid()
ax.legend()
#fig.savefig("test.png")
plt.show()

#TODO
#graph of health over time


print("done.")
 