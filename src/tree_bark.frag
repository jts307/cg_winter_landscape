#version 330 core
#define LIGHT_SNOW // Comment this out for a blizzard

#ifdef LIGHT_SNOW
	#define LAYERS 50
	#define DEPTH .5
	#define WIDTH .3
	#define SPEED .6
#else // BLIZZARD
	#define LAYERS 200
	#define DEPTH .1
	#define WIDTH .8
	#define SPEED 1.5
#endif

/*default camera matrices. do not modify.*/
layout (std140) uniform camera
{
	mat4 projection;	/*camera's projection matrix*/
	mat4 view;			/*camera's view matrix*/
	mat4 pvm;			/*camera's projection*view*model matrix*/
	mat4 ortho;			/*camera's ortho projection matrix*/
	vec4 position;		/*camera's position in world space*/
};

/*uniform variables*/
uniform float iTime;					////time
uniform sampler2D tex_albedo;			////texture color
uniform sampler2D tex_normal;			////texture normal

/*input variables*/
in vec4 vtx_uv;
in vec4 vtx_normal;
in vec4 vtx_pos;
in vec4 vtx_tangent;

/*output variables*/
out vec4 frag_color;

// The snow effect is a version of a program that I slightly edited the credit for the source is as follows:
// Copyright (c) 2013 Andrew Baldwin (twitter: baldand, www: http://thndl.com)
// License = Attribution-NonCommercial-ShareAlike (http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US)
// shadertoy LINK: https://www.shadertoy.com/view/ldsGDn
// "Just snow"
// Simple snow made from multiple parallax layers with randomly positioned 
// flakes and directions.
vec4 snow_effect()
{
	const mat3 p = mat3(13.323122,23.5112,21.71123,21.1212,28.7312,11.9312,21.8112,14.7212,61.3934);
	vec2 constant = vec2(300,300);
	vec2 hash = constant.xy/constant + vec2(1.,constant.y/constant.x)*gl_FragCoord.xy / constant.xy;
    vec3 acc = vec3(0.0);
	float dof = 5.*sin(iTime*.1);
	for (int i=0;i<LAYERS;i++) {
		float fi = float(i);
		vec2 q = hash*(1.+fi*DEPTH);
		q += vec2(q.y*(WIDTH*mod(fi*7.238917,1.)-WIDTH*.5),SPEED*iTime/(1.+fi*DEPTH*.03));
		vec3 n = vec3(floor(q),31.189+fi);
		vec3 m = floor(n)*.00001 + fract(n);
		vec3 mp = (31415.9+m)/fract(p*m);
		vec3 r = fract(mp);
		vec2 s = abs(mod(q,1.)-.5+.9*r.xy-.45);
		s += .01*abs(2.*fract(10.*q.yx)-1.); 
		float d = .6*max(s.x-s.y,s.x+s.y)+max(s.x,s.y)-.01;
		float edge = .005+.05*min(.5*abs(fi-5.-dof),1.);
		acc += vec3(smoothstep(edge,-edge,d)*(r.x/(1.+.02*fi*DEPTH)));
	}
	return vec4(vec3(acc),1.0);
}

const vec3 LightPosition = vec3(-20, 15, 25);
const vec3 LightIntensity = vec3(1);
const vec3 ka = 0.01*vec3(1., 1., 1.);
const vec3 kd = 0.6*vec3(1., 1., 1.);
const vec3 ks = 0.6*vec3(1.);
const float n = 10.0;

vec2 hash2(vec2 v)
{
	vec2 rand = vec2(0,0);
	
	// hash function 
	rand = fract(sin(vec2(dot(v,vec2(127.1,311.7)),dot(v,vec2(269.5,183.3))))*43758.5453);
	rand = -1.0 + 2 * 1.05 * fract((rand.x + rand.y) * rand);

	return rand;
}

///////////// Part 1b /////////////////////
/*  Using i, f, and m, compute the perlin noise at point v */
float perlin_noise(vec2 v) 
{
	float noise = 0.;

	// the size of a cell on the grid, assumed to be 1x1 for this assignment 
	vec2 size = vec2(1.);

	// floor(v) mod size, mod is component-wise
	vec2 mod1 = floor(v)-(size*floor(floor(v)/size));

	// bottom left corner of square point is in
	vec2 bottom_left = floor(v)-mod1;
	
	// relative position inside cell
	vec2 relative_v = (mod1+fract(v))/size;

	/// dot products 
	float product0 = (dot((v-bottom_left),normalize(hash2(bottom_left))));
	float product1 = (dot((v-(bottom_left+vec2(size.x,0.))),normalize(hash2(bottom_left+vec2(size.x,0.)))));
	float product2 = (dot((v-(bottom_left+vec2(0.,size.y))),normalize(hash2(bottom_left+vec2(0.,size.y)))));
	float product3 = (dot((v-(bottom_left+vec2(size.x,size.y))),normalize(hash2(bottom_left+vec2(size.x,size.y)))));

	// S(x) and S(y)
	float sx = smoothstep(0.,1., relative_v.x);
	float sy = smoothstep(0.,1., relative_v.y);

	// interpolation using mix
	noise = mix(mix(product0, product1, sx), mix(product2, product3, sx), sy);

	// interpolation using geometric interpretation
	// noise = (size.x-sx)*(size.y-sy)*product0+sx*(size.y-sy)*product1+(size.x-sx)*sy*product2+sx*sy*product3;

	return noise;
}

///////////// Part 1c /////////////////////
/*  Given a point v and an int num, compute the perlin noise octave for point v with octave num
	num will be greater than 0 */
float noiseOctave(vec2 v, int num)
{
	float sum = 0;
	// Your implementation starts here
	for(int i=0; i<num; i++) {
		sum+=(pow(2,-i)*perlin_noise(pow(2,i)*v));
	}
	// Your implementation ends here
	return sum;
}

// this is the shader for normal mapping but with an added perlin noise generated snow on top of the texture loaded in. It also adds the snow effect.  
void main()							
{		
	bool use_normal_mapping = true;	
	
	if(!use_normal_mapping){
		//// TODO (Step 1 and 2): texture with shading
		////Step 1.0: load the texture color from tex_albedo and then set it to frag_color
		////Step 2.0: use the loaded texture color as the local material color and multiply the local color with the Lambertian shading model you implemented previously to render a textured and shaded sphere.
		////The way to read a texture is to call texture(texture_name,uv). It will return a vec4.

		//// Phong and Lambertian shading calculations 
		vec3 lightDirection=normalize(LightPosition-vtx_pos.xyz);
		vec3 normal=normalize(vtx_normal.xyz);

		// Lambert's cosine law
		float lambert = dot(lightDirection, normal);

		// reflection direction from light source
		vec3 r = normalize(-lightDirection + 2.*(dot(lightDirection, normal))*normal);

		// camera direction
		vec3 v = normalize(position.xyz-vtx_pos.xyz);

		// Phong term
		vec3 specular = ks*LightIntensity*pow(max(dot(r,v), 0.0), n);

		// combining texture color and shading 
		vec4 tex = texture(tex_albedo, vtx_uv.xy);
		vec3 col = (ka*LightIntensity+kd*LightIntensity*max(lambert,0.0)+specular)*tex.rgb;

		frag_color = vec4(col.rgb, 1);
	}
	else{
		
		////Step 3.0: load the texture color from tex_albedo
		vec4 tex_col=texture(tex_albedo, vtx_uv.xy);

		////Step 3.1: load the texture normal from tex_normal, and remap each component from [0, 1] to [-1, 1] (notice that currently the loaded normal is in the local tangent space)
		vec4 tex_normal=2*texture(tex_normal, vtx_uv.xy)-1;

		////Step 3.2: calculate the TBN matrix using the vertex normal and tangent
		vec4 bi_tangent= vec4(cross(vtx_normal.xyz, vtx_tangent.xyz), 1.f);

		mat3 tbn = mat3(normalize(vtx_tangent.xyz), normalize(bi_tangent.xyz), normalize(vtx_normal.xyz));

		mat4 tbn_homogenous=mat4(tbn);

		////Step 3.3: transform the texture normal from the local tangent space to the global world space
		vec4 global_tangent=tbn_homogenous*tex_normal;

		////Step 3.4 and following: use the transformed normal and the loaded texture color to conduct the further lighting calculation
		//// Phong and Lambertian shading calculations 
		vec3 lightDirection=normalize(LightPosition-vtx_pos.xyz);
		vec3 normal=normalize(global_tangent.xyz);
		
		// Lambert's cosine law
		float lambert = dot(lightDirection, normal);

		// reflection direction from light source
		vec3 r = normalize(-lightDirection + 2.*(dot(lightDirection, normal))*normal);

		// camera direction
		vec3 v = normalize(position.xyz-vtx_pos.xyz);

		// Phong term
		vec3 specular = ks*LightIntensity*pow(max(dot(r,v), 0.0), n);

		// add snow on top of tree's bark texture by using perlin noise 
		if (.1*tan(noiseOctave(1.5*vtx_pos.xz, 3)) > 0.){
			tex_col.xyz = vec3(1,.98,.98);
		}

		// combining shading and texture color
		vec3 col = (ka*LightIntensity+kd*LightIntensity*max(lambert,0.0)+specular)*tex_col.rgb;

		frag_color = vec4(col.rgb+snow_effect().rgb, tex_col[3]);
	}
}	