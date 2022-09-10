#!/usr/bin/env python3

import argparse
import sys

def any_int(x):
    try:
        return int(x, 0)
    except:
        raise argparse.ArgumentTypeError("expected an integer, not '{!r}'".format(x))

def embed_part(infile, partnum, outfile):
    try:
        with open(infile, "rb") as f:
            indata = f.read()
    except Exception:
        print(f"Could not read input file '{infile}'", file=sys.stderr)
        raise

    try:
        with open(outfile, "w") as f:
            f.write(f"// assembled and linked version of part {partnum}\n\n")
            f.write(".cpu cortex-m0plus\n")
            f.write(".thumb\n")
            f.write(f".section .part{partnum}, \"ax\"\n\n")
            for offs in range(0, len(indata), 16):
                chunk = indata[offs : min(offs + 16, len(indata))]
                f.write(".byte {}\n".format(", ".join("0x{:02x}".format(b) for b in chunk)))
    except Exception:
        print(f"Could not write output file '{outfile}'", file=sys.stderr)
        raise

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("infile", help="Input File (binary)")
    parser.add_argument("partnum", help="Part Number (for section name, integer)", type=any_int)
    parser.add_argument("outfile", help="Output file (assembly)")
    return parser.parse_args()

def main():
    args = parse_args()
    embed_part(args.infile, args.partnum, args.outfile)

if __name__ == "__main__":
    main()
