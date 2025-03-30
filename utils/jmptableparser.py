def main():
    
    # load in the xenon log file
    with open('xenonlog.txt', 'r') as file:
        errors = file.readlines()

        # loop line by line and get the distinct hex values after the last semi colon
        # {DATA} -> "ERROR: Switch case at 82073674 is trying to jump outside function: 820738C8"

        # list of hex values
        hex_values = []

        for error in errors:
            # ignore anything that doesnt start with "ERROR"
            if not error.startswith("ERROR"):
                continue

            hex = error.split(':')[-1].strip()
            hex_values.append(hex)

        # remove duplicates
        hex_values = list(set(hex_values))

        # print out the hex values
        for hex in hex_values:
            print("0x" + hex + ",")

if __name__ == '__main__':
    main()