#!/usr/bin/python
# This is a modified version of the script posted by mchahn here: https://community.platformio.org/t/spiffs-file-system-shortcut-auto-upload/8511/7
Import ("env")
import os

def update_spiffs(source, target, env):
    dataDirPath  = "data"
    timeFilePath = "spiffs-timesum.txt"

    if(not os.path.exists(timeFilePath)):
        timeFile = open(timeFilePath, "w")
        timeFile.write("0.0")
        timeFile.close()

    timeFile = open(timeFilePath, "r")
    oldTime = float(timeFile.read())
    timeFile.close()

    newTime = float((sum(os.path.getmtime(os.path.join(dataDirPath, file_name)) for file_name in os.listdir(dataDirPath))))

    if newTime != oldTime:
        print("\nfile updated in data directory -- building/uploading spiffs image\n")
        env.Execute('pio run -t uploadfs -e ' + env['PIOENV'])

        timeFile = open(timeFilePath, "w")
        timeFile.write(str(newTime))
        timeFile.close()

env.AddPreAction("upload", update_spiffs)
