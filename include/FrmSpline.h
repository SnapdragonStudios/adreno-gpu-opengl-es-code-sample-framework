// Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _FRM_SPLINE_H_
#define _FRM_SPLINE_H_

#include <tinygltf/examples/common/glm/glm/glm.hpp>

namespace Adreno
{
	glm::vec3 SplineEvaluate(const float t, const glm::vec3* const splinePoints, const size_t splinePointsNum);
	float SplineTIncrement(const float t, const float v, const float splinePointsNum);
}

#endif // _FRM_SPLINE_H_
