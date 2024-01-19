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

uniform vec4 color=vec4(0.f,1.f,0.f,1.f);
uniform sampler2D tex_albedo;
uniform float iTime;

in vec3 vtx_frg_pos;

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

// loading background texture with snow effect added on top 
void main()								
{ 
	frag_color=texture(tex_albedo, vec2(vtx_frg_pos.x, -vtx_frg_pos.y))+vec4(snow_effect().rgb, 0.);
}	