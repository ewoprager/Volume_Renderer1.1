#version 120

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec3 a_colour;

uniform mat4 u_M;
uniform mat3 u_M_normal;

varying vec3 v_normal;
varying vec3 v_colour;

void main() {
	v_normal = normalize(u_M_normal * a_normal);
	v_colour = a_colour;
	
	gl_Position = u_M * vec4(a_position, 1.0);
}
