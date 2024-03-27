
//-----------------------------------��ͷ�ļ�������-----------------------------------
#include <SDKDDKVer.h>//���ڸ�����ϣ������֧�ֵĲ���ϵͳ��Windowsͷ���ƽ���Щ�����������Ȱ����������С�copy��ע�ͣ�֪����Windows��ؾ͵��ˡ�ɾ�˶�������
#define WIN32_LEAN_AND_MEAN // �� Windows ͷ���ų�����ʹ�õ�����
#include <windows.h>
#include <tchar.h>//�ַ�����صģ�����ʹ�õ�Windows���ַ������������ҡ�
#include <strsafe.h>//�ַ�����ص�
#include <wrl.h>        //���WTL֧�� ����ʹ��COM������COMֻ��ָ����Ҫ��
#include <dxgi1_6.h>//DXGI��ͷ
#include <DirectXMath.h>//�����漰�ı任����������ͷ
#include <d3d12.h>       //for d3d12
#include <d3dcompiler.h>//D3D ������ص�
#if defined(_DEBUG)
#include <dxgidebug.h>//DXGI��һ�����Եĵ��ԣ����滹��һ��flag��ص�
#endif
#include <wincodec.h>   //for WIC������������ص�
//----------------------------------------------------------------------------------



//-----------------------------------�������ռ䡢���ӿ⡿----------------------------------------
// ���ӿ����ص㣬DX����Ķ���
//-----------------------------------------------------------------------------------------------

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace Microsoft;
using namespace Microsoft::WRL;
using namespace DirectX;


//-----------------------------------���궨�岿�֡�-----------------------------------
#define MAX_PATH 260
//�����жϺ���ִ�д���ģ���������˾ͻ������쳣
#define GRS_THROW_IF_FAILED(hr) {HRESULT _hr = (hr);if (FAILED(_hr)){ throw CGRSCOMException(_hr); }}


//���������
#define GRS_WND_CLASS_NAME _T("Chili Game Window Class")
#define GRS_WND_TITLE   _T("����������Ϩ�����Ϸ��������~")

//�¶���ĺ�������ȡ������
#define GRS_UPPER_DIV(A,B) ((UINT)(((A)+((B)-1))/(B)))

//���������ϱ߽�����㷨 �ڴ�����г��� ���ס
#define GRS_UPPER(A,B) ((UINT)(((A)+((B)-1))&~(B - 1)))
//----------------------------------------------------------------------------------

// ---------------------------------- - ���׳��쳣��ص��ࡿ----------------------------------------
// �ⲿ��C++����Ŷ�������Ļ����Լ��ҵ����Ͽ�����
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


//-----------------------------------��������صĺںС�----------------------------------------
// ������ʱ������ע��UploadTexture�����ڲ���·�����Ƿ������ļ�·��һ��
//-----------------------------------------------------------------------------------------------
struct WICTranslate
{
    GUID wic;
    DXGI_FORMAT format;
};

static WICTranslate g_WICFormats[] = {//WIC��ʽ��DXGI���ظ�ʽ�Ķ�Ӧ���ñ��еĸ�ʽΪ��֧�ֵĸ�ʽ
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

// WIC ���ظ�ʽת����.
struct WICConvert
{
    GUID source;
    GUID target;
};

static WICConvert g_WICConvert[] = {
    // Ŀ���ʽһ������ӽ��ı�֧�ֵĸ�ʽ
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
{//���ȷ�����ݵ���ӽ���ʽ���ĸ�
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
{//���ȷ�����ն�Ӧ��DXGI��ʽ����һ��
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
    //ʹ�ô�COM��ʽ����WIC�೧����Ҳ�ǵ���WIC��һ��Ҫ��������
    GRS_THROW_IF_FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIWICFactory)));
    TCHAR pszTextureFileName[MAX_PATH] = {};
    //ʹ��WIC�೧����ӿڼ�������ͼƬ�����õ�һ��WIC����������ӿڣ�ͼƬ��Ϣ��������ӿڴ���Ķ�������
    StringCchPrintf(pszTextureFileName, MAX_PATH, _T("%sTexture\\cat.jpg"), pszAppPath);
    GRS_THROW_IF_FAILED(pIWICFactory->CreateDecoderFromFilename(
        pszTextureFileName,              // �ļ���
        NULL,                            // ��ָ����������ʹ��Ĭ��
        GENERIC_READ,                    // ����Ȩ��
        WICDecodeMetadataCacheOnDemand,  // ����Ҫ�ͻ������� 
        &pIWICDecoder                    // ����������
    ));

    // ��ȡ��һ֡ͼƬ(��ΪGIF�ȸ�ʽ�ļ����ܻ��ж�֡ͼƬ�������ĸ�ʽһ��ֻ��һ֡ͼƬ)
    // ʵ�ʽ���������������λͼ��ʽ����
    GRS_THROW_IF_FAILED(pIWICDecoder->GetFrame(0, &pIWICFrame));

    WICPixelFormatGUID wpf = {};
    //��ȡWICͼƬ��ʽ
    GRS_THROW_IF_FAILED(pIWICFrame->GetPixelFormat(&wpf));
    GUID tgFormat = {};

    //ͨ����һ��ת��֮���ȡDXGI�ĵȼ۸�ʽ
    if (GetTargetPixelFormat(&wpf, &tgFormat))
    {
        stTextureFormat = GetDXGIFormatFromPixelFormat(&tgFormat);
    }

    if (DXGI_FORMAT_UNKNOWN == stTextureFormat)
    {// ��֧�ֵ�ͼƬ��ʽ Ŀǰ�˳����� 
     // һ�� ��ʵ�ʵ����浱�ж����ṩ�����ʽת�����ߣ�
     // ͼƬ����Ҫ��ǰת���ã����Բ�����ֲ�֧�ֵ�����
        throw CGRSCOMException(S_FALSE);
    }

    // ����һ��λͼ��ʽ��ͼƬ���ݶ���ӿ�
    //ComPtr<IWICBitmapSource>pIBMP;
    //�Ƶ�������Ϊ��������������������������������


    if (!InlineIsEqualGUID(wpf, tgFormat))
    {// ����жϺ���Ҫ�����ԭWIC��ʽ����ֱ����ת��ΪDXGI��ʽ��ͼƬʱ
     // ������Ҫ���ľ���ת��ͼƬ��ʽΪ�ܹ�ֱ�Ӷ�ӦDXGI��ʽ����ʽ
        //����ͼƬ��ʽת����
        ComPtr<IWICFormatConverter> pIConverter;
        GRS_THROW_IF_FAILED(pIWICFactory->CreateFormatConverter(&pIConverter));

        //��ʼ��һ��ͼƬת������ʵ��Ҳ���ǽ�ͼƬ���ݽ����˸�ʽת��
        GRS_THROW_IF_FAILED(pIConverter->Initialize(
            pIWICFrame.Get(),                // ����ԭͼƬ����
            tgFormat,                        // ָ����ת����Ŀ���ʽ
            WICBitmapDitherTypeNone,         // ָ��λͼ�Ƿ��е�ɫ�壬�ִ��������λͼ�����õ�ɫ�壬����ΪNone
            NULL,                            // ָ����ɫ��ָ��
            0.f,                             // ָ��Alpha��ֵ
            WICBitmapPaletteTypeCustom       // ��ɫ�����ͣ�ʵ��û��ʹ�ã�����ָ��ΪCustom
        ));
        // ����QueryInterface������ö����λͼ����Դ�ӿ�
        GRS_THROW_IF_FAILED(pIConverter.As(&pIBMP));
    }
    else
    {
        //ͼƬ���ݸ�ʽ����Ҫת����ֱ�ӻ�ȡ��λͼ����Դ�ӿ�
        GRS_THROW_IF_FAILED(pIWICFrame.As(&pIBMP));
    }
    //���ͼƬ��С����λ�����أ�
    GRS_THROW_IF_FAILED(pIBMP->GetSize(&nTextureW, &nTextureH));

    //��ȡͼƬ���ص�λ��С��BPP��Bits Per Pixel����Ϣ�����Լ���ͼƬ�����ݵ���ʵ��С����λ���ֽڣ�
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

    // ���������ڿ��Եõ�BPP�ˣ���Ҳ���ҿ��ıȽ���Ѫ�ĵط���Ϊ��BPP��Ȼ������ô�໷��
    GRS_THROW_IF_FAILED(pIWICPixelinfo->GetBitsPerPixel(&nBPP));

    // ����ͼƬʵ�ʵ��д�С����λ���ֽڣ�������ʹ����һ����ȡ����������A+B-1��/B ��
    // ����������˵��΢���������,ϣ�����Ѿ���������ָ��
    UINT nPicRowPitch = (uint64_t(nTextureW) * uint64_t(nBPP) + 7u) / 8u;
    return nPicRowPitch;
}



D3D12_PLACED_SUBRESOURCE_FOOTPRINT CopyToUploadHeap(ComPtr<ID3D12Resource> pITexture, ComPtr<ID3D12Resource> pITextureUpload,
    ComPtr<IWICBitmapSource> pIBMP, ComPtr<ID3D12Device4>   pID3D12Device4, UINT64 n64UploadBufferSize, UINT nTextureH,
    UINT nPicRowPitch)
{
    //������Դ�����С������ʵ��ͼƬ���ݴ洢���ڴ��С
    void* pbPicData = ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, n64UploadBufferSize);
    if (nullptr == pbPicData)
    {
        throw CGRSCOMException(HRESULT_FROM_WIN32(GetLastError()));
    }

    //��ͼƬ�ж�ȡ������
    GRS_THROW_IF_FAILED(pIBMP->CopyPixels(nullptr
        , nPicRowPitch
        , static_cast<UINT>(nPicRowPitch * nTextureH)   //ע���������ͼƬ������ʵ�Ĵ�С�����ֵͨ��С�ڻ���Ĵ�С
        , reinterpret_cast<BYTE*>(pbPicData)));

    //{//������δ�������DX12��ʾ����ֱ��ͨ����仺�������һ���ڰ׷��������
    // //��ԭ��δ��룬Ȼ��ע�������CopyPixels���ÿ��Կ����ڰ׷��������Ч��
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

    //��ȡ���ϴ��ѿ����������ݵ�һЩ����ת���ߴ���Ϣ
    //���ڸ��ӵ�DDS�������Ƿǳ���Ҫ�Ĺ���
    UINT64 n64RequiredSize = 0u;
    UINT   nNumSubresources = 1u;  //����ֻ��һ��ͼƬ��������Դ����Ϊ1
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

    //��Ϊ�ϴ���ʵ�ʾ���CPU�������ݵ�GPU���н�
    //�������ǿ���ʹ����Ϥ��Map����������ӳ�䵽CPU�ڴ��ַ��
    //Ȼ�����ǰ��н����ݸ��Ƶ��ϴ�����
    //��Ҫע�����֮���԰��п�������ΪGPU��Դ���д�С
    //��ʵ��ͼƬ���д�С���в����,���ߵ��ڴ�߽����Ҫ���ǲ�һ����
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
    //ȡ��ӳ�� �����ױ��������ÿ֡�ı任��������ݣ�������������Unmap�ˣ�
    //������פ�ڴ�,������������ܣ���Ϊÿ��Map��Unmap�Ǻܺ�ʱ�Ĳ���
    //��Ϊ�������붼��64λϵͳ��Ӧ���ˣ���ַ�ռ����㹻�ģ�������ռ�ò���Ӱ��ʲô
    pITextureUpload->Unmap(0, NULL);

    //�ͷ�ͼƬ���ݣ���һ���ɾ��ĳ���Ա
    ::HeapFree(::GetProcessHeap(), 0, pbPicData);
    return stTxtLayouts;
}

//-----------------------------------��ȫ�ֱ������֡�-----------------------------------

//----------------------------------------------------------------------------------

//-----------------------------------�������������֡�-----------------------------------


//----------------------------------------------------------------------------------

//-----------------------------------�������ʼ����-----------------------------------
// ��ʼ�����ں����ݵ�
//--------------------------------------------------------------------------------------



//-----------------------------------�����»��Ƶ����ݡ�-----------------------------------
// ����������Ϣ
//--------------------------------------------------------------------------------------

//-----------------------------------�����л��ơ�-----------------------------------
// ����������Ϣ
//--------------------------------------------------------------------------------------

//-----------------------------------���ṹ�嶨�塿---------------------------------------------
// ���ǵĶ�����Ϣ��Ӧ�Ľṹ���Լ�MVP����Ľṹ
//-----------------------------------------------------------------------------------------------
struct ST_GRS_VERTEX
{//������Ƕ��������ÿ������ķ��ߣ���Shader�л���ʱû����
    XMFLOAT4 m_v4Position;      //Position
    XMFLOAT2 m_vTex;        //Texcoord
};

struct ST_GRS_FRAME_MVP_BUFFER
{
    //����ṹ��Ҫ�ͳ����������Ľṹ���Ӧ����Ϊ����׼�������������������Ƿ�������ṹ���У�
    //�����ݵ��ϴ��ѣ�GPU�����Ƿ�����HLSL�еĳ����������У���������Ҫ��Ӧ��
    XMFLOAT4X4 m_MVP;           //�����Model-view-projection(MVP)����.
};


//-----------------------------------�����ڴ�������--------------------------------------------
//������Ҫ��ͨ���������룬�޸Ĺ۲���λ�ú�������ת���ٶ�
//-----------------------------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, message, wParam, lParam);
}

//-----------------------------------��WinMain������-----------------------------------
//Win32��������
//------------------------------------------------------------------------------------
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    ::CoInitialize(nullptr);

    const UINT nFrameBackBufferCount = 3u;

    int  iWidth = 1024;
    int  iHeight = 768;
    UINT nFrameIndex = 0;

    //DXGI_FORMAT_B8G8R8A8_UINT ��ʾ0-255 UNORM ��ʾ��һ�� 0-1
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

    //WIC�����
    ComPtr<IWICImagingFactory>          pIWICFactory;
    ComPtr<IWICBitmapDecoder>           pIWICDecoder;
    ComPtr<IWICBitmapFrameDecode>       pIWICFrame;
    ComPtr<IWICBitmapSource>            pIBMP;

    D3D12_RESOURCE_DESC                 stTextureDesc = {};

    //�����������
    float                               fAspectRatio = 3.0f;
    float                               fBoxSize = 3.0f;
    float                               fTCMax = 1.0f;

    ComPtr<ID3D12Resource>              pIVertexBuffer;
    ComPtr<ID3D12Resource>              pIIndexBuffer;

    D3D12_VERTEX_BUFFER_VIEW            stVertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW             stIndexBufferView = {};

    //1.��ȡ��ǰ�������еĿ�ִ���ļ���·��

    try
    {
        //һ������Ϊnullptr����ʾ��ȡ��ǰ���̵�ģ���ļ���
        //::��C++�е���������������������������ʾ����ȫ�������ռ��µ�GetModuleFileName����
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
        {//ɾ��Debug �� Release·�� +1 ��ʾ��ɾ�����һ����б��
            *(lastSlash + 1) = _T('\0');
        }

        printf("The Path is: %ls\n", psAppPath);
    }
    catch (CGRSCOMException& e)
    {
        e;
    }

    //2 ��������
    {
        WNDCLASSEX  wcex = {};
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_GLOBALCLASS;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);//��ֹ���ĵı����ػ�
        wcex.lpszClassName = GRS_WND_CLASS_NAME;
        RegisterClassEx(&wcex);

        DWORD dwWndStyle = WS_OVERLAPPED | WS_SYSMENU;
        RECT rtWnd = { 0,0,iWidth, iHeight };
        AdjustWindowRect(&rtWnd, dwWndStyle, FALSE);

        INT posX = (GetSystemMetrics(SM_CXSCREEN) - rtWnd.right - rtWnd.left) / 2;
        INT posY = (GetSystemMetrics(SM_CYSCREEN) - rtWnd.bottom - rtWnd.top) / 2;


        hWnd = CreateWindowW(GRS_WND_CLASS_NAME
            , GRS_WND_TITLE
            , dwWndStyle //�����ر�ȫ��Ч����
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

        //3 ʹ��WIC����������һ��2D����
        {
            nPicRowPitch = UploadTexture(pIBMP, pIWICFactory, pIWICDecoder, pIWICFrame, stTextureFormat, nTextureW, nTextureH, nBPP, psAppPath);
        }

        //4 ����ʾ��ϵͳ�ĵ���֧��
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

        //5 ����DXGI Factory����
        {
            //IID_PPV_ARGS ���ز���Ϊuuid�� ָ��Com�ӿڵ�ָ�������
            CreateDXGIFactory2(nDXGIFactoryFlags, IID_PPV_ARGS(pIDXGIFactory5.GetAddressOf()));
        }

        //6 ö�������� ��ѡ����ʵ�������������3D�豸����
        {
            DXGI_ADAPTER_DESC1 stAdapterDesc = {}; //�����������Ľṹ��
            //����DXGIFactory��ѭ��������������Կ���
            for (UINT nAdapterIndex = 0; DXGI_ERROR_NOT_FOUND != pIDXGIFactory5->EnumAdapters1(nAdapterIndex, &pIDXGIAdapter1); ++nAdapterIndex)
            {
                pIDXGIAdapter1->GetDesc1(&stAdapterDesc);
                if (stAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    //�����������������
                    continue;
                }
                //�����������D3D֧�ֵļ��ݼ�������ֱ��Ҫ��֧��12.1��������ע�ⷵ�ؽӿڵ��Ǹ���������Ϊ��nullptr������
                //�Ͳ���ʵ�ʴ���һ���豸�ˣ�Ҳ�������ǆ��µ��ٵ���release���ͷŽӿڡ���Ҳ��һ����Ҫ�ļ��ɣ����ס��

                if (SUCCEEDED(D3D12CreateDevice(pIDXGIAdapter1.Get(), D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }

            }
            // ����D3D12.1���豸���������������������豸��
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

            nRTVDescriptorSize = pID3D12Device4->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); //RTV(render target view)����ȾĿ����ͼ
            // CBV(constant buffer view����������������ͼ  SRV(shader resource view)����ɫ����Դ��ͼ
            nSRVDescriptorSize = pID3D12Device4->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        }

        //7 �����������
        {
            D3D12_COMMAND_QUEUE_DESC stQueneDesc = {};
            stQueneDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            GRS_THROW_IF_FAILED(pID3D12Device4->CreateCommandQueue(&stQueneDesc, IID_PPV_ARGS(&pICMDQueue)));
        }

        //8 ���������б������
        {
            GRS_THROW_IF_FAILED(pID3D12Device4->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pICMDAlloc)));
            // ����ͼ�������б�
            GRS_THROW_IF_FAILED(pID3D12Device4->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pICMDAlloc.Get(), pIPipelineState.Get(), IID_PPV_ARGS(&pICMDList)));
        }

        //9 ����������
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
            //�õ���ǰ�󻺳�������ţ�Ҳ������һ����Ҫ������ʾ�Ļ����������
            //ע��˴�ʹ���˸߰汾��SwapChain�ӿڵĺ���
            GRS_THROW_IF_FAILED(pISwapChain1.As(&pISwapChain3));
            nFrameIndex = pISwapChain3->GetCurrentBackBufferIndex();

            //����RTV(��ȾĿ����ͼ)��������(����ѵĺ���Ӧ�����Ϊ������߹̶���СԪ�صĹ̶���С�Դ��)
            D3D12_DESCRIPTOR_HEAP_DESC stRTVHeapDesc = {};
            stRTVHeapDesc.NumDescriptors = nFrameBackBufferCount;
            stRTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            stRTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

            GRS_THROW_IF_FAILED(pID3D12Device4->CreateDescriptorHeap(&stRTVHeapDesc, IID_PPV_ARGS(&pIRTVHeap)));

            //����RTV��������
            D3D12_CPU_DESCRIPTOR_HANDLE stRTVHandle = pIRTVHeap->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < nFrameBackBufferCount; i++)
            {
                GRS_THROW_IF_FAILED(pISwapChain3->GetBuffer(i, IID_PPV_ARGS(&pIARenderTargets[i])));
                pID3D12Device4->CreateRenderTargetView(pIARenderTargets[i].Get(), nullptr, stRTVHandle);
                stRTVHandle.ptr += nRTVDescriptorSize;
            }

            // �ر�ALT+ENTER���л�ȫ���Ĺ��ܣ���Ϊ����û��ʵ��OnSize�����������ڵ�ʱ����ˣ��������ȹر�
            GRS_THROW_IF_FAILED(pIDXGIFactory5->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));

        }

        //��������ѵĽ��� 
        // ��������Ķ�ָ�Ķ���Ϊ��GPU������Դ��������ڴ� UploadHeap(ΪCPU GPU�����ڴ� CPUд GPU��)
        //DefaultHeap(��UploadHeadp�п��� ֻ��GPU��д) ReadbackHeap(GPUд CPU��)

        //���ڶѵ���ͼ(������)
        //����ÿ����Դ����Ҫһ����Դ��ͼ����� ��Դ��Ҫ��ΪBuffer,Texture 
        //��Դ��ͼ�� IndexBufferView/VertexBufferView/ShaderResourceView(SRV)/ConstantBufferViw(CBV)/Sampler/RenderTargetView/DepthStencilView/StreamTargetView

        //��������
        //����װ������������ ��ΪCBV/SRV/UAV  Sampler  RTV  DSV

        //10 ����SRV��CRV��  ��ɫ����Դ��ͼ��Shader Resource View, SRV���ͳ���������ͼ
        {
            D3D12_DESCRIPTOR_HEAP_DESC stSRVHeapDesc = {};
            stSRVHeapDesc.NumDescriptors = 2; //���������� ָSRV_CBV ��ʱ����UAV
            stSRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            stSRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; //�ɱ���ɫ������ ���VISIBLE��־�ᱻ�󶨵�Command List��
            //������������
            GRS_THROW_IF_FAILED(pID3D12Device4->CreateDescriptorHeap(&stSRVHeapDesc, IID_PPV_ARGS(&pISRVHeap)));
        }

        //11 ������ǩ��
        {
            D3D12_FEATURE_DATA_ROOT_SIGNATURE stFeatureData = {};
            // ����Ƿ�֧��V1.1�汾�ĸ�ǩ��
            stFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
            if (FAILED(pID3D12Device4->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &stFeatureData, sizeof(stFeatureData))))
            {
                stFeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
            }
            // ��GPU��ִ��SetGraphicsRootDescriptorTable�����ǲ��޸������б��е�SRV��������ǿ���ʹ��Ĭ��Rang��Ϊ:
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
            stRootParameters1[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;        //CBV������Shader�ɼ�
            stRootParameters1[1].DescriptorTable.NumDescriptorRanges = 1;
            stRootParameters1[1].DescriptorTable.pDescriptorRanges = &stDSPRanges1[1];


            //��Ϊ������������ ���ڴ����˲�����Sampler
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

        //12 ����Shader������Ⱦ����״̬����
        {
            //ID3DBlob��һ���ӿڣ���������һ���ڴ��еĶ��������ݿ飨Blob����Blob��һ���������ڴ����򣬿��Դ洢�������͵����ݣ�ͨ�����ڴ洢��������ɫ�����롢�������ݡ���������
            ComPtr<ID3DBlob> pIBlobVertexShader;
            ComPtr<ID3DBlob> pIBlobPixelShader;

#if defined(_DEBUG)
            // Enable better shader debugging with the graphics debugging tools. 
            UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
            UINT compileFlags = 0;
#endif
            //������Ϊ������ʽ
            compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
            //����shader.hlsl �ļ�
            TCHAR pszShaderFileName[MAX_PATH] = {};
            StringCchPrintf(pszShaderFileName, MAX_PATH, _T("%sShader\\shaders.hlsl"), psAppPath);
            GRS_THROW_IF_FAILED(D3DCompileFromFile(pszShaderFileName, nullptr, nullptr
                , "VSMain", "vs_5_0", compileFlags, 0, &pIBlobVertexShader, nullptr));
            GRS_THROW_IF_FAILED(D3DCompileFromFile(pszShaderFileName, nullptr, nullptr
                , "PSMain", "ps_5_0", compileFlags, 0, &pIBlobPixelShader, nullptr));
            //����Input Layout
            D3D12_INPUT_ELEMENT_DESC stInputElementDescs[] =
            {
                 { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                 { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            };

            //����PSO Pipeline State Object
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


        //13 ��������Ĭ�϶�
        {
            D3D12_HEAP_DESC stTextureHeapDesc = {};
            //Ϊ��ָ������ͼƬ����2����С�Ŀռ䣬����û����ϸȥ�����ˣ�ֻ��ָ����һ���㹻��Ŀռ䣬�����������
            //ʵ��Ӧ����Ҳ��Ҫ�ۺϿ��Ƿ���ѵĴ�С���Ա�������ö�
            stTextureHeapDesc.SizeInBytes = GRS_UPPER(2 * nPicRowPitch * nTextureH, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
            //ָ���ѵĶ��뷽ʽ������ʹ����Ĭ�ϵ�64K�߽���룬��Ϊ������ʱ����ҪMSAA֧��
            stTextureHeapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
            stTextureHeapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;        //Ĭ�϶�����
            stTextureHeapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            stTextureHeapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            //�ܾ���ȾĿ�������ܾ������������ʵ�ʾ�ֻ�������ڷ���ͨ����
            stTextureHeapDesc.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_BUFFERS;

            GRS_THROW_IF_FAILED(pID3D12Device4->CreateHeap(&stTextureHeapDesc, IID_PPV_ARGS(&pITextureHeap)));
        }

        //14 ����2D����
        {
            // ����ͼƬ��Ϣ�����2D������Դ����Ϣ�ṹ��
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
           //ʹ�á���λ��ʽ������������ע��������������ڲ�ʵ���Ѿ�û�д洢������ͷŵ�ʵ�ʲ����ˣ��������ܸܺ�
           //ͬʱ������������Ϸ�������CreatePlacedResource��������ͬ��������Ȼǰ�������ǲ��ڱ�ʹ�õ�ʱ�򣬲ſ���
           //���ö�
            GRS_THROW_IF_FAILED(pID3D12Device4->CreatePlacedResource(
                pITextureHeap.Get()
                , 0
                , &stTextureDesc                //����ʹ��CD3DX12_RESOURCE_DESC::Tex2D���򻯽ṹ��ĳ�ʼ��
                , D3D12_RESOURCE_STATE_COPY_DEST
                , nullptr
                , IID_PPV_ARGS(&pITexture)));
            //-----------------------------------------------------------------------------------------------------------
            //��ȡ�ϴ�����Դ����Ĵ�С������ߴ�ͨ������ʵ��ͼƬ�ĳߴ�
            D3D12_RESOURCE_DESC stCopyDstDesc = pITexture->GetDesc();
            n64UploadBufferSize = 0;
            pID3D12Device4->GetCopyableFootprints(&stCopyDstDesc, 0, 1, 0, nullptr, nullptr, nullptr, &n64UploadBufferSize);
        }

        //15�������ϴ���
        {
            D3D12_HEAP_DESC stUploadHeapDesc = {  };
            //�ߴ���Ȼ��ʵ���������ݴ�С��2����64K�߽�����С
            stUploadHeapDesc.SizeInBytes = GRS_UPPER(2 * n64UploadBufferSize, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
            //ע���ϴ��ѿ϶���Buffer���ͣ����Բ�ָ�����뷽ʽ����Ĭ����64k�߽����
            stUploadHeapDesc.Alignment = 0;
            stUploadHeapDesc.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;      //�ϴ�������
            stUploadHeapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            stUploadHeapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            //�ϴ��Ѿ��ǻ��壬���԰ڷ���������
            stUploadHeapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

            GRS_THROW_IF_FAILED(pID3D12Device4->CreateHeap(&stUploadHeapDesc, IID_PPV_ARGS(&pIUploadHeap)));
        }

        //16��ʹ�á���λ��ʽ�����������ϴ��������ݵĻ�����Դ
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

        //17������ͼƬ�������ϴ��ѣ�����ɵ�һ��Copy��������memcpy������֪������CPU��ɵ�
        {
            //CPU���������ݴ��ڴ渴�Ƶ��ϴ����С�
            stTxtLayouts = CopyToUploadHeap(pITexture, pITextureUpload, pIBMP, pID3D12Device4, n64UploadBufferSize, nTextureH, nPicRowPitch);
        }

        //18����������ϴ��Ѹ��Ƶ�Ĭ�϶�
        {
            //��ֱ�������б������ϴ��Ѹ����������ݵ�Ĭ�϶ѵ����ִ�в�ͬ���ȴ�������ɵڶ���Copy��������GPU�ϵĸ����������
                //ע���ʱֱ�������б�û�а�PSO���������Ҳ�ǲ���ִ��3Dͼ������ģ����ǿ���ִ�и��������Ϊ�������治��Ҫʲô
                //�����״̬����֮��Ĳ���
            D3D12_TEXTURE_COPY_LOCATION stDstCopyLocation = {};
            stDstCopyLocation.pResource = pITexture.Get();
            stDstCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            stDstCopyLocation.SubresourceIndex = 0;

            D3D12_TEXTURE_COPY_LOCATION stSrcCopyLocation = {};
            stSrcCopyLocation.pResource = pITextureUpload.Get();
            stSrcCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            stSrcCopyLocation.PlacedFootprint = stTxtLayouts;

            pICMDList->CopyTextureRegion(&stDstCopyLocation, 0, 0, 0, &stSrcCopyLocation, nullptr);

            //����һ����Դ���ϣ�ͬ����ȷ�ϸ��Ʋ������
            //ֱ��ʹ�ýṹ��Ȼ����õ���ʽ
            D3D12_RESOURCE_BARRIER stResBar = {};
            stResBar.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            stResBar.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            stResBar.Transition.pResource = pITexture.Get();
            stResBar.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            stResBar.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            stResBar.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            pICMDList->ResourceBarrier(1, &stResBar);

            //����ʹ��D3DX12���еĹ�������õĵȼ���ʽ������ķ�ʽ�����һЩ
            //pICMDList->ResourceBarrier(1
            //  , &CD3DX12_RESOURCE_BARRIER::Transition(pITexture.Get()
            //  , D3D12_RESOURCE_STATE_COPY_DEST
            //  , D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
            //);

            //---------------------------------------------------------------------------------------------
            // ִ�������б��ȴ�������Դ�ϴ���ɣ���һ���Ǳ����
            GRS_THROW_IF_FAILED(pICMDList->Close());
            ID3D12CommandList* ppCommandLists[] = { pICMDList.Get() };
            pICMDQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

            //---------------------------------------------------------------------------------------------
            // 17������һ��ͬ�����󡪡�Χ�������ڵȴ���Ⱦ��ɣ���Ϊ����Draw Call���첽����
            GRS_THROW_IF_FAILED(pID3D12Device4->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pIFence)));
            n64FenceValue = 1;

            //---------------------------------------------------------------------------------------------
            // 18������һ��Eventͬ���������ڵȴ�Χ���¼�֪ͨ
            hEventFence = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (hEventFence == nullptr)
            {
                GRS_THROW_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
            }

            //---------------------------------------------------------------------------------------------
            // 19���ȴ�������Դ��ʽ���������
            const UINT64 fence = n64FenceValue;
            GRS_THROW_IF_FAILED(pICMDQueue->Signal(pIFence.Get(), fence));
            n64FenceValue++;
            GRS_THROW_IF_FAILED(pIFence->SetEventOnCompletion(fence, hEventFence));

            WaitForSingleObject(hEventFence, INFINITE);
        }

        //19�����������嶥�����������


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


        //20���������㻺��
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


        //21����������������        
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


