#!/usr/bin/env python3

import argparse
import subprocess
import sys
import os

def find_in_path(name):
    def is_exe(fpath):
        return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

    for path in os.environ["PATH"].split(os.pathsep):
        namepath = os.path.join(path, name)
        if is_exe(namepath):
            return namepath

    return None

def find_upkr():
    upkr = os.environ.get("UPKR")
    if upkr is not None:
        return [upkr]

    upkr = find_in_path("upkr")
    if upkr is not None:
        return ["upkr"]

    print("could not find upkr compressor!", file = sys.stderr)
    sys.exit(1)

def part_upkr2ram(infile, outfile):
    upkr_cmd = find_upkr()
    upkr = subprocess.run(
        upkr_cmd + ["-l", "9", infile, outfile]
    )

    if upkr.returncode != 0:
        sys.exit("Failed to compress data with upkr")

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("infile", help="Input File (binary)")
    parser.add_argument("outfile", help="Output file (binary)")
    return parser.parse_args()

def main():
    args = parse_args()
    part_upkr2ram(args.infile, args.outfile)

if __name__ == "__main__":
    main()
