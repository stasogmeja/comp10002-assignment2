# COMP10002 Assignment 2

Grid-based route planning and replanning system implemented in C.

## Description

This program reads a grid, blocked cells, and a proposed route from
standard input. It checks the validity of the route and, when the route
is blocked, attempts to repair it by finding a new path around the
blocked cells.

## Files

- ass2.c          : main program
- test0.txt       : sample test data
- test1.txt       : sample test data
- test2.txt       : sample test data
- test3.txt       : sample test data

## Compilation

```bash
gcc -Wall -o ass2 ass2.c
```

## Example Usage

```bash
ass2 < test0.txt
```
