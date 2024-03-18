#ifndef WMCV_OBJ_H_INCLUDED
#define WMCV_OBJ_H_INCLUDED

namespace wmcv
{
	struct ObjVector3
	{
		ObjVector3() = default;
		ObjVector3(float x, float y, float z)
			: f {x, y, z}
		{}

		constexpr auto operator<=>(const ObjVector3&) const = default;

		float f[3];
	};

	struct ObjVector2
	{
		ObjVector2() = default;
		ObjVector2(float x, float y)
			: f {x, y}
		{}

		constexpr auto operator<=>(const ObjVector2&) const = default;

		float f[2];
	};

	struct ObjVertex
	{
		ObjVector3 position;
		ObjVector3 normal;
		ObjVector2 uv;

		constexpr auto operator<=>(const ObjVertex&) const = default;
	};

	struct ObjIndex
	{
		ObjIndex() = default;
		ObjIndex(int64_t ip, int64_t it, int64_t in)
			: i {ip, it, in}
		{}

		constexpr auto operator<=>(const ObjIndex&) const = default;

		int64_t i[3];
	};

	struct ObjSubObject
	{
		ObjSubObject() = default;
		ObjSubObject(const std::string_view n)
			: name(n)
		{}

		std::string name;
		std::string material;
		std::vector<ObjVector3> positions;
		std::vector<ObjVector3> normals;
		std::vector<ObjVector2> texcoords;
		std::vector<ObjIndex> indices;
	};

	struct ObjModel
	{
		std::vector<ObjSubObject> subObjects;
		std::vector<std::string> materialLibraries;
	};

	ObjModel ParseObj(const std::filesystem::path& path);
	ObjModel ParseObj(std::string_view objContents);
	void ExtractMeshData(const ObjSubObject& group, std::vector<ObjVertex>& outVertices, std::vector<uint32_t>& outIndices );
}

#endif //WMCV_OBJ_H_INCLUDED