#include <iostream>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "implot.h"
#include "implot_internal.h"
#ifdef _WIN32
    #include <windows.h>
    #include <libloaderapi.h>
#endif

#include "IconsFontAwesome6.h"

GLFWwindow *window;

#include "glfw_opengl3.hpp"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int window_init()
{
    // init glfw context, load openGL 3.3 with core profile setting
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // create window using glfw library with 'LearnOpenGL' title
    window = glfwCreateWindow(1280, 720, "DataRunner", NULL, NULL);
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
    // glViewport(0, 0, 800, 600);

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
    snprintf(fontFilePath, sizeof(fontFilePath), "%sRoboto-Medium.ttf", prog_directory);

    // io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF(fontFilePath, 20.0f);

    // chinese font
    ImFontConfig config;
    config.MergeMode = true;
    config.OversampleH = 2;
    config.OversampleV = 2;
    config.GlyphExtraSpacing.x = 1.0f;
    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    // builder.BuildRanges(&ranges);
    // snprintf(fontFilePath, sizeof(fontFilePath), "%sNotoSansSC-Medium.otf", prog_directory);
    // io.Fonts->AddFontFromFileTTF(fontFilePath, 20.0f, &config, ranges.Data);
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

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char *glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    return 0;
}

void render()
{
    ImGuiIO &io = ImGui::GetIO();
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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

void stop()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}