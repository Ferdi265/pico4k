#!/usr/bin/env python3

import argparse
import subprocess
import sys

def part_exo2ram(infile, outfile):
    exo = subprocess.run(
        ["exomizer", "raw", "-B", "-P", "12", infile, "-o", outfile]
    )

    if exo.returncode != 0:
        sys.exit("Failed to compress data with exomizer")

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("infile", help="Input File (binary)")
    parser.add_argument("outfile", help="Output file (binary)")
    return parser.parse_args()

def main():
    args = parse_args()
    part_exo2ram(args.infile, args.outfile)

if __name__ == "__main__":
    main()
