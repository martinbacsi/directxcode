#include <dxgi.h>
#include <iostream>
#include <vector>

std::vector<IDXGIAdapter*> EnumerateAdapters();
void PrintAdapterInfo(IDXGIAdapter* pAdapter);

std::vector<IDXGIOutput*> EnumerateOutputs(IDXGIAdapter* pAdapter);
void PrintOutputInfo(IDXGIOutput* output);

std::vector<DXGI_MODE_DESC*> GetDisplayModeList(IDXGIOutput* output);
void PrintDisplayModeInfo(DXGI_MODE_DESC* pModeDesc);

int main(void)
{
	std::vector<IDXGIAdapter*> adapters = EnumerateAdapters();
	for(std::vector<IDXGIAdapter*>::iterator itor = adapters.begin(); itor != adapters.end(); ++itor)
	{
		PrintAdapterInfo(*itor);

		// Get Output info
		std::vector<IDXGIOutput*> outputs = EnumerateOutputs(*itor);
		for(std::vector<IDXGIOutput*>::iterator outputItor = outputs.begin(); outputItor != outputs.end(); ++outputItor)
		{
			PrintOutputInfo(*outputItor);

			// Get display mode list
			std::vector<DXGI_MODE_DESC*> modeList = GetDisplayModeList(*outputItor);
			for(std::vector<DXGI_MODE_DESC*>::iterator modeItor = modeList.begin(); modeItor != modeList.end(); ++modeItor)
			{
				PrintDisplayModeInfo(*modeItor);
			}
		}

		std::cout << std::endl;
	}

	std::getchar();
	return 0;
}

std::vector<IDXGIAdapter*> EnumerateAdapters()
{
	// Create DXGI factory
	IDXGIFactory1* pFactory = NULL;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&pFactory));
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Create DXGI factory failed", L"Error", 0);
	}

	// Enumerate devices
	IDXGIAdapter* pAdapter = NULL;
	std::vector<IDXGIAdapter*> vAdapters;

	for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		vAdapters.push_back(pAdapter);
	}

	if (pFactory)
	{
		pFactory->Release();
		pFactory = NULL;
	}

	return vAdapters;
}

void PrintAdapterInfo(IDXGIAdapter* pAdapter)
{
	DXGI_ADAPTER_DESC aDesc;
	pAdapter->GetDesc(&aDesc);

	std::cout << "Description: " << aDesc.Description << std::endl;
	std::cout << "VendorId: " << aDesc.VendorId << std::endl;
	std::cout << "DevicedId: " << aDesc.DeviceId << std::endl;
	std::cout << "SubSysId: " << aDesc.SubSysId << std::endl;
	std::cout << "Revision: " << aDesc.Revision << std::endl;
	std::cout << "DedicatedVideoMemory: " << aDesc.DedicatedVideoMemory << std::endl;
	std::cout << "DedicatedSystemMemory: " << aDesc.DedicatedSystemMemory << std::endl;
	std::cout << "SharedSystemMemory: " << aDesc.SharedSystemMemory << std::endl;
	//std::cout << "AdapterLuid: " << aDesc.AdapterLuid << std::endl;
}

std::vector<IDXGIOutput*> EnumerateOutputs(IDXGIAdapter* pAdapter)
{
	std::vector<IDXGIOutput*> outputs;
	IDXGIOutput* pOutput = NULL;
	for (UINT i = 0; pAdapter->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		outputs.push_back(pOutput);
	}

	return outputs;
}

void PrintOutputInfo(IDXGIOutput* output)
{
	DXGI_OUTPUT_DESC oDesc;
	output->GetDesc(&oDesc);

	std::cout << "Device name: " << oDesc.DeviceName << std::endl;
	std::cout << "Desktop coordinates: " << oDesc.DesktopCoordinates.left << ", " << oDesc.DesktopCoordinates.top << ", " << oDesc.DesktopCoordinates.right << ", " << oDesc.DesktopCoordinates.bottom << std::endl;
}

std::vector<DXGI_MODE_DESC*> GetDisplayModeList(IDXGIOutput* output)
{
	UINT num = 0;
	DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_TYPELESS;
	UINT flags = DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING;

	// Get number of display modes
	output->GetDisplayModeList(format, flags, &num, 0);

	// Get display mode list
	DXGI_MODE_DESC * pDescs = new DXGI_MODE_DESC[num];
	output->GetDisplayModeList(format, flags, &num, pDescs);

	std::vector<DXGI_MODE_DESC*> displayList;
	for(int i = 0; i < num; ++i)
	{
		displayList.push_back(&pDescs[i]);
	}

	return displayList;
}

void PrintDisplayModeInfo(DXGI_MODE_DESC* pModeDesc)
{
	int refreshRate =  pModeDesc->RefreshRate.Numerator / pModeDesc->RefreshRate.Denominator;

	std::cout << pModeDesc->Width << " by " << pModeDesc->Height << ", " << refreshRate << std::endl;
}
