#include "SegmentController.h"
#include <util/Logger.h>

logger::LogChannel segmentcontrollerlog("segmentcontrollerlog", "[SegmentController] ");

SegmentController::SegmentController(std::shared_ptr<ExplicitVolume<float>> labels) :
	_labels(labels) {}

void
SegmentController::onSignal(sg_gui::VolumePointSelected& signal) {

	unsigned int x, y, z;
	_labels->getDiscreteCoordinates(
			signal.position().x(),
			signal.position().y(),
			signal.position().z(),
			x, y, z);

	float label = (*_labels)(x, y, z);

	if (label == 0)
		return;

	LOG_DEBUG(segmentcontrollerlog) << "selected label " << label << std::endl;

	toggleSegment(label);
}

void
SegmentController::onSignal(sg_gui::KeyDown& signal) {

	if (signal.key == sg_gui::keys::I) {

		LOG_USER(segmentcontrollerlog) << "enter label to show: " << std::endl;

		char input[256];
		std::cin.getline(input, 256);

		try {

			unsigned int label = boost::lexical_cast<unsigned int>(input);
			toggleSegment(label);

		} catch (std::exception& e) {

			LOG_ERROR(segmentcontrollerlog) << "invalid input" << std::endl;
			return;
		}
	}

	if (signal.key == sg_gui::keys::C) {

		for (unsigned int id : _visibleSegments)
			send<sg_gui::HideSegment>(id);
		_visibleSegments.clear();
	}
}

void
SegmentController::toggleSegment(unsigned int id) {

	if (_visibleSegments.count(id)) {

		send<sg_gui::HideSegment>(id);
		_visibleSegments.erase(id);

	} else {

		send<sg_gui::ShowSegment>(id);
		_visibleSegments.insert(id);
	}
}
