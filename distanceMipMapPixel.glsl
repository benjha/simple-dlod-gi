
uniform vec3 camPos;
uniform vec3 camDir;
uniform float sightLOD;
uniform sampler2DRect mipMap;
uniform sampler2DRect mipMapNormal;
uniform sampler2DRect LODSelection;

void main ()
{
	vec3 selector;
	vec3 vertex = vec3(gl_ModelViewMatrix* texture2DRect (mipMap, gl_TexCoord[0].st));	
	vec3 viewVector = camPos - vertex;
	vec3 normal = gl_NormalMatrix*texture2DRect (mipMapNormal, gl_TexCoord[0].st).rgb;
	
	float d = dot (viewVector,normal);
	float sel = length(viewVector);
	
	selector = texture2DRect (LODSelection, gl_TexCoord[0].st).rgb;
	
	if (d<0.0){
		if (sel > selector.r && sel < (selector.r+sightLOD*1.05))
			gl_FragColor = vec4 (vertex,1);			
	}
	else 
		discard;
	
}