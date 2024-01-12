#ifndef _INCLUDE_CR_DEFAULTSHADERS_H_
#define _INCLUDE_CR_DEFAULTSHADERS_H_

const char* CrVertexShaderSource1 =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform vec2 asp;\n"
"out vec4 vertexColor;\n"
"void main() {\n"
"  gl_Position = vec4(aPos.x * asp.x, aPos.y * asp.y, aPos.z, 1.0);\n"
"  vertexColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"}\0"
;

const char* CRFragmentShaderSource1 =
"#version 330 core\n"
"in vec4 vertexColor;\n"
"uniform vec4 paintColor;\n"
"out vec4 FragColor;\n"
"void main() {\n"
"  FragColor = vertexColor * paintColor;\n"
"}\0"
;

#endif  //incldue