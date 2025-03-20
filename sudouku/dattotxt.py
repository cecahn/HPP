import struct

def convert_dat_to_txt(input_file, output_file):
    try:
        with open(input_file, "rb") as f:
            # Read base and side length (1 byte each)
            base = struct.unpack("B", f.read(1))[0]
            side = struct.unpack("B", f.read(1))[0]

            print(f"Base: {base}, Side Length: {side}")

            # Read the Sudoku board
            size = side * side
            board_data = f.read(size)

            if len(board_data) != size:
                print("Warning: File size does not match expected board size!")
                return

            # Convert bytes to list of integers
            board = list(struct.unpack(f"{size}B", board_data))

            # Write to text file
            with open(output_file, "w") as txt:
                txt.write(f"Base: {base}\n")
                txt.write(f"Side Length: {side}\n\n")

                for i in range(side):
                    row = board[i * side:(i + 1) * side]
                    txt.write(" ".join(map(str, row)) + "\n")

            print(f"Converted {input_file} -> {output_file} successfully!")

    except Exception as e:
        print(f"Error: {e}")

# Usage Example
input_filename = "board_64x64.dat"
output_filename = "board_64x64.txt"
convert_dat_to_txt(input_filename, output_filename)
