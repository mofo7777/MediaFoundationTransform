//----------------------------------------------------------------------------------------------
// Shader.fx
//----------------------------------------------------------------------------------------------
float2 g_vTexelSize;
texture g_pNV12Texture;

sampler NV12Sampler = sampler_state{

	Texture = <g_pNV12Texture>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = Point;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VS_INPUT{

	float3 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT{

	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

VS_OUTPUT ShaderVS(VS_INPUT i){

	VS_OUTPUT o;

	o.Pos = float4(i.Pos, 1.0f);
	o.Tex = float2(i.Tex.x + g_vTexelSize.x, i.Tex.y + g_vTexelSize.y);

	return o;
}

float4 ShaderPS(VS_OUTPUT i) : COLOR{

	return tex2D(NV12Sampler, i.Tex);
}

technique UpdateNV12{

	pass P0{

		VertexShader = compile vs_3_0 ShaderVS();
		PixelShader  = compile ps_3_0 ShaderPS();
	}
}