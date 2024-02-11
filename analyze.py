#!/usr/bin/env python3
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

            #print("temp vs index: " + str(tempIndex) + " / " + str(index + scenLength))
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
            pass
        elif scenMagic == "IMGB":
            pass
        elif scenMagic == "PLTB":
            pass
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