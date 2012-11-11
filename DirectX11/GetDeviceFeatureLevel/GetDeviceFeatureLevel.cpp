#include <iostream>
#include <d3d11.h>


HRESULT hr = E_FAIL;
D3D_FEATURE_LEVEL FeatureLevel;


D3D_FEATURE_LEVEL GetDX11FeatureLevel()
{
	D3D_FEATURE_LEVEL FeatureLevel;

	HRESULT	hr = D3D11CreateDevice(
		NULL, 
		D3D_DRIVER_TYPE_HARDWARE, 
		NULL, 
		0, 
		NULL, 
		0,
        D3D11_SDK_VERSION, 
		NULL, 
		&FeatureLevel,
		NULL 
		);

	if (FAILED(hr))
	{
		MessageBox(0, L"Create DirectX 11 device failed!", L"Error", 0);
	}
	
	return FeatureLevel;
}


int main(void)
{
	D3D_FEATURE_LEVEL FeatureLevel = GetDX11FeatureLevel();
	std::cout << "Highest feature level: " << FeatureLevel << std::endl;
	getchar();
	return 0;
}