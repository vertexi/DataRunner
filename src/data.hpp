#pragma once

#include <cstdint>
#include <map>
#include <string>

enum class DataType
{
    DataType_int8_t,
    DataType_int16_t,
    DataType_int32_t,
    DataType_int64_t,
    DataType_uint8_t,
    DataType_uint16_t,
    DataType_uint32_t,
    DataType_uint64_t,
    DataType_float,
    DataType_double
};

struct data_channel
{
    size_t id;
    char *name;
    size_t byte_offset;
    enum DataType data_type;
};

#define DATA_BUF_SIZE (4 * 1024 * 1024 * 1024ULL)
#define PAKCET_BUF_DIVSION (100000) // 1/100 is 1%, 1/10 is 10%
#define RECV_BUF_SIZE (100 * 1024)

struct DataBuf
{
    char *data_buf = nullptr;
    size_t packet_size = 0;
    size_t packet_max = 0;
    size_t packet_idx = 0;
    // when data wrap up set this to 1
    size_t packet_round = 0;
    // there is a race condition between plotter func(get data from buf) and socket_data recv func
    // so when data_buf wrap up ignore a little bit data.
    size_t packet_buf_buf = 0;
    size_t data_buf_idx = 0;
    float data_freq = 10000.0f;
    int plot_downsample = 2000;
};

extern const uint16_t DataTypeSize[];
extern std::map<std::string, enum DataType> DataTypeDict;
extern std::map<enum DataType, std::string> DataTypeStringDict;
