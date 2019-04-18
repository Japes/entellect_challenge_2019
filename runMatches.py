#!/usr/bin/env python3

import platform
import os
import sys

#script for running matches/tournaments using the official runner/scripts.
#goal is to mimic actual tournament running conditions as closely as possible

#put in bot names
gameRunnerConfigFile = "./starter-pack/game-runner-config.json"
file = open(gameRunnerConfigFile + ".original", "r")
lines = file.readlines()
file.close()

for i, line in enumerate(lines):
    if("player-a" in line):
        lines[i] = "\"player-a\": \"../bots/" + sys.argv[1] + "\",\n"
    if("player-b" in line):
        lines[i] = "\"player-b\": \"../bots/" + sys.argv[2] + "\",\n"


file = open(gameRunnerConfigFile, "w")
file.writelines(lines)
file.close()

os.chdir("starter-pack")
if(platform.system() == 'Windows'):
    os.system("run.bat")
elif(platform.system() == 'Linux'):
    os.system("make run | grep -v -e [a-z=]")
    os.system("make run")
