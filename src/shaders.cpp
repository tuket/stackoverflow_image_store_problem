#include "shaders.hpp"

namespace shader_srcs
{

ConstStr header =
R"GLSL(
#version 430
#define PI 3.1415926535897932
)GLSL";

// ----------------------------------------------------------------------------------------------------------------------------------
ConstStr computeSrc =
R"GLSL(
#define KS 16 // kernel size
layout (local_size_x = KS, local_size_y = KS) in;

layout(location = 0) uniform sampler2D u_inputTex;
layout(location = 1) uniform writeonly uimage2D u_outImg;

void main()
{
	const ivec2 gid = ivec2(gl_WorkGroupID.xy);
	const ivec2 tid = ivec2(gl_LocalInvocationID.xy);
	const ivec2 pixelPos = ivec2(KS) * gid + tid;

	imageStore(u_outImg, pixelPos,
		uvec4(255.0 * texelFetch(u_inputTex, pixelPos, 0).rgb, 255u));
}
)GLSL";

}