#pragma once
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <locale.h>
#include <map>
#include <memory>
#include <fstream>
#include <iostream>
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>

#include <DDSTextureLoader.h>

#include "..\\Content\ShaderStructures.h"

namespace AlienPlanetACW
{
	class ResourceManager
	{
	public:
		ResourceManager();
		~ResourceManager();

		bool GetModel(ID3D11Device* const device, const char* const modelFileName, ID3D11Buffer* &vertexBuffer, ID3D11Buffer* &indexBuffer);
		bool GetModel(ID3D11Device* const device, const char* const modelFileName, Microsoft::WRL::ComPtr<ID3D11Buffer> &vertexBuffer, Microsoft::WRL::ComPtr<ID3D11Buffer> &indexBuffer);

		bool GetTexture(ID3D11Device* const device, const WCHAR* const textureFileName, ID3D11ShaderResourceView* &texture);

		int GetSizeOfVertexType() const;
		int GetIndexCount(const char* modelFileName) const;

	private:
		bool LoadModel(ID3D11Device* const device, const char* const modelFileName);
		bool LoadTexture(ID3D11Device* const device, const WCHAR* textureFileName);

		//struct VertexType {
		//	DirectX::XMFLOAT3 position;
		//	DirectX::XMFLOAT2 texture;
		//	DirectX::XMFLOAT3 normal;
		//	DirectX::XMFLOAT3 tangent;
		//	DirectX::XMFLOAT3 binormal;
		//};

		std::map<const char*, int> m_indexCount;
		std::map<const char*, int> m_instanceCount;

		std::map<const char*, ID3D11Buffer*> m_vertexBuffers;
		std::map<const char*, ID3D11Buffer*> m_indexBuffers;

		std::map<const WCHAR*, ID3D11ShaderResourceView*> m_textures;
	};

}