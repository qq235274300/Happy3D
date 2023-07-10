#pragma once
#include "Graphics.h"
//�ӿ��� ����BindalbeObject����Ҫ�̳� Binable Ϊ Graphics����Ԫ(������Ԫ ������������Ԫ) ֻͨ�������ṩ������Graphics��Ա

class Bindable
{
public:
	virtual void Bind(Graphics& gfx) noexcept = 0;
	virtual ~Bindable() = default;

protected:
	static ID3D11DeviceContext* GetContext(Graphics& gfx) noexcept;
	static ID3D11Device* GetDevice(Graphics& gfx) noexcept;
	static DxgiInfoManager& GetInfoManager(Graphics& gfx) noexcept(!IS_DEBUG);
};