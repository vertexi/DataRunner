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
            return snprintf(buff, size, "%g %s%s", value / v[i], p[i], unit);
        }
    }
    return snprintf(buff, size, "%g %s%s", value / v[6], p[6], unit);
}

static void ShowLogWindow(bool *p_open);
void Demo_TimeScale();
std::vector<data_channel> channels;
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
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

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

    char buf[200] = "hello";
    float f = 0.0f;

    static bool show_log_window = true;
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
            if (ImGui::BeginMenu("Examples"))
            {
                ImGui::MenuItem("Show log window", NULL, &show_log_window);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        static float progress = 0.0f;
        progress = (float)data_buf_idx / DATA_BUF_SIZE;
        ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::Text("Progress Bar");

        ImGui::Text("packet num: %lld, bytes: %lld", packet_idx, data_buf_idx);

        ImGui::PushItemWidth(160);
        static char ip_addr[128] = "192.168.1.2";
        ImGui::InputTextWithHint("IP", "192.168.1.2", ip_addr, IM_ARRAYSIZE(ip_addr));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(80);
        static int port = 5000;
        ImGui::InputInt("Port", &port);
        ImGui::PopItemWidth();

        ImGui::SameLine();
        static std::thread t1;
        static std::string start_button_string = "Start";
        if (ImGui::Button(start_button_string.data()))
        {
            if (stop_transfer)
            {
                // connection
                data_socket.connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(ip_addr), port));
                // data_socket.set_option(asio::detail::socket_option::integer<SOL_SOCKET, SO_RCVTIMEO>{200});
                t1 = std::thread(dataquery);
                stop_transfer = false;
                start_button_string = "Stop";
            }
            else
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
                start_button_string = "Start";
                t1.join();

                printf("\njoin\n");
            }
        }

        static char frequency_string[1000];
        MetricFormatter(data_freq, frequency_string, 1000, (void *)"Hz");
        ImGui::SliderFloat("data frequency", &data_freq, 1000.0f, 100000.0f, frequency_string);

        ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        if (ImGui::Button("print"))
        {
            printf("%s", buf);
            printf("%f", f);
        }
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        static int clicked = 0;
        if (ImGui::Button("Plot"))
            clicked++;
        if (clicked & 1)
        {
            ImGui::SameLine();
            ImGui::Text("Thanks for clicking me!");
            Demo_TimeScale();
        }
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

        glfwSwapBuffers(window);
    }

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
            clipper.Begin(packet_idx);
        }
        else
        {
            clipper.Begin(packet_max - packet_buf_buf);
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
                        string_buf_idx += snprintf(string_buf + string_buf_idx, sizeof(string_buf),"%s: %d ", channel.name, *((uint8_t *)(data_buf_offset + channel.byte_offset)));
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
    case DataType::DataType_uint8_t:
        return ImPlotPoint(time, *((uint8_t *)data_pos));
        break;
    case DataType::DataType_float:
        return ImPlotPoint(time, *((float *)data_pos));
        break;
    default:
        break;
    }
}

void Demo_TimeScale()
{

    if (ImPlot::BeginPlot("Singnal Plot", ImVec2(-1, 0)))
    {
        ImPlot::SetupAxesLimits(0, 1, 0, 1);
        ImPlot::SetupAxisFormat(ImAxis_X1, MetricFormatter, (void *)"s");

        int64_t plot_t_min = (size_t)(ImPlot::GetPlotLimits().X.Min * data_freq);
        int64_t plot_t_max = (size_t)(ImPlot::GetPlotLimits().X.Max * data_freq) + 3;

        int64_t t_min = data_idx_min();
        int64_t t_max = data_idx_max();

        if (plot_t_max < t_min)
        {
            printf("max %lld %lld\n", plot_t_max, t_min);
            ImPlot::EndPlot();
            return;
        }

        if (plot_t_min > t_max)
        {
            printf("min %lld %lld\n", plot_t_min, t_max);
            ImPlot::EndPlot();
            return;
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

        static bool one_time_run = true;
        static size_t channel_num = channels.size();
        static std::vector<int8_t> channel_enable_colors;
        static std::vector<ImVec4> channel_colors;
        static std::vector<float> channel_alphas;
        static std::vector<float> channel_thickness;
        static std::vector<int8_t> channel_markers;
        static std::vector<int8_t> channel_shaded;

        if (one_time_run)
        {
            one_time_run = false;
            for (int i = 0; i < channel_num; i++)
            {
                channel_enable_colors.push_back(0);
                channel_colors.push_back(ImVec4(1, 1, 0, 1));
                channel_alphas.push_back(1.0f);
                channel_thickness.push_back(1.0f);
                channel_markers.push_back(false);
                channel_shaded.push_back(false);
            }
        }

        for (const auto &channel : channels)
        {
            WaveData data5(base, channel.id, stride, plot_t_min);
            if (channel_markers[channel.id])
            {
                ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, channel_alphas[channel.id]);
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
            }

            if (channel_enable_colors[channel.id])
            {
                ImPlot::SetNextLineStyle(channel_colors[channel.id], channel_thickness[channel.id]);
            }
            else
            {
                ImPlot::SetNextLineStyle(IMPLOT_AUTO_COL, channel_thickness[channel.id]);
            }
            ImPlot::PlotLineG(channel.name, DataWave, &data5, count);
            auto lamda = [](int idx, void *data)
            {
                WaveData *wd = (WaveData *)data;
                return ImPlotPoint((wd->plot_min + idx * wd->stride) / data_freq, 0);
            };
            if (channel_shaded[channel.id])
                ImPlot::PlotShadedG(channel.name, DataWave, &data5, lamda, &data5, count);
            // custom legend context menu
            if (ImPlot::BeginLegendPopup(channel.name))
            {
                ImGui::Checkbox("Color", (bool *)(&(channel_enable_colors[channel.id])));
                if (channel_enable_colors[channel.id])
                {
                    ImGui::SameLine();
                    ImGui::ColorEdit3("Color", &channel_colors[channel.id].x);
                }
                ImGui::SliderFloat("Thickness", &(channel_thickness[channel.id]), 0, 5);
                ImGui::Checkbox("Markers", (bool *)&(channel_markers[channel.id]));
                if (channel_markers[channel.id])
                {
                    ImGui::SameLine();
                    ImGui::SliderFloat("Transparency", &(channel_alphas[channel.id]), 0, 1, "%.2f");
                }
                ImGui::Checkbox("Shaded", (bool *)&(channel_shaded[channel.id]));
                ImPlot::EndLegendPopup();
            }
        }
        ImPlot::EndPlot();
    }
}
