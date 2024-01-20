#version 120

uniform float u_depth;

attribute vec3 a_position;
attribute vec4 a_colour;

varying vec4 v_colour;

void main() {
	v_colour = a_colour;
	
	gl_Position = vec4(a_position.xy, u_depth, 1.0);
}
