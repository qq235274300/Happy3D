#pragma once
#include "Graphics.h"
//接口类 所有BindalbeObject都需要继承 Binable 为 Graphics类友元(父类友元 不代表子类友元) 只通过函数提供给子类Graphics成员

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