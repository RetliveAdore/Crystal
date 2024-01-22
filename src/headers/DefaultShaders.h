#ifndef _INCLUDE_CR_DEFAULTSHADERS_H_
#define _INCLUDE_CR_DEFAULTSHADERS_H_

const char* CrVertexShaderSource1 =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 uv_in;\n"
"uniform vec2 asp;\n"
"out vec2 uv_out;\n"
"void main() {\n"
"  gl_Position = vec4(aPos.x * asp.x, aPos.y * asp.y, aPos.z, 1.0);\n"
"  uv_out = vec2(uv_in.x, -uv_in.y);\n"
"}\0"
;

const char* CRFragmentShaderSource1 =
"#version 330 core\n"
"in vec2 uv_out;\n"
"uniform vec4 paintColor;\n"
"uniform sampler2D aTex0;"
"out vec4 FragColor;\n"
"void main() {\n"
"  FragColor = texture(aTex0, uv_out) * paintColor;\n"
"}\0"
;

#endif  //incldue