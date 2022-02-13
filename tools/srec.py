from sys import argv
from ntpath import basename

def printUsage(name: str):
    print(f"Usage:\n\t{name} infile [outfile]")

def is_valid(line: str):
    if len(line) < 4:
        return False
    
    if line[0] != "S":
        return False
    
    if line[1] not in (str(x) for x in range(0, 10)):
        return False
    
    hexDigits = [*[str(x) for x in range(0, 10)], *[chr(x) for x in range(ord("A"), ord("F") + 1)], *[chr(x) for x in range(ord("a"), ord("f") + 1)]]
    
    if any(x not in hexDigits for x in line[2:]):
        return False
    
    numBytes = int(line[2:4], 16)
    
    if len(line) != numBytes * 2 + 4:
        return False
    
    total = 0
    for i in range(2, len(line) - 2, 2):
        total += int(line[i:i + 2], 16)
    
    if (total & 0xFF) ^ 0xFF != int(line[-2:], 16):
        return False
    
    return True

def convert(infile: str, outfile: str):
    with open(infile, "r") as f:
        file = f.readlines()
    
    data = bytearray()
    
    for index, line in enumerate(file):
        line = line.rstrip("\n")
        if not is_valid(line):
            print(f"Line {index} is an invalid record")
            exit(1)
        
        recordType = line[1]
        numBytes = int(line[2:4], 16)
        
        if recordType == "0":
            if numBytes < 3:
                print(f"Line {index} is too short to be an S0 record")
                exit(1)
            
            address = int(line[4:8], 16)
            if address != 0:
                print(f"Line {index} is an invalid S0 record (address should be 0)")
                exit(1)
            
            string = ""
            for i in range(8, len(line) - 2, 2):
                string += chr(int(line[i:i + 2], 16))
            
            print(f"S0, str = {string}")
        
        elif recordType == "1":
            if numBytes < 3:
                print(f"Line {index} is too short to be an S1 record")
                exit(1)
            
            address = int(line[4:8], 16)
            
            toAdd = bytearray()
            for i in range(8, len(line) - 2, 2):
                toAdd.append(int(line[i:i + 2], 16))
            
            diff = address + len(toAdd) - len(data) + 1
            
            if diff > 0:
                data.extend([0] * diff)
            
            data[address:address + len(toAdd)] = toAdd
        
        elif recordType == "2":
            if numBytes < 4:
                print(f"Line {index} is too short to be an S2 record")
                exit(1)
            
            address = int(line[4:10], 16)
            
            toAdd = bytearray()
            for i in range(10, len(line) - 2, 2):
                toAdd.append(int(line[i:i + 2], 16))
            
            diff = address - len(data)
            
            if diff > 0:
                data.extend([0] * diff)
            
            data[address:address + len(toAdd)] = toAdd
        
        elif recordType == "8":
            if numBytes < 4:
                print(f"Line {index} is too short to be an S8 record")
                exit(1)
            
            address = int(line[4:10], 16)
            
            print(f"Start address = {address:06X}")
        
        else:
            print(f"Unhandled record type S{recordType} on line {index}")
            exit(1)
    
    with open(outfile, "wb") as g:
        g.write(data)


if __name__ == "__main__":
    if len(argv) < 2:
        printUsage(argv[0]);
        exit(1)
    
    if len(argv) < 3:
        name = basename(argv[1])
        outfile = argv[1].rstrip(name) + (".".join(name.split(".")[:-1]) if len(name.split(".")) > 1 else name) + ".bin"
    else:
        outfile = argv[2]
    
    convert(argv[1], outfile)