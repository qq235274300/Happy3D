#pragma once
#include "Bindable.h"

//VertextBufferLayout 告诉VertextShader VertextBuffer传入的数据类型大小 等

class InputLayout : public Bindable
{
public:
	InputLayout(Graphics& gfx,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout,
		ID3DBlob* pVertexShaderBytecode);
	void Bind(Graphics& gfx) noexcept override;
protected:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
};