#!/usr/bin/env python3
from time import perf_counter
import ndspy.lz10
import argparse

from pkg_resources import UnknownExtra

parser = argparse.ArgumentParser("analyzer")
parser.add_argument("filename")

def readUint32(data: bytearray, start: int) -> int:
    return (data[start+3] << 24) + (data[start+2] << 16) + (data[start+1] << 8) + data[start]

def readUint16(data: bytearray, start: int) -> int:
    return (data[start+1] << 8) + data[start]

def ind(indent: int) -> str:
    result = ""
    for _ in range(0,indent):
        result += "  " # two spaces
    return result

def handleSCEN(data: bytearray, index: int, stop: int) -> None:
    while index < stop:
        scenMagic = data[index:index+4].decode("ascii")
        index += 4
        scenLength = readUint32(data,index)
        index += 4
        print(ind(2) + scenMagic + " (length = " + hex(scenLength) + ")")
        tempIndex = index + 0 # Just in case it's wrong
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
            print(ind(3) + "Color Mode (Maybe): " + hex(colorMode))

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
                except UnicodeDecodeError:
                    print("Failed to decode:")
                    print(strBytes)
                except:
                    print("Unknown exception when getting IMBZ filename")
        elif scenMagic == "ANMZ":
            #print(ind(3) + "Decompressing...")
            compressedAnmzBytes = data[tempIndex:tempIndex+scenLength]
            anmz = bytearray(ndspy.lz10.decompress(compressedAnmzBytes))
            print(ind(3) + "Decompressed to " + hex(len(anmz)) + " bytes")
            anmzIndex = 0
            frameCount = anmz[anmzIndex]
            anmzIndex += 1
            print(ind(3) + "Frame count: " + str(frameCount))
            if frameCount > 5:
                # It would pass 0xC!
                print("UNUSUALLY HIGH FRAME COUNT: " + hex(frameCount))
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
            anmzIndex = 0xC
            remainingBytes = len(anmz) - anmzIndex
            if remainingBytes % 0x20 != 0:
                print("REMAINING BYTES NOT DIVISIBLE BY 0x20: " + hex(remainingBytes))
            tilesRemaining = int(remainingBytes / 0x20)
            print(ind(3) + "32 Byte Tiles: " + str(tilesRemaining) + " (" + hex(tilesRemaining) + ")")
            if tilesRemaining % frameCount != 0:
                print("TILES NOT DIVISIBLE BY FRAME COUNT")
            perFrame = int(tilesRemaining / frameCount)
            print(ind(3) + "Tiles per frame: " + str(perFrame) + " (" + hex(perFrame) + ")")
        elif scenMagic == "IMGB":
            # Uncompressed!
            if scenLength % 0x20 != 0:
                print("TILES NOT DIVISIBLE BY 0x20")
            imgbTileCount = int(scenLength / 0x20)
            print(ind(3) + "32 byte tile count: " + str(imgbTileCount) + "/" + hex(imgbTileCount))
            tempIndex += scenLength
        elif scenMagic == "PLTB":
            # An uncompressed list of palettes, each of which is 0x20
            if scenLength % 0x20 != 0:
                print("PALETTES NOT DIVISIBLE BY 0x20")
            pltbPaletteCount = int(scenLength / 0x20)
            print(ind(3) + "Palette count: " + str(pltbPaletteCount) + "/" + hex(pltbPaletteCount))
            tempIndex += scenLength
        elif scenMagic == "COLZ":
            pass
        elif scenMagic == "MPBZ":
            pass
        else:
            print("Unknown sub-SCEN")

        index += scenLength # Jump to next
        if tempIndex != index:
            #print("Failed to match sub-SCEN length: " + hex(tempIndex) + " vs " + hex(index))
            pass

def handleGRAD(data: bytearray, index: int, stop: int):
    pass

def handleSETD(data: bytearray, index: int, stop: int):
    pass

def main():
    args = parser.parse_args()
    filename = args.filename
    mpdz = bytearray(ndspy.lz10.decompressFromFile(filename))
    # ndspy.lz10.decompressToFile(open(filename,"rb").read(),str(filename).replace("mpdz","mpdb"))
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
        readIndex += 4
        topLength = readUint32(mpdz,readIndex)
        readIndex += 4
        print(ind(1) + topMagic + " (length = " + hex(topLength) + " bytes)")
        if topMagic == "SCEN":
            handleSCEN(mpdz,readIndex+0,readIndex+topLength)
        elif topMagic == "GRAD":
            handleGRAD(mpdz,readIndex+0,readIndex+topLength)
        elif topMagic == "SETD":
            handleSETD(mpdz,readIndex+0,readIndex+topLength)
        readIndex += topLength
    

if __name__ == "__main__":
    main()