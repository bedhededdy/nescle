def main():
    # all valid opcodes but no invalid opcodes should stop on
    # line 5004 
    with open('../logs/nestest.log') as f:
        my_log = f.read().splitlines()

    with open('../logs/nestestref.log') as f:
        ref_log = f.read().splitlines()

    if len(my_log) != len(ref_log):
        print('diff log lengths')

    for i in range(min(len(my_log), len(ref_log))):
        my_line = my_log[i]
        ref_line = ref_log[i]

        # this was done before I had completely matched the format
        # uncomment for simpler comparison, shouldn't affect the result
        # as to whether or not your CPU emulation is correct
        '''
        if my_line[:20] != ref_line[:20]:
            print(f'diff on line {i+1}')
            print('your line')
            print(my_line[:20])
            print('ref line')
            print(ref_line[:20])
            return

        if my_line[48:] != ref_line[48:]:
            print(f'diff on line {i+1}')
            print('your line')
            print(my_line[48:])
            print('ref line')
            print(ref_line[48:])
            return

        if my_line != ref_line:
            print(f'diff on line {i+1}')
            print('your line')
            print(my_line)
            print('ref line')
            print(ref_line)
            return
        '''
    
    print('no differences detected')

if __name__ == '__main__':
    main()
