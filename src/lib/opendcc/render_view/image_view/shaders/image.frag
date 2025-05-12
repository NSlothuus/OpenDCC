uniform sampler2D imgtex;
uniform sampler3D lut3d;
varying vec2 vTexCoord;
uniform int startchannel;
uniform int colormode;
uniform int imgchannels;
uniform int transparent;

vec4 rgba_mode (vec4 C)
{
    if (imgchannels <= 2) {
        if (startchannel == 1)
           return vec4(C.aaa, 1.0);
        return C.rrra;
    }
    return C;
}

vec4 rgb_mode (vec4 C, int transparent_mode)
{
    if (imgchannels <= 2) {
        if (startchannel == 1)
           return vec4(C.aaa, 1.0);
        return vec4 (C.rrr, 1.0);
    }
    float C2[4];
    C2[0] = C.x;
		C2[1] = C.y;
		C2[2] = C.z;
		C2[3] = C.w;
    return vec4 (C2[startchannel], C2[startchannel+1], C2[startchannel+2],  (transparent_mode != 0)? C.w : 1.0);
}

vec4 singlechannel_mode (vec4 C, int transparent_mode)
{
    float C2[4];
    C2[0] = C.x;
		C2[1] = C.y;
		C2[2] = C.z;
		C2[3] = C.w;
    if (startchannel > imgchannels)
		{
        return vec4 (0.0,0.0,0.0,1.0);
		}
    return vec4 (C2[startchannel], C2[startchannel], C2[startchannel],  (transparent_mode != 0)? C.w : 1.0);
}

vec4 luminance_mode (vec4 C, int transparent_mode)
{
    if (imgchannels <= 2)
        return vec4 (C.rrr, C.a);
    float lum = dot (C.rgb, vec3(0.2126, 0.7152, 0.0722));
    return vec4 (lum, lum, lum, (transparent_mode != 0) ? C.w : 1.0);
}

void main()
{
	vec2 st = vTexCoord;
	vec4 C = texture2D(imgtex,st);
	int _transparent = transparent;
	if (startchannel < 0)
	{
	        C = vec4(0.0,0.0,0.0,1.0);
	}
	else if (colormode == 0)//Full Color.
	{
	        C = rgb_mode (C, transparent);
	}
	else if (colormode == 1)  //Single channel.
	{
	  C = singlechannel_mode (C, transparent);
	}
	else if (colormode == 2)  //Luminance.
	{
	        C = luminance_mode (C, transparent);
	}
	float alpha = C.w;
	if (startchannel !=3)
	{
#if OCIO_VERSION_MAJOR < 2
		C = OCIODisplay(C, lut3d);
#else
		C = OCIODisplay(C);
#endif
	}
	C.w = alpha;
	gl_FragColor = C;
}
