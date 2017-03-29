#include "MeshViewController.h"
#include <sg_gui/MarchingCubes.h>
#include <util/ProgramOptions.h>
#include <util/Logger.h>
#include <fstream>
#include <thread>

logger::LogChannel meshviewcontrollerlog("meshviewcontrollerlog", "[MeshViewController] ");

util::ProgramOption optionCubeSize(
		util::_long_name        = "cubeSize",
		util::_description_text = "The size of a cube for the marching cubes visualization.",
		util::_default_value    = 10);

MeshViewController::MeshViewController(std::shared_ptr<ExplicitVolume<float>> labels) :
	_labels(labels),
	_meshes(std::make_shared<sg_gui::Meshes>()),
	_minCubeSize(optionCubeSize) {}

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

	bool meshesChanged = false;
	{
		LockGuard guard(*_meshes);

		if (_meshes->contains(label)) {

			removeMesh(label);
			meshesChanged = true;

		} else {

			// meshes are added asynchronously, meshesChanged not true yet
			addMesh(label);
		}
	}

	if (meshesChanged)
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
			{
				LockGuard guard(*_meshes);
				addMesh(label);
			}
			send<sg_gui::SetMeshes>(_meshes);

		} catch (std::exception& e) {

			LOG_ERROR(meshviewcontrollerlog) << "invalid input" << std::endl;
			return;
		}
	}

	if (signal.key == sg_gui::keys::F8) {

		LOG_USER(meshviewcontrollerlog) << "exporting currently visible meshes" << std::endl;

		exportMeshes();
	}
}

void
MeshViewController::addMesh(float label) {

	// _meshes locked by caller

	LOG_USER(meshviewcontrollerlog) << "showing label " << label << std::endl;

	if (_meshCache.count(label)) {

		_meshes->add(label, _meshCache[label]);
		return;
	}

	typedef ExplicitVolumeLabelAdaptor<ExplicitVolume<float>> Adaptor;

	for (float downsample : {32, 16, 8, 4, 2, 1}) {

		auto extractMesh = 
				std::packaged_task<std::shared_ptr<sg_gui::Mesh>()>(
						[this, label, downsample]() {

							Adaptor adaptor(*this->_labels, label);
							float cubeSize = this->_minCubeSize*downsample;

							sg_gui::MarchingCubes<Adaptor> marchingCubes;
							std::shared_ptr<sg_gui::Mesh> mesh = marchingCubes.generateSurface(
									adaptor,
									sg_gui::MarchingCubes<Adaptor>::AcceptAbove(0),
									cubeSize,
									cubeSize,
									cubeSize);

							this->notifyMeshExtracted(mesh, label);

							return mesh;
						}
				);

		if (downsample == 1)
			_highresMeshFutures.push_back(extractMesh.get_future());

		std::thread(std::move(extractMesh)).detach();
	}
}

void
MeshViewController::notifyMeshExtracted(std::shared_ptr<sg_gui::Mesh> mesh, float label) {

	{
		LockGuard guard(*_meshes);

		// don't replace existing mesh with lower resolution mesh
		if (_meshes->contains(label))
			if (_meshes->get(label)->getNumVertices() > mesh->getNumVertices())
				return;

		_meshes->add(label, mesh);
		_meshCache[label] = mesh;
	}

	send<sg_gui::SetMeshes>(_meshes);
}

void
MeshViewController::removeMesh(float label) {

	// _meshes locked by caller

	_meshes->remove(label);
}

void
MeshViewController::exportMeshes() {

	std::vector<unsigned int> currentMeshIds;
	std::vector<std::future<std::shared_ptr<sg_gui::Mesh>>> pendingFutures;

	{
		LockGuard guard(*_meshes);

		// get IDs of currently visible meshes
		currentMeshIds = _meshes->getMeshIds();

		// get all currently pending high-res mesh futures
		std::swap(pendingFutures, _highresMeshFutures);
	}

	// make sure all pending high-res meshes are done
	for (auto& future : pendingFutures)
		future.get();

	LockGuard guard(*_meshes);

	for (int id : currentMeshIds) {

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
}
