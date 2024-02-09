#!/usr/bin/env python3
import ndspy.lz10
import argparse

parser = argparse.ArgumentParser(
    "lzss_util"
)
parser.add_argument("filename")
parser.add_argument("-o","--out")
args = parser.parse_args()

decomressed = ndspy.lz10.decompressFromFile(args.filename)
with open(args.out, "wb") as decompLz:
    decompLz.write(decomressed)