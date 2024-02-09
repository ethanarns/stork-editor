#!/usr/bin/env python3
import ndspy.codeCompression
import argparse

parser = argparse.ArgumentParser(
    "blz_util"
)
parser.add_argument("filename")
parser.add_argument("-o","--out",default="bin9.bin")
args = parser.parse_args()

decompressed = ndspy.codeCompression.decompressFromFile(args.filename)
with open(args.out,"wb") as decompBin9:
    decompBin9.write(decompressed)