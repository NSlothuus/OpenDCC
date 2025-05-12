uniform int width;
uniform int height;
uniform int mode;
varying vec2 vTexCoord;

vec4 checker2D(vec2 texc, vec4 color0, vec4 color1)
{
	int quad_ind = int(floor(texc.x) + floor(texc.y));
	int mod_by_2 = quad_ind - 2 * (quad_ind / 2);
	if (mod_by_2 == 0)
		return color0;
	else
		return color1;
}

void main()
{
	if (mode == 0)
	{
		vec2 st = vTexCoord;
		st.x = st.x * float(width);
		st.y = st.y * float(height);
		st = st / vec2(10);
		gl_FragColor = checker2D(st, vec4(0.2,0.2,0.2,1), vec4(0.1,0.1,0.1,1) );
	}
	else if (mode == 1)
	{
		gl_FragColor = vec4(0 ,0, 0, 1);
	}
	else if (mode == 2)
	{
		gl_FragColor = vec4(0.5, 0.5, 0.5, 1);
	}
	else
	{
	    gl_FragColor = vec4(1 ,1, 1, 1);
	}
}
