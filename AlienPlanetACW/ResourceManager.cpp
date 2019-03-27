#include "pch.h"
#include "ResourceManager.h"

using namespace AlienPlanetACW;

ResourceManager::ResourceManager()
{
}


ResourceManager::~ResourceManager()
{
	try
	{
		for (auto& shaderResource : m_textures)
		{
			shaderResource.second->Release();
			shaderResource.second = nullptr;
		}

		for (auto& buffer : m_indexBuffers)
		{
			buffer.second->Release();
			buffer.second = nullptr;
		}

		for (auto& buffer : m_vertexBuffers)
		{
			buffer.second->Release();
			buffer.second = nullptr;
		}
	}
	catch (std::exception& e)
	{

	}
}

bool ResourceManager::GetModel(ID3D11Device* const device, const char* const modelFileName, ID3D11Buffer* &vertexBuffer, ID3D11Buffer* &indexBuffer)
{
	if (0 == m_vertexBuffers.count(modelFileName))
	{
		auto const result = LoadModel(device, modelFileName);

		if (!result)
		{
			return false;
		}
	}

	vertexBuffer = m_vertexBuffers.at(modelFileName);
	indexBuffer = m_indexBuffers.at(modelFileName);

	return true;
}

bool ResourceManager::GetModel(ID3D11Device* const device, const char* const modelFileName, Microsoft::WRL::ComPtr<ID3D11Buffer> &vertexBuffer, Microsoft::WRL::ComPtr<ID3D11Buffer> &indexBuffer)
{
	if (0 == m_vertexBuffers.count(modelFileName))
	{
		auto const result = LoadModel(device, modelFileName);

		if (!result)
		{
			return false;
		}
	}

	vertexBuffer = m_vertexBuffers.at(modelFileName);
	indexBuffer = m_indexBuffers.at(modelFileName);

	return true;
}

bool ResourceManager::GetTexture(ID3D11Device* const device, const WCHAR* const textureFileName, ID3D11ShaderResourceView* &texture)
{
	if (0 == m_textures.count(textureFileName))
	{
		auto const result = LoadTexture(device, textureFileName);

		if (!result)
		{
			return false;
		}
	}

	texture = m_textures.at(textureFileName);

	return true;
}

int ResourceManager::GetSizeOfVertexType() const {
	return sizeof(AlienPlanetACW::VertexPositionTexcoordNormalTangentBinormal);
}

int ResourceManager::GetIndexCount(const char* const modelFileName) const {
	return m_indexCount.at(modelFileName);
}

bool ResourceManager::LoadModel(ID3D11Device* const device, const char* const modelFileName)
{
	//Load Model
	std::ifstream fin;

	//Open obj file
	fin.open(modelFileName);

	if (fin.fail())
	{
		return false;
	}

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	std::vector<DirectX::XMFLOAT3> positions;
	std::vector<DirectX::XMFLOAT2> textures;
	std::vector<DirectX::XMFLOAT3> normals;

	char cmd[256] = { 0 };

	auto faceCount = 0;
	auto vertexCount = 0;
	auto indexCount = 0;

	VertexPositionTexcoordNormalTangentBinormal* vertices = nullptr;
	unsigned long* indices = nullptr;

	while (!fin.eof())
	{
		float x, y, z;

		fin >> cmd;

		if (0 == strcmp(cmd, "faces"))
		{
			fin >> faceCount;
			vertices = new VertexPositionTexcoordNormalTangentBinormal[faceCount * 3];

			if (!vertices)
			{
				return false;
			}

			vertexCount = faceCount * 3;

			indices = new unsigned long[faceCount * 3];

			if (!indices)
			{
				return false;
			}

			indexCount = faceCount * 3;
		}

		if (0 == strcmp(cmd, "v"))
		{
			fin >> x >> y >> z;
			positions.emplace_back(DirectX::XMFLOAT3(x, y, z));
		}
		else if (0 == strcmp(cmd, "vn"))
		{
			fin >> x >> y >> z;
			normals.emplace_back(DirectX::XMFLOAT3(x, y, z));
		}
		else if (0 == strcmp(cmd, "vt"))
		{
			fin >> x >> y >> z;
			textures.emplace_back(DirectX::XMFLOAT2(x, y));
		}
		else if (0 == strcmp(cmd, "f"))
		{
			int value;
			auto count = 0;

			while (0 == strcmp(cmd, "f"))
			{
				VertexPositionTexcoordNormalTangentBinormal* tempVertexFace[3];

				for (auto i = 0; i < 3; i++)
				{
					fin >> value;
					vertices[count].position = (positions[value - 1]);
					fin.ignore();

					fin >> value;
					vertices[count].texcoord = (textures[value - 1]);
					fin.ignore();

					fin >> value;
					vertices[count].normal = (normals[value - 1]);
					fin.ignore();

					indices[count] = count;

					tempVertexFace[i] = &vertices[count];

					count++;
				}

				//Calculate the tangent and binormal

				auto positionOne = DirectX::XMFLOAT3();
				auto positionTwo = DirectX::XMFLOAT3();
				auto textureOne = DirectX::XMFLOAT2();
				auto textureTwo = DirectX::XMFLOAT2();

				auto tangent = DirectX::XMFLOAT3();
				auto binormal = DirectX::XMFLOAT3();

				//Calculate the two vertex positions from the face
				positionOne.x = tempVertexFace[1]->position.x - tempVertexFace[0]->position.x;
				positionOne.y = tempVertexFace[1]->position.y - tempVertexFace[0]->position.y;
				positionOne.z = tempVertexFace[1]->position.z - tempVertexFace[0]->position.z;

				positionTwo.x = tempVertexFace[2]->position.x - tempVertexFace[0]->position.x;
				positionTwo.y = tempVertexFace[2]->position.y - tempVertexFace[0]->position.y;
				positionTwo.z = tempVertexFace[2]->position.z - tempVertexFace[0]->position.z;

				//Calculate the two texture coords from the face
				textureOne.x = tempVertexFace[1]->texcoord.x - tempVertexFace[0]->texcoord.x;
				textureOne.y = tempVertexFace[1]->texcoord.y - tempVertexFace[0]->texcoord.y;

				textureTwo.x = tempVertexFace[2]->texcoord.x - tempVertexFace[0]->texcoord.x;
				textureTwo.y = tempVertexFace[2]->texcoord.y - tempVertexFace[0]->texcoord.y;

				//Calculate the denominator of the tangent/binormal (This is so we don't have to normalize after, we can do it as we go along
				const auto denominator = 1.0f / (textureOne.x * textureTwo.y - textureTwo.x * textureOne.y);

				//Calculate the cross products and scale it by our denominator to get the normalize tangent and binormal
				tangent.x = (textureTwo.y * positionOne.x - textureOne.y * positionTwo.x) * denominator;
				tangent.y = (textureTwo.y * positionOne.y - textureOne.y * positionTwo.y) * denominator;
				tangent.z = (textureTwo.y * positionOne.z - textureOne.y * positionTwo.z) * denominator;

				binormal.x = (textureOne.x * positionTwo.x - textureTwo.x * positionOne.x) * denominator;
				binormal.y = (textureOne.x * positionTwo.y - textureTwo.x * positionOne.y) * denominator;
				binormal.z = (textureOne.x * positionTwo.z - textureTwo.x * positionOne.z) * denominator;

				//Calculate the length of the tangent normal
				auto length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

				//Normalize our tangent based off the length
				tangent.x = tangent.x / length;
				tangent.y = tangent.y / length;
				tangent.z = tangent.z / length;

				//Calculate the length of the binormal
				length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

				//Normalize it
				binormal.x = binormal.x / length;
				binormal.y = binormal.y / length;
				binormal.z = binormal.z / length;

				//Calculate new normal based off the tangent and binormal
				auto newNormal = DirectX::XMFLOAT3();

				//Do a cross product between the tangent and binormal to get the new normal
				newNormal.x = (tangent.y * binormal.z) - (tangent.z * binormal.y);
				newNormal.y = (tangent.z * binormal.x) - (tangent.x * binormal.z);
				newNormal.z = (tangent.x * binormal.y) - (tangent.y * binormal.x);

				//Calculate length of normal
				length = sqrt((newNormal.x * newNormal.x) + (newNormal.y * newNormal.y) + (newNormal.z * newNormal.z));

				//Normalize it
				newNormal.x = newNormal.x / length;
				newNormal.y = newNormal.y / length;
				newNormal.z = newNormal.z / length;

				//Store new normal, tangent and binormal back into the face
				tempVertexFace[0]->normal = newNormal;
				tempVertexFace[0]->tangent = tangent;
				tempVertexFace[0]->binormal = binormal;
				tempVertexFace[0] = nullptr;

				tempVertexFace[1]->normal = newNormal;
				tempVertexFace[1]->tangent = tangent;
				tempVertexFace[1]->binormal = binormal;
				tempVertexFace[1] = nullptr;

				tempVertexFace[2]->normal = newNormal;
				tempVertexFace[2]->tangent = tangent;
				tempVertexFace[2]->binormal = binormal;
				tempVertexFace[2] = nullptr;

				fin >> cmd;
			}
		}
	}

	//Initialize buffers
	D3D11_BUFFER_DESC vertexBufferDescription;
	D3D11_SUBRESOURCE_DATA vertexData;

	//Initialize vertex and index descriptions and then create buffers
	vertexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDescription.ByteWidth = sizeof(VertexPositionTexcoordNormalTangentBinormal) * vertexCount;
	vertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDescription.CPUAccessFlags = 0;
	vertexBufferDescription.MiscFlags = 0;
	vertexBufferDescription.StructureByteStride = 0;

	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	auto result = device->CreateBuffer(&vertexBufferDescription, &vertexData, &vertexBuffer);

	if (FAILED(result))
	{
		return false;
	}

	D3D11_BUFFER_DESC indexBufferDescription;

	indexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDescription.ByteWidth = sizeof(unsigned long) * indexCount;
	indexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDescription.CPUAccessFlags = 0;
	indexBufferDescription.MiscFlags = 0;
	indexBufferDescription.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;

	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&indexBufferDescription, &indexData, &indexBuffer);

	if (FAILED(result))
	{
		return false;
	}

	m_indexCount.insert(std::pair<const char*, int>(modelFileName, indexCount));

	m_vertexBuffers.insert(std::pair<const char*, ID3D11Buffer*>(modelFileName, vertexBuffer));
	m_indexBuffers.insert(std::pair<const char*, ID3D11Buffer*>(modelFileName, indexBuffer));

	vertexBuffer = nullptr;
	indexBuffer = nullptr;

	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}

bool ResourceManager::LoadTexture(ID3D11Device* const device, const WCHAR* textureFileName)
{
	ID3D11ShaderResourceView* texture;

	const auto result = DirectX::CreateDDSTextureFromFile(device, textureFileName, nullptr, &texture);

	if (SUCCEEDED(result))
	{
		m_textures.insert(std::pair<const WCHAR*, ID3D11ShaderResourceView*>(textureFileName, texture));
		texture = nullptr;

		return true;
	}
	else
	{
		delete texture;
		texture = nullptr;
		return false;
	}
}