#version 330 core

layout (std140) uniform camera
{
	mat4 projection;
	mat4 view;
	mat4 pvm;
	mat4 ortho;
	vec4 position;
};

/*uniform variables*/
uniform float iTime;    ////time
uniform mat4 model;

/*input variables*/
layout (location=0) in vec4 pos;
layout (location=2) in vec4 normal;
layout (location=3) in vec4 uv;
layout (location=4) in vec4 tangent;

/*output variables*/
out vec4 vtx_uv;
out vec4 vtx_normal;
out vec3 vtx_pos;
out vec4 vtx_tangent;

///////////// Part 1a /////////////////////
/* Create a function that takes in an xy coordinate and returns a 'random' 2d vector. */
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
	// Your implementation starts here

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

	// Your implementation ends here
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

///////////// Part 2a /////////////////////
/* create a function that takes in a 2D point and returns a height using perlin noise 
    There is no right answer. Think about what functions will create what shapes.
    If you want steep mountains with flat tops, use a function like sqrt(noiseOctave(v,num)). 
    If you want jagged mountains, use a function like e^(noiseOctave(v,num))
    You can also add functions on top of each other and change the frequency of the noise
    by multiplying v by some value other than 1*/
float height(vec2 v){
    float h = 0;

	h=2.*noiseOctave(.01*2.*v,10);

	if (h < 0.) {
		h=9.5*abs(sin(noiseOctave(.01*2.*v,10)));
	}
	
	return h;
}

void main()												
{
	vtx_pos = (vec4(pos.xy, height(pos.xy), 1)).xyz;
	gl_Position = pvm * vec4(vtx_pos,1);
	vtx_uv=uv;
	vtx_normal=vec4(normal.xyz,1.f);
	vtx_pos=vec3(pos.xyz);
	vtx_tangent=vec4(tangent.xyz,1.f);
}