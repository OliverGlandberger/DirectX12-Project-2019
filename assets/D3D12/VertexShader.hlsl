struct VSIn {
	float4 position : POSITION0;
	float4 normal 	: NORMAL0;
	float2 texCoord : TEXCOORD0;
};

struct VSOut {
	float4 position : SV_POSITION;
	float4 normal 	: NORMAL0;
	float2 texCoord : TEXCOORD0;
};

cbuffer TRANSLATION_MATRIX : register(b0) {
	float4x4 worldMatrix;
}

cbuffer VIEWPROJ_MATRIX : register(b1) {
	float4x4 viewProjMatrix;
}

VSOut VS_main(VSIn input) {
	VSOut output = (VSOut)0;
	output.position = mul(mul(viewProjMatrix, worldMatrix), input.position);

	output.normal = input.normal;
		
	output.texCoord = input.texCoord;


	return output;
}