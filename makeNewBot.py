#!/usr/bin/env python3

import platform
import os
import sys
import shutil
import time
import datetime
import glob

#creates a new bot folder and updates stuff with the new bot name

def copy(src, dest, ignore_pattern):
    try:
        shutil.copytree(src, dest, ignore=shutil.ignore_patterns(ignore_pattern))
    except OSError as e:
        # If the error was caused because the source wasn't a directory
        if e.errno == shutil.errno.ENOTDIR:
            shutil.copy(src, dest)
        else:
            print('Directory not copied. Error: %s' % e)

def EditFile(fileName, searchPattern, replacement):
    file = open(fileName, "r")
    lines = file.readlines()
    file.close()

    for i, line in enumerate(lines):
        if(searchPattern in line):
            lines[i] = replacement

    file = open(fileName, "w")
    file.writelines(lines)
    file.close()

#the script  ###############################################################################

# copy latest jp folder
# - up version in bot.json
# - up version in command.cpp
# - add descriptions to runs file
# - tag commit

# copy latest jp folder
jpBotFolderBase = "./bots/jp"
botNumber = 1
jpBotFolder = jpBotFolderBase + str(botNumber)
oldJpBotFolder = jpBotFolder

while os.path.exists(jpBotFolder):
    oldJpBotFolder = jpBotFolder
    botNumber += 1
    jpBotFolder = jpBotFolderBase + str(botNumber)

print("making new bot ", jpBotFolder, " from ", oldJpBotFolder)
copy(oldJpBotFolder, jpBotFolder, 'rounds')

botName = "JP" + str(botNumber)

# - up version in bot.json
botJson = jpBotFolder + "/bot.json"
EditFile(botJson, "nickName", "    \"nickName\": \"" + botName + "\",\n")

# - up version in command.cpp
commandCpp = "./src/GameEngine/Commands/Command.cpp"
EditFile(commandCpp, "latestBot", "const std::string Command::latestBot = \"" + botName + "\";\n")

# - add descriptions to runs file
# - tag commit