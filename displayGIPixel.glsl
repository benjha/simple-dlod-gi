
uniform float specularCoeff;
uniform float diffuseCoeff;

varying vec4 color;

uniform sampler2D sprite;

// gl_TexCoord[1]	stores eye position information
// gl_TexCoord[2]	stores normal information
// gl_TexCoord[3]	stores light Direction
// gl_TexCoord[4]	stores the reflect vector

void main ()
{  

	float diffuse = max(dot(gl_TexCoord[3],gl_TexCoord[2]), 0.0) * diffuseCoeff;
	//float specular = max(dot(gl_TexCoord[4], gl_TexCoord[1]), 0.0);
	//specular = pow(specular, 56.0) * specularCoeff;

	gl_FragColor =  vec4(vec3(diffuse),1);
	gl_FragColor.a = color.a;
	
	if (color.a < 1.0)
		gl_FragColor=0.0;
}