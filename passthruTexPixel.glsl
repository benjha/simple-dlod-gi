
uniform sampler2DRect tex;

void main ()
{
	vec4 color  = texture2DRect (tex, gl_TexCoord[0].st);
	
	gl_FragColor = color;
		
}