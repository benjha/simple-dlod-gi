
uniform sampler2DRect GI;

vec4 texRectBilinear (uniform sampler2DRect tex, vec2 s)
{
	vec2 f = frac (s);
	vec4 s2 = s.xyxy + vec4 (0,0,1,1);
	vec4 t0 = texture2DRect (tex, s2.xy);
	vec4 t1 = texture2DRect (tex, s2.zy);
	vec4 t2  = lerp(t0, t1, f[0]);
	
	t0 = texture2DRect (tex, s2.xw);
	t1 = texture2DRect (tex, s2.zw);
	t0 = lerp(t0, t1, f[0]);
	t0 = lerp(t2, t0, f[1]);
	return t0;
	
	/*
	float2 f = frac(s);
	float4 s2 = s.xyxy + float4(0, 0, 1, 1);
	half4 t0 = h4texRECT(tex, s2.xy);
	half4 t1 = h4texRECT(tex, s2.zy);
	half4 t2  = lerp(t0, t1, f[0]);
	t0 = h4texRECT(tex, s2.xw);
	t1 = h4texRECT(tex, s2.zw);
	t0 = lerp(t0, t1, f[0]);
	t0 = lerp(t2, t0, f[1]);
	return t0;
	*/
}


void main () {

	vec2 uv = gl_TexCoord[0].st;
	
	vec3 vert0 = texRectBilinear (GI, uv).rgb;
	vec3 vert1 = texRectBilinear (GI, vec2(uv.x,uv.y+1)).rgb;
	vec3 vert2 = texRectBilinear (GI, vec2(uv.x+1, uv.y)).rgb;
	vec3 v1 = vert1 - vert0;
	vec3 v2 = vert2 - vert0;
	vec3 normal = cross (v1,v2);
	normal = normalize (normal);
	gl_FragColor = vec4(normal,1);
  
}