// Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmSpline.h"
#include "FrmUtils.h"

static float GetT(float t, const glm::vec3& p0, const glm::vec3& p1)
{
    const glm::vec3 d = p1 - p0;
    const float a = glm::dot(d, d); // Dot product
    const float b = glm::pow(a, .25f/*centripetal CatmullRom for no cusps or loops -- https://en.wikipedia.org/wiki/Centripetal_Catmull%E2%80%93Rom_spline*/);
    return (b + t);
}

static glm::vec3 CatmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t)
{
    ADRENO_ASSERT(t >= 0.f, __FILE__, __LINE__);
    ADRENO_ASSERT(t <= 1.f, __FILE__, __LINE__);

    float t0 = 0.0f;
    float t1 = GetT(t0, p0, p1);
    float t2 = GetT(t1, p1, p2);
    float t3 = GetT(t2, p2, p3);
    t = (1.f - t) * t1 + t * t2;
    const glm::vec3 A1 = (t1 - t) / (t1 - t0) * p0 + (t - t0) / (t1 - t0) * p1;
    const glm::vec3 A2 = (t2 - t) / (t2 - t1) * p1 + (t - t1) / (t2 - t1) * p2;
    const glm::vec3 A3 = (t3 - t) / (t3 - t2) * p2 + (t - t2) / (t3 - t2) * p3;
    const glm::vec3 B1 = (t2 - t) / (t2 - t0) * A1 + (t - t0) / (t2 - t0) * A2;
    const glm::vec3 B2 = (t3 - t) / (t3 - t1) * A2 + (t - t1) / (t3 - t1) * A3;
    const glm::vec3 C = (t2 - t) / (t2 - t1) * B1 + (t - t1) / (t2 - t1) * B2;
    return C;
}

namespace Adreno
{
    glm::vec3 SplineEvaluate(const float t, const glm::vec3* const splinePoints, const size_t splinePointsNum)
    {
        ADRENO_ASSERT(t >= 0.f, __FILE__, __LINE__);
        ADRENO_ASSERT(t <= static_cast<float>(splinePointsNum), __FILE__, __LINE__);

        ADRENO_ASSERT(splinePoints, __FILE__, __LINE__);
        ADRENO_ASSERT(splinePointsNum >= 4, __FILE__, __LINE__);

        const size_t splinePointIndex = floor(t);
        return CatmullRom(
            splinePoints[(splinePointIndex + 0) % splinePointsNum],
            splinePoints[(splinePointIndex + 1) % splinePointsNum],
            splinePoints[(splinePointIndex + 2) % splinePointsNum],
            splinePoints[(splinePointIndex + 3) % splinePointsNum],
            fmod(t, 1.f));
    }
    float SplineTIncrement(const float t, const float v, const float splinePointsNum)
    {
        ADRENO_ASSERT(t >= 0.f, __FILE__, __LINE__);
        ADRENO_ASSERT(t <= splinePointsNum, __FILE__, __LINE__);

        ADRENO_ASSERT(v >= 0.f, __FILE__, __LINE__);
        ADRENO_ASSERT(splinePointsNum >= 4.f, __FILE__, __LINE__);

        return fmod(t + v, splinePointsNum);
    }
}