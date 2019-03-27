struct HullShaderInput
{
	float3 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float tessellationFactor : TESS;
};

struct PatchConstantOutput
{
	float edges[4] : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

struct DomainShaderInput
{
	float3 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 binormal : BINORMAL;
};

PatchConstantOutput PatchConstantFunction(InputPatch<HullShaderInput, 4> inputPatch, uint patchId : SV_PrimitiveID)
{
	PatchConstantOutput output;

	output.edges[0] = output.edges[1] = output.edges[2] = output.edges[3] = inputPatch[0].tessellationFactor;

	output.inside[0] = output.inside[1] = inputPatch[0].tessellationFactor;

	return output;
}

[domain("quad")]
//[partitioning("integer")]
//[partitioning("fractional_odd")]
[partitioning("fractional_even")]
//[outputtopology("point")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFunction")]
[maxtessfactor(64.0f)]
DomainShaderInput main(InputPatch<HullShaderInput, 4> inputPatch, uint outputControlPointID : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
	DomainShaderInput output;
	output.position = inputPatch[outputControlPointID].position;
	output.tex = inputPatch[outputControlPointID].tex;
	output.normal = inputPatch[outputControlPointID].normal;
	output.tangent = inputPatch[outputControlPointID].tangent;
	output.binormal = inputPatch[outputControlPointID].binormal;

	return output;
}