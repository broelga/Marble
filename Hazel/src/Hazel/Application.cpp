#include "hzpch.h"
#include "Application.h"

#include "Hazel/Log.h"
#include "Input.h"
#include "KeyCodes.h"

#include "Hazel/Renderer/Renderer.h"

namespace Hazel {

    Application *Application::s_Instance = nullptr;

    Application::Application()
            : m_Camera(-1.6f, 1.6f, -0.9f, 0.9) {

        HZ_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        m_Window = std::unique_ptr<Window>(Window::Create());
        m_Window->SetEventCallback(HZ_BIND_EVENT_FN(Application::OnEvent));

        m_ImGuiLayer = new ImGuiLayer();
        PushLayer(m_ImGuiLayer);

        m_VertexArray.reset(VertexArray::Create());

        float vertices[3 * 7] = {
                -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f, // Left side of the screen
                0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,// Right side of the screen
                0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f // Top of screen
        };

        std::shared_ptr<VertexBuffer> vertexBuffer;
        vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
        BufferLayout layout = {
                {ShaderDataType::Float3, "a_Position"},
                {ShaderDataType::Float4, "a_Color"}
        };
        vertexBuffer->SetLayout(layout);
        m_VertexArray->AddVertexBuffer(vertexBuffer);

        uint32_t indices[3] = {0, 1, 2};
        std::shared_ptr<IndexBuffer> indexBuffer;
        indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
        m_VertexArray->SetIndexBuffer(indexBuffer);

        m_SquareVA.reset(VertexArray::Create());

        float squareVertices[3 * 4] = {
                -0.75f, -0.75f, 0.0f,
                0.75f, -0.75f, 0.0f,
                0.75f, 0.75f, 0.0f,
                -0.75f, 0.75f, 0.0f
        };

        std::shared_ptr<VertexBuffer> squareVB;
        squareVB.reset(VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
        squareVB->SetLayout({{ShaderDataType::Float3, "a_Position"}});
        m_SquareVA->AddVertexBuffer(squareVB);

        uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};

        std::shared_ptr<IndexBuffer> squareIB;
        squareIB.reset(IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
        m_SquareVA->SetIndexBuffer(squareIB);

        // Provide source code to OpenGL
        // `R` (raw_characters) precursor allows you to write multi-line string
        /** @param a_Position the position of @vertices
         * adding to a_Position will change the location of vertices */
        std::string vertexSrc = R"(
            #version 330 core

            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec4 a_Color;

            uniform mat4 u_ViewProjection;

            out vec3 v_Position;
            out vec4 v_Color;

            void main() {
                v_Position = a_Position;
                v_Color = a_Color;
                gl_Position =  u_ViewProjection * vec4(a_Position, 1.0);
            }
        )";

        // (1.0f, 0.0f, 0.0f) == Red
        // (1.0f, 0.5f, 0.15f) == Orange
        std::string fragmentSrc = R"(
            #version 330 core

            layout(location = 0) out vec4 color;

            in vec3 v_Position;
            in vec4 v_Color;

            void main() {
                color = vec4(v_Position * 0.5 + 0.5, 1.0); // This moves color range from [-1,1] to [0,1] range
                color = v_Color;
            }
        )";

        // Use std::reset to reset the shader
        m_Shader.reset(new Shader(vertexSrc, fragmentSrc));

        std::string blueShaderVertexSrc = R"(
            #version 330 core

            layout(location = 0) in vec3 a_Position;

            uniform mat4 u_ViewProjection;

            out vec3 v_Position;

            void main() {
                v_Position = a_Position;
                gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
            }
        )";

        // (1.0f, 0.0f, 0.0f) == Red
        // (1.0f, 0.5f, 0.15f) == Orange
        std::string blueShaderFragmentSrc = R"(
            #version 330 core

            layout(location = 0) out vec4 color;

            in vec3 v_Position;

            void main() {
                color = vec4(0.2, 0.3, 0.8, 1.0);
            }
        )";

        // Use std::reset to reset the shader
        m_BlueShader.reset(new Shader(blueShaderVertexSrc, blueShaderFragmentSrc));
    }

    Application::~Application() {}

    void Application::PushLayer(Layer *layer) {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer *layer) {
        m_LayerStack.PushOverlay(layer);
        layer->OnAttach();
    }

    void Application::OnEvent(Event &e) {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(HZ_BIND_EVENT_FN(Application::OnWindowClose));

        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
            (*--it)->OnEvent(e);
            if (e.Handled()) {
                break;
            }
        }
    }

    void Application::Run() {
        while (m_Running) {
            RenderCommand::GetError(); // Print any errors
            RenderCommand::SetClearColor({0.2f, 0.1f, 0.2f, 1});
            RenderCommand::Clear();

            static float xPos = 0.0f;
            static float yPos = 0.0f;
            static float rotation = 0.0f;
            {
                if (Input::IsKeyPressed(HZ_KEY_S)) {
                    yPos += 0.01;
                }
                if (Input::IsKeyPressed(HZ_KEY_W)) {
                    yPos -= 0.01;
                }
                if (Input::IsKeyPressed(HZ_KEY_A)) {
                    xPos += 0.01;
                }
                if (Input::IsKeyPressed(HZ_KEY_D)) {
                    xPos -= 0.01;
                }
                if (Input::IsKeyPressed(HZ_KEY_Q)) {
                    rotation += 0.5;
                }
                if (Input::IsKeyPressed(HZ_KEY_E)) {
                    rotation -= 0.5;
                }
                if (Input::IsKeyPressed(HZ_KEY_SPACE)) {
                    xPos = 0.0f;
                    yPos = 0.0f;
                    rotation = 0.0f;
                }
            }

            m_Camera.SetPosition({xPos, yPos, 0.0f});
            m_Camera.SetRotation(rotation);

            Renderer::BeginScene(m_Camera);
            {
                Renderer::Submit(m_BlueShader, m_SquareVA);
                Renderer::Submit(m_Shader, m_VertexArray);
                Renderer::EndScene();
            }

            for (Layer *layer : m_LayerStack) {
                layer->OnUpdate();
            }

            m_ImGuiLayer->Begin();
            for (Layer *layer : m_LayerStack) {
                layer->OnImGuiRender();
            }
            m_ImGuiLayer->End();

            m_Window->OnUpdate();
        }
    }

    bool Application::OnWindowClose(WindowCloseEvent(&e)) {
        m_Running = false;
        return true;
    }

}




