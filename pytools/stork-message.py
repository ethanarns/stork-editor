#!/usr/bin/env python3
import argparse
from PIL import Image
from ndspy import lz10

parser = argparse.ArgumentParser("stork-message")
parser.add_argument("filename",help="mespack.mes file")
parser.add_argument("-e","--extract",help="Hexadecimal ID of message block")

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

HEADER_INDEX_ENGLISH = 1 # multiplied by 2 later because 2 byte values
BASE_POINTERS_LENGTH = 0x388 # End of this starts the actual data
LANGUAGE_SELECT_HEADER_SIZE = 0xc
IMAGE_WIDTH = 80*2 # 80 BYTES wide, but this is 4bits per pixel, not 8 bits
IMAGE_HEIGHT = 0x60

def generate(filename: str,messageId: int):
    if (filename.endswith("mespack.mes") == False):
        print("Warning: file is not called mespack.mes")
    if messageInt < 0:
        print("ERROR: messageInt cannot be negative")
        return
    mespack = bytearray(open(filename,'rb').read())
    # 020ccdc8
    index = 0
    messageTarget = 0
    maxCount = readUint32(mespack,0)
    shouldBreak = False
    while (index <= maxCount or shouldBreak == False):
        checkLoc = index * 4
        checkValue = readUint16(mespack,checkLoc)
        if (checkValue == messageId):
            break
        if (checkValue == 0xffff):
            print("Message search overflow")
            break
        index += 1
        messageTarget = messageTarget + readUint16(mespack,checkLoc + 2)
    messageLocation = BASE_POINTERS_LENGTH + messageTarget
    headerVector = mespack[messageLocation:messageLocation+LANGUAGE_SELECT_HEADER_SIZE]
    # The header is a bunch of offsets pertaining to each language, 6 different ones, 0 being Japanese...
    messageLocation += readUint16(headerVector,HEADER_INDEX_ENGLISH*2) #020ccf7c
    hasMore = 1
    byteVector = bytearray()
    numPulled = 0
    while hasMore != 0:
        #print(hex(messageLocation))
        length = readUint16(mespack,messageLocation)
        #print(hex(length))
        messageLocation += 2
        hasMore = readUint16(mespack,messageLocation) # if hasMore is 0, end
        messageLocation += 2
        compressedVector = mespack[messageLocation:messageLocation+length]
        uncomped = lz10.decompress(compressedVector)
        byteVector.extend(uncomped)
        numPulled += 1
        messageLocation += length

    # create blank image
    baseImg = Image.new("RGB",(IMAGE_WIDTH,IMAGE_HEIGHT*numPulled),"black")
    pixels = baseImg.load()

    # load pixels
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
    messageId = args.extract
    if messageId == None:
        print("No message ID")
        exit(1)
    if str(messageId).lower() == "all":
        print("Extracting all")
        exit(0)
    messageInt = int(messageId,base=16)
    print(hex(messageInt))
    generate(filename, messageInt) # 3 is 1-1's first block