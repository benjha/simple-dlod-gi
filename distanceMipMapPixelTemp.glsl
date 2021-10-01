
uniform vec3 camPos;
uniform vec3 camDir;
uniform float sightLOD;
uniform sampler2DRect mipMap;
uniform sampler2DRect mipMapNormal;
uniform sampler2DRect LODSelection;
//uniform sampler1D LODSelection;


void main ()
{
	vec3 sel, selector;
	vec3 vertex = vec3(gl_ModelViewMatrix* texture2DRect (mipMap, gl_TexCoord[0].st));	
	vec3 viewVector = camPos - vertex;
	vec3 normal = gl_NormalMatrix*texture2DRect (mipMapNormal, gl_TexCoord[0].st).rgb;
	
	float d = dot (viewVector,normal);
	float d2 = dot (camDir,viewVector);
	float lengthView = length(viewVector);

	if ( /*d2<=0 &&*/ d<0.0){
		//vec3 selector = texture2DRect (LODSelection, gl_TexCoord[0].st).rgb;
		sel = vec3(lengthView, lengthView, lengthView);
		for (int x=0;x<10;x++){
			selector = texture2DRect (LODSelection, vec2(x,0.0)).rgb;	
			if (sel.z >= selector.z && sel.z < (selector.z+sightLOD))
				gl_FragColor = vec4(vertex,1);
			else 
				discard;
			break;		
		}
			
	}
	else 
		discard;
}