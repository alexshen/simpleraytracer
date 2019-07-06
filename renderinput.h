#ifndef RENDER_INPUT_H
#define RENDER_INPUT_H

#pragma once

class GLSLProgram;

class RenderInput
{
public:
    virtual ~RenderInput() = default;
    virtual void setInput(GLSLProgram& prog) const = 0;
};

#endif // RENDER_INPUT_H