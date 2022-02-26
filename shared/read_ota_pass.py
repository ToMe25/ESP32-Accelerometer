#!/usr/bin/python
Import ("env")
import os

def main():
    input = os.path.join("data", "otapass.txt")

    if not os.path.exists(input) or not os.path.isfile(input):
        print(f"Error: {input} does not exist.")
        env.Exit(1)

    password = ""

    with open(input) as file:
        for line in file:
            trimmed = line.strip()
            if not trimmed[0] == '#':
                password = line
                break

    password = password.rstrip('\n')
    password = password.rstrip('\00') # C strings are 0 terminated so removing ending 0 bytes isn't an issue.

    env.Append(UPLOADERFLAGS=["-a", password])

main()
