struct Input {
    float2 position : POSITION;
    float2 textureCoordinate : TEXCOORD;
};

struct Output {
    float4 position : SV_POSITION;
    float2 textureCoordinate : TEXCOORD;
};

cbuffer Uniforms {
    float4x4 modelMatrix;
};

Output main(Input input){
    Output output;

    output.position = mul(modelMatrix, float4(input.position.x, input.position.y, 0, 1));
    output.textureCoordinate = input.textureCoordinate;

    return output;
}