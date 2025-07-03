#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 4

//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 modelPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float3 modelTangent : TANGENT;
	float3 modelBitangent : BITANGENT;
	float3 modelNormal : NORMAL;
};

//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 clipPosition : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float4 worldTangent : TANGENT;
	float4 worldBitangent : BITANGENT;
	float4 worldNormal : NORMAL;
	float4 worldFragPosition : POSITION;
};

//------------------------------------------------------------------------------------------------
cbuffer LightConstants : register(b7)
{
	float3 SunDirection;
	float SunIntensity;
	float AmbientIntensity;
	float3 AmbientLightColor;
};

//------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 WorldToCameraTransform;	// View transform
	float4x4 CameraToRenderTransform;	// Non-standard transform from game to DirectX conventions
	float4x4 RenderToClipTransform;		// Projection transform
};

//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 ModelToWorldTransform;		// Model transform
	float4 ModelColor;
};

//------------------------------------------------------------------------------------------------
struct PointLightConstants
{
	float3 PointLightPosition;
	float LightIntensity;
	float3 LightColor;
	float LightRange;
};

cbuffer PointLight : register(b4)
{
	PointLightConstants pointLights[MAX_POINT_LIGHTS];
};

//------------------------------------------------------------------------------------------------
struct SpotLightConstants
{
	float3 SpotLightPosition;
	float SpotLightCutOff;
	float3 SpotLightDirection;
	float pad0;
	float3 SpotLightColor;
	float pad1;
};

cbuffer SpotLight : register(b5)
{
	SpotLightConstants spotLights[MAX_SPOT_LIGHTS];
};
//------------------------------------------------------------------------------------------------
cbuffer ShadowConstants : register(b6)
{
	float4x4 LightViewProjMatrix;
};

//------------------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);
Texture2D shadowTexture : register(t1);         // 新增，ShadowMap绑定t1

//------------------------------------------------------------------------------------------------
SamplerState samplerState : register(s0);
SamplerState shadowSampler : register(s1);       // 新增，ShadowMap采样器绑定s1

//------------------------------------------------------------------------------------------------
float ComputeShadowFactor(float3 worldPos)
{
    float4 lightSpacePos = mul(LightViewProjMatrix, float4(worldPos, 1.0f));
    lightSpacePos.xyz /= lightSpacePos.w;
    float2 shadowUV = lightSpacePos.xy * 0.5f + 0.5f;

    float currentDepth = lightSpacePos.z * 0.5f + 0.5f;
    float bias = 0.003f;

    if (shadowUV.x < 0.0f || shadowUV.x > 1.0f || shadowUV.y < 0.0f || shadowUV.y > 1.0f)
    {
        return 1.0f;
    }
    else
    {
        float shadowMapDepth = shadowTexture.Sample(shadowSampler, shadowUV).r;
        return (currentDepth - bias) <= shadowMapDepth ? 1.0f : 0.0f;
    }
}

//------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	float4 modelPosition = float4(input.modelPosition, 1);
	float4 worldPosition = mul(ModelToWorldTransform, modelPosition);
	float4 cameraPosition = mul(WorldToCameraTransform, worldPosition);
	float4 renderPosition = mul(CameraToRenderTransform, cameraPosition);
	float4 clipPosition = mul(RenderToClipTransform, renderPosition);

	float4 worldTangent = mul(ModelToWorldTransform, float4(input.modelTangent, 0.0f));
	float4 worldBitangent = mul(ModelToWorldTransform, float4(input.modelBitangent, 0.0f));
	float4 worldNormal = mul(ModelToWorldTransform, float4(input.modelNormal, 0.0f));

	v2p_t v2p;
	v2p.clipPosition = clipPosition;
	v2p.color = input.color;
	v2p.uv = input.uv;
	v2p.worldTangent = worldTangent;
	v2p.worldBitangent = worldBitangent;
	v2p.worldNormal = worldNormal;
	v2p.worldFragPosition = worldPosition;  
	return v2p;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	float3 normal = normalize(input.worldNormal.xyz);
	float3 worldPos = input.worldFragPosition.xyz;
	float4 texColor = diffuseTexture.Sample(samplerState, input.uv);
	float4 vertexColor = input.color;
	float4 modelColor = ModelColor;

	float3 totalLight = AmbientIntensity.xxx;
	//totalLight += (SunIntensity * max(dot(normal, -SunDirection), 0.0)).xxx;
	//Shadow
    float sunDiffuse = max(dot(normal, -SunDirection), 0.0f);
    float shadowFactor = ComputeShadowFactor(input.worldNormal.xyz);
    totalLight += (SunIntensity * sunDiffuse * shadowFactor).xxx;


	// Point lights
	for (int pointIndex = 0; pointIndex < MAX_POINT_LIGHTS; ++pointIndex)
	{
		float3 lightDir = normalize(pointLights[pointIndex].PointLightPosition - worldPos);
		float dist = length(pointLights[pointIndex].PointLightPosition - worldPos);
		float attn = saturate(1.0 - dist / pointLights[pointIndex].LightRange);
		float diffuse = max(dot(normal, lightDir), 0.0);

		float3 pointLight = pointLights[pointIndex].LightColor * pointLights[pointIndex].LightIntensity * diffuse * attn;
		totalLight += pointLight;
	}

	// Spot lights
	for (int spotIndex = 0; spotIndex < MAX_SPOT_LIGHTS; ++spotIndex)
	{
		float3 toFrag = normalize(spotLights[spotIndex].SpotLightPosition - worldPos);
		float theta = dot(toFrag, -normalize(spotLights[spotIndex].SpotLightDirection));
		float falloff = smoothstep(spotLights[spotIndex].SpotLightCutOff, 1.0, theta);
		float diffuse = max(dot(normal, toFrag), 0.0);

		float3 spotLight = spotLights[spotIndex].SpotLightColor * diffuse * falloff;
		totalLight += spotLight;
	}

	float3 finalRGB = totalLight * texColor.rgb * vertexColor.rgb * modelColor.rgb;
	float alpha = texColor.a * vertexColor.a * modelColor.a;
	clip(alpha - 0.01f);
	return float4(finalRGB, alpha);
}
