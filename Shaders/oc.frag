#version 120

varying vec3 v_normal;
varying vec3 v_colour;

void main() {
	gl_FragColor = vec4(v_colour * dot(v_normal, vec3(0.0, 0.0, 1.0)), 1.0);
}
