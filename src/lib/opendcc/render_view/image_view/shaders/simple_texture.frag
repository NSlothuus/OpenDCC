uniform sampler2D imgtex;
varying vec2 vTexCoord;

void main()
{
	vec2 st = vTexCoord;
	gl_FragColor =  texture2D(imgtex,st);
}
