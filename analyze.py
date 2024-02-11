#!/usr/bin/env python3
import ndspy.lz10
import argparse

parser = argparse.ArgumentParser("analyzer")
parser.add_argument("filename")

def readUint32(data: bytearray, start: int) -> int:
    return (data[start+3] << 24) + (data[start+2] << 16) + (data[start+1] << 8) + data[start]

def ind(indent: int) -> str:
    result = ""
    for _ in range(0,indent):
        result += "  " # two spaces
    return result

def main():
    args = parser.parse_args()
    filename = args.filename
    mpdz = bytearray(ndspy.lz10.decompressFromFile(filename))
    #ndspy.lz10.decompressToFile(open(filename,"rb").read(),str(filename).replace("mpdz","mpd"))
    readIndex = 0
    magicNum = mpdz[readIndex:readIndex+3].decode("ascii")
    if magicNum != "SET":
        print("ERROR: Bad magic number, should be 'SET', was '" + magicNum + "'")
        return
    
    readIndex += 4
    mpdzLengthUncompressed = readUint32(mpdz,readIndex)
    print(ind(0) + "SET_ (length = " + hex(mpdzLengthUncompressed) + " bytes)")
    readIndex += 4
    # Start SCEN/GRAD/SETD loop
    topLoopStop = mpdzLengthUncompressed + readIndex
    while readIndex < topLoopStop:
        topMagic = mpdz[readIndex:readIndex+4].decode("ascii")
        print(ind(1) + topMagic)
        exit()
    

if __name__ == "__main__":
    main()