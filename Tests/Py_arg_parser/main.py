# This program will test the functionality of how to use arguments
import argparse

# Create object to contain arguments
parser = argparse.ArgumentParser(description='Downloads weather prognosis from yr.no')

# What to look for, nargs is how many arguments after the -- are arguments of this type
parser.add_argument('-dn', nargs=2, help='Name as which the data should be saved, can be a path')
parser.add_argument('-wl', nargs=1, help='Address of the webpage to download data from')

# Prepare args
args = parser.parse_args()

# Work with the args
print(args)
print(args.dn)
