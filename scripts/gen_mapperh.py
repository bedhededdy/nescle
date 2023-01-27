def main():
    # generate lookup table and header declarations for Mapper.h
    with open('../logs/mapperh.txt', 'w') as f:
        # declarations
        for mapper_no in range(256):
            padded_no = f'{mapper_no:03}'

            # comment
            f.write(f'/* Mapper{padded_no} */\n')

            # constructor
            f.write(f'void Mapper{padded_no}(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks);\n')

            # read/write functions
            f.write(f'uint8_t Mapper{padded_no}_CPURead(Mapper* mapper, uint16_t addr);\n')
            f.write(f'bool Mapper{padded_no}_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);\n')
            f.write(f'uint8_t Mapper{padded_no}_PPURead(Mapper* mapper, uint16_t addr);\n')
            f.write(f'bool Mapper{padded_no}_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data);\n')

            # blank line
            f.write('\n')

        # lookup table
        f.write('/* Lookup Table */\n')
        f.write('static const void (*mapper_funcs[])(Mapper*, uint8_t, uint8_t) = {\n')

        for i in range(16):
            for j in range(16):
                mapper_no = 16*i + j
                padded_no = f'{mapper_no:03}'
                f.write(f'[{mapper_no}] = &Mapper{padded_no}, ')
            f.write('\n')

        f.write('};\n')

    # put in some empty definitions in the c file
    with open('../logs/mapperc.txt', 'w') as f:
        for mapper_no in range(256):
            padded_no = f'{mapper_no:03}'

            # comment
            f.write(f'/* Mapper{padded_no} */\n')

            # constructor
            f.write(f'void Mapper{padded_no}(Mapper* mapper, uint8_t rom_banks, uint8_t char_banks) {{\n}}\n')

            # read/write functions
            f.write(f'uint8_t Mapper{padded_no}_CPURead(Mapper* mapper, uint16_t addr){{\nreturn 0;\n}}\n')
            f.write(f'bool Mapper{padded_no}_CPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){{\nreturn false;\n}}\n')
            f.write(f'uint8_t Mapper{padded_no}_PPURead(Mapper* mapper, uint16_t addr){{\nreturn 0;\n}}\n')
            f.write(f'bool Mapper{padded_no}_PPUWrite(Mapper* mapper, uint16_t addr, uint8_t data){{\nreturn false;\n}}\n')

            # blank line
            f.write('\n')

if __name__ == '__main__':
    main()
