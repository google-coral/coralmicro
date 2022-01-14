#!/usr/bin/python3
import argparse
import os

def main():
    parser = argparse.ArgumentParser(description='Valaint Build Directory Cache Creator',
                                    formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--build_dir', type=str, required=True)
    args = parser.parse_args()

    build_dir = os.path.relpath(args.build_dir)
    with open(os.path.join(build_dir, 'cached_files.txt'), 'w') as f:
        for dirname, dirnames, filenames in os.walk(build_dir):
            for filename in filenames:
                f.write(os.path.join(dirname, filename) + '\n')

if __name__ == '__main__':
    main()