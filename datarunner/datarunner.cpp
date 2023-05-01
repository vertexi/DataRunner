#include <iostream>
#include <fstream>
#include <string>

#include <sstream>
#include <vector>
#include <map>
#include <stdint.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "implot.h"
#include "implot_internal.h"

#include "IconsFontAwesome6.h"

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

uint16_t DataTypeSize[] =
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

struct data_channel
{
    size_t id;
    char *name;
    size_t byte_offset;
    enum class DataType data_type;
};

std::string trim(const std::string &str)
{
    // Find the first non-whitespace character
    size_t start = str.find_first_not_of(" \t\n\r\f\v");

    // If the string is all whitespace, return an empty string
    if (start == std::string::npos)
    {
        return "";
    }

    // Find the last non-whitespace character
    size_t end = str.find_last_not_of(" \t\n\r\f\v");

    // Extract the substring between the first and last non-whitespace characters
    return str.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(str);
    while (std::getline(token_stream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

std::vector<data_channel> create_data_channel_array(const std::string &code_str, size_t *packet_size)
{
    std::vector<data_channel> channels;

    std::string line;
    size_t byte_offset = 0;
    size_t id = 0;
    std::string array_name;
    std::vector<std::string> element_names;

    // Split the code string into lines
    std::istringstream code_stream(code_str);

    std::map<std::string, enum DataType> custom_data_type_dict;

    while (std::getline(code_stream, line))
    {
        std::cout << line << std::endl;

        if (line.find("//") == 0)
        {
            std::string custom_data_type_name;
            std::string data_type_name;
            size_t key_end = line.find(' ', 3);
            custom_data_type_name = trim(line.substr(2, key_end - 2));
            data_type_name = trim(line.substr(key_end, line.length() - key_end));
            custom_data_type_dict[custom_data_type_name] = DataTypeDict[data_type_name];
            continue;
        }

        // Check if this line declares an array
        auto array_start = line.find_first_of("[");
        auto array_end = line.find_last_of("]");

        if (array_start != std::string::npos && array_end != std::string::npos)
        {
            // The line declares an array, so extract the size
            auto size_str = line.substr(array_start + 1, array_end - array_start - 1);
            int size = std::stoi(size_str);

            // If the array name hasn't been extracted yet, extract it from this line
            if (array_name.empty())
            {
                size_t name_start = line.find(' ', 0) + 1;
                size_t name_end = line.find('[') - 1;
                array_name = line.substr(name_start, name_end - name_start + 1);
            }
            size_t type_name_end = line.find(' ', 0);
            std::string type_name = line.substr(0, type_name_end);
            // Extract the names of the elements from the same line as the declaration
            element_names.clear();
            size_t comment_start = line.find("//", array_end);
            if (comment_start != std::string::npos)
            {
                std::string names_str = line.substr(comment_start + 2);
                std::istringstream names_stream(names_str);
                std::string name;
                while (std::getline(names_stream, name, ','))
                {
                    name = trim(name);
                    element_names.push_back(name);
                }
            }

            // Add a new data channel for each element of the array
            for (int i = 0; i < size; i++)
            {
                std::string element_name;
                if (i < element_names.size())
                {
                    element_name = element_names[i];
                }
                else
                {
                    element_name = array_name + std::to_string(i);
                }

                data_channel channel = {id, new char[element_name.length() + 1], byte_offset};
                std::copy(element_name.begin(), element_name.end(), channel.name);
                channel.name[element_name.length()] = '\0';

                // Update the byte offset and channel ID
                if (custom_data_type_dict.count(type_name) > 0)
                {
                    byte_offset += DataTypeSize[static_cast<int>(custom_data_type_dict[type_name])];
                    channel.data_type = custom_data_type_dict[type_name];
                }
                else
                {
                    byte_offset += DataTypeSize[static_cast<int>(DataTypeDict[type_name])];
                    channel.data_type = DataTypeDict[type_name];
                }

                channels.push_back(channel);

                id++;
            }

            // Reset the array name and element names so that they can be extracted from the next declaration
            array_name = "";
            element_names.clear();
        }
        else
        {
            // The line does not declare an array, so extract the variable name and type
            size_t type_end = line.find(' ', 0);
            std::string type = line.substr(0, type_end);

            // If there is no space between the type and variable name
            if (type_end == std::string::npos)
            {
                continue;
            }

            // Find the start and end of the variable name
            size_t name_start = line.find("//", type_end) + 2;
            size_t name_end = line.find(',', name_start);
            if (name_end == std::string::npos)
            {
                name_end = line.length();
            }

            size_t type_name_end = line.find(' ', 0);
            std::string type_name = line.substr(0, type_name_end);

            // Extract the variable name
            std::string name = line.substr(name_start, name_end - name_start);
            name = trim(name);

            // Add a new data channel to the array for non-array variables
            data_channel channel = {id, new char[name.length() + 1], byte_offset};
            std::copy(name.begin(), name.end(), channel.name);
            channel.name[name.length()] = '\0';

            // Update the byte offset and channel ID
            if (custom_data_type_dict.count(type_name) > 0)
            {
                byte_offset += DataTypeSize[static_cast<int>(custom_data_type_dict[type_name])];
                channel.data_type = custom_data_type_dict[type_name];
            }
            else
            {
                byte_offset += DataTypeSize[static_cast<int>(DataTypeDict[type_name])];
                channel.data_type = DataTypeDict[type_name];
            }

            channels.push_back(channel);

            id++;
        }
        *packet_size = byte_offset;
    }

    return channels;
}

#include "asio.hpp"
using asio::ip::tcp;

bool stop_transfer = true;
#define DATA_BUF_SIZE (1024 * 1024 * 1024)
#define PAKCET_BUF_DIVSION (100) // 100 for 1%, 10 for 10%
#define RECV_BUF_SIZE (10 * 1024)
char data_buf[DATA_BUF_SIZE];
size_t packet_size = 0;
size_t packet_max = 0;
size_t packet_idx = 0;
size_t packet_round = 0;
size_t packet_buf_buf = 0; // buffer for when reading from plot not changed by socket
size_t data_buf_idx = 0;
float data_freq = 10000.0f;
size_t data_idx_min()
{
    if (packet_round > 0)
    {
        return (packet_idx + packet_buf_buf + (packet_round - 1) * packet_max);
    }
    else
    {
        return (0);
    }
}

size_t data_idx_max()
{
    if (packet_round > 0)
    {
        return (packet_idx + packet_round * packet_max);
    }
    else
    {
        return (packet_idx);
    }
}

asio::io_service io_service;
// socket creation
asio::ip::tcp::socket data_socket(io_service);
void extractData(char *data, size_t data_len);

void print_memory_hex(const void *memory_block, size_t size)
{
    const unsigned char *p = (unsigned char *)memory_block;
    for (size_t i = 0; i < size; i++)
    {
        printf("%02X ", p[i]);
    }
    printf("\n");
}

void dataquery()
{
    // request/message from client
    const std::string msg = "hello from Client!\n";
    asio::error_code error;
    asio::write(data_socket, asio::buffer(msg), error);
    if (!error)
    {
        std::cout << "Client sent hello message!" << std::endl;
    }
    else
    {
        std::cout << "send failed: " << error.message() << std::endl;
    }
    // getting response from server
    // asio::streambuf receive_buffer;
    std::array<char, RECV_BUF_SIZE> receive_buffer{0};
    receive_buffer.fill(0);
    size_t read_len = 0;
    size_t total = 0;
    static bool one_more_time = false;
    //
    while ((read_len = data_socket.read_some(asio::buffer(receive_buffer), error)) >= 0)
    {
        if (error && error != asio::error::eof)
        {
            std::cout << "receive failed: " << error.message() << std::endl;
        }
        else
        {
            char *data = receive_buffer.data();
            extractData(data, read_len);
        }
        if (one_more_time)
        {
            one_more_time = false;
            data_socket.close();
            printf("stop.\n");
            std::cout << std::endl;
            return;
        }
        if (stop_transfer)
        {
            one_more_time = true;
        }
    }
}

void mem_cpy(char *dst, char *source, size_t bytesize)
{
    for (uint32_t memidx = 0; memidx < bytesize; memidx++)
    {
        *(dst++) = *(source++);
    }
}

void extractData(char *data, size_t data_len)
{
    static char receive_buffer_temp[RECV_BUF_SIZE] = {0};
    static size_t receive_buffer_temp_len = 0;
    static bool overflow = false;

    if (overflow)
    {
        size_t end_pos = packet_size - receive_buffer_temp_len + 1;
        print_memory_hex(data, packet_size);
        if (*(data + end_pos) == '\n')
        {

            mem_cpy(data_buf + data_buf_idx, receive_buffer_temp + 1, receive_buffer_temp_len - 1);
            data_buf_idx += (receive_buffer_temp_len - 1);
            mem_cpy(data_buf + data_buf_idx, data, end_pos);
            data_buf_idx += end_pos;
            packet_idx += 1;
            if (packet_idx >= packet_max)
            {
                packet_idx = 0;
                data_buf_idx = 0;
                packet_round += 1;
            }
            printf("good\n");
        }
        else
        {
            printf("false\n");
        }
        overflow = false;
        receive_buffer_temp_len = 0;
    }
    for (size_t idx = 0; idx < data_len; idx++)
    {
        char *cur_pos = data + idx;
        if (*cur_pos == '&')
        {
            size_t end_pos = idx + packet_size + 1;
            if (end_pos < data_len)
            {
                if (*(data + end_pos) == '\n')
                {
                    mem_cpy(data_buf + data_buf_idx, cur_pos + 1, packet_size);
                    data_buf_idx += packet_size;
                    packet_idx += 1;
                    if (packet_idx >= packet_max)
                    {
                        packet_idx = 0;
                        data_buf_idx = 0;
                        packet_round += 1;
                    }
                    idx = end_pos;
                }
            }
            else
            {
                receive_buffer_temp_len = data_len - idx;
                _memccpy(receive_buffer_temp, cur_pos, sizeof(char), receive_buffer_temp_len);
                overflow = true;
                print_memory_hex(receive_buffer_temp, receive_buffer_temp_len);
                return;
            }
        }
    }
}

int MetricFormatter(double value, char *buff, int size, void *data)
{
    const char *unit = (const char *)data;
    static double v[] = {1000000000, 1000000, 1000, 1, 0.001, 0.000001, 0.000000001};
    static const char *p[] = {"G", "M", "k", "", "m", "u", "n"};
    if (value == 0)
    {
        return snprintf(buff, size, "0 %s", unit);
    }
    for (int i = 0; i < 7; ++i)
    {
        if (fabs(value) >= v[i])
        {
            if (i > 3)
            {
                return snprintf(buff, size, "%g %s%s", value / v[i], p[i], unit);
            }
            else
            {
                int temp = snprintf(buff, size, "%lld %s%s", (int64_t)value / (int64_t)v[i], p[i], unit);
                double temp_value = (value - (int64_t)value);
                if (temp_value != 0)
                {
                    return MetricFormatter(temp_value, buff + temp, size - temp, data);
                }
                return temp;
            }
        }
    }
    return snprintf(buff, size, "%g %s%s", value / v[6], p[6], unit);
}

int MetricFormatter_orig(double value, char *buff, int size, void *data)
{
    const char *unit = (const char *)data;
    static double v[] = {1000000000, 1000000, 1000, 1, 0.001, 0.000001, 0.000000001};
    static const char *p[] = {"G", "M", "k", "", "m", "u", "n"};
    if (value == 0)
    {
        return snprintf(buff, size, "0 %s", unit);
    }
    for (int i = 0; i < 7; ++i)
    {
        if (fabs(value) >= v[i])
        {
            return snprintf(buff, size, "%g %s%s", value / v[i], p[i], unit);
        }
    }
    return snprintf(buff, size, "%g %s%s", value / v[6], p[6], unit);
}

static void ShowLogWindow(bool *p_open);
void Demo_TimeScale();
std::vector<data_channel> channels;
static std::thread t1;

void stop_socket()
{
    asio::error_code error;
    const std::string end_msg = "end!!";
    asio::write(data_socket, asio::buffer(end_msg), error);
    if (!error)
    {
        std::cout << "\nClient sent end message!" << std::endl;
    }
    else
    {
        std::cout << "send failed: " << error.message() << std::endl;
    }
    stop_transfer = true;
    if (t1.joinable())
    {
        t1.join();
        printf("\nprocess join\n");
    }
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char *desc)
{
    ImGui::TextDisabled("(" ICON_FA_QUESTION ")");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

int main()
{
    std::string code_str =
        "float vol; // vol\n"
        "uint8_t send_buf[7]; \n";

    channels = create_data_channel_array(code_str, &packet_size);
    packet_max = DATA_BUF_SIZE / packet_size;
    packet_buf_buf = packet_max / PAKCET_BUF_DIVSION;

    for (const auto &channel : channels)
    {
        std::cout << "ID: " << channel.id << ", Name: " << channel.name
                  << ", Byte Offset: " << channel.byte_offset << ", Data type: " << DataTypeStringDict[channel.data_type] << std::endl;
    }

    // init glfw context, load openGL 3.3 with core profile setting
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // create window using glfw library with 'LearnOpenGL' title
    GLFWwindow *window = glfwCreateWindow(800, 600, "DataRunner", NULL, NULL);
    // handle the failure
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // successfully create the window, now bind to the current context, make it activate.
    glfwMakeContextCurrent(window);

    // use GLAD library to load openGL function pointer give it to glfw
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // set openGL view port to help it transform normalize device coordinate to window coordinate
    // this value can be less than the real window value.
    glViewport(0, 0, 800, 600);

    // vsync
    glfwSwapInterval(1);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    char prog_directory[256];
    size_t len = sizeof(prog_directory);
// _WIN32 = we're in windows
#ifdef _WIN32
    // Windows
    int bytes = GetModuleFileName(NULL, prog_directory, (DWORD)len);
    char *lastBackslash = strrchr(prog_directory, '\\');
    if (lastBackslash != NULL)
    {
        *(lastBackslash + 1) = '\0'; // add null terminator after last backslash
    }
#else
    // Not windows
    int bytes = MIN(readlink("/proc/self/exe", prog_directory, len), len - 1);
    if (bytes >= 0)
        prog_directory[bytes] = '\0';
#endif

    char fontFilePath[512];

    // io.Fonts->AddFontDefault();
    snprintf(fontFilePath, sizeof(fontFilePath), "%sRoboto-Medium.ttf", prog_directory);
    io.Fonts->AddFontFromFileTTF(fontFilePath, 20.0f);

    // chinese font
    ImFontConfig config;
    config.MergeMode = true;
    config.OversampleH = 2;
    config.OversampleV = 2;
    config.GlyphExtraSpacing.x = 1.0f;
    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddText(u8"你好呀！");
    builder.BuildRanges(&ranges);
    snprintf(fontFilePath, sizeof(fontFilePath), "%sNotoSansSC-Medium.otf", prog_directory);
    io.Fonts->AddFontFromFileTTF(fontFilePath, 20.0f, &config, ranges.Data);
    // chinese font

    float baseFontSize = 20.0f;                      // 13.0f is the size of the default font. Change to the font size you use.
    float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.OversampleH = 2;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    snprintf(fontFilePath, sizeof(fontFilePath), "%s" FONT_ICON_FILE_NAME_FAS, prog_directory);
    io.Fonts->AddFontFromFileTTF(fontFilePath, iconFontSize, &icons_config, icons_ranges);
    // use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid
    io.Fonts->Build();

    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char *glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    static bool show_log_window = false;
    static bool show_implot_metrics = false;
    static bool show_implot_style_editor = false;
    static bool show_imgui_metrics = false;
    static bool show_imgui_style_editor = false;
    static bool show_app_stack_tool = false;
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_MenuBar); // Create a window called "Hello, world!" and append into it.
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Tools"))
            {
                ImGui::MenuItem("Show log window", NULL, &show_log_window);
                ImGui::MenuItem("ImPlot Metrics", nullptr, &show_implot_metrics);
                ImGui::MenuItem("ImPlot Style Editor", nullptr, &show_implot_style_editor);
                ImGui::Separator();
                ImGui::MenuItem("ImGui Metrics", nullptr, &show_imgui_metrics);
                ImGui::MenuItem("ImGui Style Editor", nullptr, &show_imgui_style_editor);
                ImGui::MenuItem("Stack Tool", NULL, &show_app_stack_tool);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        if (show_implot_metrics)
        {
            ImPlot::ShowMetricsWindow(&show_implot_metrics);
        }

        if (show_implot_style_editor)
        {
            ImGui::SetNextWindowSize(ImVec2(415, 762), ImGuiCond_Appearing);
            ImGui::Begin("Style Editor (ImPlot)", &show_implot_style_editor);
            ImPlot::ShowStyleEditor();
            ImGui::End();
        }
        if (show_imgui_style_editor)
        {
            ImGui::Begin("Style Editor (ImGui)", &show_imgui_style_editor);
            ImGui::ShowStyleEditor();
            ImGui::End();
        }
        if (show_imgui_metrics)
        {
            ImGui::ShowMetricsWindow(&show_imgui_metrics);
        }
        if (show_app_stack_tool)
        {
            ImGui::ShowStackToolWindow(&show_app_stack_tool);
        }

        static float progress = 0.0f;
        ImGui::PushItemWidth(220);
        if (packet_round == 0)
        {
            progress = (float)data_buf_idx / DATA_BUF_SIZE;
            ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
            ImGui::SameLine();
            static char memory_string[100];
            MetricFormatter_orig((double)data_buf_idx, memory_string, sizeof(memory_string), (void *)"B");
            ImGui::Text("packet num: %lld/%lld, memory usage: %s", packet_idx, packet_max, memory_string);
        }
        else
        {
            progress = 1.0f;
            ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
            ImGui::SameLine();
            ImGui::Text("packet num: %lld/%lld, bytes: %lld", packet_max, packet_max, data_buf_idx);
        }
        ImGui::PushItemWidth(0);

        ImGui::PushItemWidth(160);
        static char ip_addr[128] = "192.168.1.2";
        ImGui::InputTextWithHint("IP", "192.168.1.2", ip_addr, IM_ARRAYSIZE(ip_addr));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(160);
        static int port = 5000;
        ImGui::InputInt("Port", &port);
        ImGui::PopItemWidth();

        ImGui::SameLine();
        static std::string start_button_string = "Start";
        static bool connect_error = false;
        static std::string connect_error_msg;
        if (ImGui::Button(start_button_string.data()))
        {
            do
            {
                if (stop_transfer)
                {
                    try
                    {
                        // connection
                        data_socket.connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(ip_addr), port));
                    }
                    catch (std::exception &e)
                    {
                        std::cerr << e.what() << std::endl;
                        connect_error_msg = e.what();
                        connect_error = true;
                        break;
                    }
                    // data_socket.set_option(asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>{200});
                    t1 = std::thread(dataquery);
                    stop_transfer = false;
                    start_button_string = "Stop";
                }
                else
                {
                    stop_socket();
                    start_button_string = "Start";
                }
            } while (0);
        }

        if (connect_error)
            ImGui::OpenPopup("connect?");

        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("connect?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Seems that connect wrong.");
            ImGui::Text("Error: %s", connect_error_msg.data());
            ImGui::Separator();

            static bool dont_ask_me_next_time = false;
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
            ImGui::PopStyleVar();

            ImGui::Text("");
            ImGui::SameLine(ImGui::GetWindowWidth() / 2 - 120 / 2);
            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                connect_error = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::EndPopup();
        }

        ImGui::SameLine();
        static char frequency_string[100];
        MetricFormatter_orig(data_freq, frequency_string, 100, (void *)"Hz");
        ImGui::PushItemWidth(120);
        ImGui::DragFloat("data frequency", &data_freq, 1000.0f, 0.0f, 100000.0f, frequency_string);
        ImGui::PopItemWidth();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        Demo_TimeScale();

        ImGui::End();

        if (show_log_window)
            ShowLogWindow(&show_log_window);

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    stop_socket();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

static void ShowLogWindow(bool *p_open)
{
    ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Example: Long text display", p_open))
    {
        ImGui::End();
        return;
    }
    static int test_type = 0;
    static int lines = 0;
    ImGui::Text("Printing unusually long amount of text.");
    ImGui::Combo("Test type", &test_type,
                 "Single call to TextUnformatted()\0"
                 "Multiple calls to Text(), clipped\0"
                 "Multiple calls to Text(), not clipped (slow)\0");

    ImGui::BeginChild("Log");
    switch (test_type)
    {
    case 1:
    {
        // Multiple calls to Text(), manually coarsely clipped - demonstrate how to use the ImGuiListClipper helper.
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGuiListClipper clipper;
        if (packet_round == 0)
        {
            clipper.Begin((int)packet_idx);
        }
        else
        {
            clipper.Begin((int)(packet_max - packet_buf_buf));
        }
        char string_buf[1000];
        int16_t string_buf_idx = 0;
        while (clipper.Step())
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            {
                string_buf_idx = 0;
                char *data_buf_offset = data_buf + i * packet_size;
                for (const auto &channel : channels)
                {
                    switch (channel.data_type)
                    {
                    case DataType::DataType_uint8_t:
                        string_buf_idx += snprintf(string_buf + string_buf_idx, sizeof(string_buf), "%s: %d ", channel.name, *((uint8_t *)(data_buf_offset + channel.byte_offset)));
                        break;
                    case DataType::DataType_float:
                        string_buf_idx += snprintf(string_buf + string_buf_idx, sizeof(string_buf), "%s: %f ", channel.name, *((float *)(data_buf_offset + channel.byte_offset)));
                        break;
                    default:
                        break;
                    }
                }
                ImGui::Text("%s", string_buf);
            }
        ImGui::PopStyleVar();
        break;
    }
    }
    ImGui::EndChild();
    ImGui::End();
}

struct WaveData
{
    size_t base, id, stride, plot_min;
    WaveData(size_t Base, size_t Id, size_t Stride, size_t Plot_min)
    {
        base = Base;
        id = Id;
        stride = Stride;
        plot_min = Plot_min;
    }
};

ImPlotPoint DataWave(int idx, void *data)
{
    WaveData *wd = (WaveData *)data;
    char *packet_pos;
    if (packet_round > 0)
    {
        packet_pos = data_buf + (((wd->base + idx * wd->stride) % packet_max) * packet_size);
    }
    else
    {
        packet_pos = data_buf + ((wd->base + idx * wd->stride) * packet_size);
    }
    char *data_pos = packet_pos + channels[wd->id].byte_offset;

    double time = (wd->plot_min + idx * wd->stride) / data_freq;
    switch (channels[wd->id].data_type)
    {
    case DataType::DataType_int8_t:
        return ImPlotPoint(time, *((int8_t *)data_pos));
        break;
    case DataType::DataType_int16_t:
        return ImPlotPoint(time, *((int16_t *)data_pos));
        break;
    case DataType::DataType_int32_t:
        return ImPlotPoint(time, *((int32_t *)data_pos));
        break;
    case DataType::DataType_int64_t:
        return ImPlotPoint(time, (double)(*((int64_t *)data_pos)));
        break;
    case DataType::DataType_uint8_t:
        return ImPlotPoint(time, *((uint8_t *)data_pos));
        break;
    case DataType::DataType_uint16_t:
        return ImPlotPoint(time, *((uint16_t *)data_pos));
        break;
    case DataType::DataType_uint32_t:
        return ImPlotPoint(time, *((uint32_t *)data_pos));
        break;
    case DataType::DataType_uint64_t:
        return ImPlotPoint(time, (double)(*((uint64_t *)data_pos)));
        break;
    case DataType::DataType_float:
        return ImPlotPoint(time, *((float *)data_pos));
        break;
    case DataType::DataType_double:
        return ImPlotPoint(time, *((double *)data_pos));
        break;
    default:
        return ImPlotPoint(time, NAN);
        break;
    }
}

template <typename T>
inline T RandomRange(T min, T max)
{
    T scale = rand() / (T)RAND_MAX;
    return min + scale * (max - min);
}

ImVec4 RandomColor()
{
    ImVec4 col;
    col.x = RandomRange(0.0f, 1.0f);
    col.y = RandomRange(0.0f, 1.0f);
    col.z = RandomRange(0.0f, 1.0f);
    col.w = 1.0f;
    return col;
}

static const ImU32 Dark_Data[9] = {
    4280031972,
    4290281015,
    4283084621,
    4288892568,
    4278222847,
    4281597951,
    4280833702,
    4290740727,
    4288256409};

#define MAX_SUBPLOTS 4
void Demo_TimeScale()
{

    static bool one_time_run = true;
    static size_t channel_num = channels.size();

    // manage each channel plot configuration
    struct MyDndItem
    {
        int Plt;
        int plot_row;
        int plot_col;
        ImAxis Yax;
        char Label[100];
        bool color_enable;
        ImVec4 Color;
        float alpha;
        float thickness;
        bool markers;
        bool shaded;

        MyDndItem(char *name, ImVec4 color)
        {
            Plt = 0;
            plot_row = 0;
            plot_col = 0;
            Yax = ImAxis_Y1;
            Color = color;
            alpha = 1.0f;
            thickness = 1.0f;
            markers = true;
            shaded = false;
            color_enable = true;
            snprintf(Label, sizeof(Label), "%s", name);
        }
        void Reset()
        {
            Plt = 0;
            Yax = ImAxis_Y1;
        }
    };

    static std::vector<MyDndItem> dnd;

    // initialize each channel plot configuration
    if (one_time_run)
    {
        one_time_run = false;
        for (int i = 0; i < channel_num; i++)
        {
            if (i < 9)
            {
                dnd.push_back(MyDndItem(channels[i].name, ImGui::ColorConvertU32ToFloat4(Dark_Data[i])));
            }
            else
            {
                dnd.push_back(MyDndItem(channels[i].name, RandomColor()));
            }
        }
    }

    // control the subplot layout
    static int rows = 1;
    static int cols = 1;

    if (ImGui::Button("Subplot Layout"))
        ImGui::OpenPopup("Layout_popup");

    static char layout_selected[MAX_SUBPLOTS][MAX_SUBPLOTS] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
    static char lay_selection_string[10];
    static std::vector<ImVec4> axis_swap; // for swap subplot axis
    if (ImGui::BeginPopup("Layout_popup"))
    {
        for (int y = 0; y < MAX_SUBPLOTS; y++)
            for (int x = 0; x < MAX_SUBPLOTS; x++)
            {
                if (x > 0)
                    ImGui::SameLine();
                ImGui::PushID(y * MAX_SUBPLOTS + x);
                snprintf(lay_selection_string, sizeof(lay_selection_string), "(%dx%d)", y + 1, x + 1);
                if (ImGui::Selectable(lay_selection_string, layout_selected[y][x] != 0, 0, ImVec2(50, 50)))
                {
                    rows = y + 1;
                    cols = x + 1;
                    for (int col = 0; col < MAX_SUBPLOTS; col++)
                    {
                        for (int row = 0; row < MAX_SUBPLOTS; row++)
                        {
                            if (row < rows && col < cols)
                            {
                                layout_selected[row][col] = 1;
                            }
                            else
                            {
                                layout_selected[row][col] = 0;
                            }
                        }
                    }
                }
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    ImGui::SetDragDropPayload("subplot_DND", &ImVec2((float)y, (float)x), sizeof(ImVec2));
                    ImGui::Text("(%d,%d)", y + 1, x + 1);
                    ImGui::EndDragDropSource();
                }
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("subplot_DND"))
                    {
                        ImVec2 colrow = *(ImVec2 *)payload->Data;
                        for (int i = 0; i < channel_num; i++)
                        {
                            if (dnd[i].Plt == 1 && dnd[i].plot_col == colrow.y && dnd[i].plot_row == colrow.x)
                            {
                                dnd[i].plot_col = x;
                                dnd[i].plot_row = y;
                            }
                            else if (dnd[i].Plt == 1 && dnd[i].plot_col == x && dnd[i].plot_row == y)
                            {
                                dnd[i].plot_col = (int)colrow.y;
                                dnd[i].plot_row = (int)colrow.x;
                            }
                        }
                        axis_swap.push_back(ImVec4(colrow.x, colrow.y, y, x));
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::PopID();
            }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::Button(ICON_FA_RULER_VERTICAL);
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        ImGui::SetDragDropPayload("Vertical Cursor", nullptr, 0);
        ImGui::TextUnformatted(ICON_FA_RULER_VERTICAL "Vertical Cursor");
        ImGui::EndDragDropSource();
    }
    ImGui::SameLine();
    ImGui::Button(ICON_FA_RULER_HORIZONTAL);
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        ImGui::SetDragDropPayload("Horizontal Cursor", nullptr, 0);
        ImGui::TextUnformatted(ICON_FA_RULER_HORIZONTAL "Horizontal Cursor");
        ImGui::EndDragDropSource();
    }
    ImGui::SameLine();
    HelpMarker("Drag and drop to add cursor.");

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar;
    ImGui::Begin("Channel Pannel", nullptr, window_flags);
    static bool markers_check = false;
    ImGui::Checkbox("Markers", &markers_check);
    ImGui::Separator();
    // control the channel drag and drop labels
    ImGui::BeginChild("DND_LEFT", ImVec2(200, 400));
    if (ImGui::Button("Reset Data"))
    {
        for (int k = 0; k < channel_num; ++k)
            dnd[k].Reset();
    }

    for (int k = 0; k < channel_num; ++k)
    {
        if (dnd[k].Plt > 0)
            continue;
        ImPlot::ItemIcon(dnd[k].Color);
        ImGui::SameLine();
        ImGui::Selectable(dnd[k].Label, false, 0, ImVec2(200, 0));
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("MY_DND", &k, sizeof(int));
            ImPlot::ItemIcon(dnd[k].Color);
            ImGui::SameLine();
            ImGui::TextUnformatted(dnd[k].Label);
            ImGui::EndDragDropSource();
        }
    }
    ImGui::EndChild();
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("MY_DND"))
        {
            int i = *(int *)payload->Data;
            dnd[i].Reset();
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::End();

    // start drop subplot
    static int plot_height = 800;

    if (ImPlot::BeginSubplots("Signal Plot", rows, cols, ImVec2(-20.0f, (float)plot_height)))
    {
        // if subplot swap, swap the axis range
        static char plot_name[10];
        static ImGuiID plot_id;
        static ImGuiID subplot_id;
        if (axis_swap.size() > 0)
        {
            snprintf(plot_name, sizeof(plot_name), "##%d,%d", (int)axis_swap[0].y, (int)axis_swap[0].x);
            subplot_id = (*GImGui).CurrentWindow->GetID((int)axis_swap[0].y + (int)axis_swap[0].x * cols);
            plot_id = ImHashStr(plot_name, NULL, subplot_id);
            ImPlotPlot &plot = *((*GImPlot).Plots.GetByKey(plot_id));

            snprintf(plot_name, sizeof(plot_name), "##%d,%d", (int)axis_swap[0].w, (int)axis_swap[0].z);
            subplot_id = (*GImGui).CurrentWindow->GetID((int)axis_swap[0].w + (int)axis_swap[0].z * cols);
            plot_id = ImHashStr(plot_name, NULL, subplot_id);
            ImPlotPlot &plot2 = *((*GImPlot).Plots.GetByKey(plot_id));

            if (&plot != NULL && &plot2 != NULL)
            {
                for (int y = ImAxis_X1; y < ImAxis_COUNT; ++y)
                {
                    ImPlotRange range = plot.Axes[y].Range;
                    plot.Axes[y].SetRange(plot2.Axes[y].Range);
                    plot2.Axes[y].SetRange(range);
                }
            }

            axis_swap.pop_back();
        }
        struct cursor_info
        {
            double pos;
            ImVec4 color;

            cursor_info(double Pos)
            {
                pos = Pos;
                color = RandomColor();
            }
        };
        static std::vector<cursor_info> x_cursor[MAX_SUBPLOTS][MAX_SUBPLOTS];
        static std::vector<cursor_info> y_cursor[MAX_SUBPLOTS][MAX_SUBPLOTS];
        for (int plot_row = 0; plot_row < rows; ++plot_row)
        {
            for (int plot_col = 0; plot_col < cols; ++plot_col)
            {
                snprintf(plot_name, sizeof(plot_name), "##%d,%d", plot_col, plot_row);
                if (ImPlot::BeginPlot(plot_name, ImVec2(-1, 400)))
                {
                    ImPlot::SetupAxesLimits(0, 1, 0, 1);
                    ImPlot::SetupAxisFormat(ImAxis_X1, MetricFormatter, (void *)"S");
                    ImPlot::SetupAxisFormat(ImAxis_Y1, MetricFormatter, (void *)"V");
                    ImPlot::SetupAxis(ImAxis_Y1, nullptr);
                    ImPlot::SetupAxis(ImAxis_Y2, nullptr);
                    ImPlot::SetupAxis(ImAxis_Y3, nullptr, ImPlotAxisFlags_Opposite);

                    // allow the main plot area to be a DND target
                    if (ImPlot::BeginDragDropTargetPlot())
                    {
                        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("MY_DND"))
                        {
                            int i = *(int *)payload->Data;
                            dnd[i].Plt = 1;
                            dnd[i].plot_col = plot_col;
                            dnd[i].plot_row = plot_row;
                            dnd[i].Yax = ImAxis_Y1;
                        }
                        else if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Horizontal Cursor"))
                        {
                            y_cursor[plot_col][plot_row].push_back(cursor_info(ImPlot::GetPlotMousePos().y));
                        }
                        else if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("Vertical Cursor"))
                        {
                            x_cursor[plot_col][plot_row].push_back(cursor_info(ImPlot::GetPlotMousePos().x));
                        }
                        ImPlot::EndDragDropTarget();
                    }

                    for (int idx = 0; idx < x_cursor[plot_col][plot_row].size(); idx++)
                    {
                        ImPlot::DragLineX(idx + 3000, &x_cursor[plot_col][plot_row][idx].pos, x_cursor[plot_col][plot_row][idx].color, 1, ImPlotDragToolFlags_None);
                        char tag_string[100];
                        MetricFormatter(x_cursor[plot_col][plot_row][idx].pos, tag_string, sizeof(tag_string), (void *)"S");
                        if (ImPlot::BeginDragLineXPopup(idx + 3000, x_cursor[plot_col][plot_row][idx].pos))
                        {
                            ImGui::Text("%d", idx + 3000);
                            ImGui::ColorEdit3("Color", &x_cursor[plot_col][plot_row][idx].color.x);
                            ImPlot::EndDragLinePopup();
                        }
                        ImPlot::TagX(x_cursor[plot_col][plot_row][idx].pos, x_cursor[plot_col][plot_row][idx].color, "%d:%s", idx, tag_string);
                        const ImVec2 size = ImGui::CalcTextSize("hello");
                        const ImVec2 pos = ImPlot::GetLocationPos(GImPlot->CurrentPlot->PlotRect, size, ImPlotLocation_NorthEast, GImPlot->Style.MousePosPadding);
                        GImGui->CurrentWindow->DrawList->AddText(pos, ImPlot::GetStyleColorU32(ImPlotCol_InlayText), "hello");
                    }
                    for (int idx = 0; idx < y_cursor[plot_col][plot_row].size(); idx++)
                    {
                        ImPlot::DragLineY(idx + 2000, &y_cursor[plot_col][plot_row][idx].pos, y_cursor[plot_col][plot_row][idx].color, 1, ImPlotDragToolFlags_None);
                        char tag_string[100];
                        MetricFormatter(y_cursor[plot_col][plot_row][idx].pos, tag_string, sizeof(tag_string), (void *)"V");
                        if (ImPlot::BeginDragLineYPopup(idx + 2000, y_cursor[plot_col][plot_row][idx].pos))
                        {
                            ImGui::Text("%d", idx + 2000);
                            ImGui::ColorEdit3("Color", &y_cursor[plot_col][plot_row][idx].color.x);
                            ImPlot::EndDragLinePopup();
                        }
                        ImPlot::TagY(y_cursor[plot_col][plot_row][idx].pos, y_cursor[plot_col][plot_row][idx].color, "%d:%s", idx, tag_string);
                    }


                    // allow each y-axis to be a DND target
                    for (int y = ImAxis_Y1; y <= ImAxis_Y3; ++y)
                    {
                        if (ImPlot::BeginDragDropTargetAxis(y))
                        {
                            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("MY_DND"))
                            {
                                int i = *(int *)payload->Data;
                                dnd[i].Plt = 1;
                                dnd[i].plot_col = plot_col;
                                dnd[i].plot_row = plot_row;
                                dnd[i].Yax = y;
                            }
                            ImPlot::EndDragDropTarget();
                        }
                    }
                    // allow the legend to be a DND target
                    if (ImPlot::BeginDragDropTargetLegend())
                    {
                        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("MY_DND"))
                        {
                            int i = *(int *)payload->Data;
                            dnd[i].Plt = 1;
                            dnd[i].plot_col = plot_col;
                            dnd[i].plot_row = plot_row;
                            dnd[i].Yax = ImAxis_Y1;
                        }
                        ImPlot::EndDragDropTarget();
                    }

                    // calculate the axis and real data index mapping
                    int64_t plot_t_min = (size_t)(ImPlot::GetPlotLimits().X.Min * data_freq);
                    int64_t plot_t_max = (size_t)(ImPlot::GetPlotLimits().X.Max * data_freq) + 3;

                    int64_t t_min = data_idx_min();
                    int64_t t_max = data_idx_max();

                    if (plot_t_max < t_min)
                    {
                        printf("max %lld %lld\n", plot_t_max, t_min);
                        ImPlot::EndPlot();
                        continue;
                    }

                    if (plot_t_min > t_max)
                    {
                        printf("min %lld %lld\n", plot_t_min, t_max);
                        ImPlot::EndPlot();
                        continue;
                    }

                    if (plot_t_min < t_min)
                    {
                        plot_t_min = t_min;
                    }
                    if (plot_t_max > t_max)
                    {
                        plot_t_max = t_max;
                    }

                    size_t base = 0;
                    if (packet_round > 0)
                    {
                        base = plot_t_min - t_min + packet_idx + packet_buf_buf;
                    }
                    else
                    {
                        base = plot_t_min - t_min;
                    }

                    size_t size = plot_t_max - plot_t_min;
                    size_t stride = 0;
                    if (size < 1000)
                    {
                        stride = 1;
                    }
                    else
                    {
                        stride = size / 1000;
                    }

                    size_t count = size / stride;

                    // start plot each channel in the plot
                    for (const auto &channel : channels)
                    {
                        if (dnd[channel.id].Plt == 1 && dnd[channel.id].plot_col == plot_col && dnd[channel.id].plot_row == plot_row)
                        {
                            // allow legend item labels to be DND sources
                            if (ImPlot::BeginDragDropSourceItem(dnd[channel.id].Label))
                            {
                                ImGui::SetDragDropPayload("MY_DND", &channel.id, sizeof(int));
                                ImPlot::ItemIcon(dnd[channel.id].Color);
                                ImGui::SameLine();
                                ImGui::TextUnformatted(dnd[channel.id].Label);
                                ImPlot::EndDragDropSource();
                            }
                            ImPlot::SetAxis(dnd[channel.id].Yax);
                            WaveData data5(base, channel.id, stride, plot_t_min);
                            if (dnd[channel.id].markers && markers_check)
                            {
                                ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, dnd[channel.id].alpha);
                                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
                            }

                            if (dnd[channel.id].color_enable)
                            {
                                ImPlot::SetNextLineStyle(dnd[channel.id].Color, dnd[channel.id].thickness);
                            }
                            else
                            {
                                ImPlot::SetNextLineStyle(IMPLOT_AUTO_COL, dnd[channel.id].thickness);
                            }
                            ImPlot::PlotLineG(channel.name, DataWave, &data5, (int)count);
                            auto lamda = [](int idx, void *data)
                            {
                                WaveData *wd = (WaveData *)data;
                                return ImPlotPoint((wd->plot_min + idx * wd->stride) / data_freq, 0);
                            };
                            if (dnd[channel.id].shaded)
                                ImPlot::PlotShadedG(channel.name, DataWave, &data5, lamda, &data5, (int)count);
                            // custom legend context menu
                            if (ImPlot::BeginLegendPopup(channel.name))
                            {
                                ImGui::Checkbox("Color", &dnd[channel.id].color_enable);
                                if (dnd[channel.id].color_enable)
                                {
                                    ImGui::SameLine();
                                    ImGui::ColorEdit3("Color", &dnd[channel.id].Color.x);
                                }
                                ImGui::PushItemWidth(100);
                                ImGui::SliderFloat("Thickness", &(dnd[channel.id].thickness), 0, 5);
                                ImGui::PopItemWidth();
                                ImGui::SameLine();
                                ImGui::PushItemWidth(100);
                                ImGui::SliderFloat("Line transparency", &(dnd[channel.id].Color.w), 0, 1);
                                ImGui::PopItemWidth();
                                ImGui::Checkbox("Markers", &dnd[channel.id].markers);
                                if (dnd[channel.id].markers)
                                {
                                    ImGui::SameLine();
                                    ImGui::SliderFloat("Transparency", &dnd[channel.id].alpha, 0, 1, "%.2f");
                                }
                                ImGui::Checkbox("Shaded", &dnd[channel.id].shaded);
                                ImPlot::EndLegendPopup();
                            }
                        }
                    }
                    ImPlot::EndPlot();
                }
            }
        }
        ImPlot::EndSubplots();
    }

    ImGui::PushID("vsliderforplot");
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(4 / 7.0f, 0.5f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(4 / 7.0f, 0.6f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(4 / 7.0f, 0.7f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, (ImVec4)ImColor::HSV(4 / 7.0f, 0.9f, 0.9f));
    ImGui::VSliderInt("##v", ImVec2(18, 400), &plot_height, 400, 2000, "");
    if (ImGui::IsItemActive() || ImGui::IsItemHovered())
        ImGui::SetTooltip("height: %d", plot_height);
    ImGui::PopStyleColor(4);
    ImGui::PopID();
}

// TODO: self-test socket and plot(socket server and transfer data)
// TODO: cursor delta-x
// TODO: Config window and dataform
// TODO: Find max, min, mean, frequency
// TODO: magnifier
// TODO: multiple draw in subplot for a single signal
// TODO: data saving
// TODO: image export
// TODO: Math expression

// features:
// TODO: logic analyser
// TODO: system analyser
// TODO: code generator and simulation(separate program)

// do a general data plotter separate the data protocol
