#!/usr/bin/env python3

import argparse
import sys

def part_copy2ram(infile, outfile):
    try:
        with open(infile, "rb") as f:
            indata = f.read()
    except Exception:
        print(f"Could not read input file '{infile}'", file=sys.stderr)
        raise

    try:
        with open(outfile, "wb") as f:
            f.write(len(indata).to_bytes(4, 'little'))
            f.write(indata)
    except Exception:
        print(f"Could not write output file '{outfile}'", file=sys.stderr)
        raise

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("infile", help="Input File (binary)")
    parser.add_argument("outfile", help="Output file (binary)")
    return parser.parse_args()

def main():
    args = parse_args()
    part_copy2ram(args.infile, args.outfile)

if __name__ == "__main__":
    main()
