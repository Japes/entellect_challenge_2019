#!/usr/bin/env python3
import sys
import os
import shutil

#script for packaging bot into zip for entellect

def copy(src, dest, ignore_pattern):
    print("cp ", src, " -> ", dest)
    try:
        shutil.copytree(src, dest, ignore=shutil.ignore_patterns(ignore_pattern))
    except OSError as e:
        # If the error was caused because the source wasn't a directory
        if e.errno == shutil.errno.ENOTDIR:
            shutil.copy(src, dest)
        else:
            print('Directory not copied. Error: %s' % e)

def updateMakefile(outputFolder):
    makefile = outputFolder + "makefile"
    print("Scanning makefile at ", makefile)
    file = open(makefile, "r")
    lines = file.readlines()
    file.close()


    for i, line in enumerate(lines):
        if(line == "MY_LIBS_SRC_DIR := ../../src\n"):
            print("making change to makefile...")
            lines[i] = "MY_LIBS_SRC_DIR := ./src\n"

    file = open(makefile, "w")
    file.writelines(lines)
    file.close()

def checkBuild(outputFolder):
    starting_path = os.getcwd()

    os.chdir(outputFolder)

    if(os.system("make clean") != 0):
        print("build cleanup failed...")
        exit()

    if(os.system("make -j8") != 0):
        print("build failed...")
        exit()

    if(os.system("make clean") != 0):
        print("build cleanup failed...")
        exit()

    os.chdir(starting_path)


############################################################################

if len(sys.argv) < 2:
    print("Give me a bot name: \"deployBot.py [BOTNAME]\"")

botFolder = "../bots/" + sys.argv[1]
outputFolder = "./" + sys.argv[1] + "_deployment/"
print("Packaging ", botFolder, " into ", outputFolder)

if os.path.exists(outputFolder):
    shutil.rmtree(outputFolder)

copy(botFolder, outputFolder, 'rounds')
copy("../src", outputFolder + "/src", 'unittests')

updateMakefile(outputFolder)

checkBuild(outputFolder)

zipName = outputFolder[:-1] + ".zip"
print("Zipping up to ", zipName)
print("zip -FSrq " + zipName + " " + outputFolder)
os.system("zip -FSrq " + zipName + " " + outputFolder)

shutil.rmtree(outputFolder)

print("done.")
 