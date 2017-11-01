#ifndef CANDIDATE_MC_GUI_SEGMENT_CONTROLLER_H__
#define CANDIDATE_MC_GUI_SEGMENT_CONTROLLER_H__

#include <scopegraph/Agent.h>
#include <sg_gui/KeySignals.h>
#include <sg_gui/SegmentSignals.h>
#include <sg_gui/VolumeView.h>
#include <imageprocessing/ExplicitVolume.h>

class SegmentController :
		public sg::Agent<
				SegmentController,
				sg::Accepts<
						sg_gui::VolumePointSelected,
						sg_gui::KeyDown
				>,
				sg::Provides<
						sg_gui::ShowSegment,
						sg_gui::HideSegment
				>
		> {

public:

	SegmentController(std::shared_ptr<ExplicitVolume<uint64_t>> labels);

	void onSignal(sg_gui::VolumePointSelected& signal);

	void onSignal(sg_gui::KeyDown& signal);

private:

	void toggleSegment(uint64_t id);

	void showAllSegments();

	void showLargestSegments(size_t k);

	std::shared_ptr<ExplicitVolume<uint64_t>> _labels;

	std::set<uint64_t> _visibleSegments;
};

#endif // CANDIDATE_MC_GUI_SEGMENT_CONTROLLER_H__

