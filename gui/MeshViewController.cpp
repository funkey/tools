#include "MeshViewController.h"
#include <sg_gui/MarchingCubes.h>
#include <util/ProgramOptions.h>
#include <util/Logger.h>
#include <fstream>

logger::LogChannel meshviewcontrollerlog("meshviewcontrollerlog", "[MeshViewController] ");

util::ProgramOption optionCubeSize(
		util::_long_name        = "cubeSize",
		util::_description_text = "The size of a cube for the marching cubes visualization.",
		util::_default_value    = 10);

MeshViewController::MeshViewController(std::shared_ptr<ExplicitVolume<float>> labels) :
	_labels(labels),
	_meshes(std::make_shared<sg_gui::Meshes>()) {}

void
MeshViewController::onSignal(sg_gui::VolumePointSelected& signal) {

	unsigned int x, y, z;
	_labels->getDiscreteCoordinates(
			signal.position().x(),
			signal.position().y(),
			signal.position().z(),
			x, y, z);

	float label = (*_labels)(x, y, z);

	if (label == 0)
		return;

	LOG_DEBUG(meshviewcontrollerlog) << "selected label " << label << std::endl;

	if (_meshes->contains(label)) {

		removeMesh(label);

	} else {

		addMesh(label);
	}

	send<sg_gui::SetMeshes>(_meshes);
}

void
MeshViewController::onSignal(sg_gui::KeyDown& signal) {

	if (signal.key == sg_gui::keys::I) {

		LOG_USER(meshviewcontrollerlog) << "enter label to show: " << std::endl;

		char input[256];
		std::cin.getline(input, 256);

		try {

			float label = boost::lexical_cast<float>(input);
			addMesh(label);
			send<sg_gui::SetMeshes>(_meshes);

		} catch (std::exception& e) {

			LOG_ERROR(meshviewcontrollerlog) << "invalid input" << std::endl;
			return;
		}
	}

	if (signal.key == sg_gui::keys::F8) {

		LOG_USER(meshviewcontrollerlog) << "exporting currently visible meshes" << std::endl;

		for (unsigned int id : _meshes->getMeshIds())
			exportMesh(id);
	}
}

void
MeshViewController::addMesh(float label) {

	LOG_USER(meshviewcontrollerlog) << "showing label " << label << std::endl;

	if (_meshCache.count(label)) {

		_meshes->add(label, _meshCache[label]);
		return;
	}

	typedef ExplicitVolumeLabelAdaptor<ExplicitVolume<float>> Adaptor;
	Adaptor adaptor(*_labels, label);

	sg_gui::MarchingCubes<Adaptor> marchingCubes;
	std::shared_ptr<sg_gui::Mesh> mesh = marchingCubes.generateSurface(
			adaptor,
			sg_gui::MarchingCubes<Adaptor>::AcceptAbove(0),
			optionCubeSize,
			optionCubeSize,
			optionCubeSize);
	_meshes->add(label, mesh);

	_meshCache[label] = mesh;
}

void
MeshViewController::removeMesh(float label) {

	_meshes->remove(label);
}

void
MeshViewController::exportMesh(unsigned int id) {

	std::stringstream filename;
	filename << "mesh_" << id << ".raw";

	std::ofstream file(filename.str().c_str());
	std::shared_ptr<sg_gui::Mesh> mesh = _meshes->get(id);

	for (int i = 0; i < mesh->getNumTriangles(); i++) {

		const sg_gui::Triangle& triangle = mesh->getTriangle(i);

		for (const sg_gui::Point3d& v : {
				mesh->getVertex(triangle.v0),
				mesh->getVertex(triangle.v1),
				mesh->getVertex(triangle.v2) }) {

			file << v.x() << " " << v.y() << " " << v.z() << " ";
		}
		file << std::endl;
	}
}
