//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once
#include <glad/glad.h>

#include "LoongFoundation/LoongMath.h"
#include "LoongRenderer/LoongPipelineFixedState.h"
#include <cstdint>
#include <string>
#include <vector>

namespace Loong::Foundation {
class Frustum;
class Transform;
}
namespace Loong::Resource {
class LoongGpuModel;
class LoongGpuMesh;
}

namespace Loong::Renderer {

class LoongCamera;

class Renderer {
public:
    struct FrameInfo {
        uint64_t batchCount { 0 };
        uint64_t instanceCount { 0 };
        uint64_t polyCount { 0 };

        void Clear()
        {
            batchCount = 0;
            instanceCount = 0;
            polyCount = 0;
        }
    };

    // clang-format off
    enum class PolygonMode {
        kPoint = GL_POINT,
        kLine = GL_LINE,
        kFill = GL_FILL,
    };
    enum class Capability {
        kBlend                  = GL_BLEND,
        kCullFace               = GL_CULL_FACE,
        kDepthTest              = GL_DEPTH_TEST,
        kDither                 = GL_DITHER,
        kPolygonOffsetFill      = GL_POLYGON_OFFSET_FILL,
        kSampleAlphaToCoverage  = GL_SAMPLE_ALPHA_TO_COVERAGE,
        kSampleCoverage         = GL_SAMPLE_COVERAGE,
        kScissorTest            = GL_SCISSOR_TEST,
        kStencilTest            = GL_STENCIL_TEST,
        kMultisample            = GL_MULTISAMPLE,
    };
    enum class ComparisonAlgorithm {
        kNever                  = GL_NEVER,
        kLess                   = GL_LESS,
        kEqual                  = GL_EQUAL,
        kLessEqual              = GL_LEQUAL,
        kGreater                = GL_GREATER,
        kNotequal               = GL_NOTEQUAL,
        kGreaterEqual           = GL_GEQUAL,
        kAlways                 = GL_ALWAYS,
    };
    enum class Operation {
        kKeep                   = GL_KEEP,
        kZero                   = GL_ZERO,
        kReplace                = GL_REPLACE,
        kIncrement              = GL_INCR,
        kIncrementWrap          = GL_INCR_WRAP,
        kDecrement              = GL_DECR,
        kDecrementWrap          = GL_DECR_WRAP,
        kInvert                 = GL_INVERT,
    };
    enum class CullMode {
        kFront                  = GL_FRONT,
        kBack                   = GL_BACK,
        kFrontAndBack           = GL_FRONT_AND_BACK,
    };
    enum class PrimitiveMode {
        kPoints                 = GL_POINTS,
        kLines                  = GL_LINES,
        kLineLoop               = GL_LINE_LOOP,
        kLineStrip              = GL_LINE_STRIP,
        kTriangles              = GL_TRIANGLES,
        kTriangleStrip          = GL_TRIANGLE_STRIP,
        kTriangleFan            = GL_TRIANGLE_FAN,
        kLinesAdjacency         = GL_LINES_ADJACENCY,
        kLineStripAdjacency     = GL_LINE_STRIP_ADJACENCY,
        kTrianglesAdjacency     = GL_TRIANGLES_ADJACENCY,
        kTriangleStripAdjacency = GL_TRIANGLE_STRIP_ADJACENCY,
        kPatches                = GL_PATCHES,
    };
    enum class CullLevel {
        kNone = 0x0,
        kModel = 0x1,
        kMesh = 0x2,
    };
    // clang-format on

    Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    ~Renderer() = default;

    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    void SetClearColor(float r, float g, float b, float a = 1.0f);

    void Clear(bool colorBuffer = true, bool depthBuffer = true, bool stencilBuffer = true);

    // NOTE: This function will restore (only) the `clear color` after the `color buffer` is cleared
    void Clear(const LoongCamera& camera, bool colorBuffer = true, bool depthBuffer = true, bool stencilBuffer = true);

    void SetLineWidth(float width);

    void SetPolygonMode(PolygonMode mode);

    void SetCapability(Capability capability, bool value);

    bool IsCapabilityEnabled(Capability capability) const;

    void SetStencilAlgorithm(ComparisonAlgorithm algorithm, int32_t reference, uint32_t mask);

    void SetDepthAlgorithm(ComparisonAlgorithm algorithm);

    void SetStencilMask(uint32_t mask);

    void SetStencilOperations(Operation stencilFail = Operation::kKeep, Operation depthFail = Operation::kKeep, Operation bothPass = Operation::kKeep);

    void SetCullFace(CullMode cullMode);

    void SetDepthWriting(bool enable);

    void SetColorWriting(bool enableRed, bool enableGreen, bool enableBlue, bool enableAlpha);

    void SetColorWriting(bool enable);

    bool GetBool(GLenum parameter);

    bool GetBool(GLenum parameter, uint32_t index);

    GLint GetInt(GLenum parameter);

    GLint GetInt(GLenum parameter, uint32_t index);

    GLfloat GetFloat(GLenum parameter);

    GLfloat GetFloat(GLenum parameter, uint32_t index);

    GLdouble GetDouble(GLenum parameter);

    GLdouble GetDouble(GLenum parameter, uint32_t index);

    GLint64 GetInt64(GLenum parameter);

    GLint64 GetInt64(GLenum parameter, uint32_t index);

    std::string GetString(GLenum parameter);

    std::string GetString(GLenum parameter, uint32_t index);

    void ClearFrameInfo();

    void Draw(const Resource::LoongGpuMesh& mesh, PrimitiveMode primitiveMode = PrimitiveMode::kTriangles, uint32_t instances = 1);

    std::vector<Resource::LoongGpuMesh*> GetMeshesInFrustum(const Resource::LoongGpuModel& model, const Foundation::Transform& modelTransform, const Foundation::Frustum& frustum);

    std::vector<Resource::LoongGpuMesh*> GetMeshesInFrustum(const Resource::LoongGpuModel& model, const Math::Matrix4& modelTransform, const Foundation::Frustum& frustum);

    LoongPipelineFixedState FetchGLState();

    void ApplyStateMask(LoongPipelineFixedState mask);

    const FrameInfo& GetFrameInfo() const;

private:
    FrameInfo frameInfo_;
    LoongPipelineFixedState state_;
};

}
