#!/usr/bin/python
Import ("env")
import os
import sys

def main():
    input = "otapass.txt"

    if not os.path.exists(input) or not os.path.isfile(input):
        print(f"Error: {input} does not exist.")
        sys.exit(1)

    password = ""

    with open(input) as f:
        password = f.readline()

    env.Append(UPLOADERFLAGS=["-a", password])

main()
