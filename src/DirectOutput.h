#ifndef DIRECTOUTPUT_H
#define DIRECTOUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

//=============================================================================
// constants
//=============================================================================

// Device Type Guids
// {29DAD506-F93B-4f20-85FA-1E02C04FAC17}
static const GUID DeviceType_X52Pro		= { 0x29DAD506, 0xF93B, 0x4F20, { 0x85, 0xFA, 0x1E, 0x02, 0xC0, 0x4F, 0xAC, 0x17 } };

// Soft Buttons
static const DWORD SoftButton_Select	= 0x00000001;
static const DWORD SoftButton_Up		= 0x00000002;
static const DWORD SoftButton_Down		= 0x00000004;
// Unused soft buttons
static const DWORD SoftButton_Left		= 0x00000008;
static const DWORD SoftButton_Right		= 0x00000010;
static const DWORD SoftButton_Back		= 0x00000020;
static const DWORD Softbutton_Increment = 0x00000040;
static const DWORD SoftButton_Decrement = 0x00000080;

//=============================================================================
// callbacks
//=============================================================================

typedef void (__stdcall *Pfn_DirectOutput_Device_Callback)(IN void* hDevice, IN BOOL bAdded, IN void* pCtxt);
typedef void (__stdcall *Pfn_DirectOutput_SoftButton_Callback)(IN void* hDevice, IN DWORD dwButtons, IN void* pCtxt);
typedef void (__stdcall *Pfn_DirectOutput_Page_Callback)(IN void* hDevice, IN DWORD dwPage, IN BOOL bActivated, IN void* pCtxt);

//=============================================================================
// functions
//=============================================================================

HRESULT __stdcall DirectOutput_Initialize			(IN const wchar_t* wszAppName);
HRESULT __stdcall DirectOutput_Deinitialize			();
HRESULT __stdcall DirectOutput_RegisterDeviceChangeCallback(IN Pfn_DirectOutput_Device_Callback pfnCb, IN void* pCtxt);
HRESULT __stdcall DirectOutput_Enumerate			();

HRESULT __stdcall DirectOutput_GetDeviceType		(IN void* hDevice, OUT LPGUID pGdDevice);
HRESULT __stdcall DirectOutput_GetDeviceInstance	(IN void* hDevice, OUT LPGUID pGdInstance);
HRESULT __stdcall DirectOutput_RegisterSoftButtonChangeCallback(IN void* hDevice, IN Pfn_DirectOutput_SoftButton_Callback pfnCb, IN void* pCtxt);
HRESULT __stdcall DirectOutput_RegisterPageChangeCallback(IN void* hDevice, IN Pfn_DirectOutput_Page_Callback pfnCb, IN void* pCtxt);

HRESULT __stdcall DirectOutput_AddPage				(IN void* hDevice, IN DWORD dwPage, IN const wchar_t* wszValue, IN BOOL bSetAsActive);
HRESULT __stdcall DirectOutput_RemovePage			(IN void* hDevice, IN DWORD dwPage);
HRESULT __stdcall DirectOutput_SetLed				(IN void* hDevice, IN DWORD dwPage, IN DWORD dwIndex, IN DWORD dwValue);
HRESULT __stdcall DirectOutput_SetString			(IN void* hDevice, IN DWORD dwPage, IN DWORD dwIndex, IN DWORD cchValue, IN const wchar_t* wszValue);
HRESULT __stdcall DirectOutput_SetImage				(IN void* hDevice, IN DWORD dwPage, IN DWORD dwIndex, IN DWORD cbValue, IN const unsigned char* pbValue);

HRESULT __stdcall DirectOutput_SetProfile			(IN void* hDevice, IN DWORD cchFilename, IN const wchar_t* wszFilename);

//=============================================================================
// function pointers
//=============================================================================

typedef HRESULT (__stdcall *Pfn_DirectOutput_Initialize)		(IN const wchar_t* wszAppName);
typedef HRESULT (__stdcall *Pfn_DirectOutput_Deinitialize)		();
typedef HRESULT (__stdcall *Pfn_DirectOutput_RegisterDeviceChangeCallback)(IN Pfn_DirectOutput_Device_Callback pfnCb, IN void* pCtxt);
typedef HRESULT (__stdcall *Pfn_DirectOutput_Enumerate)			();

typedef HRESULT (__stdcall *Pfn_DirectOutput_GetDeviceType)		(IN void* hDevice, OUT LPGUID pGdDevice);
typedef HRESULT (__stdcall *Pfn_DirectOutput_GetDeviceInstance)	(IN void* hDevice, OUT LPGUID pGdInstance);
typedef HRESULT (__stdcall *Pfn_DirectOutput_RegisterSoftButtonChangeCallback)(IN void* hDevice, IN Pfn_DirectOutput_SoftButton_Callback pfnCb, IN void* pCtxt);
typedef HRESULT (__stdcall *Pfn_DirectOutput_RegisterPageChangeCallback)(IN void* hDevice, IN Pfn_DirectOutput_Page_Callback pfnCb, IN void* pCtxt);

typedef HRESULT (__stdcall *Pfn_DirectOutput_AddPage)			(IN void* hDevice, IN DWORD dwPage, IN const wchar_t* wszValue, IN BOOL bSetAsActive);
typedef HRESULT (__stdcall *Pfn_DirectOutput_RemovePage)		(IN void* hDevice, IN DWORD dwPage);
typedef HRESULT (__stdcall *Pfn_DirectOutput_SetLed)			(IN	void* hDevice, IN DWORD dwPage, IN DWORD dwIndex, IN DWORD dwValue);
typedef HRESULT (__stdcall *Pfn_DirectOutput_SetString)			(IN void* hDevice, IN DWORD dwPage, IN DWORD dwIndex, IN DWORD cchValue, IN const wchar_t* wszValue);
typedef HRESULT (__stdcall *Pfn_DirectOutput_SetImage)			(IN void* hDevice, IN DWORD dwPage, IN DWORD dwIndex, IN DWORD cbValue, IN const unsigned char* pbValue);

typedef HRESULT (__stdcall *Pfn_DirectOutput_SetProfile)		(IN void* hDevice, IN DWORD cchFilename, IN const wchar_t* wszFilename);

//=============================================================================

#ifdef __cplusplus
};
#endif

class CDirectOutput
{
public:
	CDirectOutput() : m_module(0),
		m_i(0), m_d(0), m_rdcc(0), m_e(0),
		m_gdt(0), m_gdi(0), m_rscc(0), m_rpcc(0),
		m_sl(0), m_ss(0), m_si(0), m_sp(0)
	{

			m_module = LoadLibraryW(L"DirectOutput.dll");
				m_i   = (Pfn_DirectOutput_Initialize)GetProcAddress(m_module, "DirectOutput_Initialize");
				m_d   = (Pfn_DirectOutput_Deinitialize)GetProcAddress(m_module, "DirectOutput_Deinitialize");
				m_rdcc= (Pfn_DirectOutput_RegisterDeviceChangeCallback)GetProcAddress(m_module, "DirectOutput_RegisterDeviceChangeCallback");
				m_e   = (Pfn_DirectOutput_Enumerate)GetProcAddress(m_module, "DirectOutput_Enumerate");
													
				m_gdt = (Pfn_DirectOutput_GetDeviceType)GetProcAddress(m_module, "DirectOutput_GetDeviceType");
				m_gdi = (Pfn_DirectOutput_GetDeviceInstance)GetProcAddress(m_module, "DirectOutput_GetDeviceInstance");
				m_rscc= (Pfn_DirectOutput_RegisterSoftButtonChangeCallback)GetProcAddress(m_module, "DirectOutput_RegisterSoftButtonChangeCallback");
				m_rpcc= (Pfn_DirectOutput_RegisterPageChangeCallback)GetProcAddress(m_module, "DirectOutput_RegisterPageChangeCallback");
													
				m_ap  = (Pfn_DirectOutput_AddPage)GetProcAddress(m_module, "DirectOutput_AddPage");
				m_rp  = (Pfn_DirectOutput_RemovePage)GetProcAddress(m_module, "DirectOutput_RemovePage");
				m_sl  = (Pfn_DirectOutput_SetLed)GetProcAddress(m_module, "DirectOutput_SetLed");
				m_ss  = (Pfn_DirectOutput_SetString)GetProcAddress(m_module, "DirectOutput_SetString");	
				m_si  = (Pfn_DirectOutput_SetImage)GetProcAddress(m_module, "DirectOutput_SetImage");
						 
				m_sp  = (Pfn_DirectOutput_SetProfile)GetProcAddress(m_module, "DirectOutput_SetProfile");
			
	}
	~CDirectOutput()
	{
		if (m_module)
		{
			FreeLibrary(m_module);
			m_module = 0;
		}
	}
	static CDirectOutput& Instance()
	{
		static CDirectOutput inst;
		return inst;
	}
	
public:
	HRESULT Initialize			(IN const wchar_t* wszAppName)
	{
		if (m_module && m_i)
			return m_i(wszAppName);
		return E_NOTIMPL;
	}
	HRESULT Deinitialize			()
	{
		if (m_module && m_d)
			return m_d();
		return E_NOTIMPL;
	}
	HRESULT RegisterDeviceChangeCallback(IN Pfn_DirectOutput_Device_Callback pfnCb, IN void* pCtxt)
	{
		if (m_module && m_rdcc)
			return m_rdcc(pfnCb, pCtxt);
		return E_NOTIMPL;
	}
	HRESULT Enumerate			()
	{
		if (m_module && m_e)
			return m_e();
		return E_NOTIMPL;
	}

	HRESULT GetDeviceType		(IN void* hDevice, OUT LPGUID pGdDevice)
	{
		if (m_module && m_gdt)
			return m_gdt(hDevice, pGdDevice);
		return E_NOTIMPL;
	}
	HRESULT GetDeviceInstance	(IN void* hDevice, OUT LPGUID pGdInstance)
	{
		if (m_module && m_gdi)
			m_gdi(hDevice, pGdInstance);
		return E_NOTIMPL;
	}
	HRESULT RegisterSoftButtonChangeCallback(IN void* hDevice, IN Pfn_DirectOutput_SoftButton_Callback pfnCb, IN void* pCtxt)
	{
		if (m_module && m_rscc)
			return m_rscc(hDevice, pfnCb, pCtxt);
		return E_NOTIMPL;
	}
	HRESULT RegisterPageChangeCallback(IN void* hDevice, IN Pfn_DirectOutput_Page_Callback pfnCb, IN void* pCtxt)
	{
		if (m_module && m_rpcc)
			return m_rpcc(hDevice, pfnCb, pCtxt);
		return E_NOTIMPL;
	}

	HRESULT AddPage				(IN void* hDevice, IN DWORD dwPage, IN const wchar_t* wszValue, IN BOOL bSetAsActive)
	{
		if (m_module && m_ap)
			return m_ap(hDevice, dwPage, wszValue, bSetAsActive);
		return E_NOTIMPL;
	}
	HRESULT RemovePage			(IN void* hDevice, IN DWORD dwPage)
	{
		if (m_module && m_rp)
			return m_rp(hDevice, dwPage);
		return E_NOTIMPL;
	}
	HRESULT SetLed				(IN void* hDevice, IN DWORD dwPage, IN DWORD dwIndex, IN DWORD dwValue)
	{
		if (m_module && m_sl)
			return m_sl(hDevice, dwPage, dwIndex, dwValue);
		return E_NOTIMPL;
	}
	HRESULT SetString			(IN void* hDevice, IN DWORD dwPage, IN DWORD dwIndex, IN DWORD cchValue, IN const wchar_t* wszValue)
	{
		if (m_module && m_ss)
			return m_ss(hDevice, dwPage, dwIndex, cchValue, wszValue);
		return E_NOTIMPL;
	}
	HRESULT SetImage				(IN void* hDevice, IN DWORD dwPage, IN DWORD dwIndex, IN DWORD cbValue, IN const unsigned char* pbValue)
	{
		if (m_module && m_si)
			return m_si(hDevice, dwPage, dwIndex, cbValue, pbValue);
		return E_NOTIMPL;
	}

	HRESULT SetProfile			(IN void* hDevice, IN DWORD cchFilename, IN const wchar_t* wszFilename)
	{
		if (m_module && m_sp)
			return m_sp(hDevice, cchFilename, wszFilename);
		return E_NOTIMPL;
	}
	
private:
	HMODULE												m_module;

	Pfn_DirectOutput_Initialize							m_i;
	Pfn_DirectOutput_Deinitialize						m_d;
	Pfn_DirectOutput_RegisterDeviceChangeCallback		m_rdcc;
	Pfn_DirectOutput_Enumerate							m_e;

	Pfn_DirectOutput_GetDeviceType						m_gdt;
	Pfn_DirectOutput_GetDeviceInstance					m_gdi;
	Pfn_DirectOutput_RegisterSoftButtonChangeCallback	m_rscc;
	Pfn_DirectOutput_RegisterPageChangeCallback			m_rpcc;

	Pfn_DirectOutput_AddPage							m_ap;
	Pfn_DirectOutput_RemovePage							m_rp;
	Pfn_DirectOutput_SetLed								m_sl;
	Pfn_DirectOutput_SetString							m_ss;
	Pfn_DirectOutput_SetImage							m_si;

	Pfn_DirectOutput_SetProfile							m_sp;
};

#endif