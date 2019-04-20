#pragma once

#include "global.hpp"
#include "Renderer/Renderer.hpp"
#include <BuildVersion.hpp>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <sstream>
#include <thread>

namespace KompotEngine {

struct EngineConfig
{
    bool isEditMode;
    bool isFullscreen;
    uint32_t windowWidth;
    uint32_t windowHeight;
    bool isMaximized;
};

class Engine
{
public:
    Engine(const std::string&, const EngineConfig&);
    ~Engine();

    void run();
private:
    std::string         m_instanceName;
    EngineConfig        m_engineSettings;
    Renderer::Renderer *m_renderer = nullptr;

    GLFWwindow* m_glfwWindowHandler;

    // GLFW callbacks

    static void keyCallback(GLFWwindow*, int, int, int, int);
    static void characterCallback(GLFWwindow*, unsigned int, int);
    static void resizeCallback(GLFWwindow*, int, int);
};

} // KompotEngine namespace
