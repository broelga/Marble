#include "knpch.h"
#include "ImGuiLayer.h"

#include "imgui.h"
#include "Kono/Platform/OpenGL/ImGuiOpenGLRenderer.h"
#include "Kono/Platform/GLFW/ImGuiGLFWRenderer.h"

#include "Kono/Application.h"
#include "Kono/KeyCodes.h"

// TEMPORARY
#include <glad/glad.h>

namespace Kono {

    ImGuiLayer::ImGuiLayer()
            : Layer("ImGuiLayer") {}

    ImGuiLayer::~ImGuiLayer() {}

    void ImGuiLayer::OnAttach() {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO &io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        // TEMPORARY: should eventually use Kono key codes
        io.KeyMap[ImGuiKey_Tab] = KN_KEY_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = KN_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = KN_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = KN_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = KN_KEY_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = KN_KEY_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = KN_KEY_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = KN_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = KN_KEY_END;
        io.KeyMap[ImGuiKey_Insert] = KN_KEY_INSERT;
        io.KeyMap[ImGuiKey_Delete] = KN_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = KN_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Space] = KN_KEY_SPACE;
        io.KeyMap[ImGuiKey_Enter] = KN_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = KN_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_KeyPadEnter] = KN_KEY_KP_ENTER;
        io.KeyMap[ImGuiKey_A] = KN_KEY_A;
        io.KeyMap[ImGuiKey_C] = KN_KEY_C;
        io.KeyMap[ImGuiKey_V] = KN_KEY_V;
        io.KeyMap[ImGuiKey_X] = KN_KEY_X;
        io.KeyMap[ImGuiKey_Y] = KN_KEY_Y;
        io.KeyMap[ImGuiKey_Z] = KN_KEY_Z;

        Application &app = Application::Get();
        GLFWwindow *window = static_cast<GLFWwindow *>(app.GetWindow().GetNativeWindow());

        io.ClipboardUserData = window;
        ImGui_ImplOpenGL3_Init("#version 410 core");
    }

    void ImGuiLayer::OnDetach() {
    }

    void ImGuiLayer::OnUpdate() {
        ImGuiIO &io = ImGui::GetIO();
        Application &app = Application::Get();
        io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

        float time = static_cast<float>(ImGui::GetTime());
        io.DeltaTime = m_Time > 0.0f ? (time - m_Time) : (1.0f / 60.0f);
        m_Time = time;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        static bool show = true;
        ImGui::ShowDemoWindow(&show);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiLayer::OnEvent(Event &event) {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<MouseButtonPressedEvent>(KN_BIND_EVENT_FN(ImGuiLayer::OnMouseButtonPressedEvent));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(KN_BIND_EVENT_FN(ImGuiLayer::OnMouseButtonReleasedEvent));
        dispatcher.Dispatch<MouseMovedEvent>(KN_BIND_EVENT_FN(ImGuiLayer::OnMouseMovedEvent));
        dispatcher.Dispatch<MouseScrolledEvent>(KN_BIND_EVENT_FN(ImGuiLayer::OnMouseScrolledEvent));
        dispatcher.Dispatch<KeyPressedEvent>(KN_BIND_EVENT_FN(ImGuiLayer::OnKeyPressedEvent));
        dispatcher.Dispatch<KeyReleasedEvent>(KN_BIND_EVENT_FN(ImGuiLayer::OnKeyReleasedEvent));
        dispatcher.Dispatch<KeyTypedEvent>(KN_BIND_EVENT_FN(ImGuiLayer::OnKeyTypedEvent));
        dispatcher.Dispatch<WindowResizeEvent>(KN_BIND_EVENT_FN(ImGuiLayer::OnWindowResizeEvent));
    }

    bool ImGuiLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseDown[e.GetMouseButton()] = true;

        return false;
    }

    bool ImGuiLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseDown[e.GetMouseButton()] = false;

        return false;
    }

    bool ImGuiLayer::OnMouseMovedEvent(MouseMovedEvent &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.MousePos = ImVec2(e.GetX(), e.GetY());

        return false;
    }

    bool ImGuiLayer::OnMouseScrolledEvent(MouseScrolledEvent &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.MouseWheel += e.GetYOffset();
        io.MouseWheelH += e.GetXOffset();

        return false;
    }

    bool ImGuiLayer::OnKeyPressedEvent(KeyPressedEvent &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.KeysDown[e.GetKeyCode()] = true;

        // Check if key modifiers are pressed
        io.KeyCtrl = io.KeysDown[KN_KEY_LEFT_CONTROL] || io.KeysDown[KN_KEY_RIGHT_CONTROL];
        io.KeyShift = io.KeysDown[KN_KEY_LEFT_SHIFT] || io.KeysDown[KN_KEY_RIGHT_SHIFT];
        io.KeyAlt = io.KeysDown[KN_KEY_LEFT_ALT] || io.KeysDown[KN_KEY_RIGHT_ALT];
        io.KeySuper = io.KeysDown[KN_KEY_LEFT_SUPER] || io.KeysDown[KN_KEY_RIGHT_SUPER];

        // TODO: Implement copy and paste -- see: OnAttach()

        return false;
    }

    bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.KeysDown[e.GetKeyCode()] = false;

        return false;
    }

    bool ImGuiLayer::OnKeyTypedEvent(KeyTypedEvent &e) {
        ImGuiIO &io = ImGui::GetIO();
        int keycode = e.GetKeyCode();
        io.AddInputCharacter(keycode);

        return false;
    }

    bool ImGuiLayer::OnWindowResizeEvent(WindowResizeEvent &e) {
        ImGuiIO &io = ImGui::GetIO();
        io.DisplaySize = ImVec2(e.GetWidth(), e.GetHeight());
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
        glViewport(0, 0, e.GetWidth(), e.GetHeight());

        return false;
    }
}