#!/usr/bin/env python3
import argparse, os

parser = argparse.ArgumentParser("analyzeCrsb")
parser.add_argument("filedir")

def readUint32(data: bytearray, start: int) -> int:
    return (data[start+3] << 24) + (data[start+2] << 16) + (data[start+1] << 8) + data[start]

def readUint16(data: bytearray, start: int) -> int:
    return (data[start+1] << 8) + data[start]

def ind(indent: int) -> str:
    result = "  " # pre-pad two spaces
    for _ in range(0,indent):
        result += "  " # two spaces
    return result

def handleCrsb(filename: str):
    print(filename)
    crsb = bytearray(open(filename,"rb").read())
    index = 0
    mainMagic = crsb[index:index+4].decode("ascii")
    if (mainMagic != "CRSB"):
        print("ERROR: not CRSB")
        exit(1)
    index += 8
    fileCount = readUint32(crsb,index)
    index += 4
    for x in range(0,fileCount):
        subMagic = crsb[index:index+4].decode("ascii")
        if (subMagic != "CSCN"):
            print("ERROR not CSCN: " + subMagic)
            exit(1)
        cscnLen = readUint32(crsb,index+4)
        indexOfCString = index + 12
        print(ind(1) + crsb[indexOfCString:indexOfCString+10].decode("ascii"))
        index += cscnLen + 8


if __name__ == "__main__":
    args = parser.parse_args()
    fileDir = args.filedir
    for root, dir, files in os.walk(fileDir):
        for file in files:
            if str(file).endswith('.crsb'):
                handleCrsb(root + file)