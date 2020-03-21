static const float Far = 80000000.0;
static const float C = 1.0f;

float LogDepthBuffer(float w) {
    return log(C * w + 1) / log(C * Far + 1) * w;
}