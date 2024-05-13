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

// globals
ivec3 image_size = ivec3(512, 512, 279); // ToDo: derive from uniform

const mat4 M3_topleft = mat4(-1.0, 1.0, 1.0, -1.0,
							 1.0, -1.0, -1.0, 1.0,
							 1.0, 0.0, -1.0, 0.0,
							 -1.0, 0.0, 1.0, 0.0);

const mat4 M3_topright = mat4(1.0, -1.0, 0.0, 0.0,
							  -1.0, 1.0, 0.0, 0.0,
							  -1.0, 0.0, 0.0, 0.0,
							  1.0, 0.0, 0.0, 0.0);

const mat4 M3_botleft = mat4(1.0, -1.0, -1.0, 1.0,
							 0.0, 0.0, 0.0, 0.0,
							 -1.0, 0.0, 1.0, 0.0,
							 0.0, 0.0, 0.0, 0.0);

const mat4 M3_botright = mat4(-1.0, 1.0, 0.0, 0.0,
							  0.0, 0.0, 0.0, 0.0,
							  1.0, 0.0, 0.0, 0.0,
							  0.0, 0.0, 0.0, 0.0);

// functions
vec4 CalcXVector(float x0, float x1){
	float x02 = x0 * x0;
	float x12 = x1 * x1;
	return vec4(0.25 * (x12 * x12 - x02 * x02), (x12 * x1 - x02 * x0) / 3.0, 0.5 * (x12 - x02), x1 - x0);
}

mat3 CalcW3a(float my, float mz, float ky, float kz){
	return mat3(mz * my, mz * ky + kz * my, kz * ky, 0.0, mz, kz, 0.0, my, ky);
}

mat4 CalcW3Left(mat3 W3a){
	return mat4(
		vec4(W3a[0], 0.0),
		vec4(W3a[1], 0.0),
		vec4(W3a[2], 0.0),
		vec4(0.0, 0.0, 1.0, 0.0)
	);
}

mat4 CalcW3Right(mat3 W3a){
	return mat4(
		vec4(0.0, W3a[0]),
		vec4(0.0, W3a[1]),
		vec4(0.0, W3a[2]),
		vec4(0.0, 0.0, 0.0, 1.0)
	);
}

struct imat3 {
	ivec3 c0;
	ivec3 c1;
	ivec3 c2;
};

int idot(ivec3 lhs, ivec3 rhs){
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

ivec3 idot(ivec3 vector, imat3 matrix){
	return ivec3(idot(vector, matrix.c0), idot(vector, matrix.c1), idot(vector, matrix.c2));
}

float GetImgVal(ivec3 pos_transformed, imat3 transform_inverse, ivec3 image_size_minus_1, ivec3 image_size_transformed_minus_1){
	ivec3 p = (idot(ivec3(2) * pos_transformed - image_size_transformed_minus_1, transform_inverse) + image_size_minus_1) / ivec3(2);
	return texture3D(u_data, vec3(p) / vec3(image_size)).r;
}

void main(){
	
	vec3 v_ray_direction = u_B1_noTranslation * normalize(v_start_SourceSpace);
	 
	// basis vectors of the new coordinate system
	ivec3 ei, ej, ek;
	vec3 sign_direction = sign(v_ray_direction);
	ivec3 isign_direction = ivec3(sign_direction);
	vec3 mod_direction = sign_direction * v_ray_direction;
	if(mod_direction.x > mod_direction.y){
		if(mod_direction.x > mod_direction.z){
			// x is biggest
			ei = ivec3(isign_direction.x, 0, 0);
			ej = ivec3(0, isign_direction.y, 0);
			ek = ivec3(0, 0, isign_direction.z);
		} else {
			// z is biggest
			ei = ivec3(0, 0, isign_direction.z);
			ej = ivec3(isign_direction.x, 0, 0);
			ek = ivec3(0, isign_direction.y, 0);
		}
	} else if(mod_direction.y > mod_direction.z){
		// y is biggest
		ei = ivec3(0, isign_direction.y, 0);
		ej = ivec3(0, 0, isign_direction.z);
		ek = ivec3(isign_direction.x, 0, 0);
	} else {
		// z is biggest
		ei = ivec3(0, 0, isign_direction.z);
		ej = ivec3(isign_direction.x, 0, 0);
		ek = ivec3(0, isign_direction.y, 0);
	}
	
	// transformations between the coordinate systems
	mat3 matrix_reorient = mat3(vec3(ei), vec3(ej), vec3(ek));
	imat3 matrixi_reorient = imat3(ei, ej, ek);
	imat3 matrixi_reorient_inverse = imat3(ivec3(ei.x, ej.x, ek.x),
												 ivec3(ei.y, ej.y, ek.y),
												 ivec3(ei.z, ej.z, ek.z));
	
	// image size variables
	ivec3 image_size_minus_1 = image_size - ivec3(1);
	ivec3 _bla_ = idot(image_size_minus_1, matrixi_reorient);
	ivec3 image_size_transformed_minus_1 = ivec3(abs(_bla_.x), abs(_bla_.y), abs(_bla_.z));
	int size_largest_minus_1 = image_size_minus_1.x > image_size_minus_1.y ? image_size_minus_1.x : image_size_minus_1.y;
	size_largest_minus_1 = size_largest_minus_1 > image_size_minus_1.z ? size_largest_minus_1 : image_size_minus_1.z;
	
	// ray scaled according to the image size
	vec3 rescaling = vec3(float(size_largest_minus_1)) / vec3(image_size_minus_1);//pcs / pcs.spacing;
	vec3 origin_scaled = rescaling * v_start_SourceSpace;
	vec3 direction_scaled = rescaling * v_ray_direction;
	
	// ray transformed to the space of a single voxel
	vec3 beta_hat = direction_scaled * matrix_reorient;
	vec3 alpha = vec3(0.5) + origin_scaled * matrix_reorient;
	
	// first voxel the ray passes through
	ivec3 voxel;
	vec3 yzPlane_intersect = alpha - beta_hat * alpha.x / beta_hat.x;
	if(yzPlane_intersect.y >= 0.0 &&
	   yzPlane_intersect.y < 1.0 &&
	   yzPlane_intersect.z >= 0.0 &&
	   yzPlane_intersect.z < 1.0){
		voxel = ivec3(vec3(image_size_transformed_minus_1) * yzPlane_intersect);
	} else {
		vec3 xzPlane_intersect = alpha - beta_hat * alpha.y / beta_hat.y;
		if(xzPlane_intersect.x >= 0.0 &&
		   xzPlane_intersect.x < 1.0 &&
		   xzPlane_intersect.z >= 0.0 &&
		   xzPlane_intersect.z < 1.0){
			voxel = ivec3(vec3(image_size_transformed_minus_1) * xzPlane_intersect);
		} else {
			vec3 xyPlane_intersect = alpha - beta_hat * alpha.z / beta_hat.z;
			if(xyPlane_intersect.x >= 0.0 &&
			   xyPlane_intersect.x < 1.0 &&
			   xyPlane_intersect.y >= 0.0 &&
			   xyPlane_intersect.y < 1.0){
				voxel = ivec3(vec3(image_size_transformed_minus_1) * xyPlane_intersect);
			} else {
				gl_FragColor = vec4(vec3(0.0), 1.0);
				return;
			}
		}
	}
	
	// ray gradients and intercepts
	vec2 m_yz = beta_hat.yz / beta_hat.xx * vec2(image_size_transformed_minus_1.yz) / vec2(image_size_transformed_minus_1.xx);
	vec2 k_yz = yzPlane_intersect.yz * vec2(image_size_transformed_minus_1.yz) + m_yz * vec2(voxel.xx) - vec2(voxel.yz);
	
	float sum_I = 0.0;
	
	vec2 x_at_yz0, x_at_yz1;
	float x0, x1;
	float none = -1.0;
	vec4 abcd = vec4(none);
	vec4 efgh = vec4(none);
	vec4 M3_imgvals_top, M3_imgvals_bot;
	mat3 W3a;
	mat4 W3_left, W3_right;
	vec4 W3_M3_imgvals;
	while(all(lessThan(voxel, image_size_transformed_minus_1))){
		
		x_at_yz0 = - k_yz / m_yz;
		x_at_yz1 = (vec2(1.0) - k_yz) / m_yz;
		
		x0 = max(max(x_at_yz0.r, x_at_yz0.g), 0.0);
		x1 = min(min(x_at_yz1.r, x_at_yz1.g), 1.0);
		
		if(abcd.r < 0.0) abcd.r = GetImgVal(voxel + ivec3(0, 0, 0), matrixi_reorient_inverse, image_size_minus_1, image_size_transformed_minus_1);
		if(abcd.g < 0.0) abcd.g = GetImgVal(voxel + ivec3(1, 0, 0), matrixi_reorient_inverse, image_size_minus_1, image_size_transformed_minus_1);
		if(abcd.b < 0.0) abcd.b = GetImgVal(voxel + ivec3(0, 1, 0), matrixi_reorient_inverse, image_size_minus_1, image_size_transformed_minus_1);
		if(abcd.a < 0.0) abcd.a = GetImgVal(voxel + ivec3(1, 1, 0), matrixi_reorient_inverse, image_size_minus_1, image_size_transformed_minus_1);
		if(efgh.r < 0.0) efgh.r = GetImgVal(voxel + ivec3(0, 0, 1), matrixi_reorient_inverse, image_size_minus_1, image_size_transformed_minus_1);
		if(efgh.g < 0.0) efgh.g = GetImgVal(voxel + ivec3(1, 0, 1), matrixi_reorient_inverse, image_size_minus_1, image_size_transformed_minus_1);
		if(efgh.b < 0.0) efgh.b = GetImgVal(voxel + ivec3(0, 1, 1), matrixi_reorient_inverse, image_size_minus_1, image_size_transformed_minus_1);
		if(efgh.a < 0.0) efgh.a = GetImgVal(voxel + ivec3(1, 1, 1), matrixi_reorient_inverse, image_size_minus_1, image_size_transformed_minus_1);
		
		M3_imgvals_top = M3_topleft * abcd + M3_topright * efgh;
		M3_imgvals_bot = M3_botleft * abcd + M3_botright * efgh;
		
		W3a = CalcW3a(m_yz.r, m_yz.g, k_yz.r, k_yz.g);
		W3_left = CalcW3Left(W3a);
		W3_right = CalcW3Right(W3a);
		
		W3_M3_imgvals = W3_left * M3_imgvals_top + W3_right * M3_imgvals_bot;
		
		sum_I += dot(CalcXVector(x0, x1), W3_M3_imgvals);
		
		if(x_at_yz1.r < 1.0){
			if(x_at_yz1.g < x_at_yz1.r){
				k_yz.g -= 1.0;
				++voxel.z;
				abcd = efgh;
				efgh = vec4(none);
			} else {
				k_yz.r -= 1.0;
				++voxel.y;
				abcd.xy = abcd.zw;
				abcd.zw = vec2(none);
				efgh.xy = efgh.zw;
				efgh.zw = vec2(none);
			}
		} else if(x_at_yz1.g < 1.0){
			k_yz.g -= 1.0;
			++voxel.z;
			abcd = efgh;
			efgh = vec4(none);
		} else {
			k_yz += m_yz;
			++voxel.x;
			abcd.xz = abcd.yw;
			abcd.yw = vec2(none);
			efgh.xz = efgh.yw;
			efgh.yw = vec2(none);
		}
	}
	
	float centre = 0.5;
	float range = 1.0;
	float v = 1.0 - (exp(-sum_I * sqrt(1.0 + m_yz.r * m_yz.r + m_yz.g * m_yz.g) / (1.73 * float(size_largest_minus_1))) - centre + range * 0.5) / range;
	
	if(u_drawMode == 0 || u_drawMode == 2){ // x-ray comparison or static drr comparison preview
		gl_FragColor.b = v;
		gl_FragColor.r = gl_FragColor.g = texture2D(u_xRay, v_texCoord).r;
		gl_FragColor.a = 1.0;
	} else if(u_drawMode == 1){ // deep drr only (for comparison)
		v *= 65536.0;
		gl_FragColor = vec4(mod(v, 256.0)/256.0, floor(v / 256.0) / 256.0, 0.0, 1.0);
	} else { // shallow drr only (for preview)
		gl_FragColor = vec4(vec3(v), 1.0);
	}
}

