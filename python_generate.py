input_code = """
// int 2
// float 4
int a; // proportion
int b; // integral
float c; // dc_vol
int d[3]; // IO data
float e[3]; // input_vol
"""
input_code = """
float vol; // vol
uint8_t send_buf[7]; // data_buffer
"""
input_code = """
float ad[10]; // AD
"""

# parse input code and extract variable information
variables = []
for line in input_code.strip().split('\n'):
    if '//' not in line or line[0] == '/':
        continue
    parts = line.strip().split()
    var_type = parts[0]
    var_name = parts[1].rstrip(';')
    if '[' in var_name:
        var_name, var_size = var_name.split('[')
        var_size = var_size.rstrip(']')
    else:
        var_size = '1'
    variables.append((var_type, var_name, var_size))

# generate output code
output_code = """
#define BUFFER_MAX (10) // user-define
"""

output_code += "#define PACKET_SIZE ("

for var_type, var_name, var_size in variables:
    output_code += f"sizeof({var_type}) * {var_size} + "

output_code += "sizeof(char) * 2)\n\n"

output_code += """char buffer_array[BUFFER_MAX][PACKET_SIZE];
uint16_t buffer_count = 0;
char* buffer_ptr;

void mem_cpy(char *source, char *dst, uint32_t count, uint32_t size)
{
    uint32_t bytesize = size * count;
    char *source_ptr = source;
    char *dst_ptr = dst;
    for (uint32_t memidx = 0; memidx < bytesize; memidx++)
    {
        *(dst_ptr++) = *(source_ptr++);
    }
}

int main()
{
    char* buffer_ptr = (char *)buffer_array;
    while (1) {
        *buffer_ptr = '&';
        buffer_ptr++;
"""

for var_type, var_name, var_size in variables:
    output_code += f"        mem_cpy((char*){'' if int(var_size) > 1 else '&'}{var_name}, buffer_ptr, {var_size},sizeof({var_type}));\n"
    output_code += f"        buffer_ptr += {var_size} * sizeof({var_type});\n"
output_code += """        *buffer_ptr = '\\n';
        buffer_ptr++;
        buffer_count++;
        if (buffer_count >= BUFFER_MAX)
        {
            retval = transfer(SOCKET_LOOPBACK, g_loopback_buf, &recv_size, (char *)buffer_array, BUFFER_MAX * PACKET_SIZE, PORT_LOOPBACK);
            buffer_count = 0;
            buffer_ptr = (char *)buffer_array;
        }
    }
}
"""

# print the output code
print(output_code)