#include <windows.h>  
#include <d3d11.h>
#include <d3dcompiler.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)  {  
    if(message == WM_DESTROY || message == WM_CLOSE){
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
    float vertices[] = {
        -1, -1, 1, 0, 0,
         0,  1, 0, 1, 0,
         1, -1, 0, 0, 1
    };

    ID3D11Buffer* vertexBuffer = 0;

    CD3D11_BUFFER_DESC vertexBufferDescriptor(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);
    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = vertices;

    device->CreateBuffer(&vertexBufferDescriptor, &vertexData, &vertexBuffer);
    UINT stride = sizeof(float) * 5;
    UINT offset = 0;
    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

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
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 2 /*D3D11_APPEND_ALIGNED_ELEMENT*/, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    device->CreateInputLayout(layout, 2, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &inputLayout);
    deviceContext->IASetInputLayout(inputLayout);

    deviceContext->OMSetRenderTargets(1, &renderTargetView, 0);

    CD3D11_VIEWPORT viewPort(0.0f, 0.0f, (float)backBufferDesc.Width, (float)backBufferDesc.Height);
    deviceContext->RSSetViewports(1, &viewPort);

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
        deviceContext->Draw(3, 0);

        swapChain->Present(1, 0);
    }

    return 0;
}   