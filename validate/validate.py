def read_file_to_2d_list(filename):
    """Reads a file and returns its contents as a 2D list of integers."""
    with open(filename, 'r') as file:
        return [list(map(int, line.split())) for line in file]

def compare_outputs(verilog_file, cpp_file):
    """Compares the contents of two files."""
    verilog_output = read_file_to_2d_list(verilog_file)
    cpp_output = read_file_to_2d_list(cpp_file)

    if len(verilog_output) != len(cpp_output):
        print("Files have different number of rows.")
        return False

    for i in range(len(verilog_output)):
        if len(verilog_output[i]) != len(cpp_output[i]):
            print(f"Files differ in row {i}.")
            return False

        for j in range(len(verilog_output[i])):
            if verilog_output[i][j] != cpp_output[i][j]:
                print(f"Difference found at [{i}][{j}]: {verilog_output[i][j]} != {cpp_output[i][j]}")
                return False

    print("The files are identical.")
    return True

if __name__ == "__main__":
    verilog_file = "../src/verilog_module_output.txt"  # Replace with the actual path to your Verilog output file
    cpp_file = "../golden_model/golden_model_output.txt"  # Replace with the actual path to your C++ output file

    if not compare_outputs(verilog_file, cpp_file):
        print("The files are different.")
