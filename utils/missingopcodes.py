def extract_missing_opcodes(file_path):
    missing_opcodes = set()

    with open(file_path, 'r') as f:
        for line in f:
            line = line.strip()
            if "Unrecognized instruction" in line:
                try:
                    # Extract opcode after the last colon
                    opcode = line.split(":")[-1].strip()
                    if opcode:
                        missing_opcodes.add(opcode)
                except IndexError:
                    continue

    return sorted(missing_opcodes)


if __name__ == "__main__":
    file_path = "xenonlog.txt"  # Change this to your actual filename
    opcodes = extract_missing_opcodes(file_path)

    print("Distinct Missing Opcodes:")
    for opcode in opcodes:
        print(opcode)
