#version 120

uniform mat4 u_B1;
uniform mat4 u_B2;

attribute vec3 a_position;
attribute vec2 a_texCoord;

varying vec2 v_texCoord;
varying vec3 v_start_ctTextureSpace;
varying vec3 v_start_SourceSpace;

void main() {
	v_start_ctTextureSpace = (u_B1 * u_B2 * vec4(a_texCoord, 0.0, 1.0)).xyz;
	v_start_SourceSpace = (u_B2 * vec4(a_texCoord, 0.0, 1.0)).xyz;
	
	vec2 flippedTexCoord = a_texCoord;
	flippedTexCoord.y = 1.0 - flippedTexCoord.y;
	v_texCoord = flippedTexCoord;
	
	gl_Position = vec4(a_position.xy, 0.9, 1.0);
}
