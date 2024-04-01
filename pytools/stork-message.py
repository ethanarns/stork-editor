#!/usr/bin/env python3
import argparse

parser = argparse.ArgumentParser("stork-message")
parser.add_argument("filename",help="mespack.mes file")

def readUint32(data: bytearray, start: int) -> int:
    return (data[start+3] << 24) + (data[start+2] << 16) + (data[start+1] << 8) + data[start]

def readUint16(data: bytearray, start: int) -> int:
    return (data[start+1] << 8) + data[start]

def main(filename):
    if (filename != "mespack.mes"):
        print("Warning: file is not called mespack.mes")
    mespack = bytearray(open(filename,'rb').read())
    index = 0
    dest = 0
    messageTarget = 0
    maxCount = readUint32(mespack,dest)
    print(hex(maxCount))

if __name__ == '__main__':
    args = parser.parse_args()
    filename = args.filename
    main(filename)