#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer

      Summary:  Constructor

      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                  m_immediateContext, m_immediateContext1, m_swapChain,
                  m_swapChain1, m_renderTargetView, m_vertexShader,
                  m_pixelShader, m_vertexLayout, m_vertexBuffer].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_d3dDevice(nullptr)
        , m_d3dDevice1(nullptr)
        , m_immediateContext(nullptr)
        , m_immediateContext1(nullptr)
        , m_swapChain(nullptr)
        , m_swapChain1(nullptr)
        , m_renderTargetView(nullptr)
        , m_vertexShader(nullptr)
        , m_pixelShader(nullptr)
        , m_vertexLayout(nullptr)
        , m_vertexBuffer(nullptr)
    {
        // blank
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize

      Summary:  Creates Direct3D device and swap chain

      Args:     HWND hWnd
                  Handle to the window

      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                  m_d3dDevice1, m_immediateContext1, m_swapChain1,
                  m_swapChain, m_renderTargetView, m_vertexShader, 
                  m_vertexLayout, m_pixelShader, m_vertexBuffer].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect(hWnd, &rc);
        UINT uWidth = static_cast<UINT> (rc.right - rc.left);
        UINT uHeight = static_cast<UINT> (rc.bottom - rc.top);

        UINT uCreateDeviceFlags = 0u;
#ifdef _DEBUG
        uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT uNumDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT uNumFeatureLevels = ARRAYSIZE(featureLevels);

        for (UINT driverTypeIndex = 0u; driverTypeIndex < uNumDriverTypes; ++driverTypeIndex)
        {
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, featureLevels, uNumFeatureLevels,
                                   D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            
            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, uCreateDeviceFlags, &featureLevels[1], uNumFeatureLevels - 1,
                                       D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
            {
                break;
            }
        }
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to D3D11CreateDevice failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Obtain DXGI factory from device
        ComPtr<IDXGIFactory1> dxgiFactory = nullptr;
        {
            ComPtr<IDXGIDevice> dxgiDevice = nullptr;
            if (SUCCEEDED(m_d3dDevice.As(&dxgiDevice)))
            {
                ComPtr<IDXGIAdapter> adapter = nullptr;
                if (SUCCEEDED(dxgiDevice->GetAdapter(&adapter)))
                {
                    hr = adapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
                }
            }
        }
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to DXGI factory setting failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2 = nullptr;
        if (SUCCEEDED(dxgiFactory.As(&dxgiFactory2)))
        {
            // DirectX 11.1 or later
            if (SUCCEEDED(m_d3dDevice.As(&m_d3dDevice1)))
            {
                m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = uWidth,
                .Height = uHeight,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {.Count = 1u,
                               .Quality = 0u},
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, &m_swapChain1);
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd =
            {
                .BufferDesc = {.Width = uWidth,
                               .Height = uHeight,
                               .RefreshRate = {.Numerator = 60u,
                                               .Denominator = 1u},
                               .Format = DXGI_FORMAT_R8G8B8A8_UNORM},
                .SampleDesc = {.Count = 1u,
                               .Quality = 0u},
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1u,
                .OutputWindow = hWnd,
                .Windowed = TRUE
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, &m_swapChain);
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreateSwapChain failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer = nullptr;
        hr = m_swapChain->GetBuffer(0u, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to GetBuffer failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreateRenderTargetView failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0.0f,
            .TopLeftY = 0.0f,
            .Width = static_cast<FLOAT>(uWidth),
            .Height = static_cast<FLOAT>(uHeight),
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };
        m_immediateContext->RSSetViewports(1u, &vp);

        // Compile the vertex shader
        ComPtr<ID3DBlob> pVSBlob = nullptr;
        hr = compileShaderFromFile(L"../Library\\Shaders\\Lab03.fxh", "VS", "vs_5_0", pVSBlob.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CompileVertexShader failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Create the vertex shader
        hr = m_d3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreateVertexShader failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Define the input layout
        D3D11_INPUT_ELEMENT_DESC aLayouts[] =
        {
            { "POSITION",
              0,
              DXGI_FORMAT_R32G32B32_FLOAT,
              0,
              0,
              D3D11_INPUT_PER_VERTEX_DATA,
              0 }
        };
        UINT uNumElements = ARRAYSIZE(aLayouts);

        // Create the input layout
        hr = m_d3dDevice->CreateInputLayout(aLayouts, uNumElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), m_vertexLayout.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreateInputLayout failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Set the input layout
        m_immediateContext->IASetInputLayout(m_vertexLayout.Get());

        // Compile the pixel shader
        ComPtr<ID3DBlob> pPSBlob = nullptr;
        hr = compileShaderFromFile(L"../Library\\Shaders\\Lab03.fxh", "PS", "ps_5_0", pPSBlob.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CompilePixelShader failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Create the pixel shader
        hr = m_d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreatePixelShader failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Create vertex buffer
        SimpleVertex aVertices[] =
        {
            XMFLOAT3(0.0f, 0.5f, 0.5f),
            XMFLOAT3(0.5f, -0.5f, 0.5f),
            XMFLOAT3(-0.5f, -0.5f, 0.5f)
        };
        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(SimpleVertex) * 3u,
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_VERTEX_BUFFER,
            .CPUAccessFlags = 0u,
            .MiscFlags = 0u
        };

        D3D11_SUBRESOURCE_DATA InitData =
        {
            .pSysMem = aVertices,
            .SysMemPitch = 0u,
            .SysMemSlicePitch = 0u
        };
        hr = m_d3dDevice->CreateBuffer(&bd, &InitData, m_vertexBuffer.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreateBuffer failed!",
                L"Game Graphics Programming",
                NULL
            );
            return hr;
        }

        // Set vertex buffer
        UINT uStride = sizeof(SimpleVertex);
        UINT uOffset = 0u;
        m_immediateContext->IASetVertexBuffers(0u, 1u, m_vertexBuffer.GetAddressOf(), &uStride, &uOffset);

        // Set primitive topology
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Render()
    {
        // Clear the back buffer
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

        // Render a triangle
        m_immediateContext->VSSetShader(m_vertexShader.Get(), nullptr, 0u);
        m_immediateContext->PSSetShader(m_pixelShader.Get(), nullptr, 0u);
        m_immediateContext->Draw(3u, 0u);

        // Present the information rendered to the back buffer to the front buffer (the screen)
        m_swapChain->Present(0u, 0u);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::compileShaderFromFile

      Summary:  Helper for compiling shaders with D3DCompile

      Args:     PCWSTR pszFileName
                  A pointer to a constant null-terminated string that
                  contains the name of the file that contains the
                  shader code
                PCSTR pszEntryPoint
                  A pointer to a constant null-terminated string that
                  contains the name of the shader entry point function
                  where shader execution begins
                PCSTR pszShaderModel
                  A pointer to a constant null-terminated string that
                  specifies the shader target or set of shader
                  features to compile against
                ID3DBlob** ppBlobOut
                  A pointer to a variable that receives a pointer to
                  the ID3DBlob interface that you can use to access
                  the compiled code

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::compileShaderFromFile(_In_ PCWSTR pszFileName, _In_ PCSTR pszEntryPoint, _In_ PCSTR szShaderModel, _Outptr_ ID3DBlob** ppBlobOut)
    {
        HRESULT hr = S_OK;

        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
        // Setting this flag improves the shader debugging experience, but still allows 
        // the shaders to be optimized and to run exactly the way they will run in 
        // the release configuration of this program.
        dwShaderFlags |= D3DCOMPILE_DEBUG;

        // Disable optimizations to further improve shader debugging
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // _DEBUG

        ComPtr<ID3DBlob> pErrorBlob = nullptr;
        hr = D3DCompileFromFile(pszFileName, nullptr, nullptr, pszEntryPoint, szShaderModel, dwShaderFlags, 0u, ppBlobOut, pErrorBlob.GetAddressOf());
        if (FAILED(hr))
        {
            if (pErrorBlob)
            {
                OutputDebugStringA(reinterpret_cast<LPCSTR>(pErrorBlob->GetBufferPointer()));
            }
            return hr;
        }

        return S_OK;
    }
}