#ifndef DAEEXPORTER_H
#define DAEEXPORTER_H
#include "pugixml.hpp"
#include "pugiconfig.hpp"
#include "rmv2parser.h"
#include "AnimParser.h"

class DaeExporter
{
public:

	DaeExporter(const std::string&);
	DaeExporter(const std::string&, const Mesh&);
	DaeExporter(const DaeExporter&) = delete;
	DaeExporter(DaeExporter&&) = delete;
	DaeExporter& operator=(const DaeExporter&) = delete;
	DaeExporter& operator=(DaeExporter&&) = delete;
	~DaeExporter();

	bool ExportMesh();
	bool ExportMeshAnim();

private:

	void WriteNode(const uint32_t&, const Skeleton&, pugi::xml_node&);
	void WriteHeader();
	void WriteGeometry();
	void WriteControllers();
	void WriteVisualScenes();
	void WriteScenes();

	Mesh mMesh;
	std::string mFilename;
	pugi::xml_document doc;
	pugi::xml_node collada;
};

#endif	//	!DAEEXPORTER_H