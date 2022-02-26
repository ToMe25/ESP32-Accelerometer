#!/usr/bin/python
Import ("env")
import os

def main():
    input = "otapass.txt"

    if not os.path.exists(input) or not os.path.isfile(input):
        print(f"Error: {input} does not exist.")
        env.Exit(1)

    password = ""

    with open(input) as f:
        password = f.readline()

    password = password.strip('\00')
    env.Append(UPLOADERFLAGS=["-a", password])

main()
