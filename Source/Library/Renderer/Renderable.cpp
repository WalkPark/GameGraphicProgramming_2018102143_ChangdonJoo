#include "Renderer/Renderable.h"
#include "Texture/DDSTextureLoader.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::Renderable

      Summary:  Constructor

      Args:     const std::filesystem::path& textureFilePath
                  Path to the texture to use

      Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer, 
                 m_textureRV, m_samplerLinear, m_vertexShader, 
                 m_pixelShader, m_textureFilePath, m_world].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Renderable::Renderable(_In_ const std::filesystem::path& textureFilePath)
        : m_vertexBuffer(nullptr)
        , m_indexBuffer(nullptr)
        , m_constantBuffer(nullptr)
        , m_textureRV(nullptr)
        , m_samplerLinear(nullptr)
        , m_vertexShader(nullptr)
        , m_pixelShader(nullptr)
        , m_textureFilePath(textureFilePath)
        , m_world()
    {
        // empty
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::initialize

      Summary:  Initializes the buffers, texture, and the world matrix

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the buffers
                ID3D11DeviceContext* pImmediateContext
                  The Direct3D context to set buffers

      Modifies: [m_vertexBuffer, m_indexBuffer, m_constantBuffer, 
                 m_textureRV, m_samplerLinear, m_world].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderable::initialize(_In_ ID3D11Device* pDevice, _In_ ID3D11DeviceContext* pImmediateContext)
    {
        HRESULT hr = S_OK;

        // Create vertex buffer
        D3D11_BUFFER_DESC bd =
        {
            .ByteWidth = sizeof(SimpleVertex) * GetNumVertices(),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_VERTEX_BUFFER,
            .CPUAccessFlags = 0u
        };

        D3D11_SUBRESOURCE_DATA InitData =
        {
            .pSysMem = getVertices(),
        };
        hr = pDevice->CreateBuffer(&bd, &InitData, m_vertexBuffer.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreateVertexBuffer failed!",
                L"Game Graphics Programming",
                NULL
            );
            return E_FAIL;
        }

        // Create index buffer
        bd =
        {
            .ByteWidth = static_cast<UINT>(sizeof(WORD)) * GetNumIndices(),
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_INDEX_BUFFER,
            .CPUAccessFlags = 0u
        };
        InitData.pSysMem = getIndices();
        hr = pDevice->CreateBuffer(&bd, &InitData, m_indexBuffer.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreateIndexBuffer failed!",
                L"Game Graphics Programming",
                NULL
            );
            return E_FAIL;
        }

        // Load the Texture
        hr = CreateDDSTextureFromFile(pDevice, m_textureFilePath.filename().wstring().c_str(), nullptr, m_textureRV.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreateDDSTextureFromFile failed!",
                L"Game Graphics Programming",
                NULL
            );
            return E_FAIL;
        }

        // Create the sample state
        D3D11_SAMPLER_DESC sampDesc =
        {
            .Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
            .AddressU = D3D11_TEXTURE_ADDRESS_WRAP,
            .AddressV = D3D11_TEXTURE_ADDRESS_WRAP,
            .AddressW = D3D11_TEXTURE_ADDRESS_WRAP,
            .ComparisonFunc = D3D11_COMPARISON_NEVER,
            .MinLOD = 0.0f,
            .MaxLOD = D3D11_FLOAT32_MAX,
        };
        hr = pDevice->CreateSamplerState(&sampDesc, m_samplerLinear.GetAddressOf());
        if (FAILED(hr))
        {
            MessageBox(
                nullptr,
                L"Call to CreateSamplerState failed!",
                L"Game Graphics Programming",
                NULL
            );
            return E_FAIL;
        }

        // Initialize the world matrix
        m_world = XMMatrixIdentity();

        return hr;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::SetVertexShader

      Summary:  Sets the vertex shader to be used for this renderable
                object

      Args:     const std::shared_ptr<VertexShader>& vertexShader
                  Vertex shader to set to

      Modifies: [m_vertexShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderable::SetVertexShader(_In_ const std::shared_ptr<VertexShader>& vertexShader)
    {
        m_vertexShader = vertexShader;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::SetPixelShader

      Summary:  Sets the pixel shader to be used for this renderable
                object

      Args:     const std::shared_ptr<PixelShader>& pixelShader
                  Pixel shader to set to

      Modifies: [m_pixelShader].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderable::SetPixelShader(_In_ const std::shared_ptr<PixelShader>& pixelShader)
    {
        m_pixelShader = pixelShader;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetVertexShader

      Summary:  Returns the vertex shader

      Returns:  ComPtr<ID3D11VertexShader>&
                  Vertex shader. Could be a nullptr
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11VertexShader>& Renderable::GetVertexShader()
    {
        return m_vertexShader->GetVertexShader();
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetPixelShader

      Summary:  Returns the vertex shader

      Returns:  ComPtr<ID3D11PixelShader>&
                  Pixel shader. Could be a nullptr
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11PixelShader>& Renderable::GetPixelShader()
    {
        return m_pixelShader->GetPixelShader();
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetVertexLayout

      Summary:  Returns the vertex input layout

      Returns:  ComPtr<ID3D11InputLayout>&
                  Vertex input layout
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11InputLayout>& Renderable::GetVertexLayout()
    {
        return m_vertexShader->GetVertexLayout();
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetVertexBuffer

      Summary:  Returns the vertex buffer

      Returns:  ComPtr<ID3D11Buffer>&
                  Vertex buffer
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11Buffer>& Renderable::GetVertexBuffer()
    {
        return m_vertexBuffer;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetIndexBuffer

      Summary:  Returns the index buffer

      Returns:  ComPtr<ID3D11Buffer>&
                  Index buffer
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11Buffer>& Renderable::GetIndexBuffer()
    {
        return m_indexBuffer;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetConstantBuffer

      Summary:  Returns the constant buffer

      Returns:  ComPtr<ID3D11Buffer>&
                  Constant buffer
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11Buffer>& Renderable::GetConstantBuffer()
    {
        return m_constantBuffer;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetWorldMatrix

      Summary:  Returns the world matrix

      Returns:  const XMMATRIX&
                  World matrix
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    const XMMATRIX& Renderable::GetWorldMatrix() const
    {
        return m_world;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetTextureResourceView

      Summary:  Returns the texture resource view

      Returns:  ComPtr<ID3D11ShaderResourceView>&
                  The texture resource view
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11ShaderResourceView>& Renderable::GetTextureResourceView()
    {
        return m_textureRV;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::GetSamplerState

      Summary:  Returns the sampler state

      Returns:  ComPtr<ID3D11SamplerState>&
                  The sampler state
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11SamplerState>& Renderable::GetSamplerState()
    {
        return m_samplerLinear;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::RotateX
      Summary:  Rotates around the x-axis
      Args:     FLOAT angle
                  Angle of rotation around the x-axis, in radians
      Modifies: [m_world].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderable::RotateX(_In_ FLOAT angle)
    {
        // m_world *= x-axis rotation by angle matrix
        m_world *= XMMatrixRotationX(angle);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::RotateY
      Summary:  Rotates around the y-axis
      Args:     FLOAT angle
                  Angle of rotation around the y-axis, in radians
      Modifies: [m_world].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderable::RotateY(_In_ FLOAT angle)
    {
        // m_world *= y-axis rotation by angle matrix
        m_world *= XMMatrixRotationY(angle);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::RotateZ
      Summary:  Rotates around the z-axis
      Args:     FLOAT angle
                  Angle of rotation around the z-axis, in radians
      Modifies: [m_world].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderable::RotateZ(_In_ FLOAT angle)
    {
        // m_world *= z-axis rotation by angle matrix
        m_world *= XMMatrixRotationZ(angle);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::RotateRollPitchYaw
      Summary:  Rotates based on a given pitch, yaw, and roll (Euler angles)
      Args:     FLOAT pitch
                  Angle of rotation around the x-axis, in radians
                FLOAT yaw
                  Angle of rotation around the y-axis, in radians
                FLOAT roll
                  Angle of rotation around the z-axis, in radians
      Modifies: [m_world].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderable::RotateRollPitchYaw(_In_ FLOAT pitch, _In_ FLOAT yaw, _In_ FLOAT roll)
    {
        // m_world *= x, y, z-axis rotation by pitch, yaw, roll matrix
        m_world *= XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::Scale
      Summary:  Scales along the x-axis, y-axis, and z-axis
      Args:     FLOAT scaleX
                  Scaling factor along the x-axis.
                FLOAT scaleY
                  Scaling factor along the y-axis.
                FLOAT scaleZ
                  Scaling factor along the z-axis.
      Modifies: [m_world].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderable::Scale(_In_ FLOAT scaleX, _In_ FLOAT scaleY, _In_ FLOAT scaleZ)
    {
        // m_world *= x, y, z-axis scaling by scale factor matrix
        m_world *= XMMatrixScaling(scaleX, scaleY, scaleZ);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderable::Translate
      Summary:  Translates matrix from a vector
      Args:     const XMVECTOR& offset
                  3D vector describing the translations along the x-axis, y-axis, and z-axis
      Modifies: [m_world].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderable::Translate(_In_ const XMVECTOR& offset)
    {
        // m_world *= translate by offset vector matrix
        m_world *= XMMatrixTranslationFromVector(offset);
    }
}