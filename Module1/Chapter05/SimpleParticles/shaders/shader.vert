#version 330 core
  
smooth out vec4 vSmoothColor;	//output to fragment shader

//shader uniforms
uniform mat4 MVP;				//combined modelview matrix 
uniform float time;				//current time
 
//particle attributes
const vec3 a = vec3(0,2,0);		//acceleration of particles
//vec3 g = vec3(0,-9.8,0);		//if you want acceleration due to gravity 

const float rate = 1/500.0;		//rate of emission
const float life = 2;			//life of particle

//constants
const float PI = 3.14159;
const float TWO_PI = 2*PI;

//colormap colours
const vec3 RED = vec3(1,0,0);
const vec3 GREEN = vec3(0,1,0);
const vec3 YELLOW = vec3(1,1,0); 

//pseudorandom number generator
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

//pseudorandom direction on a sphere
vec3 uniformRadomDir(vec2 v, out vec2 r) {
	r.x = rand(v.xy);
	r.y = rand(v.yx);
	float theta = mix(0.0, PI / 6.0, r.x);
	float phi = mix(0.0, TWO_PI, r.y);
	return vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
}

void main()
{
	//local variables
	vec3 pos=vec3(0);

	//get the spawn time by multiplying the current vertex ID with
	//the rate of emission
	float t = gl_VertexID*rate;
	//start with alpha of 1 (fully visible)
	float alpha = 1;
	
	if(time>t) {
		float dt = mod((time-t), life);
		vec2 xy = vec2(gl_VertexID,t); 
		vec2 rdm=vec2(0);
		//point emitter source
		//pos = ((uniformRadomDir(xy) + 0.5*(a+g)*dt)*dt); //for adding gravity 	   
	   
		pos = ((uniformRadomDir(xy, rdm) + 0.5*a*dt)*dt);       //for fire effect from a point emitter
	   
		/*
	    pos = ( uniformRadomDir(xy, rdm) + 0.5*a*dt)*dt;       //for fire effect from a quad emitter
	    vec2 rect = (rdm*2.0 - 1.0) ;
	    pos += vec3(rect.x, 0, rect.y) ;
	    */
	    
	    /*
	    pos = ( uniformRadomDir(xy, rdm) + 0.5*a*dt)*dt;       //for fire effect from a disc emitter
	    vec2 rect = (rdm*2.0 - 1.0);
	    float dotP = dot(rect, rect);
	    
	    if(dotP<1)
	        pos += vec3(rect.x, 0, rect.y) ;
	    */
	    
		alpha = 1.0 - (dt/life);	  
	}
   
	//linearly interpolate between red and yellow colour
	vSmoothColor = vec4(mix(RED,YELLOW,alpha),alpha);
	//get clipspace position
	gl_Position = MVP*vec4(pos,1);
}
