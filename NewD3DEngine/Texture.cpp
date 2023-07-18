#include "Texture.h"
#include "Surface.h"
#include "GraphicsThrowMacros.h"

namespace wrl =  Microsoft::WRL ;

Texture::Texture(Graphics& gfx, const Surface& s)
{
	INFOMAN(gfx);
	//����Texture��Դ������
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = s.GetWidth();
	textureDesc.Height = s.GetHeight();
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;//����Texture��ʱ�� ���Զ��
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //8��ʾһ���ֽ�0-255 ,UNORM ��ʾ 0-255 ��Shader�����Map�� 0-1 
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; //Texture��һ��Resource ��shader�󶨵�
	textureDesc.CPUAccessFlags = 0; //�õ���̶�
	textureDesc.MiscFlags = 0;
	//Textureʵ�ʵ��ڴ�
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = s.GetBufferPtr();
	sd.SysMemPitch = s.GetWidth() * sizeof(Surface::Color); //һ��row�Ĵ�С
	//ʵ�ʴ���
	wrl::ComPtr<ID3D11Texture2D> pTexture;
	GFX_THROW_INFO(
		GetDevice(gfx)->CreateTexture2D(&textureDesc, &sd, &pTexture)
	);
	
	//�󶨵�Shader�Ĳ���Texture ���Ǹ���Texture���ɵ�TextureView
	D3D11_SHADER_RESOURCE_VIEW_DESC srcDesc = {};
	srcDesc.Format = textureDesc.Format;
	srcDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srcDesc.Texture2D.MostDetailedMip = 0;
	srcDesc.Texture2D.MipLevels = 1;
	GFX_THROW_INFO(
		GetDevice(gfx)->CreateShaderResourceView(pTexture.Get(), &srcDesc, &pTextureView)
	);
	
}

void Texture::Bind(Graphics& gfx) noexcept
{
	GetContext(gfx)->PSSetShaderResources(0u, 1u, pTextureView.GetAddressOf());
}
