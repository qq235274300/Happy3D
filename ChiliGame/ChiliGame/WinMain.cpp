
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


//-----------------------------------【游戏运行函数】-----------------------------------
// 游戏循环框架封装在其中
//--------------------------------------------------------------------------------------


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
    }



    return 0;
}
