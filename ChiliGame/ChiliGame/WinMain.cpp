
//-----------------------------------【头文件包含】-----------------------------------
#include <SDKDDKVer.h>//用于根据您希望程序支持的操作系统从Windows头控制将哪些函数、常量等包含到代码中。copy的注释，知道和Windows相关就得了。删了都不报错
#define WIN32_LEAN_AND_MEAN // 从 Windows 头中排除极少使用的资料
#include <windows.h>
#include <tchar.h>//字符串相关的，这里使用的Windows的字符串，东西很乱。
#include <strsafe.h>//字符串相关的
#include <wrl.h>        //添加WTL支持 方便使用COM，就是COM只能指针需要的
#include <dxgi1_6.h>//DXGI的头
#include <DirectXMath.h>//我们涉及的变换矩阵，向量的头
#include <d3d12.h>       //for d3d12
#include <d3dcompiler.h>//D3D 编译相关的
#if defined(_DEBUG)
#include <dxgidebug.h>//DXGI有一个独自的调试，下面还有一个flag相关的
#endif
#include <wincodec.h>   //for WIC，就是纹理相关的
//----------------------------------------------------------------------------------



//-----------------------------------【命名空间、链接库】----------------------------------------
// 链接库是重点，DX必须的东西
//-----------------------------------------------------------------------------------------------

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace Microsoft;
using namespace Microsoft::WRL;
using namespace DirectX;


//-----------------------------------【宏定义部分】-----------------------------------
#define MAX_PATH 260
//用于判断函数执行错误的，如果出错了就会引发异常
#define GRS_THROW_IF_FAILED(hr) {HRESULT _hr = (hr);if (FAILED(_hr)){ throw CGRSCOMException(_hr); }}


//窗口类的名
#define GRS_WND_CLASS_NAME _T("Chili Game Window Class")
#define GRS_WND_TITLE   _T("致我们永不熄灭的游戏开发梦想~")

//新定义的宏用于上取整除法
#define GRS_UPPER_DIV(A,B) ((UINT)(((A)+((B)-1))/(B)))

//更简洁的向上边界对齐算法 内存管理中常用 请记住
#define GRS_UPPER(A,B) ((UINT)(((A)+((B)-1))&~(B - 1)))
//----------------------------------------------------------------------------------

// ---------------------------------- - 【抛出异常相关的类】----------------------------------------
// 这部分C++基础哦，不懂的话，自己找点资料看明白
//-----------------------------------------------------------------------------------------------

class CGRSCOMException
{
public:
    CGRSCOMException(HRESULT hr) : m_hrError(hr)
    {

    }

    HRESULT Error()const
    {
        return m_hrError;
    }

private:
    const HRESULT m_hrError;
};


//-----------------------------------【纹理相关的黑盒】----------------------------------------
// 可以暂时不看，注意UploadTexture函数内部的路径，是否和你的文件路径一致
//-----------------------------------------------------------------------------------------------
struct WICTranslate
{
    GUID wic;
    DXGI_FORMAT format;
};

static WICTranslate g_WICFormats[] = {//WIC格式与DXGI像素格式的对应表，该表中的格式为被支持的格式
    { GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },

    { GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
    { GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },

    { GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
    { GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM }, // DXGI 1.1
    { GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM }, // DXGI 1.1

    { GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM }, // DXGI 1.1
    { GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },

    { GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
    { GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },

    { GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
    { GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
    { GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
    { GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },

    { GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM },
};

// WIC 像素格式转换表.
struct WICConvert
{
    GUID source;
    GUID target;
};

static WICConvert g_WICConvert[] = {
    // 目标格式一定是最接近的被支持的格式
    { GUID_WICPixelFormatBlackWhite,            GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

    { GUID_WICPixelFormat1bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat2bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat4bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat8bppIndexed,           GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

    { GUID_WICPixelFormat2bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM
    { GUID_WICPixelFormat4bppGray,              GUID_WICPixelFormat8bppGray }, // DXGI_FORMAT_R8_UNORM

    { GUID_WICPixelFormat16bppGrayFixedPoint,   GUID_WICPixelFormat16bppGrayHalf }, // DXGI_FORMAT_R16_FLOAT
    { GUID_WICPixelFormat32bppGrayFixedPoint,   GUID_WICPixelFormat32bppGrayFloat }, // DXGI_FORMAT_R32_FLOAT

    { GUID_WICPixelFormat16bppBGR555,           GUID_WICPixelFormat16bppBGRA5551 }, // DXGI_FORMAT_B5G5R5A1_UNORM

    { GUID_WICPixelFormat32bppBGR101010,        GUID_WICPixelFormat32bppRGBA1010102 }, // DXGI_FORMAT_R10G10B10A2_UNORM

    { GUID_WICPixelFormat24bppBGR,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat24bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat32bppPBGRA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat32bppPRGBA,            GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM

    { GUID_WICPixelFormat48bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat48bppBGR,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppBGRA,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppPRGBA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppPBGRA,            GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

    { GUID_WICPixelFormat48bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat48bppBGRFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat64bppRGBAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat64bppBGRAFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat64bppRGBFixedPoint,    GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat48bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
    { GUID_WICPixelFormat64bppRGBHalf,          GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT

    { GUID_WICPixelFormat128bppPRGBAFloat,      GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
    { GUID_WICPixelFormat128bppRGBFloat,        GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
    { GUID_WICPixelFormat128bppRGBAFixedPoint,  GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
    { GUID_WICPixelFormat128bppRGBFixedPoint,   GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
    { GUID_WICPixelFormat32bppRGBE,             GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT

    { GUID_WICPixelFormat32bppCMYK,             GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat64bppCMYK,             GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat40bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat80bppCMYKAlpha,        GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM

    { GUID_WICPixelFormat32bppRGB,              GUID_WICPixelFormat32bppRGBA }, // DXGI_FORMAT_R8G8B8A8_UNORM
    { GUID_WICPixelFormat64bppRGB,              GUID_WICPixelFormat64bppRGBA }, // DXGI_FORMAT_R16G16B16A16_UNORM
    { GUID_WICPixelFormat64bppPRGBAHalf,        GUID_WICPixelFormat64bppRGBAHalf }, // DXGI_FORMAT_R16G16B16A16_FLOAT
};


bool GetTargetPixelFormat(const GUID* pSourceFormat, GUID* pTargetFormat)
{//查表确定兼容的最接近格式是哪个
    *pTargetFormat = *pSourceFormat;
    for (size_t i = 0; i < _countof(g_WICConvert); ++i)
    {
        if (InlineIsEqualGUID(g_WICConvert[i].source, *pSourceFormat))
        {
            *pTargetFormat = g_WICConvert[i].target;
            return true;
        }
    }
    return false;
}

DXGI_FORMAT GetDXGIFormatFromPixelFormat(const GUID* pPixelFormat)
{//查表确定最终对应的DXGI格式是哪一个
    for (size_t i = 0; i < _countof(g_WICFormats); ++i)
    {
        if (InlineIsEqualGUID(g_WICFormats[i].wic, *pPixelFormat))
        {
            return g_WICFormats[i].format;
        }
    }
    return DXGI_FORMAT_UNKNOWN;
}


UINT UploadTexture(ComPtr<IWICBitmapSource>& pIBMP, ComPtr<IWICImagingFactory> pIWICFactory,
    ComPtr<IWICBitmapDecoder> pIWICDecoder, ComPtr<IWICBitmapFrameDecode> pIWICFrame,
    DXGI_FORMAT& stTextureFormat, UINT& nTextureW, UINT& nTextureH, UINT& nBPP, TCHAR* pszAppPath)
{
    //使用纯COM方式创建WIC类厂对象，也是调用WIC第一步要做的事情
    GRS_THROW_IF_FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIWICFactory)));
    TCHAR pszTextureFileName[MAX_PATH] = {};
    //使用WIC类厂对象接口加载纹理图片，并得到一个WIC解码器对象接口，图片信息就在这个接口代表的对象中了
    StringCchPrintf(pszTextureFileName, MAX_PATH, _T("%sTexture\\cat.jpg"), pszAppPath);
    GRS_THROW_IF_FAILED(pIWICFactory->CreateDecoderFromFilename(
        pszTextureFileName,              // 文件名
        NULL,                            // 不指定解码器，使用默认
        GENERIC_READ,                    // 访问权限
        WICDecodeMetadataCacheOnDemand,  // 若需要就缓冲数据 
        &pIWICDecoder                    // 解码器对象
    ));

    // 获取第一帧图片(因为GIF等格式文件可能会有多帧图片，其他的格式一般只有一帧图片)
    // 实际解析出来的往往是位图格式数据
    GRS_THROW_IF_FAILED(pIWICDecoder->GetFrame(0, &pIWICFrame));

    WICPixelFormatGUID wpf = {};
    //获取WIC图片格式
    GRS_THROW_IF_FAILED(pIWICFrame->GetPixelFormat(&wpf));
    GUID tgFormat = {};

    //通过第一道转换之后获取DXGI的等价格式
    if (GetTargetPixelFormat(&wpf, &tgFormat))
    {
        stTextureFormat = GetDXGIFormatFromPixelFormat(&tgFormat);
    }

    if (DXGI_FORMAT_UNKNOWN == stTextureFormat)
    {// 不支持的图片格式 目前退出了事 
     // 一般 在实际的引擎当中都会提供纹理格式转换工具，
     // 图片都需要提前转换好，所以不会出现不支持的现象
        throw CGRSCOMException(S_FALSE);
    }

    // 定义一个位图格式的图片数据对象接口
    //ComPtr<IWICBitmapSource>pIBMP;
    //移到外面作为参数！！！！！！！！！！！！！！


    if (!InlineIsEqualGUID(wpf, tgFormat))
    {// 这个判断很重要，如果原WIC格式不是直接能转换为DXGI格式的图片时
     // 我们需要做的就是转换图片格式为能够直接对应DXGI格式的形式
        //创建图片格式转换器
        ComPtr<IWICFormatConverter> pIConverter;
        GRS_THROW_IF_FAILED(pIWICFactory->CreateFormatConverter(&pIConverter));

        //初始化一个图片转换器，实际也就是将图片数据进行了格式转换
        GRS_THROW_IF_FAILED(pIConverter->Initialize(
            pIWICFrame.Get(),                // 输入原图片数据
            tgFormat,                        // 指定待转换的目标格式
            WICBitmapDitherTypeNone,         // 指定位图是否有调色板，现代都是真彩位图，不用调色板，所以为None
            NULL,                            // 指定调色板指针
            0.f,                             // 指定Alpha阀值
            WICBitmapPaletteTypeCustom       // 调色板类型，实际没有使用，所以指定为Custom
        ));
        // 调用QueryInterface方法获得对象的位图数据源接口
        GRS_THROW_IF_FAILED(pIConverter.As(&pIBMP));
    }
    else
    {
        //图片数据格式不需要转换，直接获取其位图数据源接口
        GRS_THROW_IF_FAILED(pIWICFrame.As(&pIBMP));
    }
    //获得图片大小（单位：像素）
    GRS_THROW_IF_FAILED(pIBMP->GetSize(&nTextureW, &nTextureH));

    //获取图片像素的位大小的BPP（Bits Per Pixel）信息，用以计算图片行数据的真实大小（单位：字节）
    ComPtr<IWICComponentInfo> pIWICmntinfo;
    GRS_THROW_IF_FAILED(pIWICFactory->CreateComponentInfo(tgFormat, pIWICmntinfo.GetAddressOf()));

    WICComponentType type;
    GRS_THROW_IF_FAILED(pIWICmntinfo->GetComponentType(&type));

    if (type != WICPixelFormat)
    {
        throw CGRSCOMException(S_FALSE);
    }

    ComPtr<IWICPixelFormatInfo> pIWICPixelinfo;
    GRS_THROW_IF_FAILED(pIWICmntinfo.As(&pIWICPixelinfo));

    // 到这里终于可以得到BPP了，这也是我看的比较吐血的地方，为了BPP居然饶了这么多环节
    GRS_THROW_IF_FAILED(pIWICPixelinfo->GetBitsPerPixel(&nBPP));

    // 计算图片实际的行大小（单位：字节），这里使用了一个上取整除法即（A+B-1）/B ，
    // 这曾经被传说是微软的面试题,希望你已经对它了如指掌
    UINT nPicRowPitch = (uint64_t(nTextureW) * uint64_t(nBPP) + 7u) / 8u;
    return nPicRowPitch;
}



D3D12_PLACED_SUBRESOURCE_FOOTPRINT CopyToUploadHeap(ComPtr<ID3D12Resource> pITexture, ComPtr<ID3D12Resource> pITextureUpload,
    ComPtr<IWICBitmapSource> pIBMP, ComPtr<ID3D12Device4>   pID3D12Device4, UINT64 n64UploadBufferSize, UINT nTextureH,
    UINT nPicRowPitch)
{
    //按照资源缓冲大小来分配实际图片数据存储的内存大小
    void* pbPicData = ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, n64UploadBufferSize);
    if (nullptr == pbPicData)
    {
        throw CGRSCOMException(HRESULT_FROM_WIN32(GetLastError()));
    }

    //从图片中读取出数据
    GRS_THROW_IF_FAILED(pIBMP->CopyPixels(nullptr
        , nPicRowPitch
        , static_cast<UINT>(nPicRowPitch * nTextureH)   //注意这里才是图片数据真实的大小，这个值通常小于缓冲的大小
        , reinterpret_cast<BYTE*>(pbPicData)));

    //{//下面这段代码来自DX12的示例，直接通过填充缓冲绘制了一个黑白方格的纹理
    // //还原这段代码，然后注释上面的CopyPixels调用可以看到黑白方格纹理的效果
    //  const UINT rowPitch = nPicRowPitch; //nTextureW * 4; //static_cast<UINT>(n64UploadBufferSize / nTextureH);
    //  const UINT cellPitch = rowPitch >> 3;       // The width of a cell in the checkboard texture.
    //  const UINT cellHeight = nTextureW >> 3; // The height of a cell in the checkerboard texture.
    //  const UINT textureSize = static_cast<UINT>(n64UploadBufferSize);
    //  UINT nTexturePixelSize = static_cast<UINT>(n64UploadBufferSize / nTextureH / nTextureW);

    //  UINT8* pData = reinterpret_cast<UINT8*>(pbPicData);

    //  for (UINT n = 0; n < textureSize; n += nTexturePixelSize)
    //  {
    //      UINT x = n % rowPitch;
    //      UINT y = n / rowPitch;
    //      UINT i = x / cellPitch;
    //      UINT j = y / cellHeight;

    //      if (i % 2 == j % 2)
    //      {
    //          pData[n] = 0x00;        // R
    //          pData[n + 1] = 0x00;    // G
    //          pData[n + 2] = 0x00;    // B
    //          pData[n + 3] = 0xff;    // A
    //      }
    //      else
    //      {
    //          pData[n] = 0xff;        // R
    //          pData[n + 1] = 0xff;    // G
    //          pData[n + 2] = 0xff;    // B
    //          pData[n + 3] = 0xff;    // A
    //      }
    //  }
    //}

    //获取向上传堆拷贝纹理数据的一些纹理转换尺寸信息
    //对于复杂的DDS纹理这是非常必要的过程
    UINT64 n64RequiredSize = 0u;
    UINT   nNumSubresources = 1u;  //我们只有一副图片，即子资源个数为1
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT stTxtLayouts = {};
    UINT64 n64TextureRowSizes = 0u;
    UINT   nTextureRowNum = 0u;

    D3D12_RESOURCE_DESC stDestDesc = pITexture->GetDesc();

    pID3D12Device4->GetCopyableFootprints(&stDestDesc
        , 0
        , nNumSubresources
        , 0
        , &stTxtLayouts
        , &nTextureRowNum
        , &n64TextureRowSizes
        , &n64RequiredSize);

    //因为上传堆实际就是CPU传递数据到GPU的中介
    //所以我们可以使用熟悉的Map方法将它先映射到CPU内存地址中
    //然后我们按行将数据复制到上传堆中
    //需要注意的是之所以按行拷贝是因为GPU资源的行大小
    //与实际图片的行大小是有差异的,二者的内存边界对齐要求是不一样的
    BYTE* pData = nullptr;
    GRS_THROW_IF_FAILED(pITextureUpload->Map(0, NULL, reinterpret_cast<void**>(&pData)));

    BYTE* pDestSlice = reinterpret_cast<BYTE*>(pData) + stTxtLayouts.Offset;
    const BYTE* pSrcSlice = reinterpret_cast<const BYTE*>(pbPicData);
    for (UINT y = 0; y < nTextureRowNum; ++y)
    {
        memcpy(pDestSlice + static_cast<SIZE_T>(stTxtLayouts.Footprint.RowPitch) * y
            , pSrcSlice + static_cast<SIZE_T>(nPicRowPitch) * y
            , nPicRowPitch);
    }
    //取消映射 对于易变的数据如每帧的变换矩阵等数据，可以撒懒不用Unmap了，
    //让它常驻内存,以提高整体性能，因为每次Map和Unmap是很耗时的操作
    //因为现在起码都是64位系统和应用了，地址空间是足够的，被长期占用不会影响什么
    pITextureUpload->Unmap(0, NULL);

    //释放图片数据，做一个干净的程序员
    ::HeapFree(::GetProcessHeap(), 0, pbPicData);
    return stTxtLayouts;
}

//-----------------------------------【全局变量部分】-----------------------------------

//----------------------------------------------------------------------------------

//-----------------------------------【函数声明部分】-----------------------------------


//----------------------------------------------------------------------------------

//-----------------------------------【程序初始化】-----------------------------------
// 初始化窗口和数据等
//--------------------------------------------------------------------------------------



//-----------------------------------【更新绘制的数据】-----------------------------------
// 处理外来信息
//--------------------------------------------------------------------------------------

//-----------------------------------【进行绘制】-----------------------------------
// 处理外来信息
//--------------------------------------------------------------------------------------

//-----------------------------------【结构体定义】---------------------------------------------
// 我们的顶点信息对应的结构体以及MVP矩阵的结构
//-----------------------------------------------------------------------------------------------
struct ST_GRS_VERTEX
{//这次我们额外加入了每个顶点的法线，但Shader中还暂时没有用
    XMFLOAT4 m_v4Position;      //Position
    XMFLOAT2 m_vTex;        //Texcoord
};

struct ST_GRS_FRAME_MVP_BUFFER
{
    //这个结构体要和常量缓冲区的结构相对应，因为我们准备常量缓冲区的数据是放在这个结构体中，
    //而传递到上传堆，GPU拿走是放在它HLSL中的常量缓冲区中，所以这俩要对应。
    XMFLOAT4X4 m_MVP;           //经典的Model-view-projection(MVP)矩阵.
};


//-----------------------------------【窗口处理函数】--------------------------------------------
//里面主要是通过我们输入，修改观察点的位置和物体旋转的速度
//-----------------------------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, message, wParam, lParam);
}

//-----------------------------------【WinMain函数】-----------------------------------
//Win32程序的入口
//------------------------------------------------------------------------------------
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    ::CoInitialize(nullptr);

    const UINT nFrameBackBufferCount = 3u;

    int  iWidth = 1024;
    int  iHeight = 768;
    UINT nFrameIndex = 0;

    //DXGI_FORMAT_B8G8R8A8_UINT 表示0-255 UNORM 表示归一化 0-1
    DXGI_FORMAT emRenderTarget = DXGI_FORMAT_B8G8R8A8_UNORM;
    const float faClearColor[] = { 0.0f,0.2f,0.4f,1.0f };



    TCHAR psAppPath[MAX_PATH] = {};

    HWND hWnd = nullptr;

    UINT                                nDXGIFactoryFlags = 0U;
    UINT                                nRTVDescriptorSize = 0U;
    UINT                                nSRVDescriptorSize = 0U;
    UINT64                              n64UploadBufferSize = 0;
    UINT64                              n64FenceValue = 0ui64;

    HANDLE                              hEventFence = nullptr;

    ComPtr<IDXGIFactory5> pIDXGIFactory5;
    ComPtr<IDXGIAdapter1> pIDXGIAdapter1;
    ComPtr<ID3D12Device4> pID3D12Device4;
    ComPtr<ID3D12CommandQueue>          pICMDQueue;
    ComPtr<ID3D12CommandAllocator> pICMDAlloc;
    ComPtr<ID3D12GraphicsCommandList>   pICMDList;
    ComPtr<IDXGISwapChain1>             pISwapChain1;
    ComPtr<IDXGISwapChain3>             pISwapChain3;
    ComPtr<ID3D12DescriptorHeap>        pIRTVHeap;
    ComPtr<ID3D12DescriptorHeap>        pISRVHeap;
    ComPtr<ID3D12Resource>              pIARenderTargets[nFrameBackBufferCount];


    ComPtr<ID3D12RootSignature>         pIRootSignature;
    ComPtr<ID3D12PipelineState>         pIPipelineState;


    ComPtr<ID3D12Heap>                  pITextureHeap;
    ComPtr<ID3D12Heap>                  pIUploadHeap;
    ComPtr<ID3D12Resource>              pITexture;
    ComPtr<ID3D12Resource>              pITextureUpload;
    ComPtr<ID3D12Resource>              pICBVUpload;
    UINT                                nTextureW = 0u;
    UINT                                nTextureH = 0u;
    UINT                                nBPP = 0u;
    UINT                                nPicRowPitch = 0;//new
    DXGI_FORMAT                         stTextureFormat = DXGI_FORMAT_UNKNOWN;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT  stTxtLayouts = {};

    ComPtr<ID3D12Fence>                 pIFence;

    //WIC库相关
    ComPtr<IWICImagingFactory>          pIWICFactory;
    ComPtr<IWICBitmapDecoder>           pIWICDecoder;
    ComPtr<IWICBitmapFrameDecode>       pIWICFrame;
    ComPtr<IWICBitmapSource>            pIBMP;

    D3D12_RESOURCE_DESC                 stTextureDesc = {};

    //顶点数据相关
    float                               fAspectRatio = 3.0f;
    float                               fBoxSize = 3.0f;
    float                               fTCMax = 1.0f;

    ComPtr<ID3D12Resource>              pIVertexBuffer;
    ComPtr<ID3D12Resource>              pIIndexBuffer;

    D3D12_VERTEX_BUFFER_VIEW            stVertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW             stIndexBufferView = {};

    //1.获取当前正在运行的可执行文件的路径

    try
    {
        //一个参数为nullptr，表示获取当前进程的模块文件名
        //::是C++中的作用域解析操作符。在这里，它表示调用全局命名空间下的GetModuleFileName函数
        if (::GetModuleFileName(nullptr, psAppPath, MAX_PATH) == 0)
        {
            GRS_THROW_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
        }

        printf("The Path is: %ls\n", psAppPath);

        WCHAR* lastSlash = _tcsrchr(psAppPath, _T('\\'));
        if (lastSlash)
        {
            *(lastSlash) = _T('\0');
        }

        printf("The Path is: %ls\n", psAppPath);

        lastSlash = _tcsrchr(psAppPath, _T('\\'));
        if (lastSlash)
        {
            *(lastSlash) = _T('\0');
        }

        printf("The Path is: %ls\n", psAppPath);

        lastSlash = _tcsrchr(psAppPath, _T('\\'));
        if (lastSlash)
        {//删除Debug 或 Release路径 +1 表示不删除最后一个反斜杠
            *(lastSlash + 1) = _T('\0');
        }

        printf("The Path is: %ls\n", psAppPath);
    }
    catch (CGRSCOMException& e)
    {
        e;
    }

    //2 创建窗口
    {
        WNDCLASSEX  wcex = {};
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_GLOBALCLASS;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);//防止无聊的背景重绘
        wcex.lpszClassName = GRS_WND_CLASS_NAME;
        RegisterClassEx(&wcex);

        DWORD dwWndStyle = WS_OVERLAPPED | WS_SYSMENU;
        RECT rtWnd = { 0,0,iWidth, iHeight };
        AdjustWindowRect(&rtWnd, dwWndStyle, FALSE);

        INT posX = (GetSystemMetrics(SM_CXSCREEN) - rtWnd.right - rtWnd.left) / 2;
        INT posY = (GetSystemMetrics(SM_CYSCREEN) - rtWnd.bottom - rtWnd.top) / 2;


        hWnd = CreateWindowW(GRS_WND_CLASS_NAME
            , GRS_WND_TITLE
            , dwWndStyle //这里会关闭全屏效果。
            , posX
            , posY
            , rtWnd.right - rtWnd.left
            , rtWnd.bottom - rtWnd.top
            , nullptr
            , nullptr
            , hInstance
            , nullptr);

        if (!hWnd)
        {
            return FALSE;
        }

        //3 使用WIC创建并加载一个2D纹理
        {
            nPicRowPitch = UploadTexture(pIBMP, pIWICFactory, pIWICDecoder, pIWICFrame, stTextureFormat, nTextureW, nTextureH, nBPP, psAppPath);
        }

        //4 打开显示子系统的调试支持
#if  defined(_DEBUG)
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
                nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
#endif //  defined(_DEBUG)

        //5 创建DXGI Factory对象
        {
            //IID_PPV_ARGS 返回参数为uuid及 指向Com接口的指针给函数
            CreateDXGIFactory2(nDXGIFactoryFlags, IID_PPV_ARGS(pIDXGIFactory5.GetAddressOf()));
        }

        //6 枚举适配器 并选择合适的适配器来创建3D设备对象
        {
            DXGI_ADAPTER_DESC1 stAdapterDesc = {}; //适配器描述的结构体
            //利用DXGIFactory来循环遍历你的所有显卡。
            for (UINT nAdapterIndex = 0; DXGI_ERROR_NOT_FOUND != pIDXGIFactory5->EnumAdapters1(nAdapterIndex, &pIDXGIAdapter1); ++nAdapterIndex)
            {
                pIDXGIAdapter1->GetDesc1(&stAdapterDesc);
                if (stAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    //跳过软件虚拟设配器
                    continue;
                }
                //检查适配器对D3D支持的兼容级别，这里直接要求支持12.1的能力，注意返回接口的那个参数被置为了nullptr，这样
                //就不会实际创建一个设备了，也不用我们啰嗦的再调用release来释放接口。这也是一个重要的技巧，请记住！

                if (SUCCEEDED(D3D12CreateDevice(pIDXGIAdapter1.Get(), D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }

            }
            // 创建D3D12.1的设备，这里是真正创建出来设备。
            GRS_THROW_IF_FAILED(D3D12CreateDevice(pIDXGIAdapter1.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&pID3D12Device4)));

            TCHAR pszWndTitle[MAX_PATH] = {};
            GRS_THROW_IF_FAILED(pIDXGIAdapter1->GetDesc1(&stAdapterDesc));
            ::GetWindowText(hWnd, pszWndTitle, MAX_PATH);
            StringCchPrintf(pszWndTitle
                , MAX_PATH
                , _T("%s (GPU:%s)")
                , pszWndTitle
                , stAdapterDesc.Description);
            ::SetWindowText(hWnd, pszWndTitle);

            nRTVDescriptorSize = pID3D12Device4->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); //RTV(render target view)：渲染目标视图
            // CBV(constant buffer view）：常量缓冲区视图  SRV(shader resource view)：着色器资源视图
            nSRVDescriptorSize = pID3D12Device4->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        }

        //7 创建命令队列
        {
            D3D12_COMMAND_QUEUE_DESC stQueneDesc = {};
            stQueneDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            GRS_THROW_IF_FAILED(pID3D12Device4->CreateCommandQueue(&stQueneDesc, IID_PPV_ARGS(&pICMDQueue)));
        }

        //8 创建命令列表分配器
        {
            GRS_THROW_IF_FAILED(pID3D12Device4->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pICMDAlloc)));
            // 创建图形命令列表
            GRS_THROW_IF_FAILED(pID3D12Device4->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pICMDAlloc.Get(), pIPipelineState.Get(), IID_PPV_ARGS(&pICMDList)));
        }

        //9 创建交换链
        {
            DXGI_SWAP_CHAIN_DESC1 stSwapChainDesc = {};
            stSwapChainDesc.BufferCount = nFrameBackBufferCount;
            stSwapChainDesc.Width = iWidth;
            stSwapChainDesc.Height = iHeight;
            stSwapChainDesc.Format = emRenderTarget;
            stSwapChainDesc.BufferUsage = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            stSwapChainDesc.SampleDesc.Count = 1;

            GRS_THROW_IF_FAILED(
                pIDXGIFactory5->CreateSwapChainForHwnd(
                    pICMDQueue.Get(),
                    hWnd,
                    &stSwapChainDesc,
                    nullptr,
                    nullptr,
                    &pISwapChain1
                )
            );
            //得到当前后缓冲区的序号，也就是下一个将要呈送显示的缓冲区的序号
            //注意此处使用了高版本的SwapChain接口的函数
            GRS_THROW_IF_FAILED(pISwapChain1.As(&pISwapChain3));
            nFrameIndex = pISwapChain3->GetCurrentBackBufferIndex();

            //创建RTV(渲染目标视图)描述符堆(这里堆的含义应当理解为数组或者固定大小元素的固定大小显存池)
            D3D12_DESCRIPTOR_HEAP_DESC stRTVHeapDesc = {};
            stRTVHeapDesc.NumDescriptors = nFrameBackBufferCount;
            stRTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            stRTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

            GRS_THROW_IF_FAILED(pID3D12Device4->CreateDescriptorHeap(&stRTVHeapDesc, IID_PPV_ARGS(&pIRTVHeap)));

            //创建RTV的描述符
            D3D12_CPU_DESCRIPTOR_HANDLE stRTVHandle = pIRTVHeap->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < nFrameBackBufferCount; i++)
            {
                GRS_THROW_IF_FAILED(pISwapChain3->GetBuffer(i, IID_PPV_ARGS(&pIARenderTargets[i])));
                pID3D12Device4->CreateRenderTargetView(pIARenderTargets[i].Get(), nullptr, stRTVHandle);
                stRTVHandle.ptr += nRTVDescriptorSize;
            }

            // 关闭ALT+ENTER键切换全屏的功能，因为我们没有实现OnSize处理（创建窗口的时候关了），所以先关闭
            GRS_THROW_IF_FAILED(pIDXGIFactory5->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

        }

        //对于这里堆的解释 
        // 首先这里的堆指的都是为了GPU访问资源而申请的内存 UploadHeap(为CPU GPU共享内存 CPU写 GPU读)
        //DefaultHeap(从UploadHeadp中拷贝 只对GPU读写) ReadbackHeap(GPU写 CPU读)

        //关于堆的视图(描述符)
        //首先每个资源都需要一个资源视图与其绑定 资源主要分为Buffer,Texture 
        //资源视图有 IndexBufferView/VertexBufferView/ShaderResourceView(SRV)/ConstantBufferViw(CBV)/Sampler/RenderTargetView/DepthStencilView/StreamTargetView

        //描述符堆
        //就是装描述符的数组 分为CBV/SRV/UAV  Sampler  RTV  DSV

        //10 创建SRV和CRV堆  着色器资源视图（Shader Resource View, SRV）和常量缓冲视图
        {
            D3D12_DESCRIPTOR_HEAP_DESC stSRVHeapDesc = {};
            stSRVHeapDesc.NumDescriptors = 2; //描述符数量 指SRV_CBV 暂时不管UAV
            stSRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            stSRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; //可被着色器访问 这个VISIBLE标志会被绑定到Command List上
            //创建描述符堆
            GRS_THROW_IF_FAILED(pID3D12Device4->CreateDescriptorHeap(&stSRVHeapDesc, IID_PPV_ARGS(&pISRVHeap)));
        }

        //11 创建根签名
        {
            D3D12_FEATURE_DATA_ROOT_SIGNATURE stFeatureData = {};
            // 检测是否支持V1.1版本的根签名
            stFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
            if (FAILED(pID3D12Device4->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &stFeatureData, sizeof(stFeatureData))))
            {
                stFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
            }
            // 在GPU上执行SetGraphicsRootDescriptorTable后，我们不修改命令列表中的SRV，因此我们可以使用默认Rang行为:
            // D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE
            D3D12_DESCRIPTOR_RANGE1 stDSPRanges1[2] = {};
            //SRV
            stDSPRanges1[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            stDSPRanges1[0].NumDescriptors = 1;
            stDSPRanges1[0].BaseShaderRegister = 0;
            stDSPRanges1[0].RegisterSpace = 0;
            stDSPRanges1[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;
            stDSPRanges1[0].OffsetInDescriptorsFromTableStart = 0;
            //CBV
            stDSPRanges1[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            stDSPRanges1[1].NumDescriptors = 1;
            stDSPRanges1[1].BaseShaderRegister = 0;
            stDSPRanges1[1].RegisterSpace = 0;
            stDSPRanges1[1].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
            stDSPRanges1[1].OffsetInDescriptorsFromTableStart = 0;

            D3D12_ROOT_PARAMETER1 stRootParameters1[2] = {};
            //SRV
            stRootParameters1[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            stRootParameters1[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
            stRootParameters1[0].DescriptorTable.NumDescriptorRanges = 1;
            stRootParameters1[0].DescriptorTable.pDescriptorRanges = &stDSPRanges1[0];
            //CBV
            stRootParameters1[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            stRootParameters1[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;        //CBV是所有Shader可见
            stRootParameters1[1].DescriptorTable.NumDescriptorRanges = 1;
            stRootParameters1[1].DescriptorTable.pDescriptorRanges = &stDSPRanges1[1];


            //因为参数存在纹理 对于创建了采样器Sampler
            D3D12_STATIC_SAMPLER_DESC stSamplerDesc[1] = {};
            stSamplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
            stSamplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            stSamplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            stSamplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            stSamplerDesc[0].MipLODBias = 0;
            stSamplerDesc[0].MaxAnisotropy = 0;
            stSamplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            stSamplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            stSamplerDesc[0].MinLOD = 0.0f;
            stSamplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;
            stSamplerDesc[0].ShaderRegister = 0;
            stSamplerDesc[0].RegisterSpace = 0;
            stSamplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

            D3D12_VERSIONED_ROOT_SIGNATURE_DESC stRootSignatureDesc = {};
            stRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
            stRootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
            stRootSignatureDesc.Desc_1_1.NumParameters = _countof(stRootParameters1);
            stRootSignatureDesc.Desc_1_1.pParameters = stRootParameters1;
            stRootSignatureDesc.Desc_1_1.NumStaticSamplers = _countof(stSamplerDesc);
            stRootSignatureDesc.Desc_1_1.pStaticSamplers = stSamplerDesc;

            ComPtr<ID3DBlob> pISignatureBlob;
            ComPtr<ID3DBlob> pIErrorBlob;
            GRS_THROW_IF_FAILED(D3D12SerializeVersionedRootSignature(&stRootSignatureDesc
                , &pISignatureBlob
                , &pIErrorBlob));

            GRS_THROW_IF_FAILED(pID3D12Device4->CreateRootSignature(0
                , pISignatureBlob->GetBufferPointer()
                , pISignatureBlob->GetBufferSize()
                , IID_PPV_ARGS(&pIRootSignature)));
        }

        //12 编译Shader创建渲染管线状态对象
        {
            //ID3DBlob是一个接口，它代表了一个内存中的二进制数据块（Blob）。Blob是一个连续的内存区域，可以存储任意类型的数据，通常用于存储编译后的着色器代码、顶点数据、索引数据
            ComPtr<ID3DBlob> pIBlobVertexShader;
            ComPtr<ID3DBlob> pIBlobPixelShader;

#if defined(_DEBUG)
            // Enable better shader debugging with the graphics debugging tools. 
            UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
            UINT compileFlags = 0;
#endif
            //编译行为矩阵形式
            compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
            //编译shader.hlsl 文件
            TCHAR pszShaderFileName[MAX_PATH] = {};
            StringCchPrintf(pszShaderFileName, MAX_PATH, _T("%sShader\\shaders.hlsl"), psAppPath);
            GRS_THROW_IF_FAILED(D3DCompileFromFile(pszShaderFileName, nullptr, nullptr
                , "VSMain", "vs_5_0", compileFlags, 0, &pIBlobVertexShader, nullptr));
            GRS_THROW_IF_FAILED(D3DCompileFromFile(pszShaderFileName, nullptr, nullptr
                , "PSMain", "ps_5_0", compileFlags, 0, &pIBlobPixelShader, nullptr));
            //定义Input Layout
            D3D12_INPUT_ELEMENT_DESC stInputElementDescs[] =
            {
                 { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                 { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            };

            //创建PSO Pipeline State Object
            D3D12_GRAPHICS_PIPELINE_STATE_DESC stPSODesc = {};
            stPSODesc.InputLayout = { stInputElementDescs, _countof(stInputElementDescs) };
            stPSODesc.pRootSignature = pIRootSignature.Get();

            stPSODesc.VS.pShaderBytecode = pIBlobVertexShader->GetBufferPointer();
            stPSODesc.VS.BytecodeLength = pIBlobVertexShader->GetBufferSize();

            stPSODesc.PS.pShaderBytecode = pIBlobPixelShader->GetBufferPointer();
            stPSODesc.PS.BytecodeLength = pIBlobPixelShader->GetBufferSize();

            stPSODesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
            stPSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

            stPSODesc.BlendState.AlphaToCoverageEnable = FALSE;
            stPSODesc.BlendState.IndependentBlendEnable = FALSE;
            stPSODesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

            stPSODesc.DepthStencilState.DepthEnable = FALSE;
            stPSODesc.DepthStencilState.StencilEnable = FALSE;

            stPSODesc.SampleMask = UINT_MAX;
            stPSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            stPSODesc.NumRenderTargets = 1;
            stPSODesc.RTVFormats[0] = emRenderTarget;
            stPSODesc.SampleDesc.Count = 1;

            GRS_THROW_IF_FAILED(pID3D12Device4->CreateGraphicsPipelineState(&stPSODesc
                , IID_PPV_ARGS(&pIPipelineState)));
        }


        //13 创建纹理默认堆
        {
            D3D12_HEAP_DESC stTextureHeapDesc = {};
            //为堆指定纹理图片至少2倍大小的空间，这里没有详细去计算了，只是指定了一个足够大的空间，够放纹理就行
            //实际应用中也是要综合考虑分配堆的大小，以便可以重用堆
            stTextureHeapDesc.SizeInBytes = GRS_UPPER(2 * nPicRowPitch * nTextureH, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
            //指定堆的对齐方式，这里使用了默认的64K边界对齐，因为我们暂时不需要MSAA支持
            stTextureHeapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
            stTextureHeapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;        //默认堆类型
            stTextureHeapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            stTextureHeapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            //拒绝渲染目标纹理、拒绝深度蜡板纹理，实际就只是用来摆放普通纹理
            stTextureHeapDesc.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_BUFFERS;

            GRS_THROW_IF_FAILED(pID3D12Device4->CreateHeap(&stTextureHeapDesc, IID_PPV_ARGS(&pITextureHeap)));
        }

        //14 创建2D纹理
        {
            // 根据图片信息，填充2D纹理资源的信息结构体
            stTextureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            stTextureDesc.MipLevels = 1;
            stTextureDesc.Format = stTextureFormat;
            stTextureDesc.Width = nTextureW;
            stTextureDesc.Height = nTextureH;
            stTextureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
            stTextureDesc.DepthOrArraySize = 1;
            stTextureDesc.SampleDesc.Count = 1;
            stTextureDesc.SampleDesc.Quality = 0;

            //-----------------------------------------------------------------------------------------------------------
           //使用“定位方式”来创建纹理，注意下面这个调用内部实际已经没有存储分配和释放的实际操作了，所以性能很高
           //同时可以在这个堆上反复调用CreatePlacedResource来创建不同的纹理，当然前提是它们不在被使用的时候，才考虑
           //重用堆
            GRS_THROW_IF_FAILED(pID3D12Device4->CreatePlacedResource(
                pITextureHeap.Get()
                , 0
                , &stTextureDesc                //可以使用CD3DX12_RESOURCE_DESC::Tex2D来简化结构体的初始化
                , D3D12_RESOURCE_STATE_COPY_DEST
                , nullptr
                , IID_PPV_ARGS(&pITexture)));
            //-----------------------------------------------------------------------------------------------------------
            //获取上传堆资源缓冲的大小，这个尺寸通常大于实际图片的尺寸
            D3D12_RESOURCE_DESC stCopyDstDesc = pITexture->GetDesc();
            n64UploadBufferSize = 0;
            pID3D12Device4->GetCopyableFootprints(&stCopyDstDesc, 0, 1, 0, nullptr, nullptr, nullptr, &n64UploadBufferSize);
        }

        //15、创建上传堆
        {
            D3D12_HEAP_DESC stUploadHeapDesc = {  };
            //尺寸依然是实际纹理数据大小的2倍并64K边界对齐大小
            stUploadHeapDesc.SizeInBytes = GRS_UPPER(2 * n64UploadBufferSize, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
            //注意上传堆肯定是Buffer类型，可以不指定对齐方式，其默认是64k边界对齐
            stUploadHeapDesc.Alignment = 0;
            stUploadHeapDesc.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;      //上传堆类型
            stUploadHeapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            stUploadHeapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            //上传堆就是缓冲，可以摆放任意数据
            stUploadHeapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

            GRS_THROW_IF_FAILED(pID3D12Device4->CreateHeap(&stUploadHeapDesc, IID_PPV_ARGS(&pIUploadHeap)));
        }

        //16、使用“定位方式”创建用于上传纹理数据的缓冲资源
        {
            D3D12_RESOURCE_DESC stUploadResDesc = {};
            stUploadResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            stUploadResDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
            stUploadResDesc.Width = n64UploadBufferSize;
            stUploadResDesc.Height = 1;
            stUploadResDesc.DepthOrArraySize = 1;
            stUploadResDesc.MipLevels = 1;
            stUploadResDesc.Format = DXGI_FORMAT_UNKNOWN;
            stUploadResDesc.SampleDesc.Count = 1;
            stUploadResDesc.SampleDesc.Quality = 0;
            stUploadResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            stUploadResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            GRS_THROW_IF_FAILED(pID3D12Device4->CreatePlacedResource(pIUploadHeap.Get()
                , 0
                , &stUploadResDesc
                , D3D12_RESOURCE_STATE_GENERIC_READ
                , nullptr
                , IID_PPV_ARGS(&pITextureUpload)));
        }

        //17、加载图片数据至上传堆，即完成第一个Copy动作，从memcpy函数可知这是由CPU完成的
        {
            //CPU把纹理数据从内存复制到上传堆中。
            stTxtLayouts = CopyToUploadHeap(pITexture, pITextureUpload, pIBMP, pID3D12Device4, n64UploadBufferSize, nTextureH, nPicRowPitch);
        }

        //18、把纹理从上传堆复制到默认堆
        {
            //向直接命令列表发出从上传堆复制纹理数据到默认堆的命令，执行并同步等待，即完成第二个Copy动作，由GPU上的复制引擎完成
                //注意此时直接命令列表还没有绑定PSO对象，因此它也是不能执行3D图形命令的，但是可以执行复制命令，因为复制引擎不需要什么
                //额外的状态设置之类的参数
            D3D12_TEXTURE_COPY_LOCATION stDstCopyLocation = {};
            stDstCopyLocation.pResource = pITexture.Get();
            stDstCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            stDstCopyLocation.SubresourceIndex = 0;

            D3D12_TEXTURE_COPY_LOCATION stSrcCopyLocation = {};
            stSrcCopyLocation.pResource = pITextureUpload.Get();
            stSrcCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            stSrcCopyLocation.PlacedFootprint = stTxtLayouts;

            pICMDList->CopyTextureRegion(&stDstCopyLocation, 0, 0, 0, &stSrcCopyLocation, nullptr);

            //设置一个资源屏障，同步并确认复制操作完成
            //直接使用结构体然后调用的形式
            D3D12_RESOURCE_BARRIER stResBar = {};
            stResBar.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            stResBar.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            stResBar.Transition.pResource = pITexture.Get();
            stResBar.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            stResBar.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            stResBar.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            pICMDList->ResourceBarrier(1, &stResBar);

            //或者使用D3DX12库中的工具类调用的等价形式，下面的方式更简洁一些
            //pICMDList->ResourceBarrier(1
            //  , &CD3DX12_RESOURCE_BARRIER::Transition(pITexture.Get()
            //  , D3D12_RESOURCE_STATE_COPY_DEST
            //  , D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
            //);

            //---------------------------------------------------------------------------------------------
            // 执行命令列表并等待纹理资源上传完成，这一步是必须的
            GRS_THROW_IF_FAILED(pICMDList->Close());
            ID3D12CommandList* ppCommandLists[] = { pICMDList.Get() };
            pICMDQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

            //---------------------------------------------------------------------------------------------
            // 17、创建一个同步对象——围栏，用于等待渲染完成，因为现在Draw Call是异步的了
            GRS_THROW_IF_FAILED(pID3D12Device4->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pIFence)));
            n64FenceValue = 1;

            //---------------------------------------------------------------------------------------------
            // 18、创建一个Event同步对象，用于等待围栏事件通知
            hEventFence = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (hEventFence == nullptr)
            {
                GRS_THROW_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
            }

            //---------------------------------------------------------------------------------------------
            // 19、等待纹理资源正式复制完成先
            const UINT64 fence = n64FenceValue;
            GRS_THROW_IF_FAILED(pICMDQueue->Signal(pIFence.Get(), fence));
            n64FenceValue++;
            GRS_THROW_IF_FAILED(pIFence->SetEventOnCompletion(fence, hEventFence));

            WaitForSingleObject(hEventFence, INFINITE);
        }

        //19、定义立方体顶点和索引数据


        ST_GRS_VERTEX stTriangleVertices[] = {
      { {-1.0f * fBoxSize,  1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 0.0f * fTCMax}},
      { {1.0f * fBoxSize,  1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 0.0f * fTCMax} },
      { {-1.0f * fBoxSize, -1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 1.0f * fTCMax} },
      { {-1.0f * fBoxSize, -1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 1.0f * fTCMax} },
      { {1.0f * fBoxSize,  1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 0.0f * fTCMax}},
      { {1.0f * fBoxSize, -1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 1.0f * fTCMax} },
      { {1.0f * fBoxSize,  1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 0.0f * fTCMax} },
      { {1.0f * fBoxSize,  1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 0.0f * fTCMax} },
      { {1.0f * fBoxSize, -1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 1.0f * fTCMax} },
      { {1.0f * fBoxSize, -1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 1.0f * fTCMax} },
      { {1.0f * fBoxSize,  1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 0.0f * fTCMax} },
      { {1.0f * fBoxSize, -1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 1.0f * fTCMax} },
      { {1.0f * fBoxSize,  1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 0.0f * fTCMax}},
      { {-1.0f * fBoxSize,  1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 0.0f * fTCMax}},
      { {1.0f * fBoxSize, -1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 1.0f * fTCMax} },
      { {1.0f * fBoxSize, -1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 1.0f * fTCMax}},
      { {-1.0f * fBoxSize,  1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 0.0f * fTCMax}},
      { {-1.0f * fBoxSize, -1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 1.0f * fTCMax}},
      { {-1.0f * fBoxSize,  1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 0.0f * fTCMax}},
      { {-1.0f * fBoxSize,  1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 0.0f * fTCMax}},
      { {-1.0f * fBoxSize, -1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 1.0f * fTCMax}},
      { {-1.0f * fBoxSize, -1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 1.0f * fTCMax}},
      { {-1.0f * fBoxSize,  1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 0.0f * fTCMax}},
      { {-1.0f * fBoxSize, -1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 1.0f * fTCMax}},
      { {-1.0f * fBoxSize,  1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 0.0f * fTCMax} },
      { {1.0f * fBoxSize,  1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 0.0f * fTCMax} },
      { {-1.0f * fBoxSize,  1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 1.0f * fTCMax} },
      { {-1.0f * fBoxSize,  1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 1.0f * fTCMax} },
      { {1.0f * fBoxSize,  1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 0.0f * fTCMax} },
      { {1.0f * fBoxSize,  1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 1.0f * fTCMax}},
      { {-1.0f * fBoxSize, -1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 0.0f * fTCMax}},
      { {1.0f * fBoxSize, -1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 0.0f * fTCMax}},
      { {-1.0f * fBoxSize, -1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 1.0f * fTCMax}},
      { {-1.0f * fBoxSize, -1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {0.0f * fTCMax, 1.0f * fTCMax}},
      { {1.0f * fBoxSize, -1.0f * fBoxSize, -1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 0.0f * fTCMax}},
      { {1.0f * fBoxSize, -1.0f * fBoxSize,  1.0f * fBoxSize, 1.0f}, {1.0f * fTCMax, 1.0f * fTCMax}},
        };
        UINT32 pBoxIndices[] = {
            0,1,2,
            3,4,5,

            6,7,8,
            9,10,11,

            12,13,14,
            15,16,17,

            18,19,20,
            21,22,23,

            24,25,26,
            27,28,29,

            30,31,32,
            33,34,35,
        };


        //20、创建顶点缓冲
        {
            const UINT nVertexBufferSize = sizeof(stTriangleVertices);

            D3D12_HEAP_PROPERTIES stHeapProp = { D3D12_HEAP_TYPE_UPLOAD };

            D3D12_RESOURCE_DESC stResSesc = {};
            stResSesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            stResSesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            stResSesc.Flags = D3D12_RESOURCE_FLAG_NONE;
            stResSesc.Format = DXGI_FORMAT_UNKNOWN;
            stResSesc.Width = nVertexBufferSize;
            stResSesc.Height = 1;
            stResSesc.DepthOrArraySize = 1;
            stResSesc.MipLevels = 1;
            stResSesc.SampleDesc.Count = 1;
            stResSesc.SampleDesc.Quality = 0;

            GRS_THROW_IF_FAILED(pID3D12Device4->CreateCommittedResource(
                &stHeapProp,
                D3D12_HEAP_FLAG_NONE,
                &stResSesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&pIVertexBuffer)));

            UINT8* pVertexDataBegin = nullptr;
            D3D12_RANGE stReadRange = { 0, 0 };     // We do not intend to read from this resource on the CPU.
            GRS_THROW_IF_FAILED(pIVertexBuffer->Map(0, &stReadRange, reinterpret_cast<void**>(&pVertexDataBegin)));
            memcpy(pVertexDataBegin, stTriangleVertices, sizeof(stTriangleVertices));
            pIVertexBuffer->Unmap(0, nullptr);

            stVertexBufferView.BufferLocation = pIVertexBuffer->GetGPUVirtualAddress();
            stVertexBufferView.StrideInBytes = sizeof(ST_GRS_VERTEX);
            stVertexBufferView.SizeInBytes = nVertexBufferSize;
        }


        //21、创建索引缓冲区        
        {

            const UINT nszIndexBuffer = sizeof(pBoxIndices);


            D3D12_HEAP_PROPERTIES stHeapProp = { D3D12_HEAP_TYPE_UPLOAD };

            D3D12_RESOURCE_DESC stIBResDesc = {};
            stIBResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            stIBResDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
            stIBResDesc.Width = nszIndexBuffer;
            stIBResDesc.Height = 1;
            stIBResDesc.DepthOrArraySize = 1;
            stIBResDesc.MipLevels = 1;
            stIBResDesc.Format = DXGI_FORMAT_UNKNOWN;
            stIBResDesc.SampleDesc.Count = 1;
            stIBResDesc.SampleDesc.Quality = 0;
            stIBResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            stIBResDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            GRS_THROW_IF_FAILED(pID3D12Device4->CreateCommittedResource(
                &stHeapProp,
                D3D12_HEAP_FLAG_NONE,
                &stIBResDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&pIIndexBuffer)));

            UINT8* pIndexDataBegin = nullptr;
            D3D12_RANGE stReadRange = { 0, 0 };     // We do not intend to read from this resource on the CPU.
            GRS_THROW_IF_FAILED(pIIndexBuffer->Map(0, &stReadRange, reinterpret_cast<void**>(&pIndexDataBegin)));
            memcpy(pIndexDataBegin, pBoxIndices, nszIndexBuffer);
            pIIndexBuffer->Unmap(0, nullptr);

            stIndexBufferView.BufferLocation = pIIndexBuffer->GetGPUVirtualAddress();
            stIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
            stIndexBufferView.SizeInBytes = nszIndexBuffer;

        }








    }



    return 0;
}


