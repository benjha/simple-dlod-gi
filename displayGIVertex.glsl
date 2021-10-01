
uniform vec3 lPos;
varying vec4 color;

void main(void)
{ 	 
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_ModelViewMatrix*gl_Vertex;
	gl_TexCoord[3] = normalize (vec4(lPos,1) - gl_TexCoord[1]); //light Direction
	//	gl_TexCoord[1] = normalize (- gl_TexCoord[1]); //eye Direction, eye Pos is 0
	gl_TexCoord[2].stp = normalize(gl_NormalMatrix*gl_Normal);
	//	gl_TexCoord[4] = normalize (-reflect(gl_TexCoord[3], gl_TexCoord[2]));

	
	
	if (gl_Vertex.x==0.0 && gl_Vertex.y==0.0 && gl_Vertex.z==0.0)
		gl_Vertex.w=0.0;
	
	color =  gl_Vertex;
	
    //const vec4 pointSize = 10.0;
    //const vec3 atten = vec3(0.0, 1.0, 0.0);
    //float d = length(gl_TexCoord[1]);
    
    //gl_PointSize = clamp (pointSize * sqrt(1/(atten[0] + atten[1]*d + atten[2]*d*d)), 1.0,pointSize);
    //gl_PointSize =(atten[0] + atten[1]*d + atten[2]*d*d)/pointSize;
    //gl_PointSize =10;

	gl_Position =ftransform ();
}