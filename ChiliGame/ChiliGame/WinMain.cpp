
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


//-----------------------------------����Ϸ���к�����-----------------------------------
// ��Ϸѭ����ܷ�װ������
//--------------------------------------------------------------------------------------


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

    ComPtr<ID3D12PipelineState>         pIPipelineState;






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
    }



    return 0;
}
