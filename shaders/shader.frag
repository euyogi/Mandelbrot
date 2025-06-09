#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inCoord;

layout(push_constant) uniform PushConstants {
    vec2 offset;
    float zoom;
    float aspectRatio;
} pc;

const int MAX_IT = 5000;

bool dentroCardioide(float x, float y) {
    float q = (x - 0.25f) * (x - 0.25f) + y*y;
    return q * (q + (x - 0.25f)) < 0.25f * y*y;
}

// ponto:           c = (cx, cy)
// número complexo: c = (x + yi)
int iterarMandelbrot(float cx, float cy) {
    if (dentroCardioide(cx, cy)) return 0;
    float x = 0.0f, y = 0.0f;
    float xx = 0.0f, yy = 0.0f;
    for (int i = 1; i <= MAX_IT; ++i) {
        y = 2.0f * x*y + cy;
        x = xx - yy + cx;
        xx = x*x;
        yy = y*y;
        if (xx + yy > 4.0f) return i;
    }
    return 0;
}

// (x + yi)^3 = x^3 + 3x^2yi + 3x(yi)^2 + y^3i^3
//            = (x^3 - 3xy^2) + (3x^2y - y^3)i
int iterarCubo(float cx, float cy) {
    float x = 0.0f, y = 0.0f;
    float xx = 0.0f, yy = 0.0f;
    for (int i = 1; i <= MAX_IT; ++i) {
        y = 3.0f * y * (xx - yy) + cy;
        x = x * (xx - 3.0f * yy) + cx;
        xx = x*x;
        yy = y*y;
        if (xx + yy > 4.0f) return i;
    }
    return 0;
}

// (x + yi)^4 = x^4 + 4x^3yi + 6x^2(yi)^2 + 4x(yi)^3 + (yi)^4
//            = (x^4 - 6x^2y^2 + y^4) (4x^3y - 4xy^3)i
int iterarQuarta(float cx, float cy) {
    float x = 0.0f, y = 0.0f;
    float xx = 0.0f, yy = 0.0f, xy = 0.0f;
    for (int i = 1; i <= MAX_IT; ++i) {
        y = 4.0f * x*y * (xx - yy) + cy;
        x = xx*xx - 6.0f * yy * (xx + yy) + cx;
        xx = x*x;
        yy = y*y;
        if (xx + yy > 4.0f) return i;
    }
    return 0;
}

int iterar(float cx, float cy) {
    return iterarMandelbrot(cx, cy);
    // return iterarCubo(cx, cy);
    // return iterarQuarta(cx, cy);
}

void main() {
    float x = (inCoord.x * pc.aspectRatio) / pc.zoom + pc.offset.x * pc.aspectRatio;
    float y = inCoord.y / pc.zoom + pc.offset.y;

    int its = iterar(x, y);
    its *= its;
    outColor = vec4(0.00001f * its, 0.000001f * its, 0.000001f * its, 1.0f);
}