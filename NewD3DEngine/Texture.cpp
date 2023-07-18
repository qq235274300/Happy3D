#include "Texture.h"
#include "Surface.h"
#include "GraphicsThrowMacros.h"

namespace wrl =  Microsoft::WRL ;

Texture::Texture(Graphics& gfx, const Surface& s)
{
	INFOMAN(gfx);
	//创建Texture资源的藐视
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = s.GetWidth();
	textureDesc.Height = s.GetHeight();
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;//创建Texture的时候 可以多个
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //8表示一个字节0-255 ,UNORM 表示 0-255 到Shader里面会Map成 0-1 
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; //Texture是一个Resource 给shader绑定的
	textureDesc.CPUAccessFlags = 0; //拿到后固定
	textureDesc.MiscFlags = 0;
	//Texture实际的内存
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = s.GetBufferPtr();
	sd.SysMemPitch = s.GetWidth() * sizeof(Surface::Color); //一个row的大小
	//实际创建
	wrl::ComPtr<ID3D11Texture2D> pTexture;
	GFX_THROW_INFO(
		GetDevice(gfx)->CreateTexture2D(&textureDesc, &sd, &pTexture)
	);
	
	//绑定到Shader的不是Texture 而是根据Texture生成的TextureView
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
