#pragma once

#include <vector>
#include <SimpleMath.h>

using namespace DirectX;

namespace Shapes
{
	template <class Vertex, class Index = uint32_t>
	void ComputeTorus(std::vector<Vertex>& vertices, std::vector<Index>& indices, float diameter, float thickness, size_t tessellation)
	{
		vertices.clear();
		indices.clear();

		if (tessellation < 3)
			throw std::out_of_range("tesselation parameter out of range");

		size_t stride = tessellation + 1;

		// First we loop around the main ring of the torus.
		for (size_t i = 0; i <= tessellation; i++)
		{
			float u = float(i) / tessellation;

			float outerAngle = i * XM_2PI / tessellation - XM_PIDIV2;

			// Create a transform matrix that will align geometry to
			// slice perpendicularly though the current ring position.
			XMMATRIX transform = XMMatrixTranslation(diameter / 2, 0, 0) * XMMatrixRotationY(outerAngle);

			// Now we loop along the other axis, around the side of the tube.
			for (size_t j = 0; j <= tessellation; j++)
			{
				float v = 1 - float(j) / tessellation;

				float innerAngle = j * XM_2PI / tessellation + XM_PI;
				float dx, dy;

				XMScalarSinCos(&dy, &dx, innerAngle);

				// Create a vertex.
				XMVECTOR normal = XMVectorSet(dx, dy, 0, 0);
				XMVECTOR position = XMVectorScale(normal, thickness / 2);
				XMVECTOR textureCoordinate = XMVectorSet(u, v, 0, 0);

				position = XMVector3Transform(position, transform);
				normal = XMVector3TransformNormal(normal, transform);

				vertices.push_back({ position, normal, textureCoordinate });

				// And create indices for two triangles.
				size_t nextI = (i + 1) % stride;
				size_t nextJ = (j + 1) % stride;

				indices.push_back(static_cast<Index>(i * stride + j));
				indices.push_back(static_cast<Index>(i * stride + nextJ));
				indices.push_back(static_cast<Index>(nextI * stride + j));

				indices.push_back(static_cast<Index>(i * stride + nextJ));
				indices.push_back(static_cast<Index>(nextI * stride + nextJ));
				indices.push_back(static_cast<Index>(nextI * stride + j));
			}
		}
	}
}