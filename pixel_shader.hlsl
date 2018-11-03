struct Input {
    float4 position : SV_POSITION;
    float2 textureCoordinate : TEXCOORD;
};

Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

float4 main(Input input) : SV_TARGET {
    return shaderTexture.Sample(sampleType, input.textureCoordinate);
    //return float4(1, 0.7, 0.5, 1);
}