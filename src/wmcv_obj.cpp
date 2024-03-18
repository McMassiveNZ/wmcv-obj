#include "pch.h"
#include "wmcv_obj/wmcv_obj.h"

namespace wmcv
{
	enum class ObjToken : uint8_t
	{
		Invalid,

		Comment,
		Whitespace,

		Position,
		Normal,
		TexCoord,
		Face,

		Object,
		Group,
		SmoothingGroup,
		MaterialLib,
		UseMaterial,
	};

	struct ObjInfo
	{
		size_t numSubObjects;
		size_t numMaterialLibs;
		std::vector<size_t> numPositions;
		std::vector<size_t> numNormals;
		std::vector<size_t> numUVs;
		std::vector<size_t> numIndices;
		std::vector<size_t> numMaterials;
	};

	inline ObjToken ParseToken(const char* s)
	{
		if (*s == '\0')
			return ObjToken::Invalid;

		if (*s == '#')
			return ObjToken::Comment;

		if ( std::isspace(*s) )
			return ObjToken::Whitespace;

		if (*s == 'f')
			return ObjToken::Face;

		if (*s == 'g')
			return ObjToken::Group;

		if (*s == 'o')
			return ObjToken::Object;

		if (*s == 's')
			return ObjToken::SmoothingGroup;

		if (*s == 'v')
		{
			const char c = *(s + 1);
			if (c == ' ')
				return ObjToken::Position;
			else if (c == 'n')
				return ObjToken::Normal;
			else if (c == 't')
				return ObjToken::TexCoord;
		}

		std::string_view token = [](const char* s)
		{
			const char* end = s;
			while (*end != ' ')
				end++;

			const auto size = std::distance(s, end);
			return std::string_view{ s, static_cast<size_t>(size) };
		}(s);

		if (token == "mtllib")
			return ObjToken::MaterialLib;
		else if (token == "usemtl")
			return ObjToken::UseMaterial;

		return ObjToken::Invalid;
	}

	inline constexpr bool IsNewLine(const char* s)
	{
		return (*s == '\n' || *s == '\r' || *s == '\r\n');
	}

	inline const char* StripLeadingWhiteSpace(const char* s)
	{
		while (std::isspace(*s))
			++s;

		return s;
	}

	inline const char* StripTrailingWhiteSpace(const char* s)
	{
		while (std::isspace(*s))
			--s;

		return s;
	}

	inline const char* EndOfLine(const char* s, bool stripTrailingWhiteSpace = true)
	{
		while (IsNewLine(s) == false)
			++s;

		if (stripTrailingWhiteSpace && s[-1] == ' ')
			s = StripTrailingWhiteSpace(s-1);

		return s;
	}

	inline const char* NextToken(const char* s)
	{
		while (std::isspace(*s))
			++s;
		return s;
	}

	inline const char* NextLine(const char* s)
	{
		s = EndOfLine(s);
		return s + static_cast<size_t>(*s != '\0');
	}


	inline std::tuple<float, float, float> ParseVertexAttribute(const char* s)
	{
		//if its a vt or a vn rather than just a v, go to the space
		if (*(s + 1) != ' ')
			++s;

		const char* begin = StripLeadingWhiteSpace(s + 1);
		const char* end = EndOfLine(begin);
		while (IsNewLine(end) == false)
			++end;

		std::string_view fmt{begin, end};

		float x, y, z;
		sscanf_s(fmt.data(), "%f %f %f", &x, &y, &z);
		return std::make_tuple(x, y, z);
	}

	inline std::string_view ParseMatLib(const char* s)
	{
		const char* begin = s + sizeof("mtllib");
		const char* end = EndOfLine(begin);
		return std::string_view{begin, end};
	}
	
	inline std::string_view ParseUseMtl(const char* s)
	{
		const char* begin = StripLeadingWhiteSpace(s + sizeof("usemtl"));
		const char* end = EndOfLine(begin);
		return std::string_view{begin, end};
	}

	inline std::string_view ParseSubObject(const char* s)
	{
		const char* begin = StripLeadingWhiteSpace(s+1);
		const char* end = EndOfLine(begin);
		return std::string_view{begin, end};
	}

	inline uint32_t ParseNumVerticesPerFace(const char* s)
	{
		const char* begin = StripLeadingWhiteSpace(s + 1);
		const char* end = EndOfLine(begin);

		const auto line = std::string_view{begin, end};
		const bool isQuad = ( std::count(begin, end, ' ') == 3 );
		return isQuad ? 6 : 3;
	}

	inline std::vector<ObjIndex> ParseFace(const char* s)
	{
		const char* begin = StripLeadingWhiteSpace(s + 1);
		const char* end = EndOfLine(begin);

		const auto line = std::string_view{begin, end};
		const bool quads = ( std::count(begin, end, ' ') == 3 );

		std::vector<ObjIndex> result;
		result.reserve(6);

		constexpr auto parseFace = [](const char* itr, ObjIndex& vtx) -> const char*
		{
			constexpr auto nextIndex = [](const char* s) -> const char*
			{
				while (*s != '/')
					++s;

				return ++s;
			};

			vtx.i[0] = std::atoll(itr);
			itr = nextIndex(itr);
			vtx.i[1] = (*itr == '/') ? 0 : std::atoll(itr);
			itr = nextIndex(itr);
			vtx.i[2] = std::atoll(itr);

			while (*itr != '\0' && *itr != ' ')
				++itr;

			if (*itr == ' ')
				++itr;

			return itr;
		};

		constexpr auto nextIndex = [](const char* s) -> const char*
		{
			while (*s != ' ')
				++s;

			return s;
		};

		ObjIndex _0, _1, _2, _3;
		const char* itr = begin;
		itr = parseFace(itr, _0);
		itr = parseFace(itr, _1);
		itr = parseFace(itr, _2);

		result.emplace_back(_0);
		result.emplace_back(_1);
		result.emplace_back(_2);

		if (quads)
		{
			itr = parseFace(itr, _3);
			result.emplace_back(_0);
			result.emplace_back(_2);
			result.emplace_back(_3);
		}

		return result;
	}

	
	class MemoryMappedFile
	{
	public:
		MemoryMappedFile(const char* path)
		{
			if (file = CreateFileA(path,
					GENERIC_READ,
					0,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_READONLY,
					NULL);
				file != INVALID_HANDLE_VALUE)
			{
				if (mapping = CreateFileMappingA(file, NULL, PAGE_READONLY, 0, 0, NULL); mapping != INVALID_HANDLE_VALUE)
				{
					const size_t fileSize = GetFileSize(file, NULL);
					if (void* begin = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, fileSize); begin)
					{
						view = std::string_view{static_cast<char*>(begin), fileSize};
						valid = true;
					}
					else
					{
						valid = false;
					}
				}
			}
		}

		~MemoryMappedFile()
		{
			UnmapViewOfFile(view.data());
			CloseHandle(mapping);
			CloseHandle(file);
		}

		std::string_view contents() const
		{
			return view;
		}

		bool isValid() const
		{
			return valid;
		}

	private:
		HANDLE file = INVALID_HANDLE_VALUE;
		HANDLE mapping = INVALID_HANDLE_VALUE;
		std::string_view view = {};
		bool valid = false;
	};

	ObjInfo ParseInfo(const char* s)
	{
		ObjInfo result{};
		ObjToken token{};

		do
		{
			token = ParseToken(s);
			switch (token)
			{
				case ObjToken::Invalid:
				break;

				case ObjToken::Whitespace:
					s = NextToken(s);
				break;

				case ObjToken::Comment:
				case ObjToken::SmoothingGroup:
					s = NextLine(s);
				break;

				case ObjToken::Group:
				case ObjToken::Object:
				{
					result.numSubObjects++;
					s = NextLine(s);
				} break;

				case ObjToken::Position:
				{
					if ( result.numPositions.empty() || result.numSubObjects > result.numPositions.size())
						result.numPositions.emplace_back();

					result.numPositions.back()++;
					s = NextLine(s);
				} break;

				case ObjToken::Normal:
				{
					if (result.numNormals.empty() || result.numSubObjects > result.numNormals.size())
						result.numNormals.emplace_back();

					result.numNormals.back()++;
					s = NextLine(s);
				}
				break;

				case ObjToken::TexCoord:
				{
					if (result.numUVs.empty() || result.numSubObjects > result.numUVs.size())
						result.numUVs.emplace_back();

					result.numUVs.back()++;
					s = NextLine(s);
				}
				break;

				case ObjToken::Face:
				{
					if (result.numIndices.empty() || result.numSubObjects > result.numIndices.size())
						result.numIndices.emplace_back();

					result.numIndices.back() += ParseNumVerticesPerFace(s);
					s = NextLine(s);
				} break;

				case ObjToken::MaterialLib:
				{
					result.numMaterialLibs++;
					s = NextLine(s);
				} break;

				case ObjToken::UseMaterial:
				{
					if (result.numMaterials.empty() || result.numMaterialLibs > result.numMaterials.size())
						result.numMaterials.emplace_back();

					result.numMaterials.back()++;
					s = NextLine(s);
				} break;

				default:
					s = NextLine(s);
			}
		}
		while (token != ObjToken::Invalid);
		
		return result;
	}

	ObjModel ParseObj(const std::filesystem::path& path)
	{
#ifndef _WIN32 
		std::vector<char> fileContents;
		if (FILE* f = nullptr; fopen_s(&f, path.string().c_str(), "r") == EINVAL)
		{
			return {};
		}
		else
		{
			fseek(f, 0, SEEK_END);
			size_t numBytes = ftell(f);
			fseek(f, 0, SEEK_SET);

			fileContents.resize(numBytes);
			fread(fileContents.data(), 1, numBytes, f);
			fclose(f);
		}
		const char* itr = fileContents.data();
#else
		MemoryMappedFile file(path.string().c_str());
		if (file.isValid() == false)
		{
			return {};
		}
		const char* itr = file.contents().data();
#endif
		return ParseObj(std::string_view{itr});
	}

	ObjModel ParseObj(std::string_view objContents)
	{
		const char* itr = objContents.data();
		ObjInfo info = ParseInfo(itr);
		ObjToken token{};
		ObjModel result{};

		result.materialLibraries.resize(info.numMaterialLibs);
		result.subObjects.resize(info.numSubObjects);

		for (size_t i = 0; i < result.subObjects.size(); ++i)
		{
			auto& subObj = result.subObjects[i];
			subObj.indices.reserve(info.numIndices[i]);
			subObj.positions.reserve(info.numPositions[i]);
			subObj.normals.reserve(info.numNormals[i]);
			subObj.texcoords.reserve(info.numUVs[i]);
		}

		int64_t sobIdx = -1;

		do
		{
			token = ParseToken(itr);
			switch (token)
			{
				case ObjToken::Whitespace:
					itr = NextToken(itr);
				break;

				case ObjToken::Comment:
					itr = NextLine(itr);
				break;

				case ObjToken::SmoothingGroup:
					itr = NextLine(itr);
				break;

				case ObjToken::Group:
				case ObjToken::Object:
				{
					const auto name = ParseSubObject(itr);
					result.subObjects[++sobIdx].name = name;
					itr = NextLine(itr);
				} break;

				case ObjToken::Position:
				{
					auto[x,y,z] = ParseVertexAttribute(itr);
					result.subObjects[sobIdx].positions.emplace_back(x, y, z);
					itr = NextLine(itr);
				} break;

				case ObjToken::Normal:
				{
					auto[x,y,z] = ParseVertexAttribute(itr);
					result.subObjects[sobIdx].normals.emplace_back(x,y,z);
					itr = NextLine(itr);
				}
				break;

				case ObjToken::TexCoord:
				{
					auto[s,t,_] = ParseVertexAttribute(itr);
					result.subObjects[sobIdx].texcoords.emplace_back(s,t);
					itr = NextLine(itr);
				}
				break;

				case ObjToken::Face:
				{
					const auto indices = ParseFace(itr);
					for (const auto& idx : indices)
						result.subObjects[sobIdx].indices.emplace_back(idx);

					itr = NextLine(itr);
				} break;

				case ObjToken::MaterialLib:
				{
					auto matLib = ParseMatLib(itr);
					result.materialLibraries.emplace_back(matLib);
					itr = NextLine(itr);
				} break;

				case ObjToken::UseMaterial:
				{
					result.subObjects[sobIdx].material = ParseUseMtl(itr);
					itr = NextLine(itr);
				} break;
			}
		}
		while (token != ObjToken::Invalid);
		
		return result;
	}

	void ExtractMeshData(const ObjSubObject& group, std::vector<ObjVertex>& outVertices, std::vector<uint32_t>& outIndices  )
	{
		constexpr auto fixupIndex = [](int64_t index, size_t size) -> int64_t
		{
			return (index >= 0) ? index - 1 : int64_t(size) + index;
		};

		uint32_t idxCount = 0;
		std::map<ObjVertex, uint32_t> mappedIndices;
		for (const auto& idx : group.indices)
		{
			const size_t posIdx = fixupIndex(idx.i[0], group.positions.size());
			const size_t texIdx = fixupIndex(idx.i[1], group.texcoords.size());
			const size_t nrmIdx = fixupIndex(idx.i[2], group.normals.size());

			ObjVertex v{
				.position = group.positions[posIdx],
				.normal = group.normals[nrmIdx],
				.uv = group.texcoords[texIdx]
			};

			if (auto itr = mappedIndices.find(v); itr == mappedIndices.end())
			{
				mappedIndices.insert({v, idxCount});
				outIndices.push_back(idxCount++);
				outVertices.push_back(v);
			}
			else
			{
				outIndices.push_back(itr->second);
			}
		}
	}
}