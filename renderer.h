#ifndef RENDERER_H
#define RENDERER_H

#pragma once

class Renderer
{
public:
    Renderer() = default;
    Renderer(const Renderer&) = delete;
    virtual ~Renderer() {}

    void operator =(Renderer&) = delete;

    virtual void init(int width, int height) = 0;
    virtual void render() = 0;
};

#endif // RENDERER_H