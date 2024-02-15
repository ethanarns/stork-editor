#!/usr/bin/env python3
from genericpath import isfile
from time import perf_counter
import ndspy.lz10
import argparse, os

from pkg_resources import UnknownExtra

parser = argparse.ArgumentParser("analyzer")
parser.add_argument("filename")

def readUint32(data: bytearray, start: int) -> int:
    return (data[start+3] << 24) + (data[start+2] << 16) + (data[start+1] << 8) + data[start]

def readUint16(data: bytearray, start: int) -> int:
    return (data[start+1] << 8) + data[start]

def ind(indent: int) -> str:
    result = "  " # pre-pad two spaces
    for _ in range(0,indent):
        result += "  " # two spaces
    return result

def handleSCEN(data: bytearray, index: int, stop: int) -> None:
    # Count debugs
    instructionsSoFar = []
    while index < stop:
        scenMagic = data[index:index+4].decode("ascii")
        index += 4
        scenLength = readUint32(data,index)
        index += 4
        print(ind(2) + scenMagic + " (length = " + hex(scenLength) + ")")
        if scenLength == 0:
            print(ind(3) + "Empty")
            index += scenLength # Jump to next
            continue
        tempIndex = index + 0 # Just in case it's wrong
        if scenMagic in instructionsSoFar and scenMagic != "PLAN":
            print("ERROR: DUPLICATE INSTRUCTIONS! MULTIPLES OF " + scenMagic)
        else:
            instructionsSoFar.append(scenMagic)
        if scenMagic == "INFO":
            layerWidth = readUint16(data,tempIndex)
            tempIndex += 2
            print(ind(3) + "Layer Width: " + hex(layerWidth) + "/" + str(layerWidth))

            layerHeight = readUint16(data,tempIndex)
            tempIndex += 2
            print(ind(3) + "Layer Height: " + hex(layerHeight) + "/" + str(layerHeight))
            
            bgYoffset = readUint32(data,tempIndex)
            tempIndex += 4
            print(ind(3) + "BG Y Offset: " + hex(bgYoffset))

            xScrollOffset = readUint32(data, tempIndex)
            tempIndex += 4
            print(ind(3) + "X Scroll Offset: " + hex(xScrollOffset) + "/" + str(xScrollOffset))

            yScrollOffset = readUint32(data, tempIndex)
            tempIndex += 4
            print(ind(3) + "Y Scroll Offset: " + hex(yScrollOffset) + "/" + str(yScrollOffset))

            whichBackground = data[tempIndex]
            tempIndex += 1
            print(ind(3) + "Which Background: " + str(whichBackground))

            layerOrder = data[tempIndex]
            tempIndex += 1
            print(ind(3) + "Layer Order: " + str(layerOrder))

            unkThird = data[tempIndex]
            tempIndex += 1
            print(ind(3) + "Unknown 3rd Byte: " + hex(unkThird))

            screenBaseBlockMaybe = data[tempIndex]
            tempIndex += 1
            print(ind(3) + "Base Block (Maybe): " + hex(screenBaseBlockMaybe))

            colorMode = readUint32(data, tempIndex)
            tempIndex += 4
            print(ind(3) + "Color Mode: " + hex(colorMode))

            if tempIndex == index + scenLength:
                print(ind(3) + "IMBZ filename: N/A (end reached)")
            else:
                foundZero = False
                strBytes = bytearray()
                while foundZero == False:
                    if (data[tempIndex] == 0x0):
                        foundZero = True
                    else:
                        charByte = data[tempIndex]
                        strBytes.append(charByte)
                    tempIndex += 1
                try:
                    finishedString = strBytes.decode("ascii")
                    print(ind(3) + "IMBZ filename: " + finishedString)
                    # Padding
                    while tempIndex % 4 != 0:
                        tempIndex += 1
                except UnicodeDecodeError:
                    print("ERROR: Failed to decode:")
                    print(strBytes)
                except:
                    print("ERROR: Unknown exception when getting IMBZ filename")
        elif scenMagic == "ANMZ":
            #print(ind(3) + "Decompressing...")
            compressedAnmzBytes = data[tempIndex:tempIndex+scenLength]
            anmz = bytearray(ndspy.lz10.decompress(compressedAnmzBytes))
            print(ind(3) + "Decompressed to " + hex(len(anmz)) + " bytes")
            anmzIndex = 0
            frameCount = anmz[anmzIndex]
            anmzIndex += 1
            print(ind(3) + "Frame count: " + str(frameCount))
            anmzUnk1 = anmz[anmzIndex]
            anmzIndex += 1
            print(ind(3) + "Unknown 1: " + hex(anmzUnk1))
            anmzUnk2 = readUint16(anmz,anmzIndex)
            anmzIndex += 2
            print(ind(3) + "Unknown 2: " + hex(anmzUnk2))
            anmzVramOffset = readUint16(anmz,anmzIndex)
            anmzIndex += 2
            print(ind(3) + "VRAM Offset: " + hex(anmzVramOffset))

            anmzUnknown3 = anmz[anmzIndex]
            anmzIndex += 1
            print(ind(3) + "Unknown 3: " + hex(anmzUnknown3))

            anmzUnk4 = anmz[anmzIndex]
            anmzIndex += 1
            print(ind(3) + "Unknown 4: " + hex(anmzUnk4))

            frameIndex = 0
            while frameIndex < frameCount:
                frameHold = anmz[anmzIndex]
                anmzIndex += 1
                frameIndex += 1
                print(ind(3) + "Frame " + str(frameIndex) + " hold length: " + str(frameHold) + "/" + hex(frameHold))
            # Now skip to end of padding
            while anmzIndex % 4 != 0:
                anmzIndex += 1
            remainingBytes = len(anmz) - anmzIndex
            if remainingBytes % 0x20 != 0:
                print("FRROR: REMAINING FRAME BYTES NOT DIVISIBLE BY 0x20: " + hex(remainingBytes))
            tilesRemaining = int(remainingBytes / 0x20)
            print(ind(3) + "32 Byte Tiles: " + str(tilesRemaining) + " (" + hex(tilesRemaining) + ")")
            if tilesRemaining % frameCount != 0:
                print("ERROR: TILES NOT DIVISIBLE BY FRAME COUNT")
            perFrame = int(tilesRemaining / frameCount)
            print(ind(3) + "Tiles per frame: " + str(perFrame) + " (" + hex(perFrame) + ")")
            tempIndex += scenLength
        elif scenMagic == "IMGB":
            # Uncompressed!
            if scenLength % 0x20 != 0:
                print("ERROR: TILES NOT DIVISIBLE BY 0x20")
            imgbTileCount = int(scenLength / 0x20)
            print(ind(3) + "32 byte tile count: " + str(imgbTileCount) + "/" + hex(imgbTileCount))
            tempIndex += scenLength
        elif scenMagic == "PLTB":
            # An uncompressed list of palettes, each of which is 0x20
            if scenLength % 0x20 != 0:
                print("PRROR: PALETTES NOT DIVISIBLE BY 0x20")
            pltbPaletteCount = int(scenLength / 0x20)
            print(ind(3) + "Palette count: " + str(pltbPaletteCount) + "/" + hex(pltbPaletteCount))
            tempIndex += scenLength
        elif scenMagic == "COLZ":
            compressedColzBytes = data[tempIndex:tempIndex+scenLength]
            colz = bytearray(ndspy.lz10.decompress(compressedColzBytes))
            print(ind(3) + "Decompressed to " + hex(len(colz)) + " collision records")
            tempIndex += scenLength
        elif scenMagic == "MPBZ":
            compressedMpbzBytes = data[tempIndex:tempIndex+scenLength]
            mpbz = bytearray(ndspy.lz10.decompress(compressedMpbzBytes))
            print(ind(3) + "Decompressed to " + hex(len(mpbz)) + " bytes")
            recordCount = int(len(mpbz) / 2)
            print(ind(3) + "Map tile records: " + str(recordCount) + "/" + hex(recordCount))
            tempIndex += scenLength
        elif scenMagic == "IMBZ":
            compressedImbzBytes = data[tempIndex:tempIndex+scenLength]
            imbz = bytearray(ndspy.lz10.decompress(compressedImbzBytes))
            print(ind(3) + "Decompressed to " + hex(len(imbz)) + " bytes")
            if len(imbz) % 0x20 != 0:
                print("ERROR: IMBZ NOT DIVISIBLE BY 0x20")
            characterCount = int(len(imbz) / 0x20)
            print(ind(3) + "4bit character count: " + str(characterCount) + " / " + hex(characterCount))
            tempIndex += scenLength
        else:
            tempIndex += scenLength
            print("Unknown sub-SCEN")

        index += scenLength # Jump to next
        if tempIndex != index:
            print("ERROR: Failed to match sub-SCEN length: " + hex(tempIndex) + " vs " + hex(index))
            pass

def handleGRAD(data: bytearray, index: int, stop: int):
    while index < stop:
        gradMagic = data[index:index+4].decode("ascii")
        index += 4
        gradLength = readUint32(data,index)
        index += 4
        print(ind(2) + gradMagic + " (length = " + hex(gradLength) + ")")
        tempIndex = index + 0 # Just in case it's wrong
        if gradMagic == "GINF":
            tempIndex += 1
            tempIndex += 1
            tempIndex += 1
            tempIndex += 1
            unk1 = readUint16(data,tempIndex)
            tempIndex += 2
            print(ind(3) + "Unknown 1: " + hex(unk1))
            tempIndex += 1
            tempIndex += 1
            yOffset = readUint32(data,tempIndex)
            tempIndex += 4
            print(ind(3) + "Y Offset: " + hex(yOffset))
        elif gradMagic == "GCOL":
            # Uncompressed
            if gradLength % 2 != 0:
                print("ERROR: GCOL MUST BE EVEN")
            numberOfColorRecords = int(gradLength / 2)
            print(ind(3) + "Gradient Color Records: " + str(numberOfColorRecords) + " / " + hex(numberOfColorRecords))
            tempIndex += gradLength
        else:
            print("ERROR: Unknown sub-GRAD")
        index += gradLength
        if tempIndex != index:
            print("ERROR: Failed to match sub-SCEN length: " + hex(tempIndex) + " vs " + hex(index))
            pass

def handleSETD(data: bytearray, index: int, stop: int):
    objectCount = 0
    while index < stop:
        objectId = readUint16(data,index)
        index += 2
        objectLength = readUint16(data,index)
        index += 2
        objectXdata = readUint16(data,index)
        index += 2
        objectYdata = readUint16(data,index)
        index += 2
        # Now skip ahead, length is settings lenth in bytes
        index += objectLength
        objectCount += 1
    print(ind(2) + "Object count: " + str(objectCount) + " / " + hex(objectCount))

def handleMpdz(filename):
    print(filename)
    mpdz = bytearray(ndspy.lz10.decompressFromFile(filename))
    # Each time, make an decompressed copy
    ndspy.lz10.decompressToFile(open(filename,"rb").read(),str(filename).replace("mpdz","mpdb"))
    readIndex = 0
    magicNum = mpdz[readIndex:readIndex+3].decode("ascii")
    if magicNum != "SET":
        print("ERROR: Bad magic number, should be 'SET', was '" + magicNum + "'")
        return
    
    readIndex += 4
    mpdzLengthUncompressed = readUint32(mpdz,readIndex)
    print(ind(0) + "SET_ (length = " + hex(mpdzLengthUncompressed) + " bytes)")
    readIndex += 4
    # Start SCEN/GRAD/SETD/etc loop
    scenCount = 0
    setdCount = 0

    topLoopStop = mpdzLengthUncompressed + readIndex
    while readIndex < topLoopStop:
        topMagic = mpdz[readIndex:readIndex+4].decode("ascii")
        readIndex += 4
        topLength = readUint32(mpdz,readIndex)
        readIndex += 4
        print(ind(1) + topMagic + " (length = " + hex(topLength) + " bytes)")
        if topMagic == "SCEN":
            handleSCEN(mpdz,readIndex+0,readIndex+topLength)
            scenCount += 1
        elif topMagic == "GRAD":
            handleGRAD(mpdz,readIndex+0,readIndex+topLength)
        elif topMagic == "SETD":
            handleSETD(mpdz,readIndex+0,readIndex+topLength)
            setdCount += 1
        else:
            print("Unhandled top-length instruction")
        readIndex += topLength
    # Not hit!
    if scenCount > 3:
        print("ERROR: UNUSUAL SCEN COUNT: " + str(scenCount))
    if setdCount > 1:
        print("ERROR: UNUSUAL SETD COUNT: " + str(setdCount))
    

if __name__ == "__main__":
    args = parser.parse_args()
    filename = args.filename
    if os.path.isfile(filename):
        handleMpdz(filename)
    else:
        for root, dir, files in os.walk(filename):
            for file in files:
                if str(file).endswith('.mpdz'):
                    handleMpdz(root + file)