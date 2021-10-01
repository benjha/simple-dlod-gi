
uniform float scaleFactor;

void main ()
{
	gl_FragColor = gl_TexCoord[0]*scaleFactor;
}