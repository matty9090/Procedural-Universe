// Returns the near intersection point of a line and a sphere
float NearIntersection(float3 pos, float3 ray, float distance2, float radius2)
{
    float B = 2.0 * dot(pos, ray);
    float C = distance2 - radius2;
    float det = max(0.0, B * B - 4.0 * C);

    return 0.5 * (-B - sqrt(det));
}

// Returns the far intersection point of a line and a sphere
float FarIntersection(float3 pos, float3 ray, float distance2, float radius2)
{
    float B = 2.0 * dot(pos, ray);
    float C = distance2 - radius2;
    float det = max(0.0, B * B - 4.0 * C);

    return 0.5 * (-B + sqrt(det));
}

// Finds whether a ray hits a sphere, and the start and end hit distances
bool RaySphereIntersect(float3 s, float3 d, float r, out float ts, out float te)
{
    float r2 = r * r;
    float s2 = dot(s, s);

    if (s2 <= r2)
    {
        ts = 0.0;
        te = FarIntersection(s, d, s2, r2);
        return true;
    } 

    ts = NearIntersection(s, d, s2, r2);
    te = FarIntersection(s, d, s2, r2);

    return te > ts && ts > 0;
}