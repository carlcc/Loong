#include <glad/glad.h>

#ifdef _MSC_VER
// e.g. This function or variable may be unsafe. Consider using fopen_s instead.
#pragma warning(disable : 4996)
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Flags.h"
#include "LoongApp/Driver.h"
#include "LoongApp/LoongWindow.h"
#include "LoongAsset/LoongImage.h"
#include "LoongFoundation/LoongDefer.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongResource/Driver.h"
#include "LoongResource/LoongFrameBuffer.h"
#include "LoongResource/LoongGpuBuffer.h"
#include "LoongResource/LoongShader.h"
#include "LoongResource/LoongTexture.h"
#include "LoongResource/LoongVertexArray.h"
#include "LoongResource/loader/LoongTextureLoader.h"
#include <cstdint>
#include <imgui.h>
#include <iostream>
#include <string>
#include <vector>

namespace Loong::CubeToPanorama {

// https://github.com/carlcc/cube2equirect
const char* kVertexShaderSource = R"(#version 330 core

out vec2 vTexCoord;

void main() {
        vec3 vertices[] = vec3[6](
            vec3(-1.0, -1.0, 0.0), // left,  bottom
            vec3(1.0, -1.0, 0.0), // right, bottom
            vec3(-1.0, 1.0, 0.0), // left,  top

            vec3(1.0, -1.0, 0.0), // right, bottom
            vec3(1.0, 1.0, 0.0 ), // right, top
            vec3(-1.0, 1.0, 0.0) // left,  top
        );
        vec2 uvs[] = vec2[6](
            vec2(-1.0, -1.0), // left,  bottom
            vec2(1.0, -1.0), // right, bottom
            vec2(-1.0, 1.0), // left,  top

            vec2(1.0, -1.0), // right, bottom
            vec2(1.0, 1.0), // right, top
            vec2(-1.0, 1.0) // left,  top
        );
	vTexCoord = uvs[gl_VertexID];
	gl_Position = vec4(vertices[gl_VertexID], 1.0);
})";
const char* kFragmentShaderSource = R"(#version 330 core

#define M_PI 3.1415926535897932384626433832795

uniform sampler2D cubeLeftImage;
uniform sampler2D cubeRightImage;
uniform sampler2D cubeBottomImage;
uniform sampler2D cubeTopImage;
uniform sampler2D cubeBackImage;
uniform sampler2D cubeFrontImage;

in vec2 vTexCoord;

out vec4 FragColor;

void main() {
	float theta = vTexCoord.x * M_PI;
	float phi = (vTexCoord.y * M_PI) / 2.0;

	float x = cos(phi) * sin(theta);
	float y = sin(phi);
	float z = cos(phi) * cos(theta);

	float scale;
	vec2 px;
	vec4 src;

	if (abs(x) >= abs(y) && abs(x) >= abs(z)) {
		if (x < 0.0) {
			scale = -1.0 / x;
			px.x = ( z*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			src = texture(cubeLeftImage, px);
		} else {
			scale = 1.0 / x;
			px.x = (-z*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			src = texture(cubeRightImage, px);
		}
	} else if (abs(y) >= abs(z)) {
		if (y < 0.0) {
			scale = -1.0 / y;
			px.x = ( x*scale + 1.0) / 2.0;
			px.y = ( z*scale + 1.0) / 2.0;
			src = texture(cubeTopImage, px);
		} else {
			scale = 1.0 / y;
			px.x = ( x*scale + 1.0) / 2.0;
			px.y = (-z*scale + 1.0) / 2.0;
			src = texture(cubeBottomImage, px);
		}
	} else {
		if (z < 0.0) {
			scale = -1.0 / z;
			px.x = (-x*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			src = texture(cubeBackImage, px);
		} else {
			scale = 1.0 / z;
			px.x = ( x*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			src = texture(cubeFrontImage, px);
		}
	}

	FragColor = src;
})";

static const char* GetShaderTypeName(uint32_t type)
{
    switch (type) {
    case GL_VERTEX_SHADER:
        return "VERTEX SHADER";
    case GL_FRAGMENT_SHADER:
        return "FRAGMENT SHADER";
    case GL_GEOMETRY_SHADER:
        return "GEOMETRY SHADER";
    default:
        return "UNKNOWN SHADER";
    }
}

static GLuint CompileShader(uint32_t type, const std::string& source)
{
    const uint32_t id = glCreateShader(type);
    const char* src = source.c_str();

    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    GLint compileStatus;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
        GLint maxLength;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);
        maxLength++;

        std::vector<char> errorLog(maxLength);
        glGetShaderInfoLog(id, maxLength, &maxLength, errorLog.data());

        LOONG_ERROR("Compile {} failed: {}", GetShaderTypeName(type), errorLog.data());

        glDeleteShader(id);
        return 0;
    }

    return id;
}

static GLuint CreateProgram(const std::vector<std::pair<uint32_t, const std::string&>>& shaders)
{
    const uint32_t program = glCreateProgram();
    if (program == 0) {
        LOONG_ERROR("Create shader program failed");
        return 0;
    }
    Foundation::LoongDefer deferDeleteProgram([program]() {
        glDeleteProgram(program);
    });

    std::vector<Foundation::LoongDefer> deferDeleteShaders;

    for (auto& [shaderType, shaderSource] : shaders) {
        const uint32_t shaderId = CompileShader(shaderType, shaderSource);
        if (shaderId == 0) {
            return 0;
        }
        deferDeleteShaders.emplace_back([shaderId]() {
            glDeleteShader(shaderId);
        });
        glAttachShader(program, shaderId);
    }
    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == GL_FALSE) {
        GLint maxLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        ++maxLength;

        std::vector<char> errorLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, errorLog.data());

        LOONG_ERROR("Link program failed: {}", errorLog.data());

        return 0;
    }

    glValidateProgram(program);

    deferDeleteProgram.Cancel();
    return program;
}

std::shared_ptr<Resource::LoongShader> CreateShader()
{
    std::vector<std::pair<uint32_t, const std::string&>> shaderSources;
    std::string vSource = kVertexShaderSource;
    std::string fSource = kFragmentShaderSource;
    // clang-format off
    shaderSources.emplace_back(GL_VERTEX_SHADER,   vSource);
    shaderSources.emplace_back(GL_FRAGMENT_SHADER, fSource);
    // clang-format on

    uint32_t program = CreateProgram(shaderSources);
    if (program == 0) {
        return nullptr;
    }
    auto* shaderProgram = new Resource::LoongShader(program, "");
    return std::shared_ptr<Resource::LoongShader>(shaderProgram);
}

int Convert()
{
    Loong::App::ScopedDriver appDriver;
    Loong::Resource::ScopedDriver resourceDriver;

    Loong::App::LoongWindow::WindowConfig cfg;
    cfg.visible = 0;
    Loong::App::LoongWindow app(cfg);

    auto& flags = Flags::Get();

    Resource::LoongVertexArray vao;

    {
        vao.Bind();

        vao.Unbind();
    }

    auto shaderProgram = CreateShader();
    if (shaderProgram == nullptr) {
        return -1;
    }
    std::shared_ptr<Resource::LoongTexture> textures[6];
    Resource::LoongFrameBuffer frameBuffer(flags.outWidth, flags.outHeight, 1);

    {
        std::string kUniformNames[6] = {
            "cubeLeftImage",
            "cubeRightImage",
            "cubeBottomImage",
            "cubeTopImage",
            "cubeBackImage",
            "cubeFrontImage",
        };
        std::string kImagePaths[6] = {
            flags.negativeXPath,
            flags.positiveXPath,
            flags.negativeYPath,
            flags.positiveYPath,
            flags.positiveZPath,
            flags.negativeZPath,
        };

        Asset::LoongImage images[6];
        for (int i = 0; i < 6; ++i) {
            images[i].LoadFromPhysicalPath(kImagePaths[i]);
            if (!images[i]) {
                LOONG_ERROR("Load image '{}' failed", kImagePaths[i]);
                return false;
            }
        }

        for (int i = 0; i < 6; ++i) {
            auto& img = images[i];
            textures[i] = Resource::LoongTextureLoader::CreateFromMemory(reinterpret_cast<uint8_t*>(img.GetData()), img.GetWidth(), img.GetHeight(), true, nullptr, 3);
        }

        shaderProgram->Bind();
        for (int i = 0; i < 6; ++i) {
            textures[i]->Bind(i);
            shaderProgram->SetUniformInt(kUniformNames[i], i);
        }
        shaderProgram->Unbind();
    }

    {
        frameBuffer.Bind();
        shaderProgram->Bind();
        vao.Bind();
        glClearColor(0.5, 0.6, 0.7, 1.0);
        glViewport(0, 0, flags.outWidth, flags.outHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        vao.Unbind();
        shaderProgram->Unbind();
        // Don't unbind framebuffer, or we can not read the our image
        // frameBuffer.Unbind();

        frameBuffer.GetColorAttachments()[0]->Bind(0);
    }

    int ret = 0;
    {
        char* pixels = new char[flags.outHeight * flags.outWidth * 3];
        glReadPixels(0, 0, flags.outWidth, flags.outHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        if (flags.outputFormat == "jpg" || flags.outputFormat == ".jpg") {
            if (0 == stbi_write_jpg(flags.outputPath.c_str(), flags.outWidth, flags.outHeight, 3, pixels, 10)) {
                ret = -2;
            }
        } else if (flags.outputFormat == "png" || flags.outputFormat == ".png") {
            if (0 == stbi_write_png(flags.outputPath.c_str(), flags.outWidth, flags.outHeight, 3, pixels, 0)) {
                ret = -2;
            }
        } else if (flags.outputFormat == "bmp" || flags.outputFormat == ".bmp") {
            if (0 == stbi_write_bmp(flags.outputPath.c_str(), flags.outWidth, flags.outHeight, 3, pixels)) {
                ret = -2;
            }
        } else if (flags.outputFormat == "tga" || flags.outputFormat == ".tga") {
            if (0 == stbi_write_tga(flags.outputPath.c_str(), flags.outWidth, flags.outHeight, 3, pixels)) {
                ret = -2;
            }
        } else {
            abort(); // This should not happen, since we have checked options
        }
    }
    if (ret == -2) {
        LOONG_ERROR("Write image '{}' failed", flags.outputPath);
    }
    return ret;
}

}

int main(int argc, char* argv[])
{
    using namespace Loong::Foundation;
    using namespace Loong::CubeToPanorama;

    auto listener = Logger::Get().SubscribeLog([](const LogItem& logItem) {
        std::cout << "[" << logItem.level << "][" << logItem.location << "]: " << logItem.message << std::endl;
    });

    if (!Flags::ParseCommandLine(argc, argv)) {
        return -1;
    }

    return Convert();
}