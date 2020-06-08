//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include "LoongRenderer/LoongRenderer.h"
#include "LoongFoundation/LoongFrustum.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongFoundation/LoongTransform.h"
#include "LoongRenderer/LoongCamera.h"
#include "LoongResource/LoongGpuMesh.h"
#include "LoongResource/LoongGpuModel.h"

namespace Loong::Renderer {

void Renderer::SetClearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
}

void Renderer::Clear(bool colorBuffer, bool depthBuffer, bool stencilBuffer)
{
    glClear(
        (colorBuffer ? GL_COLOR_BUFFER_BIT : 0) | //
        (depthBuffer ? GL_DEPTH_BUFFER_BIT : 0) | //
        (stencilBuffer ? GL_STENCIL_BUFFER_BIT : 0) //
    );
}

void Renderer::Clear(const LoongCamera& camera, bool colorBuffer, bool depthBuffer, bool stencilBuffer)
{
    GLfloat previousClearColor[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, previousClearColor);

    const auto& cameraClearColor = camera.GetClearColor();
    SetClearColor(cameraClearColor.x, cameraClearColor.y, cameraClearColor.z, 1.0f);
    Clear(colorBuffer, depthBuffer, stencilBuffer);

    SetClearColor(previousClearColor[0], previousClearColor[1], previousClearColor[2], previousClearColor[3]);
}

void Renderer::SetLineWidth(float width)
{
    glLineWidth(width);
}

void Renderer::SetPolygonMode(Renderer::PolygonMode mode)
{
    glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(mode));
}

void Renderer::SetCapability(Renderer::Capability capability, bool value)
{
    (value ? glEnable : glDisable)(static_cast<GLenum>(capability));
}

bool Renderer::IsCapabilityEnabled(Renderer::Capability capability) const
{
    return glIsEnabled(static_cast<GLenum>(capability));
}

void Renderer::SetStencilAlgorithm(Renderer::ComparisonAlgorithm algorithm, int32_t reference, uint32_t mask)
{
    glStencilFunc(static_cast<GLenum>(algorithm), reference, mask);
}

void Renderer::SetDepthAlgorithm(Renderer::ComparisonAlgorithm algorithm)
{
    glDepthFunc(static_cast<GLenum>(algorithm));
}

void Renderer::SetStencilMask(uint32_t mask)
{
    glStencilMask(mask);
}

void Renderer::SetStencilOperations(Renderer::Operation stencilFail, Renderer::Operation depthFail, Renderer::Operation bothPass)
{
    glStencilOp(static_cast<GLenum>(stencilFail), static_cast<GLenum>(depthFail), static_cast<GLenum>(bothPass));
}

void Renderer::SetCullFace(Renderer::CullMode cullMode)
{
    glCullFace(static_cast<GLenum>(cullMode));
}

void Renderer::SetDepthWriting(bool enable)
{
    glDepthMask(enable);
}

void Renderer::SetColorWriting(bool enableRed, bool enableGreen, bool enableBlue, bool enableAlpha)
{
    glColorMask(enableRed, enableGreen, enableBlue, enableAlpha);
}

void Renderer::SetColorWriting(bool enable)
{
    SetColorWriting(enable, enable, enable, enable);
}

bool Renderer::GetBool(GLenum parameter)
{
    GLboolean result;
    glGetBooleanv(parameter, &result);
    return static_cast<bool>(result);
}

bool Renderer::GetBool(GLenum parameter, uint32_t index)
{
    GLboolean result;
    glGetBooleani_v(parameter, index, &result);
    return static_cast<bool>(result);
}

GLint Renderer::GetInt(GLenum parameter)
{
    GLint result;
    glGetIntegerv(parameter, &result);
    return result;
}

int Renderer::GetInt(GLenum parameter, uint32_t index)
{
    GLint result;
    glGetIntegeri_v(parameter, index, &result);
    return result;
}

GLfloat Renderer::GetFloat(GLenum parameter)
{
    GLfloat result;
    glGetFloatv(parameter, &result);
    return result;
}

GLfloat Renderer::GetFloat(GLenum parameter, uint32_t index)
{
    GLfloat result;
    glGetFloati_v(parameter, index, &result);
    return result;
}

GLdouble Renderer::GetDouble(GLenum parameter)
{
    GLdouble result;
    glGetDoublev(parameter, &result);
    return result;
}

GLdouble Renderer::GetDouble(GLenum parameter, uint32_t index)
{
    GLdouble result;
    glGetDoublei_v(parameter, index, &result);
    return result;
}

GLint64 Renderer::GetInt64(GLenum parameter)
{
    GLint64 result;
    glGetInteger64v(parameter, &result);
    return result;
}

GLint64 Renderer::GetInt64(GLenum parameter, uint32_t index)
{
    GLint64 result;
    glGetInteger64i_v(parameter, index, &result);
    return result;
}

std::string Renderer::GetString(GLenum parameter)
{
    const GLubyte* result = glGetString(parameter);
    return result ? reinterpret_cast<const char*>(result) : "";
}

std::string Renderer::GetString(GLenum parameter, uint32_t index)
{
    const GLubyte* result = glGetStringi(parameter, index);
    return result ? reinterpret_cast<const char*>(result) : "";
}

void Renderer::ClearFrameInfo()
{
    frameInfo_.Clear();
}

void Renderer::Draw(const Resource::LoongGpuMesh& mesh, Renderer::PrimitiveMode primitiveMode, uint32_t instances)
{
    if (instances <= 0) {
        return;
    }

    ++frameInfo_.batchCount;
    frameInfo_.instanceCount += instances;
    frameInfo_.polyCount += (mesh.GetIndexCount() / 3) * instances;

    mesh.Bind();

    if (mesh.GetIndexCount() > 0) {
        /* With EBO */
        if (instances == 1) {
            glDrawElements(static_cast<GLenum>(primitiveMode), mesh.GetIndexCount(), GL_UNSIGNED_INT, nullptr);
        } else {
            glDrawElementsInstanced(static_cast<GLenum>(primitiveMode), mesh.GetIndexCount(), GL_UNSIGNED_INT, nullptr, instances);
        }
    } else {
        /* Without EBO */
        assert(false); // TODO
        if (instances == 1) {
            glDrawArrays(static_cast<GLenum>(primitiveMode), 0, mesh.GetVertexCount());
        } else {
            glDrawArraysInstanced(static_cast<GLenum>(primitiveMode), 0, mesh.GetVertexCount(), instances);
        }
    }

    mesh.Unbind();
}

std::vector<Resource::LoongGpuMesh*> Renderer::GetMeshesInFrustum(const Resource::LoongGpuModel& model, const Foundation::Transform& modelTransform, const Foundation::Frustum& frustum)
{
    return GetMeshesInFrustum(model, modelTransform.GetWorldTransformMatrix(), frustum);
}

std::vector<Resource::LoongGpuMesh*> Renderer::GetMeshesInFrustum(const Resource::LoongGpuModel& model, const Math::Matrix4& modelTransform, const Foundation::Frustum& frustum)
{
    auto transformMatrix = modelTransform;
    auto actualAabb = model.GetAABB().Transformed(transformMatrix);

    if (!frustum.IsBoxVisible(actualAabb)) {
        return {};
    }

    std::vector<Resource::LoongGpuMesh*> result;

    const auto& meshes = model.GetMeshes();

    for (auto mesh : meshes) {
        // Do not check if the mesh is in frustum if the model has only one mesh, because model and mesh bounding sphere are equals
        auto meshAabb = mesh->GetAABB().Transformed(transformMatrix);

        if (meshes.size() == 1 || frustum.IsBoxVisible(meshAabb)) {
            result.push_back(mesh);
        }
    }

    return result;
}

LoongPipelineFixedState Renderer::FetchGLState()
{
    LoongPipelineFixedState result;

    // clang-format off
    GLboolean cMask[4];
    glGetBooleanv(GL_COLOR_WRITEMASK, cMask);
    if (GetBool(GL_DEPTH_WRITEMASK))                           result.SetDepthWriteEnabled(true);
    if (cMask[0])                                              result.SetColorWriteEnabled(true);
    if (IsCapabilityEnabled(Capability::kBlend))      result.SetBlendEnabled(true);
    if (IsCapabilityEnabled(Capability::kCullFace))   result.SetFaceCullEnabled(true);
    if (IsCapabilityEnabled(Capability::kDepthTest))  result.SetDepthTestEnabled(true);

    switch (static_cast<CullMode>(GetInt(GL_CULL_FACE)))
    {
    case CullMode::kBack:           result.SetBackCullEnabled(true); break;
    case CullMode::kFront:          result.SetFrontCullEnabled(true); break;
    case CullMode::kFrontAndBack:   result.SetFrontAndBackCullEnabled(true); break;
    }
    // clang-format on

    return result;
}

void Renderer::ApplyStateMask(LoongPipelineFixedState mask)
{
    auto diffrence = mask ^ state_;

    // clang-format off
    if (diffrence.IsDepthWriteEnabled())     SetDepthWriting(mask.IsDepthWriteEnabled());
    if (diffrence.IsColorWriteEnabled())     SetColorWriting(mask.IsColorWriteEnabled());
    if (diffrence.IsBlendEnabled())          SetCapability(Capability::kBlend, mask.IsBlendEnabled());
    if (diffrence.IsFaceCullEnabled())       SetCapability(Capability::kCullFace, mask.IsFaceCullEnabled());
    if (diffrence.IsDepthTestEnabled())      SetCapability(Capability::kDepthTest, mask.IsDepthTestEnabled());

    if (mask.IsFaceCullEnabled() && (diffrence.IsBackCullEnabled() || diffrence.IsFrontCullEnabled() || diffrence.IsFrontAndBackCullEnabled())) {
        if (mask.IsBackCullEnabled())        SetCullFace(CullMode::kBack);
        else if (mask.IsFrontCullEnabled())  SetCullFace(CullMode::kFront);
        else                                 SetCullFace(CullMode::kFrontAndBack);
    }
    // clang-format on

    state_ = mask;
}

const Renderer::FrameInfo& Renderer::GetFrameInfo() const
{
    return frameInfo_;
}

}