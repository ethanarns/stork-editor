#!/usr/bin/env python3
import ndspy.lz10
import argparse

parser = argparse.ArgumentParser(
    "lzss_util"
)
parser.add_argument("filename")
parser.add_argument("-o","--out")
parser.add_argument("-c","--compress",action='store_true',default=False)
args = parser.parse_args()
if args.compress == False:
    decompressed = ndspy.lz10.decompressFromFile(args.filename)
    with open(args.out, "wb") as decompLz:
        decompLz.write(decompressed)
else:
    compressed = ndspy.lz10.compressFromFile(args.filename)
    with open(args.out, "wb") as compLz:
        compLz.write(compressed)