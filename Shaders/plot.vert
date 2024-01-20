#version 120

uniform float u_depth;

attribute float a_xPosition;
attribute float a_yPosition;

varying vec4 v_colour;

void main() {
	v_colour = vec4(0.0, 0.0, 1.0, 1.0);
	
	gl_Position = vec4(a_xPosition, a_yPosition, u_depth, 1.0);
}
