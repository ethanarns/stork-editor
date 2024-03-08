#!/usr/bin/env python3
import ndspy.soundSequence
import argparse

parser = argparse.ArgumentParser("stork-audio")
parser.add_argument("filename")

def readUint32(data: bytearray, start: int) -> int:
    return (data[start+3] << 24) + (data[start+2] << 16) + (data[start+1] << 8) + data[start]

def readUint16(data: bytearray, start: int) -> int:
    return (data[start+1] << 8) + data[start]

def main(sdatFilename: str) -> None:
    # http://problemkaputt.de/gbatek.htm#dsfilessoundsdatetc
    sdat = bytearray(open(sdatFilename,'rb').read())
    sdatMagic = sdat[0:4].decode("ascii")
    if (sdatMagic != "SDAT"):
        print("FAIL: Not an SDAT file")
        exit()
    # SYMB data
    symbBase = 0x40
    fileListSseqAddrAddr = symbBase + 8
    fileListSeqAddr = readUint32(sdat,fileListSseqAddrAddr) + symbBase
    data_sseqCount = readUint32(sdat,fileListSeqAddr+0)
    print(hex(data_sseqCount))
    addr_fileListSeqListBase = fileListSeqAddr+4
    fileNameIndex = 0
    while fileNameIndex < data_sseqCount:
        addr_seqFileName = readUint32(sdat,addr_fileListSeqListBase+(fileNameIndex*4)) + symbBase
        print(hex(addr_seqFileName))
        fileNameIndex += 1
    # INFO data
    addr_infoBase = readUint32(sdat,0x18) + 0 #sdat base is 0
    print(sdat[addr_infoBase:addr_infoBase+4].decode("ascii"))
    addr_sseqLoc = readUint32(sdat,addr_infoBase+8) + addr_infoBase # 0x8 is location of sseq pointer in INFO
    data_sseqDataCount = readUint32(sdat,addr_sseqLoc+0)
    print("sseqListCountAddr: " + hex(addr_sseqLoc+0))
    print("sseqListDataCount: " + hex(data_sseqDataCount))
    addr_sseqListBase = addr_sseqLoc + 4
    sseqIndex = 0
    while sseqIndex < data_sseqDataCount:
        addr_sseqInfo = readUint32(sdat,addr_sseqListBase+(sseqIndex*4)) + addr_infoBase
        print(hex(addr_sseqInfo))
        sseqIndex += 1
    # The useful stuff in that sseqInfo block is the FAT filesystem ID
    addr_fatBase = readUint32(sdat,0x20) + 0 # 0 is SDAT base
    print("FAT base: " + hex(addr_fatBase)) # Accurate!
    offset_fatFileEntryListBase = 0xC
    addr_firstFatFileEntryBase = addr_fatBase + offset_fatFileEntryListBase + (0 * 16)
    addr_firstFileOffset = readUint32(sdat,addr_firstFatFileEntryBase)
    print(hex(addr_firstFileOffset)) # GOT IT!

if __name__ == "__main__":
    args = parser.parse_args()
    filename = args.filename
    main(filename)