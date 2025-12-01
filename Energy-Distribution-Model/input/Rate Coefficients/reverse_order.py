input_file = "amb-Ed_IeXXX_t0.5-14.0_sC1_sN1_sAgt105_c00.dat"
header_size = 1

# Open the original file and read all lines
with open(input_file, "r") as file:
    lines = file.readlines()

# Separate the header and the data lines
header = lines[:header_size] 
data_lines = lines[header_size:]

# Remove empty lines from data_lines
data_lines = [line for line in data_lines if line.strip()]

if data_lines and not data_lines[-1].endswith('\n'):
    data_lines[-1] += '\n'

# Reverse the order of the data lines
data_lines.reverse()

# Write the header and reversed data lines to a new file
with open(input_file[:-4] + " - reversed.dat", "w") as file:
    file.writelines(header)
    file.writelines(data_lines)
