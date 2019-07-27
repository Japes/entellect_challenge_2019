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

        self.selects = [i for i, m in enumerate(self.moves) if m == "sel"]
        self.bananas = [i for i, m in enumerate(self.moves) if m == "ban"]
        self.teleports = [i for i, m in enumerate(self.moves) if m == "mov"]
        self.digs = [i for i, m in enumerate(self.moves) if m == "dig"]
        self.shoots = [i for i, m in enumerate(self.moves) if m == "sho"]

    def PlotData(self, axes, xValues, playerA):

        color = 'red'
        timeColor = '#FFbbbb'
        selectIndicator = ">"

        if not playerA:
            color = 'blue'
            timeColor = '#bbbbFF'
            selectIndicator = "<"

        axes.plot(xValues, self.scores, selectIndicator, ls='-', label=self.name + " selects", color=color, markevery=self.selects)
        axes.plot(xValues, self.scores, "o", ls='-', label=self.name + " bananas", color=color, markevery=self.bananas)
        axes.plot(xValues, self.moveTimes, ls='-', label=self.name + " execution time", color=timeColor)

        axes.set(title='Player scores')
        axes.grid()
        axes.legend()

    def PlotMovesPie(self, axes, playerA):
        totalOtherMoves = len(self.moves) - (len(self.teleports) + len(self.digs) + len(self.bananas) + len(self.selects) + len(self.shoots))
        pieData = [len(self.selects), len(self.bananas), len(self.teleports), len(self.digs), len(self.shoots), totalOtherMoves]
        pieLabels = ['selects', 'bananas', 'moves', 'digs', 'shoots', 'OTHER(!)']

        axes.pie(pieData, labels=pieLabels, autopct='%1.1f%%', shadow=True, startangle=90)

        axes.axis('equal')  # Equal aspect ratio ensures that pie is drawn as a circle.
        
        axes.set(title=self.name)
        axes.legend()


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

fig2, [pieAxA, pieAxB] = plt.subplots(1,2)
playerA.PlotMovesPie(pieAxA, True)
playerB.PlotMovesPie(pieAxB, False)

#fig.savefig("test.png")
plt.show()

#TODO
#graph of health over time


print("done.")
 