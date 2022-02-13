from sys import argv

def printUsage(name: str):
    print(f"Usage:\n\t{name} outfile [pos filepos readlen infile] * n")

def copy(outfile: str, infiles: list):
    
    outbuf = bytearray([0xFF] * 2 * 1024 * 1024)
    
    for i in range(0, len(infiles), 4):
        pos, filepos, readlen = int(infiles[i], 0), int(infiles[i + 1], 0), int(infiles[i + 2], 0)
        with open(infiles[i + 3], "rb") as f:
            infile = bytearray(f.read())
        
        readmax = len(infile) - filepos
        
        if readmax <= 0:
            print(f"Position 0x{filepos:X} not found in file {infiles[i + 3]}")
            exit(1)
        
        if readlen == 0:
            readlen = readmax
        elif readlen > readmax:
            print(f"File {infiles[i + 3]} not big enough to read 0x{readlen:X} bytes from")
            exit(1)
        
        if pos + readlen > len(outbuf):
            print(f"File chunk from {infiles[i + 3]} too long to fit in buffer")
            exit(1)
        
        outbuf[pos:pos + readlen] = infile[filepos:filepos + readlen]
    
    with open(outfile, "wb") as g:
        g.write(outbuf)
    
if __name__ == "__main__":
    if len(argv) < 3 or (len(argv) - 2) % 4 != 0:
        printUsage(argv[0])
        exit(1)
    
    copy(argv[1], argv[2:])
    exit(0)
