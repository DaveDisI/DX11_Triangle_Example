#include <windows.h>  
#include <d3d11.h>
#include <d3dcompiler.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)  {  
    if(message == WM_DESTROY || message == WM_CLOSE || wParam == VK_ESCAPE){
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);  
}
  
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)  {
    WNDCLASS wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.lpszClassName = "BatteryBarrageClass";

    RegisterClass(&wc);

    HWND window = CreateWindow(wc.lpszClassName, "Battery Barrage", WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, 
                               CW_USEDEFAULT, CW_USEDEFAULT, 900, 500, 
                               0, 0, 0, 0);

    DXGI_SWAP_CHAIN_DESC swapChainDescriptor = {};
    swapChainDescriptor.BufferCount = 1;
    swapChainDescriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDescriptor.OutputWindow = window;
    swapChainDescriptor.SampleDesc.Count = 1;
    swapChainDescriptor.Windowed = true;

    IDXGISwapChain* swapChain = 0;
    ID3D11Device* device = 0;
    ID3D11DeviceContext* deviceContext = 0;
    ID3D11RenderTargetView* renderTargetView = 0;
    D3D11_TEXTURE2D_DESC backBufferDesc;

    HRESULT err = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, 0, 0, D3D11_SDK_VERSION, &swapChainDescriptor,
    &swapChain, &device, 0, &deviceContext);
    if(err != S_OK){
        MessageBox(0, "Could Not Initialize DX11", "Error", MB_OK);
        exit(1);
    }
    ID3D11Texture2D* backBuffer;
    swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    device->CreateRenderTargetView(backBuffer, 0, &renderTargetView);
    backBuffer->GetDesc(&backBufferDesc);
    backBuffer->Release();
    float clearColor[] = { 0.3, 0.5, 0.9, 1.0 };

    //buffers
    float modelMat[16] = {
        0.5, 0, 0, 0,
        0, 0.5, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };

    ID3D11Buffer* uniformBuffer = 0;
    CD3D11_BUFFER_DESC uniformBufferDescriptor(sizeof(float) * 16, D3D11_BIND_CONSTANT_BUFFER);
    D3D11_SUBRESOURCE_DATA bufferData = {};
    bufferData.pSysMem = modelMat;
    device->CreateBuffer(&uniformBufferDescriptor, &bufferData, &uniformBuffer);
    deviceContext->VSSetConstantBuffers(0, 1, &uniformBuffer);

    float vertices[] = {
        -1, -1, 0, 1,
        -1,  1, 0, 0,
         1,  1, 1, 0,
         1, -1, 1, 1,
    };

    ID3D11Buffer* vertexBuffer = 0;
    CD3D11_BUFFER_DESC vertexBufferDescriptor(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);
    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices;

    device->CreateBuffer(&vertexBufferDescriptor, &vertexData, &vertexBuffer);
    UINT stride = sizeof(float) * 4;
    UINT offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    unsigned short indices[] = {
        0, 1, 2, 2, 3, 0
    };
    ID3D11Buffer* indexBuffer = 0;
    CD3D11_BUFFER_DESC indexBufferDescriptor(sizeof(indices), D3D11_BIND_INDEX_BUFFER);
    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices;
    device->CreateBuffer(&indexBufferDescriptor, &indexData, &indexBuffer);
    deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    ID3DBlob* vertexShaderBlob = 0;
    ID3DBlob* errorBlob = 0;
    err = D3DCompileFromFile(L"vertex_shader.hlsl", 0, 0, "main", "vs_5_0", 0, 0, &vertexShaderBlob, &errorBlob);
    if(err != S_OK){
        MessageBox(0, (char*)errorBlob->GetBufferPointer(), "Error", MB_OK);
        exit(1);
    }

    ID3DBlob* pixelShaderBlob = 0;
    errorBlob = 0;
    err = D3DCompileFromFile(L"pixel_shader.hlsl", 0, 0, "main", "ps_5_0", 0, 0, &pixelShaderBlob, &errorBlob);
    if(err != S_OK){
        MessageBox(0, (char*)errorBlob->GetBufferPointer(), "Error", MB_OK);
        exit(1);
    }

    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), 0, &vertexShader);
    device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), 0, &pixelShader);

    deviceContext->VSSetShader(vertexShader, 0, 0);
    deviceContext->PSSetShader(pixelShader, 0, 0);

    ID3D11InputLayout* inputLayout = 0;
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 2, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    device->CreateInputLayout(layout, 2, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &inputLayout);
    deviceContext->IASetInputLayout(inputLayout);

    deviceContext->OMSetRenderTargets(1, &renderTargetView, 0);

    CD3D11_VIEWPORT viewPort(0.0f, 0.0f, (float)backBufferDesc.Width, (float)backBufferDesc.Height);
    deviceContext->RSSetViewports(1, &viewPort);

    unsigned char textureBits[] = {
        255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255,
        0, 0, 255, 255, 255, 0, 0, 255, 0, 255, 0, 255,
        255, 0, 255, 255, 0, 255, 255, 255, 255, 255, 0, 255,
    };

    D3D11_SUBRESOURCE_DATA textureSubData = {};
    textureSubData.pSysMem = textureBits;
    textureSubData.SysMemPitch = sizeof(unsigned char) * 12;
    textureSubData.SysMemSlicePitch = 0;

    ID3D11Texture2D* texture;             
    D3D11_SAMPLER_DESC samplerDescriptor = {};
    samplerDescriptor.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDescriptor.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescriptor.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescriptor.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDescriptor.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDescriptor.MinLOD = 0;
    samplerDescriptor.MaxLOD = D3D11_FLOAT32_MAX;
    ID3D11SamplerState* samplerState;
   

    D3D11_TEXTURE2D_DESC textureDescriptor = {};
    textureDescriptor.Width = 3;
    textureDescriptor.Height = 3;
    textureDescriptor.MipLevels = 1;
    textureDescriptor.ArraySize = 1;
    textureDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDescriptor.SampleDesc.Count = 1;
    textureDescriptor.Usage = D3D11_USAGE_DEFAULT;
    textureDescriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    device->CreateSamplerState(&samplerDescriptor, &samplerState);
    device->CreateTexture2D(&textureDescriptor, &textureSubData, &texture);
    deviceContext->PSSetSamplers(0, 1, &samplerState);

    ID3D11ShaderResourceView* resourceView;
    D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc = {};
    textureViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    textureViewDesc.Texture2D.MipLevels = 1;

    device->CreateShaderResourceView(texture, &textureViewDesc, &resourceView);
    deviceContext->PSSetShaderResources(0, 1, &resourceView);
    
    MSG msg = {};
    while(1){
        if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if(msg.message == WM_QUIT){
                break;
            }
        }
        
        deviceContext->ClearRenderTargetView(renderTargetView, clearColor);

        //draw stuff
        deviceContext->DrawIndexed(6, 0, 0);

        swapChain->Present(1, 0);
    }

    return 0;
}   