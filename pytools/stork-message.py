#!/usr/bin/env python3
import argparse, os
from PIL import Image
from ndspy import lz10

parser = argparse.ArgumentParser("stork-message")
parser.add_argument("filename",help="mespack.mes file")
parser.add_argument("-e","--extract",help="Hexadecimal ID of message block")
parser.add_argument("-b","--bitmap",help="BMP file to repack")

def readUint32(data: bytearray, start: int) -> int:
    return (data[start+3] << 24) + (data[start+2] << 16) + (data[start+1] << 8) + data[start]

def readUint16(data: bytearray, start: int) -> int:
    return (data[start+1] << 8) + data[start]

def gbaPalColsToTuple(r: int, g: int, b: int) -> tuple:
    red = int(r*8.2)
    green = int(g*8.2)
    blue = int(b*8.2)
    return (red,green,blue)

def colorTupleToGbaNum(colorTup: tuple) -> int:
    if (colorTup == gbaPalColsToTuple(0x08,0x03,0x12)):
        return 0x0
    elif (colorTup == gbaPalColsToTuple(0x00,0x00,0x00)):
        return 0x1
    elif (colorTup == gbaPalColsToTuple(0x06,0x04,0x03)):
        return 0x2
    elif (colorTup == gbaPalColsToTuple(0x0c,0x08,0x06)):
        return 0x3
    elif (colorTup == gbaPalColsToTuple(0x10,0x0b,0x08)):
        return 0x4
    elif (colorTup == gbaPalColsToTuple(0x12,0x0d,0x09)):
        return 0x5
    elif (colorTup == gbaPalColsToTuple(0x03,0x09,0x06)):
        return 0x6
    elif (colorTup == gbaPalColsToTuple(0x07,0x0d,0x0b)):
        return 0x7
    elif (colorTup == gbaPalColsToTuple(0x09,0x0f,0x0d)):
        return 0x8 # The green
    elif (colorTup == gbaPalColsToTuple(0x0b,0x11,0x0f)):
        return 0x9
    elif (colorTup == gbaPalColsToTuple(0x0e,0x13,0x11)):
        return 0xa
    elif (colorTup == gbaPalColsToTuple(0x12,0x16,0x14)):
        return 0xb
    elif (colorTup == gbaPalColsToTuple(0x16,0x18,0x17)):
        return 0xc
    elif (colorTup == gbaPalColsToTuple(0x18,0x18,0x18)):
        return 0xd
    elif (colorTup == gbaPalColsToTuple(0x1c,0x1d,0x1c)):
        return 0xe
    elif (colorTup == gbaPalColsToTuple(0x1f,0x1f,0x1f)):
        return 0xf # White
    else:
        return 0x0 # error

def fourBppToCol(bpp: int) -> tuple:
    if (bpp == 0x0):
        return gbaPalColsToTuple(0x08,0x03,0x12)
    elif (bpp == 0x1):
        return gbaPalColsToTuple(0,0,0)
    elif (bpp == 0x2):
        return gbaPalColsToTuple(0x06,0x04,0x03)
    elif (bpp == 0x3):
        return gbaPalColsToTuple(0x0c,0x08,0x06)
    elif (bpp == 0x4):
        return gbaPalColsToTuple(0x10,0x0b,0x08)
    elif (bpp == 0x5):
        return gbaPalColsToTuple(0x12,0x0d,0x09)
    elif (bpp == 0x6):
        return gbaPalColsToTuple(0x03,0x09,0x06)
    elif (bpp == 0x7):
        return gbaPalColsToTuple(0x07,0x0d,0x0b)
    elif (bpp == 0x8):
        return gbaPalColsToTuple(0x09,0x0f,0x0d) # The green
    elif (bpp == 0x9):
        return gbaPalColsToTuple(0x0b,0x11,0x0f)
    elif (bpp == 0xa):
        return gbaPalColsToTuple(0x0e,0x13,0x11)
    elif (bpp == 0xb):
        return gbaPalColsToTuple(0x12,0x16,0x14)
    elif (bpp == 0xc):
        return gbaPalColsToTuple(0x16,0x18,0x17)
    elif (bpp == 0xd):
        return gbaPalColsToTuple(0x18,0x18,0x18)
    elif (bpp == 0xe):
        return gbaPalColsToTuple(0x1c,0x1d,0x1c)
    elif (bpp == 0xf):
        return gbaPalColsToTuple(0x1f,0x1f,0x1f) # White
    else:
        return (255,0,0) # red

HEADER_INDEX_ENGLISH = 1 # multiplied by 2 later because 2 byte values
BASE_POINTERS_LENGTH = 0x388 # End of this starts the actual data
LANGUAGE_SELECT_HEADER_SIZE = 0xc
IMAGE_WIDTH = 80*2 # 80 BYTES wide, but this is 4bits per pixel, not 8 bits
IMAGE_HEIGHT = 0x60

def bmpToVector(img: Image.Image) -> bytearray:
    result = bytearray()
    pixels = img.load()
    curPixelIndex = 0
    curByteBuild = -1
    for x in range(0,IMAGE_WIDTH):
        for y in range(0,IMAGE_HEIGHT):
            curCol = pixels[x,y]
            palCol = colorTupleToGbaNum(curCol)
            if curPixelIndex % 2 == 0:
                curByteBuild = palCol
            else:
                curByteBuild += palCol << 4
                result.append(curByteBuild)
                # it is odd
            curPixelIndex += 1
    return result

def getMessageLocation(mespack: bytearray, messageId: int, printOverflow: bool = False, languageIndex: int = HEADER_INDEX_ENGLISH) -> int:
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
            if printOverflow:
                print("Message search overflow: " + hex(messageId))
            return 0
        index += 1
        messageTarget = messageTarget + readUint16(mespack,checkLoc + 2)
    messageLocation = BASE_POINTERS_LENGTH + messageTarget
    headerVector = mespack[messageLocation:messageLocation+LANGUAGE_SELECT_HEADER_SIZE]
    # The header is a bunch of offsets pertaining to each language, 6 different ones, 0 being Japanese...
    messageLocation += readUint16(headerVector,languageIndex*2) #020ccf7c
    return messageLocation

def generate(mespack: bytearray,messageId: int, show: bool, languageIndex: int = HEADER_INDEX_ENGLISH):
    if messageId < 0:
        print("ERROR: messageInt cannot be negative")
        return
    messageLocation = getMessageLocation(mespack,messageId,show)
    if messageLocation == 0:
        if show:
            print("Invalid message location")
        return None
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
    if show == True:
        baseImg.show()
    return baseImg

def writeBytes(baseData: bytearray, toWrite: bytearray, location: int):
    for b in toWrite:
        baseData[location] = b
        location += 1

if __name__ == '__main__':
    args = parser.parse_args()
    filename = args.filename
    if (filename.endswith("mespack.mes") == False):
        print("Not a mespack.mes file")
        exit(1)
    mespack = bytearray(open(filename,'rb').read())
    messageId = args.extract
    if (args.bitmap != None):
        bmpFile = Image.open(args.bitmap)
        vec = bmpToVector(bmpFile)
        compressedVector = bytearray(lz10.compress(vec))
        messageInt = 0xa
        writeLoc = getMessageLocation(mespack,messageInt,True)
        writeBytes(mespack,compressedVector,writeLoc)
        o = open(filename,"wb")
        o.write(mespack)
        o.close()
        exit(0)
    if str(messageId).lower() == "all":
        print("Extracting all")
        parentDir = os.path.abspath(os.path.dirname(__file__))
        folderPath = os.path.join(parentDir,"mespack_files")
        if not os.path.exists(folderPath):
            os.mkdir(folderPath)
        for x in range(0,1502):
            curImage = generate(mespack, x, False)
            if (curImage == None):
                continue
            # Don't use hex for file names
            imageFileName = "msg_" + str(x).zfill(4) + "_" + hex(x) + ".bmp"
            imageFilePath = os.path.join(folderPath,imageFileName)
            curImage.save(imageFilePath,"bmp")
    else:
        messageInt = int(messageId,base=16)
        print(hex(messageInt))
        generate(mespack, messageInt, True) # 3 is 1-1's first block