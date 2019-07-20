#!/usr/bin/env python3

import platform
import os
import sys
import shutil
import time
import datetime
import glob

#script for running matches/tournaments using the official runner/scripts.
#goal is to mimic actual tournament running conditions as closely as possible

#hardcoded stuff #############################
_botsPath = "./bots/" #relative path to bots from script position
_staterPackPath = "./starter-pack" #relative path to starterpack from script position
_matchLogsPath = "./match-logs" #relative path to match logs FROM STARTER PACK (this gets put in the game runner config)
_startingPath = "" #path script is run from
_resultsFolder = "runMatchesResults"

#global vars #################################
_verbosity = 2 #verbosity 0 = fast as possible; 1 = show only game map; 2 = show as much as possible
_bots = []
_numRounds = 2 #how many times to run tournament
_engineVer = "" #version of the engine we are using for this playthrough
_playall = False
_parallelID = ""

#function defs################################
def showUsage():
    print("Usage: runMatches [-v0|-v1|-v2] [-a] bot1 bot2 ...")
    print("       runs matches, bot1 vs the rest")
    print("       bot1, bot2 etc are folder names of bots in " + _botsPath)
    print("       -v specifies verbosity (with 0 being least verbose)")
    print("       -a : run all bots against each other")
    print("       -p[number] : parallel runner id. Don't run 2 with the same ID at the same time!")

def parseArgs():
    global _verbosity
    global _bots
    global _playall
    global _parallelID

    if(len(sys.argv) < 2):
        showUsage()
        exit()

    #defaults
    _verbosity = 2

    for arg in sys.argv[1:]:
        if("-v" in arg):
            if(arg == "-v0"):
                print("setting verbosity to level 0")
                _verbosity = 0
            elif(arg == "-v1"):
                print("setting verbosity to level 1")
                _verbosity = 1
        elif("-p" in arg):
            if(len(arg) < 3):        
                print("incorrect usage of the -p argument...")
                showUsage()
                exit()
            else:
                _parallelID = arg[2]
        elif(arg == "-a"):
            print("playall true")
            _playall = True
        else:
            _bots.append(arg)

def copy(src, dest, ignore_pattern):
    try:
        shutil.copytree(src, dest, ignore=shutil.ignore_patterns(ignore_pattern))
    except OSError as e:
        # If the error was caused because the source wasn't a directory
        if e.errno == shutil.errno.ENOTDIR:
            shutil.copy(src, dest)
        else:
            print('Directory not copied. Error: %s' % e)

#puts in bot1 and bot2 into the game runner config file
def setupConfig(bot1, bot2, matchLogsPath):
    global _staterPackPath

    gameRunnerConfigFile = _staterPackPath + "/game-runner-config.json"
    originalGameRunnerConfigFile = gameRunnerConfigFile + ".original"

    if not os.path.isfile(originalGameRunnerConfigFile):
        shutil.copy(gameRunnerConfigFile, originalGameRunnerConfigFile)

    file = open(gameRunnerConfigFile + ".original", "r")
    lines = file.readlines()
    file.close()

    for i, line in enumerate(lines):
        if("player-a" in line):
            lines[i] = "\"player-a\": \"../" + _botsPath + bot1 + "\",\n"
        if("player-b" in line):
            lines[i] = "\"player-b\": \"../" + _botsPath + bot2 + "\",\n"
        if("verbose-mode" in line):
            if(_verbosity == 0):
                lines[i] = "\"verbose-mode\": false,\n"
            else:
                lines[i] = "\"verbose-mode\": true,\n"
        if("round-state-output-location" in line):
            lines[i] = "\"round-state-output-location\": \"" + matchLogsPath + "\",\n"


    file = open(gameRunnerConfigFile, "w")
    file.writelines(lines)
    file.close()

def confirmDir(file):
    if not os.path.isdir(file):
        print("Can't find " + file)
        return False
    return True

def getNameScoreHealth(line):
    splitLineDash = line.split('-')
    #print(splitLineDash)
    name = splitLineDash[1][1:]
    #print(name)
    splitSpace = line.split(' ')
    #print(splitSpace)
    #print(splitSpace[-2])
    score = int(splitSpace[-2].split(':')[1])
    #print(score)
    healthStr = splitSpace[-1].split(':')[1]
    health = int(healthStr[:-1]) #strip off newline
    #print(health)

    return name, score, health

''' file looks like this:
Match seed: 15086

The winner is: B - James

A - Reference Bot- score:2114 health:0
B - James- score:2165 health:20
'''
def getMatchResult(matchFolder):
    ret = {}

    os.chdir(matchFolder)
    allRounds = [d for d in os.listdir('.') if os.path.isdir(d)]
    latest_round = max(allRounds, key=os.path.getmtime)
    #print("latest round is in " + latest_round)
    ret["finalRound"] = int(latest_round[6:])

    file = open(latest_round + "/endGameState.txt", "r")
    lines = file.readlines()
    file.close()

    for i, line in enumerate(lines):
        if("Match seed: " in line):
            ret["matchSeed"] = int(line[12:])
        if("The winner is: " in line):
            ret["winner"] = line[15]
        if("The game ended in a tie" in line):
            ret["winner"] = "tie"
        if(line.startswith("A - ")):
            ret["playerAName"], ret["playerAScore"], ret["playerAHealth"] = getNameScoreHealth(line)
        if(line.startswith("B - ")):
            ret["playerBName"], ret["playerBScore"], ret["playerBHealth"] = getNameScoreHealth(line)

    print(ret)

    return ret

#gets match info from the most recent folder in the given folder
def getLatestMatchResult(matchLogsPath):

    starting_path = os.getcwd()

    #print("Getting most recent match results in " + os.getcwd() + matchLogsPath)
    os.chdir(matchLogsPath)
    allMatches = [d for d in os.listdir('.') if os.path.isdir(d)]
    latest_match = max(allMatches, key=os.path.getmtime)
    #print("latest match is in " + latest_match)

    matchFolder = os.getcwd() + "/" + latest_match
    matchResult = getMatchResult(matchFolder)
    matchResult["MatchFolder"] = matchFolder

    os.chdir(starting_path)

    return matchResult

def copyBot(src, dst):
    if os.path.exists(dst):
        shutil.rmtree(dst)
    copy(src, dst, 'rounds')

def runMatch(bot1, bot2):
    global _staterPackPath
    global _startingPath
    global _parallelID
    global _botsPath

    print("bot1: " + bot1 + ", bot2: " + bot2 + ", verbosity: " + str(_verbosity))

    #sanity checks
    if (not confirmDir(_botsPath + bot1)) or (not confirmDir(_botsPath + bot2)):
        print("Couldn't find bot, either" + _botsPath + bot1 + " or " + _botsPath + bot2 + " is missing.  Exiting.")
        exit()

    #make a copy of the bot folder if this is a match between the same bot
    if(bot1 == bot2):
        bot2 = (bot1 + "_autogenerated_copy")
        tempCopyDest = _botsPath + bot2
        print("bot1 == bot2.  Creating copy in " + tempCopyDest)
        copy(_botsPath + bot1, tempCopyDest, 'rounds')

    #make a copy of both bot folders if this is a parallel run
    if(_parallelID != ""):
        newBot1 = "parallelCopy_" + _parallelID + "_" + bot1
        newBot2 = "parallelCopy_" + _parallelID + "_" + bot2

        print("creating copy of bot1 for parallel run: " + _botsPath + newBot1)
        copyBot(_botsPath + bot1, _botsPath + newBot1)

        print("creating copy of bot2 for parallel run: " + _botsPath + newBot2)
        copyBot(_botsPath + bot2, _botsPath + newBot2)

        bot1 = newBot1
        bot2 = newBot2

    setupConfig(bot1, bot2, _matchLogsPath)

    print("Game runner modified.  Starting match.")
    print("")

    os.chdir(_staterPackPath)
    if(platform.system() == 'Windows'):
        os.system("run.bat")
    elif(platform.system() == 'Linux'):
        if(_verbosity < 2):
            os.system("make run | grep -v -e [a-z=]")
        else:
            os.system("make run")

    matchResult = getLatestMatchResult(_matchLogsPath)

    print("Match result: " + str(matchResult))

    os.chdir(_startingPath)

    return matchResult

def GetOutputPaths():
    if not os.path.exists(_resultsFolder):
        os.makedirs(_resultsFolder)
    dateString = datetime.datetime.now().strftime("%Y_%B_%d__%I_%M_%S")
    os.makedirs(_resultsFolder + "/" + dateString)
    allMatchesFilePath = _resultsFolder + "/" + dateString + "/" + "AllMatches.txt"
    summaryFilePath = _resultsFolder + "/" + dateString + "/" + "Summary.txt"
    return allMatchesFilePath, summaryFilePath 

def GetSummaryGrid():
    grid_summary = {}
    for i in range(0, len(_bots)):
        grid_summary[_bots[i]] = {}
        for j in range(0, len(_bots)):
            grid_summary[_bots[i]][_bots[j]] = {}
            grid_summary[_bots[i]][_bots[j]]["wins"] = 0
            grid_summary[_bots[i]][_bots[j]]["draws"] = 0
            grid_summary[_bots[i]][_bots[j]]["losses"] = 0
    return grid_summary

#pad a string so that its length is a multiple of numChars
def PadToNext(numChars, line):
    for i in range(0, numChars - (len(line) % numChars)):
        line += ' '
    return line
        

def WriteSummary(grid, bots, summaryFilePath):
    global _parallelID

    maxBotNameLength = 25
    file = open(summaryFilePath, "a+")

    dateString = datetime.datetime.now().strftime("%Y_%B_%d__%I_%M_%S")
    summary = 'Engine version: ' + _engineVer + ' -- ' + dateString + ' -- ParallelID: ' + _parallelID + '\n'
    header = '(wins/draws/losses):'
    header = PadToNext(maxBotNameLength, header)
    for bot in _bots:
        if(len(bot) > maxBotNameLength):
            file.writelines("One of these bots has a long name, it might cock up the formatting here :/\n")
        header += bot
        header = PadToNext(maxBotNameLength, header)
    summary += header + "\n"

    for bot in _bots:
        line = bot + ':'
        line = PadToNext(maxBotNameLength, line)
        totalWins = 0
        totalDraws = 0
        totalLosses = 0
        for opponent in _bots:
            numWins = grid[bot][opponent]["wins"]
            numDraws = grid[bot][opponent]["draws"]
            numLosses = grid[bot][opponent]["losses"]
            totalWins += numWins
            totalDraws += numDraws
            totalLosses += numLosses
            line += str(numWins) + '/' + str(numDraws) + '/' + str(numLosses)
            line = PadToNext(maxBotNameLength, line)
        line += "(" + str(totalWins) + '/' + str(totalDraws) + '/' + str(totalLosses) + ")"
        summary += line + "\n"

    file.writelines(summary + "\n")
    file.close()
    print(summary)

#the script  ###############################################################################

#get/set global vars
_startingPath = os.getcwd()
_engineVer = glob.glob(_staterPackPath +'/*game-engine-jvm-full*.jar')[0][-12:-4]
dateString = datetime.datetime.now().strftime("%Y_%B_%d__%I_%M_%S")
print("_engineVer: " +  _engineVer + " -- " + dateString)
parseArgs()
print("Running with paralellID: \"" + _parallelID + "\" BOTS: " + str(_bots))

allMatchesFilePath, summaryFilePath = GetOutputPaths()

#adjustments if we're running in parallel...
if(_parallelID != ""):
    newStarterPack = _staterPackPath + _parallelID
    if os.path.exists(newStarterPack):
        shutil.rmtree(newStarterPack)
    copy(_staterPackPath, newStarterPack, 'match-logs')
    _staterPackPath = newStarterPack
    _matchLogsPath += _parallelID

#run matches
grid_summary = GetSummaryGrid()

endIndex = 1
if _playall is True:
    endIndex = len(_bots)-1
    
while True:
    for i in range(0, endIndex):
        for j in range(i + 1, len(_bots)):
    
            print(_bots[i] + " " + _bots[j])

            matchResult = runMatch(_bots[i], _bots[j])
            file = open(allMatchesFilePath, "a+")
            file.writelines(str(matchResult) + "\n")
            file.close()  
    
            if matchResult["winner"] == 'A':
                grid_summary[_bots[i]][_bots[j]]["wins"] += 1
                grid_summary[_bots[j]][_bots[i]]["losses"] += 1
            elif matchResult["winner"] == 'B':
                grid_summary[_bots[i]][_bots[j]]["losses"] += 1
                grid_summary[_bots[j]][_bots[i]]["wins"] += 1
            elif matchResult["winner"] == 'tie':
                grid_summary[_bots[i]][_bots[j]]["draws"] += 1
                grid_summary[_bots[j]][_bots[i]]["draws"] += 1

    WriteSummary(grid_summary, _bots, summaryFilePath)