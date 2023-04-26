```c
int a; // proportion
int b; // integral
float c; // dc_vol
int d[3]; // IO data
float e[3]; // input_vol
```


```c
char buffer[5*sizeof(int) + 4*sizeof(float)];
char* buffer_ptr = buffer;
memcpy(buffer_ptr, &a, sizeof(a));
buffer_ptr += sizeof(a);
memcpy(buffer_ptr, &b, sizeof(b));
buffer_ptr += sizeof(b);
memcpy(buffer_ptr, &c, sizeof(c));
buffer_ptr += sizeof(c);
memcpy(buffer_ptr, d, sizeof(d));
buffer_ptr += sizeof(d);
memcpy(buffer_ptr, e, sizeof(e));
buffer_ptr += sizeof(e);
```

```c
#define PACK_VALUE(type, value, count, buffer_ptr) do { \
    memcpy(buffer_ptr, value, sizeof(type) * count); \
    buffer_ptr += sizeof(type) * count; \
} while(0)
```

```python
import re

code = """
// data-output-area
int a; // proportion
int b; // integral
float c; // dc_vol
int d[3]; // IO data
float e[3]; // input_vol
// data-output-area
"""

# Extract variable declarations
var_pattern = r"\b(int|float)\b\s+(\w+)(?:\[(\d+)\])?;"
var_decls = re.findall(var_pattern, code)

# Generate buffer declaration and pointer initialization
buffer_size = sum(int(size) if size else 1 for _, _, size in var_decls)
buffer_type = "char"
buffer_decl = f"{buffer_type} buffer[{buffer_size+2}] = {{'&'}};"
buffer_ptr_init = f"{buffer_type}* buffer_ptr = buffer + 1;"

# Generate PACK_VALUE macro
pack_macro = "#define PACK_VALUE(type, value, count, buffer_ptr) do { \\\n"
pack_macro += "    memcpy(buffer_ptr, value, sizeof(type) * count); \\\n"
pack_macro += "    buffer_ptr += sizeof(type) * count; \\\n"
pack_macro += "} while(0)\n"

# Generate packing statements
pack_statements = []
for var_type, var_name, var_size in var_decls:
    if var_size:
        pack_statements.append(f"PACK_VALUE({var_type}, {var_name}, {var_size}, buffer_ptr);")
    else:
        pack_statements.append(f"PACK_VALUE({var_type}, &{var_name}, 1, buffer_ptr);")

# Add terminator byte
pack_statements.append(f"*buffer_ptr = '\\n';")

# Print generated code
print(buffer_decl)
print(buffer_ptr_init)
print(pack_macro)
for statement in pack_statements:
    print(statement)
```


```c
int a; // proportion
int b; // integral
float c; // dc_vol
int d[3]; // IO data
float e[3]; // input_vol
#define PACKET_SIZE sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d) + sizeof(e)
```

```c
#define BUFFER_MAX 10 // user-define
#define PACKET_SIZE sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d) + sizeof(e)
char buffer_array[BUFFER_MAX][PACKET_SIZE+2];
uint16_t buffer_count = 0;
char* buffer_ptr;
#define PACK_VALUE(type, value, count, buffer_ptr) do { \
    memcpy(buffer_ptr, value, sizeof(type) * count); \
    buffer_ptr += sizeof(type) * count; \
} while(0)
int main()
{
    char* buffer_ptr = buffer_array;
    while (1) {
        *buffer_ptr = '&';
        buffer_ptr++;
        PACK_VALUE(int, &a, 1, buffer_ptr);
        PACK_VALUE(int, &b, 1, buffer_ptr);
        PACK_VALUE(float, &c, 1, buffer_ptr);
        PACK_VALUE(int, d, 3, buffer_ptr);
        PACK_VALUE(float, e, 3, buffer_ptr);
        *buffer_ptr = '\n';
        buffer_count++;
        if (buffer_count >= BUFFER_MAX)
        {
            SPI_send_buffer(buffer_array, BUFFER_MAX*PACKET_SIZE); // user-define
            buffer_count = 0;
            buffer_ptr = buffer_array;
        }
    }
}
```

```python
input_code = """
// int 2
// float 4
int a; // proportion
int b; // integral
float c; // dc_vol
int d[3]; // IO data
float e[3]; // input_vol
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
output_code = """#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define BUFFER_MAX 10 // user-define
"""

output_code += "const size_t PACKET_SIZE = "

for var_type, var_name, var_size in variables:
    output_code += f"sizeof({var_type}) * {var_size} + "

output_code += "sizeof(char) * 2;\n\n"

output_code += """char buffer_array[BUFFER_MAX][PACKET_SIZE];
uint16_t buffer_count = 0;
char* buffer_ptr;

#define PACK_VALUE(type, value, count, buffer_ptr) do { \\
    memcpy(buffer_ptr, value, sizeof(type) * count); \\
    buffer_ptr += sizeof(type) * count; \\
} while(0)

void SPI_send_buffer(char buffer_array[][PACKET_SIZE], uint16_t buffer_size);

int main()
{
    char* buffer_ptr = buffer_array[0];
    while (1) {
        *buffer_ptr = '&';
        buffer_ptr++;
"""

for var_type, var_name, var_size in variables:
    output_code += f"        PACK_VALUE({var_type}, &{var_name}, {var_size}, buffer_ptr);\n"

output_code += """        *buffer_ptr = '\\n';
        buffer_ptr++;
        *buffer_ptr = '\\0';
        buffer_count++;
        if (buffer_count >= BUFFER_MAX)
        {
            SPI_send_buffer(buffer_array, BUFFER_MAX*PACKET_SIZE); // user-define
            buffer_count = 0;
            buffer_ptr = buffer_array[0];
        }
        else
        {
            buffer_ptr = buffer_array[buffer_count];
        }
    }
}

void SPI_send_buffer(char buffer_array[][PACKET_SIZE], uint16_t buffer_size)
{
    // This function should send the buffer over SPI or another communication interface.
    // For demonstration purposes, we simply print the buffer contents to the console.
    for (int i = 0; i < buffer_size; i++)
    {
        putchar(buffer_array[0][i]);
    }
    printf("\\n");
}
"""

# print the output code
print(output_code)
```




# client side

```c
int a; // proportion
int b; // integral
float c; // dc_vol
int d[3]; // IO_data1, IO_data2, IO_data3
float e[3]; // input_vol1, input_vol2, input_vol3
```c

```c
struct data_channel {
    size_t id;
    char* name;
    size_t byte_offset;
};

struct data_channel[9] = {
    [0] = {.id = 0, .name = "proportion", .byte_offset = 0},
    [1] = {.id = 1, .name = "integral", .byte_offset = sizeof(int)},
    [2] = {.id = 2, .name = "dc_vol", .byte_offset = sizeof(int)+sizeof(int)},
    [3] = {.id = 3, .name = "IO_data1", .byte_offset = sizeof(int)+sizeof(int)+sizeof(float)},
    [4] = {.id = 4, .name = "IO_data2", .byte_offset = sizeof(int)+sizeof(int)+sizeof(float)+sizeof(int)},
    [5] = {.id = 5, .name = "IO_data3", .byte_offset = sizeof(int)+sizeof(int)+sizeof(float)+sizeof(int)+sizeof(int)},
    [6] = {.id = 6, .name = "input_vol1", .byte_offset = sizeof(int)+sizeof(int)+sizeof(float)+sizeof(int)+sizeof(int)+sizeof(int)},
    [7] = {.id = 7, .name = "input_vol2", .byte_offset = sizeof(int)+sizeof(int)+sizeof(float)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(float)},
    [8] = {.id = 8, .name = "input_vol3", .byte_offset = sizeof(int)+sizeof(int)+sizeof(float)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(float)+sizeof(float)}
}
```
