#!/usr/bin/env python3
import argparse, os, math
from PIL import Image
from ndspy import lz10

parser = argparse.ArgumentParser("stork-message")
parser.add_argument("filename",help="mespack.mes file")
parser.add_argument("-e","--extract",help="Hexadecimal ID of message block")
parser.add_argument("-b","--bitmap",help="BMP file to repack")
parser.add_argument("-m","--mid",help="Message ID to repack")

def readUint32(data: bytearray, start: int) -> int:
    return (data[start+3] << 24) + (data[start+2] << 16) + (data[start+1] << 8) + data[start]

def readUint16(data: bytearray, start: int) -> int:
    return (data[start+1] << 8) + data[start]

def uint16toVec(uint16: int) -> bytearray:
    byte1 = (uint16 >> 0) % 0x100
    byte2 = (uint16 >> 8) % 0x100
    result = bytearray()
    result.append(byte1)
    result.append(byte2)
    return result

def gbaPalColsToTuple(r: int, g: int, b: int) -> tuple:
    red = int(r*8.2)
    green = int(g*8.2)
    blue = int(b*8.2)
    return (red,green,blue)

def colorDistance(c1, c2):
    (r1,g1,b1) = c1
    if (len(c2) > 3):
        c2 = c2[0:3]
    (r2,g2,b2) = c2
    return math.sqrt((r1 - r2)**2 + (g1 - g2) ** 2 + (b1 - b2) **2)

codes = {
    0x0: gbaPalColsToTuple(0x08,0x03,0x12),
    0x1: gbaPalColsToTuple(0x00,0x00,0x00),
    0x2: gbaPalColsToTuple(0x06,0x04,0x03),
    0x3: gbaPalColsToTuple(0x0c,0x08,0x06),
    0x4: gbaPalColsToTuple(0x10,0x0b,0x08),
    0x5: gbaPalColsToTuple(0x12,0x0d,0x09),
    0x6: gbaPalColsToTuple(0x03,0x09,0x06),
    0x7: gbaPalColsToTuple(0x07,0x0d,0x0b),
    0x8: gbaPalColsToTuple(0x09,0x0f,0x0d),
    0x9: gbaPalColsToTuple(0x0b,0x11,0x0f),
    0xa: gbaPalColsToTuple(0x0e,0x13,0x11),
    0xb: gbaPalColsToTuple(0x12,0x16,0x14),
    0xc: gbaPalColsToTuple(0x16,0x18,0x17),
    0xd: gbaPalColsToTuple(0x18,0x18,0x18),
    0xe: gbaPalColsToTuple(0x1c,0x1d,0x1c),
    0xf: gbaPalColsToTuple(0x1f,0x1f,0x1f)
}

def colorTupleToPal(colorTup: tuple) -> int:
    gbaColors = list(codes.values())
    closestColors = sorted(gbaColors, key=lambda color: colorDistance(color, colorTup))
    closest = closestColors[0]
    return gbaColors.index(closest)

def fourBppToCol(bpp: int) -> tuple:
    if bpp in codes:
        return codes[bpp]
    return (255,0,0)

HEADER_INDEX_ENGLISH = 1 # multiplied by 2 later because 2 byte values
BASE_POINTERS_LENGTH = 0x388 # End of this starts the actual data
LANGUAGE_SELECT_HEADER_SIZE = 0xc
IMAGE_WIDTH = 80*2 # 80 BYTES wide, but this is 4bits per pixel, not 8 bits
IMAGE_HEIGHT = 0x60

def bmpToVector(img: Image.Image, whichPanel: int) -> bytearray:
    result = bytearray()
    pixels = img.load()
    curPixelIndex = 0
    curByteBuild = -1
    yOffset = whichPanel*IMAGE_HEIGHT
    for y in range(0+yOffset,IMAGE_HEIGHT+yOffset):
        for x in range(0,IMAGE_WIDTH):
            curCol = pixels[x,y]
            palCol = colorTupleToPal(curCol)
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
        bMessageIdStr = args.mid
        if bMessageIdStr == None:
            print("No repack message ID (-m)")
            exit(1)
        bMid = int(bMessageIdStr,base=16)
        bmpFile = Image.open(args.bitmap)
        imageHeight = bmpFile.height
        frameCount = int(imageHeight / IMAGE_HEIGHT)
        writeLoc = getMessageLocation(mespack,bMid,True)
        for whichPanel in range(0,frameCount):
            vec = bmpToVector(bmpFile,whichPanel)
            #print(vec)
            compressedVector = bytearray(lz10.compress(vec))
            writeVec = uint16toVec(len(compressedVector))
            hasMoreVec = uint16toVec(1)
            if (whichPanel + 1 == frameCount):
                hasMoreVec = uint16toVec(0)
            writeVec.extend(hasMoreVec)
            writeVec.extend(compressedVector)
            writeBytes(mespack,writeVec,writeLoc)
            writeLoc += len(writeVec)
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
        generate(mespack, messageInt, True) # 3 is 1-1's first block