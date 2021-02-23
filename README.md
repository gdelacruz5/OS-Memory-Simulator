# OS-Memory-Simulator
Simulates an OS memory handler

Usage:
memSim <reference-sequence-file.txt> <FRAMES> <PRA>

    • reference-sequence-file.txt contains the list of logical memory addresses
    • FRAMES is an integer <= 256  and > 0, which is the number of frames in the system. Note that a FRAMES number of 256 means logical and physical memory are of the same size. 
    • PRA is “FIFO” or “LRU” or “OPT,” representing first-in first-out, least recently used, and optimal algorithms, respectively. 
    • FRAMES and PRA or just PRA may be omitted. If so, use 256 for FRAMES and FIFO for PRA.
