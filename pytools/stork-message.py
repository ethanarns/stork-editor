#!/usr/bin/env python3
import argparse
from PIL import Image
from ndspy import lz10

parser = argparse.ArgumentParser("stork-message")
parser.add_argument("filename",help="mespack.mes file")

def readUint32(data: bytearray, start: int) -> int:
    return (data[start+3] << 24) + (data[start+2] << 16) + (data[start+1] << 8) + data[start]

def readUint16(data: bytearray, start: int) -> int:
    return (data[start+1] << 8) + data[start]

# Change this around to support greater color set eventually
def fourBppToCol(bpp: int) -> tuple:
    if (bpp == 0x8):
        return (72,120,104) # The green
    elif (bpp == 0xf):
        return (255,255,255) # White
    else:
        return (200,200,200)

def generate(filename: str,messageId: int):
    if (filename.endswith("mespack.mes") == False):
        print("Warning: file is not called mespack.mes")
    mespack = bytearray(open(filename,'rb').read())
    # 020ccdc8
    index = 0
    dest = 0
    messageTarget = 0
    maxCount = readUint32(mespack,dest)
    shouldBreak = False
    while (index <= maxCount or shouldBreak == False):
        checkLoc = index * 4
        checkValue = readUint16(mespack,dest+index*4)
        if (checkValue == messageId):
            break
        if (checkValue == 0xffff):
            print("Message search overflow")
            break
        index += 1
        messageTarget = messageTarget + readUint16(mespack,dest + checkLoc + 2)
    print(hex(messageTarget))
    messageLocation = 0x388 + messageTarget
    print(hex(messageLocation))
    headerVector = mespack[messageLocation:messageLocation+0xC]
    messageLocation += readUint16(headerVector,2) #020ccf7c
    length = readUint32(mespack,messageLocation)
    messageLocation += 4
    compressedVector = mespack[messageLocation:messageLocation+length]
    byteVector = bytearray(lz10.decompress(compressedVector))
    print("uncomped size: " + hex(len(byteVector)))
    IMAGE_WIDTH = 80*2 # 80 BYTES wide, but this is 4bits per pixel, not 8 bits
    # create blank image
    baseImg = Image.new("RGB",(IMAGE_WIDTH,800),"black")
    pixels = baseImg.load()

    # load pixels
    byteIndex = 0
    fourBitIndex = 0
    for colsByte in byteVector:
        lowBit = colsByte % 0x10
        x = fourBitIndex % IMAGE_WIDTH
        y = int(fourBitIndex / IMAGE_WIDTH)
        pixels[x,y] = fourBppToCol(lowBit)
        fourBitIndex += 1
        highBit = colsByte >> 4
        x = fourBitIndex % IMAGE_WIDTH
        y = int(fourBitIndex / IMAGE_WIDTH)
        pixels[x,y] = fourBppToCol(highBit)
        fourBitIndex += 1

    baseImg.show()
    print("Done!")

if __name__ == '__main__':
    args = parser.parse_args()
    filename = args.filename
    generate(filename, 3) # 1-1's first block