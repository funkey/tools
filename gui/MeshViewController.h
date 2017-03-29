#ifndef CANDIDATE_MC_GUI_MESH_VIEW_CONTROLLER_H__
#define CANDIDATE_MC_GUI_MESH_VIEW_CONTROLLER_H__

#include <scopegraph/Agent.h>
#include <sg_gui/VolumeView.h>
#include <sg_gui/MeshView.h>
#include <sg_gui/Meshes.h>
#include <sg_gui/KeySignals.h>
#include <future>

/**
 * A marching cubes adaptor that binarizes an explicit volume by reporting 1 for 
 * a given label and 0 otherwise.
 */
template <typename EV>
class ExplicitVolumeLabelAdaptor {

public:

	typedef typename EV::value_type value_type;

	ExplicitVolumeLabelAdaptor(const EV& ev, value_type label) :
		_ev(ev),
		_label(label) {}

	const util::box<float,3>& getBoundingBox() const { return _ev.getBoundingBox(); }

	float operator()(float x, float y, float z) const {

		if (!getBoundingBox().contains(x, y, z))
			return 0;

		unsigned int dx, dy, dz;

		_ev.getDiscreteCoordinates(x, y, z, dx, dy, dz);

		return (_ev(dx, dy, dz) == _label);
	}

private:

	const EV& _ev;

	value_type _label;
};

class MeshViewController :
		public sg::Agent<
				MeshViewController,
				sg::Accepts<
						sg_gui::VolumePointSelected,
						sg_gui::KeyDown
				>,
				sg::Provides<
						sg_gui::SetMeshes
				>
		> {

public:

	MeshViewController(std::shared_ptr<ExplicitVolume<float>> labels);

	void onSignal(sg_gui::VolumePointSelected& signal);

	void onSignal(sg_gui::KeyDown& signal);

private:

	void nextVolume();

	void prevVolume();

	void nextNeighbor();

	void prevNeighbor();

	void addMesh(float label);

	void notifyMeshExtracted(std::shared_ptr<sg_gui::Mesh> mesh, float label);

	void removeMesh(float label);

	void exportMeshes();

	std::shared_ptr<ExplicitVolume<float>> _labels;

	std::shared_ptr<sg_gui::Meshes> _meshes;

	std::map<float, std::shared_ptr<sg_gui::Mesh>> _meshCache;

	std::vector<std::future<std::shared_ptr<sg_gui::Mesh>>> _highresMeshFutures;

	float _minCubeSize;
};

#endif // CANDIDATE_MC_GUI_MESH_VIEW_CONTROLLER_H__

