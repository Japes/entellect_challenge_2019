import sys
import os
import shutil
import json
#import plotly.express as px
import matplotlib.pyplot as plt
import numpy as np
import fnmatch

#script scrapes match logs for interesting metrics, and draws graphs

class PlayerData :
    def __init__(self):
        self.name = ""
        self.healths = []
        self.scores = []
        self.moves = []
        self.moveTimes = []

    def AppendData(self, data):
        self.healths.append(data["health"])
        self.scores.append(data["score"])
        self.moves.append(data["move"])
        self.moveTimes.append(data["executionTime"])

        self.selects = [i for i, m in enumerate(self.moves) if m == "sel"]
        self.bananas = [i for i, m in enumerate(self.moves) if m == "ban"]
        self.snowballs = [i for i, m in enumerate(self.moves) if m == "sno"]
        self.teleports = [i for i, m in enumerate(self.moves) if m == "mov"]
        self.digs = [i for i, m in enumerate(self.moves) if m == "dig"]
        self.shoots = [i for i, m in enumerate(self.moves) if m == "sho"]
        self.invalid_moves = [i for i, m in enumerate(self.moves) if m != "sel" and m != "ban" and m != "mov" and m != "dig" and m != "sho" and m != "sno" ]

    def PlotData(self, axes, xValues, playerA):
        global _matchFolder

        color = 'red'
        timeColor = '#FFbbbb'
        selectIndicator = ">"

        if not playerA:
            color = 'blue'
            timeColor = '#bbbbFF'
            selectIndicator = "<"

        axes.plot(xValues, self.scores, selectIndicator, ls='-', label=self.name + " selects", color=color, markevery=self.selects)
        axes.plot(xValues, self.scores, "x", ls='-', label=self.name + " invalid/no moves", color=color, markevery=self.invalid_moves)

        axes.plot(xValues, self.healths, "o", ls='-', label=self.name + " bananas", color=color, markevery=self.bananas)
        axes.plot(xValues, self.healths, "d", ls='-', label=self.name + " snowballs", color=color, markevery=self.snowballs)

        axes.plot(xValues, self.moveTimes, ls='-', label=self.name + " execution time", color=timeColor)

        axes.set(title='Player scores ' + _matchFolder )
        axes.grid()
        axes.legend()

    def PlotMovesPie(self, axes, playerA, matchWinner):
        numSelects = len(self.selects)
        numBananas = len(self.bananas)
        numSnowballs = len(self.snowballs)
        numTeleports = len(self.teleports)
        numDigs = len(self.digs)
        numShoots = len(self.shoots)
        numInvalid = len(self.invalid_moves)

        pieData = [numSelects, numBananas, numSnowballs, numTeleports, numDigs, numShoots, numInvalid]

        SelectsLabel = "Selects (" + str(numSelects) + ")"
        BananasLabel = "Bananas (" + str(numBananas) + ")"
        SnowballsLabel = "Snowballs (" + str(numSnowballs) + ")"
        TeleportsLabel = "Teleports (" + str(numTeleports) + ")"
        DigsLabel = "Digs (" + str(numDigs) + ")"
        ShootsLabel = "Shoots (" + str(numShoots) + ")"
        InvalidLabel = "Invalid (" + str(numInvalid) + ")"

        pieLabels = [SelectsLabel, BananasLabel, SnowballsLabel, TeleportsLabel, DigsLabel, ShootsLabel, InvalidLabel]

        axes.pie(pieData, labels=pieLabels, autopct='%1.1f%%', shadow=True, startangle=90)

        axes.axis('equal')  # Equal aspect ratio ensures that pie is drawn as a circle.
        
        plotTitle = self.name
        if playerA and matchWinner == 'A':
            plotTitle += " (winner)"
        if not playerA and matchWinner == 'B':
            plotTitle += " (winner)"
			
        axes.set(title=plotTitle)
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
    ret["health"] = json_map["myPlayer"]["health"]
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
					
############################################################################

if len(sys.argv) < 2:
    print("Give me a match folder: \"matchAnalysis.py [MATCH_FOLDER]\"")

_matchFolder = sys.argv[1]
print("Scraping ", _matchFolder, "...")

if not os.path.exists(_matchFolder):
    print("can't find that folder.")
    exit()

playerA = PlayerData()
playerB = PlayerData()
rounds = []

roundNumber = 1
roundFolder = _matchFolder + "/" + getRoundFolder(roundNumber)
while os.path.exists(roundFolder):
    playerA.name, playerB.name, playerAData, playerBData = getRoundData(roundFolder)
    rounds.append(roundNumber)

    playerA.AppendData(playerAData)
    playerB.AppendData(playerBData)

    roundNumber += 1
    roundFolder = _matchFolder + "/" + getRoundFolder(roundNumber)


matchWinner = getMatchWinner(_matchFolder).strip()

print("playerA: ", playerA.name, ", playerB: ", playerB.name, ", winner was ", matchWinner)


fig, ax = plt.subplots()

playerA.PlotData(ax, rounds, True)
playerB.PlotData(ax, rounds, False)

fig2, [pieAxA, pieAxB] = plt.subplots(1,2)
playerA.PlotMovesPie(pieAxA, True, matchWinner)
playerB.PlotMovesPie(pieAxB, False, matchWinner)

#fig.savefig("test.png")
plt.show()

#TODO
#graph of health over time


print("done.")
 