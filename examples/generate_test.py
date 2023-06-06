#!/usr/bin/env python3

CODE = """set r0, 150
set r1, 250.07 ; here's a comment describing some float
xcg r0, r1
add r0,      r1 ; random amount of whitespace
mov r2, r1

\r


mul r2, r1"""

print(CODE)
