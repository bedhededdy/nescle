def main():
    # old way goes
    # optype
    # opcode
    # addrmode
    # bytes
    # cycles
    # 
    # new way goes
    # opcode
    # addrmode
    # optype
    # bytes
    # cycles 

    with open('../logs/isa.txt', 'r') as f:
        lines = f.read().splitlines()

    with open('../logs/new_isa.txt', 'w') as f:
        # first line stays the same
        f.write(lines[0])
        f.write('\n')

        # rearrange elements of each instruction and write it
        for i in range(1, len(lines)-1):
            line = lines[i]
            offset = 0
            for _ in range(16):
                # inefficient, but works
                while line[offset] != '{':
                    offset += 1
                    # indent properly
                    if _ == 0:
                        f.write(' ')
                offset += 1
                end = offset + 1
                while line[end] != '}':
                    end += 1
                elems = line[offset:end].split(', ')
                # swap optype with opcode
                elems[0], elems[1] = elems[1], elems[0]
                # swap optype with addrmode                
                elems[1], elems[2] = elems[2], elems[1]

                f.write('{')
                f.write(f'{elems[0]}, {elems[1]}, {elems[2]}, {elems[3]}, {elems[4]}')
                f.write('}, ')
                if _ == 15:
                    f.write('\n')

        # last line stays the same
        f.write(lines[-1])


if __name__ == '__main__':
    main()
