#version 150
 
uniform mat4 viewMatrix, projMatrix;
 
in vec3 position;
//in vec2 uv;
 
//out vec2 uv_out;
 
void main()
{
    //uv_out = uv;
    gl_Position = projMatrix * viewMatrix * vec4(position,1) ;
}