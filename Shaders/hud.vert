#version 120

uniform float u_depth;

attribute vec3 a_position;

void main() {
	gl_Position = vec4(a_position.xy, u_depth, 1.0);
}
