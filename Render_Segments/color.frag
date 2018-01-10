#version 150

out vec4 outputF ;
 
void main()
{
	
	outputF.r = (gl_FragCoord.z/ gl_FragCoord.w/1000.0);
	outputF.g = (gl_FragCoord.z/ gl_FragCoord.w/1000.0);
	outputF.b = (gl_FragCoord.z/ gl_FragCoord.w/1000.0);
	outputF.a = 1;
	
}