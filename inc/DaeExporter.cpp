#include "pch.h"
#include "DaeExporter.h"
#include <ctime>

#define radiansToDegrees(angleRadians) ((angleRadians) * 180.0 / DirectX::XM_PI)

using namespace std;
using namespace pugi;
using namespace DirectX;

const float scaleFactor = 39.37008f;

DaeExporter::DaeExporter(const string& _filename) :
	mFilename(_filename)
{
}

DaeExporter::DaeExporter(const string& _filename, const Mesh& _mesh) :
	mFilename(_filename),
	mMesh(_mesh)
{
}

DaeExporter::~DaeExporter()
{
}

bool DaeExporter::ExportMesh()
{
	WriteHeader();
	WriteGeometry();
	WriteVisualScenes();
	WriteScenes();

	doc.save_file(mFilename.c_str());
	return true;
}

bool DaeExporter::ExportMeshAnim()
{
	ofstream file("data\\exported\\" + mFilename, ios::out);
	if (file.is_open())
	{

	}
	else
		return false;

	file.close();
	return true;
}

void DaeExporter::WriteHeader()
{
	xml_node decl = doc.prepend_child(node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "utf-8";

	collada = doc.append_child("COLLADA");
	collada.append_attribute("xmlns") = "http://www.collada.org/2005/11/COLLADASchema";
	collada.append_attribute("version") = "1.4.1";

	xml_node asset = collada.append_child("asset");
	xml_node contrib = asset.append_child("contributor");
	contrib.append_child("author").text().set("Mr.Jox");
	contrib.append_child("authoring_tool").text().set("RMV2 COLLADA exporter");
	contrib.append_child("comments").text().set("Exported with RMV2 Converter v2.0");

	time_t rawtime;
	struct tm timeinfo;
	char buffer[80];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	strftime(buffer, sizeof(buffer), "%Y-%m-%dT%I:%M:%Sz", &timeinfo);

	asset.append_child("created").text().set(buffer);
	asset.append_child("keywords");
	asset.append_child("modified").text().set(buffer);
	asset.append_child("revision");
	asset.append_child("subject");
	asset.append_child("title");
	xml_node unit = asset.append_child("unit");
	unit.append_attribute("meter") = "0.025400";
	unit.append_attribute("name") = "centimeter";
	asset.append_child("up_axis").text().set("Y_UP");
}

void DaeExporter::WriteGeometry()
{
	xml_node lib_geo = collada.append_child("library_geometries");
	size_t totalLods = mMesh.GetLodsCount();

	for (uint8_t lodNum = 0; lodNum < totalLods; ++lodNum)
	{
		for (uint8_t groupNum = 0; groupNum < mMesh.GetGroupsCount(lodNum); ++groupNum)
		{
			string groupName;
			xml_node geo = lib_geo.append_child("geometry");
			if (groupNum > 0)
			{
				if (mMesh.GetGroupName(lodNum, groupNum) == mMesh.GetGroupName(lodNum, groupNum - 1))
					groupName = mMesh.GetGroupName(lodNum, groupNum) + "_" + to_string(groupNum) + "_lod" + to_string(lodNum + 1);
				else
					groupName = mMesh.GetGroupName(lodNum, groupNum) + "_lod" + to_string(lodNum + 1);
			}
			else
				groupName = mMesh.GetGroupName(lodNum, groupNum) + "_lod" + to_string(lodNum + 1);
			
			geo.append_attribute("id") = (groupName + "-lib").c_str();
			geo.append_attribute("name") = (groupName + "Mesh").c_str();

			vector<Vertex> verticesArray;
			mMesh.GetVerticesArray(lodNum, groupNum, &verticesArray);
			string vPosArray;
			string vNormArray;
			string vCoordsArray;
			string vCoords2Array;
			size_t vcPerGroup = mMesh.GetVerticesCountPerGroup(lodNum, groupNum);
			RigidMaterial rmat = mMesh.GetRigidMaterialStruct(lodNum, groupNum);

			for (size_t vertexID = 0; vertexID < vcPerGroup; ++vertexID)
			{
				vPosArray += "\n" + to_string(verticesArray[vertexID].position.x * scaleFactor) + " " + to_string(verticesArray[vertexID].position.y * scaleFactor) + " " + to_string(verticesArray[vertexID].position.z * scaleFactor);
				vNormArray += "\n" + to_string(verticesArray[vertexID].normal.x) + " " + to_string(verticesArray[vertexID].normal.y) + " " + to_string(verticesArray[vertexID].normal.z);
				vCoordsArray += "\n" + to_string(verticesArray[vertexID].texCoord.x) + " " + to_string(verticesArray[vertexID].texCoord.y);
				if (rmat == RigidMaterial::ship_ambientmap || rmat == RigidMaterial::tiled_dirtmap)
					vCoords2Array += "\n" + to_string(verticesArray[vertexID].texCoord2.x) + " " + to_string(verticesArray[vertexID].texCoord2.y);
			}
			vPosArray += "\n";
			vNormArray += "\n";
			vCoordsArray += "\n";

			xml_node mesh = geo.append_child("mesh");
			xml_node source = mesh.append_child("source");
			source.append_attribute("id") = (groupName + "-POSITION").c_str();

			xml_node floatArr = source.append_child("float_array");
			floatArr.text().set(vPosArray.c_str());
			floatArr.append_attribute("id") = (groupName + "-POSITION-array").c_str();
			floatArr.append_attribute("count") = vcPerGroup * 3;
	
			xml_node tech = source.append_child("technique_common");
			xml_node accessor = tech.append_child("accessor");
			accessor.append_attribute("source") = ("#" + groupName + "-POSITION-array").c_str();
			accessor.append_attribute("count") = vcPerGroup;
			accessor.append_attribute("stride") = 3;
			xml_node param = accessor.append_child("param");
			param.append_attribute("name") = "X";
			param.append_attribute("type") = "float";
			param = accessor.append_child("param");
			param.append_attribute("name") = "Y";
			param.append_attribute("type") = "float";
			param = accessor.append_child("param");
			param.append_attribute("name") = "Z";
			param.append_attribute("type") = "float";
	
			source = mesh.append_child("source");
			source.append_attribute("id") = (groupName + "-Normal0").c_str();

			floatArr = source.append_child("float_array");
			floatArr.text().set(vNormArray.c_str());
			floatArr.append_attribute("id") = (groupName + "-Normal0-array").c_str();
			floatArr.append_attribute("count") = vcPerGroup * 3;

			tech = source.append_child("technique_common");
			accessor = tech.append_child("accessor");
			accessor.append_attribute("source") = ("#" + groupName + "-Normal0-array").c_str();
			accessor.append_attribute("count") = vcPerGroup;
			accessor.append_attribute("stride") = 3;
			param = accessor.append_child("param");
			param.append_attribute("name") = "X";
			param.append_attribute("type") = "float";
			param = accessor.append_child("param");
			param.append_attribute("name") = "Y";
			param.append_attribute("type") = "float";
			param = accessor.append_child("param");
			param.append_attribute("name") = "Z";
			param.append_attribute("type") = "float";

			source = mesh.append_child("source");
			source.append_attribute("id") = (groupName + "-UV0").c_str();

			floatArr = source.append_child("float_array");
			floatArr.text().set(vCoordsArray.c_str());
			floatArr.append_attribute("id") = (groupName + "-UV0-array").c_str();
			floatArr.append_attribute("count") = vcPerGroup * 2;

			tech = source.append_child("technique_common");
			accessor = tech.append_child("accessor");
			accessor.append_attribute("source") = ("#" + groupName + "-UV0-array").c_str();
			accessor.append_attribute("count") = vcPerGroup;
			accessor.append_attribute("stride") = 2;
			param = accessor.append_child("param");
			param.append_attribute("name") = "S";
			param.append_attribute("type") = "float";
			param = accessor.append_child("param");
			param.append_attribute("name") = "T";
			param.append_attribute("type") = "float";

			if (rmat == RigidMaterial::ship_ambientmap || rmat == RigidMaterial::tiled_dirtmap)
			{
				source = mesh.append_child("source");
				source.append_attribute("id") = (groupName + "-UV1").c_str();

				floatArr = source.append_child("float_array");
				floatArr.text().set(vCoords2Array.c_str());
				floatArr.append_attribute("id") = (groupName + "-UV1-array").c_str();
				floatArr.append_attribute("count") = vcPerGroup * 2;

				tech = source.append_child("technique_common");
				accessor = tech.append_child("accessor");
				accessor.append_attribute("source") = ("#" + groupName + "-UV1-array").c_str();
				accessor.append_attribute("count") = vcPerGroup;
				accessor.append_attribute("stride") = 2;
				param = accessor.append_child("param");
				param.append_attribute("name") = "S";
				param.append_attribute("type") = "float";
				param = accessor.append_child("param");
				param.append_attribute("name") = "T";
				param.append_attribute("type") = "float";
			}

			xml_node vertices = mesh.append_child("vertices");
			vertices.append_attribute("id") = (groupName + "-VERTEX").c_str();
			xml_node input = vertices.append_child("input");
			input.append_attribute("semantic") = "POSITION";
			input.append_attribute("source") = ("#" + groupName + "-POSITION").c_str();
			input = vertices.append_child("input");
			input.append_attribute("semantic") = "NORMAL";
			input.append_attribute("source") = ("#" + groupName + "-Normal0").c_str();
			input = vertices.append_child("input");
			input.append_attribute("semantic") = "TEXCOORD";
			input.append_attribute("source") = ("#" + groupName + "-UV0").c_str();

			if (rmat == RigidMaterial::ship_ambientmap || rmat == RigidMaterial::tiled_dirtmap)
			{
				input = vertices.append_child("input");
				input.append_attribute("semantic") = "TEXCOORD";
				input.append_attribute("source") = ("#" + groupName + "-UV1").c_str();
			}

			vector<Triangle> indicesArray;
			mMesh.GetIndicesArray(lodNum, groupNum, &indicesArray);
			string indArr;
			size_t icPerGroup = mMesh.GetIndicesCountPerGroup(lodNum, groupNum) / 3;
			for (size_t traingleID = 0; traingleID < icPerGroup; ++traingleID)
				indArr += to_string(indicesArray[traingleID].index1) + " " + to_string(indicesArray[traingleID].index2) + " " + to_string(indicesArray[traingleID].index3) + " ";

			xml_node triangles = mesh.append_child("triangles");
			triangles.append_attribute("count") = icPerGroup;
			input = triangles.append_child("input");
			input.append_attribute("semantic") = "VERTEX";
			input.append_attribute("source") = ("#" + groupName + "-VERTEX").c_str();
			input.append_attribute("offset") = 0;
			triangles.append_child("p").text().set(indArr.c_str());
		}
	}
}

void DaeExporter::WriteControllers()
{

}

void DaeExporter::WriteVisualScenes()
{
	size_t totalLods = mMesh.GetLodsCount();

	xml_node lib_vis = collada.append_child("library_visual_scenes");
	xml_node vis_scene = lib_vis.append_child("visual_scene");
	vis_scene.append_attribute("id") = "VisualScene";
	vis_scene.append_attribute("name") = "VisualScene";

	for (uint8_t lodNum = 0; lodNum < totalLods; ++lodNum)
	{
		for (uint8_t groupNum = 0; groupNum < mMesh.GetGroupsCount(lodNum); ++groupNum)
		{
			string groupName;
			xml_node geoNode = vis_scene.append_child("node");

			if (groupNum > 0)
			{
				if (mMesh.GetGroupName(lodNum, groupNum) == mMesh.GetGroupName(lodNum, groupNum - 1))
					groupName = mMesh.GetGroupName(lodNum, groupNum) + "_" + to_string(groupNum) + "_lod" + to_string(lodNum + 1);
				else
					groupName = mMesh.GetGroupName(lodNum, groupNum) + "_lod" + to_string(lodNum + 1);
			}
			else
				groupName = mMesh.GetGroupName(lodNum, groupNum) + "_lod" + to_string(lodNum + 1);

			geoNode.append_attribute("name") = groupName.c_str();
			geoNode.append_attribute("id") = groupName.c_str();
			geoNode.append_attribute("sid") = groupName.c_str();
			xml_node geoInst = geoNode.append_child("instance_geometry");
			geoInst.append_attribute("url") = ("#" + groupName + "-lib").c_str();
			xml_node tech = geoNode.append_child("extra").append_child("technique");
			tech.append_attribute("profile") = "FCOLLADA";
			tech.append_child("visibility").text().set("1.000000");
		}
	}

	string skelName = mMesh.GetSkeletonName();
	if (!skelName.empty() || skelName != "building")
	{
		Skeleton skeleton;
		if (skelName[0] != '\0')
		{
			string skelpath = mFilename.substr(0, mFilename.find_last_of("/\\"));
			skelpath = skelpath.substr(0, skelpath.find_last_of("/\\")) + "\\skeletons\\" + skelName + ".anim";
			if (!skeleton.Read(skelpath))
			{
				MessageBoxA(nullptr, "Error: The skeleton .anim file wasn't found in $AppDir/skeletons/", "Skeleton not found", MB_OK);
				return;
			}
		}

		uint32_t rBonesCount = skeleton.GetRootBonesCount();
		for (uint32_t boneID = 0; boneID < rBonesCount; ++boneID)
			WriteNode(boneID, skeleton, vis_scene);
	}

	xml_node tech = vis_scene.append_child("extra").append_child("technique");
	tech.append_attribute("profile") = "MAX3D";
	tech.append_child("frame_rate").text().set("30.000000");
	tech = vis_scene.append_child("extra").append_child("technique");
	tech.append_attribute("profile") = "FCOLLADA";
	tech.append_child("start_time").text().set("0.000000");
	tech.append_child("end_time").text().set("3.333333");
}

void DaeExporter::WriteNode(const uint32_t& _boneID, const Skeleton& _skeleton, xml_node& parent_node)
{
	xml_node node = parent_node.append_child("node");
	string boneName = _skeleton.GetBoneName(_boneID);
	node.append_attribute("name") = boneName.c_str();
	node.append_attribute("id") = boneName.c_str();
	node.append_attribute("layer") = "FBX_Import";
	node.append_attribute("sid") = boneName.c_str();
	node.append_attribute("type") = "JOINT";
	
	xml_node transf = node.append_child("translate");
	transf.append_attribute("sid") = "translate";
	XMFLOAT3 pos = _skeleton.GetPosByID(_boneID);
	transf.text().set((to_string(pos.x * scaleFactor) + " " + to_string(pos.y * scaleFactor) + " " + to_string(pos.z * scaleFactor)).c_str());
	
	if (_skeleton.GetBoneParentID(_boneID) == -1)
	{
		transf = node.append_child("rotate");
		transf.append_attribute("sid") = "jointOrientX";
		transf.text().set("1 0 0 -90.000000");
	}

	XMFLOAT3 rot;
	XMStoreFloat3(&rot, _skeleton.GetRotationVectorByID(_boneID));

	transf = node.append_child("rotate");
	transf.append_attribute("sid") = "rotateZ";
	transf.text().set(("0 0 1 " + to_string(radiansToDegrees(rot.z))).c_str());
	
	transf = node.append_child("rotate");
	transf.append_attribute("sid") = "rotateY";
	transf.text().set(("0 1 0 " + to_string(radiansToDegrees(rot.y))).c_str());

	transf = node.append_child("rotate");
	transf.append_attribute("sid") = "rotateX";
	transf.text().set(("1 0 0 " + to_string(radiansToDegrees(rot.x))).c_str());

	if (_skeleton.GetBoneParentID(_boneID) == -1)
	{
		transf = node.append_child("scale");
		transf.append_attribute("sid") = "scale";
		transf.text().set("-1.000000 -1.000000 -1.000000");
	}

	xml_node tech = node.append_child("extra").append_child("technique");
	tech.append_attribute("profile") = "FCOLLADA";
	tech.append_child("visibility").text().set("1.000000");

	uint32_t childsCount = _skeleton.GetChildBonesCount(_boneID);
	if (childsCount > 0)
	{
		vector<uint32_t> childIDs;
		_skeleton.GetChildIDs(_boneID, &childIDs);
		for (size_t id = 0; id < childsCount; ++id)
			WriteNode(childIDs[id], _skeleton, node);
	}
}

void DaeExporter::WriteScenes()
{
	collada.append_child("scene").append_child("instance_visual_scene").append_attribute("url") = "#VisualScene";
}