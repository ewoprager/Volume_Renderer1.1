#version 120

uniform mat4 u_B1;

uniform float u_jumpSize;
uniform mat3 u_B1_noTranslation;

uniform sampler3D u_data;
uniform int u_samplesN;
uniform float u_nInv;
uniform float u_range;
uniform float u_centre;

uniform int u_drawMode; // {xray, none, drr}
uniform sampler2D u_xRay;

varying vec2 v_texCoord;
varying vec3 v_start_ctTextureSpace;
varying vec3 v_start_SourceSpace;

void main(){
	
	vec3 rayJump = u_B1_noTranslation * u_jumpSize * normalize(v_start_SourceSpace);
	
	vec4 sum = vec4(0.0);
	//for(int i=-u_samplesN/2; i<u_samplesN/2; i++) sum += vec<4>(texture3D(u_data, v_start_ctTextureSpace + float(i)*rayJump).r);
	for(int i=-u_samplesN/2; i<u_samplesN/2; i++){
		vec3 vec = v_start_ctTextureSpace + float(i)*rayJump;
		sum += vec4(texture3D(u_data, vec3(vec.y, vec.x, vec.z)).r);
	}
	
	// 1.0 - (exp(-(sum.r - 0.296997)*u_nInv/8.09974) - u_centre + u_range*0.5)/u_range;
	// ((sum.r - 0.296997)*u_nInv/8.09974 - u_centre + u_range*0.5)/u_range;
	
	float v = 1.0 - (exp(-sum.r*u_nInv) - u_centre + u_range*0.5)/u_range;
	
	if(u_drawMode == 0 || u_drawMode == 2){ // x-ray comparison or static drr comparison preview
		gl_FragColor.b = v;
		gl_FragColor.r = gl_FragColor.g = texture2D(u_xRay, v_texCoord).r;
		gl_FragColor.a = 1.0;
	} else if(u_drawMode == 1){ // deep drr only (for comparison)
		v *= 65536.0;
		gl_FragColor = vec4(mod(v, 256.0)/256.0, floor(v / 256.0) / 256.0, 0.0, 1.0);
	} else { // shallow drr only (for preview)
		gl_FragColor = vec4(v, v, v, 1.0);
	}
	 
	
	//gl_FragColor = vec<4>(texture3D(u_data, vec<3>(v_texCoord, 0.0)).r);
}

