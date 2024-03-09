#!/usr/bin/env python3
import ndspy.lz10
import argparse

parser = argparse.ArgumentParser("decompresser")
parser.add_argument("filename")

args = parser.parse_args()
filename = args.filename
mpdz = bytearray(ndspy.lz10.decompressFromFile(filename))
output = open(filename + ".bin","wb")
output.write(mpdz)