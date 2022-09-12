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

def find_exoraw():
    exoraw = os.environ.get("EXORAW")
    if exoraw is not None:
        return [exoraw]

    exomizer = os.environ.get("EXOMIZER")
    if exomizer is not None:
        return [exomizer, "raw"]

    exoraw = find_in_path("exoraw")
    if exoraw is not None:
        return ["exoraw"]

    exomizer = find_in_path("exomizer")
    if exomizer is not None:
        return ["exomizer", "raw"]

    print("could not find exomizer raw compressor!", file = sys.stderr)
    sys.exit(1)

def part_exo2ram(infile, outfile):
    exo_cmd = find_exoraw()
    exo = subprocess.run(
        exo_cmd + ["-B", "-P", "12", infile, "-o", outfile]
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
