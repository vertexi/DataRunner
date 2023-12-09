#include "data.hpp"

const uint16_t DataTypeSize[] =
    {sizeof(int8_t),
     sizeof(int16_t),
     sizeof(int32_t),
     sizeof(int64_t),
     sizeof(uint8_t),
     sizeof(uint16_t),
     sizeof(uint32_t),
     sizeof(uint64_t),
     sizeof(float),
     sizeof(double)};

std::map<std::string, enum class DataType> DataTypeDict = {
    {"int8_t", DataType::DataType_int8_t},
    {"int16_t", DataType::DataType_int16_t},
    {"int32_t", DataType::DataType_int32_t},
    {"int64_t", DataType::DataType_int64_t},
    {"uint8_t", DataType::DataType_uint8_t},
    {"uint16_t", DataType::DataType_uint16_t},
    {"uint32_t", DataType::DataType_uint32_t},
    {"uint64_t", DataType::DataType_uint64_t},
    {"float", DataType::DataType_float},
    {"double", DataType::DataType_double},
};

std::map<enum class DataType, std::string> DataTypeStringDict = {
    {DataType::DataType_int8_t, "int8_t"},
    {DataType::DataType_int16_t, "int16_t"},
    {DataType::DataType_int32_t, "int32_t"},
    {DataType::DataType_int64_t, "int64_t"},
    {DataType::DataType_uint8_t, "uint8_t"},
    {DataType::DataType_uint16_t, "uint16_t"},
    {DataType::DataType_uint32_t, "uint32_t"},
    {DataType::DataType_uint64_t, "uint64_t"},
    {DataType::DataType_float, "float"},
    {DataType::DataType_double, "double"},
};
