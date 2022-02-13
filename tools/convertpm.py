from sys import argv
from struct import unpack, pack

GAMECODE = "SlLd"

GAMENAME = "Slimloader  "

def printUsage(name: str):
    print(f"Usage:\n\t{name} infile outfile")

def convert(infile: str, outfile: str):
    
    # open file and read in data
    with open(infile, "rb") as f:
        indata = bytearray(f.read())
    
    # read IRQs from infile
    irqs = []
    
    for i in range(0, 2 * 27, 2):
        irqs.append(unpack("<H", indata[i:i + 2])[0])
    
    # copy 0x2100 0xFFs into outbuf
    
    outbuf = bytearray()
    
    outbuf.extend([0xFF] * 0x2100)
    
    # MN signature - or, in this case, dev ROM signature
    
    outbuf.extend([0xBF, 0xD9])
    
    # fill in IRQs
    
    for i in irqs:
        if i == 0:
            outbuf.extend([0xFF] * 6) # 6 NOPs
        else:
            outbuf.append(0xF3) # JRL
            
            offset = len(outbuf) + 1
            diff = i - offset
            
            outbuf.extend(pack("<H", diff))
            
            outbuf.extend([0xFF] * 3) # 3 NOPs
    
    # NINTENDO signature
    
    outbuf.extend("NINTENDO".encode("ascii"))
    
    # game code
    
    outbuf.extend(GAMECODE.encode("ascii"))
    
    # game name
    
    outbuf.extend(GAMENAME.encode("ascii"))
    
    # 2P signature
    
    outbuf.extend("2P".encode("ascii"))
    
    # fill in data
    
    if len(outbuf) != 0x21BE:
        print(f"{len(outbuf):x}")
        print("An error occured while converting the IRQs")
        exit(1)
    
    outbuf.extend(indata[0x21BE:])
    
    # save file
    
    with open(outfile, "wb") as g:
        g.write(outbuf)

if __name__ == "__main__":
    if len(argv) < 3:
        printUsage(argv[0])
        exit(1)
    
    convert(argv[1], argv[2])
    exit(0)
