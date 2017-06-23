#include <queue>
#include "SegmentController.h"
#include <util/Logger.h>

logger::LogChannel segmentcontrollerlog("segmentcontrollerlog", "[SegmentController] ");

SegmentController::SegmentController(std::shared_ptr<ExplicitVolume<uint64_t>> labels) :
	_labels(labels) {}

void
SegmentController::onSignal(sg_gui::VolumePointSelected& signal) {

	unsigned int x, y, z;
	_labels->getDiscreteCoordinates(
			signal.position().x(),
			signal.position().y(),
			signal.position().z(),
			x, y, z);

	uint64_t label = (*_labels)(x, y, z);

	if (label == 0)
		return;

	LOG_DEBUG(segmentcontrollerlog) << "selected label " << label << std::endl;

	toggleSegment(label);
}

void
SegmentController::onSignal(sg_gui::KeyDown& signal) {

	if (signal.key == sg_gui::keys::I) {

		LOG_USER(segmentcontrollerlog) << "enter label to show (or 'all' or 'largest <k>'): " << std::endl;

		char input[256];
		std::cin.getline(input, 256);

		if (std::string(input) == "all") {

			showAllSegments();

		} else if (std::string(input).find("largest ") == 0) {

			size_t k = boost::lexical_cast<size_t>(std::string(input).substr(8));
			showLargestSegments(k);

		} else {

			try {

				uint64_t label = boost::lexical_cast<uint64_t>(input);

			} catch (std::exception& e) {

				LOG_ERROR(segmentcontrollerlog) << "invalid input" << std::endl;
				return;
			}
		}
	}

	if (signal.key == sg_gui::keys::C) {

		for (uint64_t id : _visibleSegments)
			send<sg_gui::HideSegment>(id);
		_visibleSegments.clear();
	}
}

void
SegmentController::toggleSegment(uint64_t id) {

	if (_visibleSegments.count(id)) {

		send<sg_gui::HideSegment>(id);
		_visibleSegments.erase(id);

	} else {

		send<sg_gui::ShowSegment>(id);
		_visibleSegments.insert(id);
	}
}

void
SegmentController::showAllSegments() {

	std::set<uint64_t> allIds;
	for (uint64_t id : _labels->data())
		allIds.insert(id);

	LOG_USER(segmentcontrollerlog) << "showing " << allIds.size() << " meshes..." << std::endl;

	for (uint64_t id : allIds) {

		send<sg_gui::ShowSegment>(id);
		_visibleSegments.insert(id);
	}
}

void
SegmentController::showLargestSegments(size_t k) {

	std::map<uint64_t, size_t> sizes;
	for (uint64_t id : _labels->data())
		sizes[id]++;

	auto sort = [](
			const std::pair<uint64_t, size_t>& a,
			const std::pair<uint64_t, size_t>& b) {

		return a.second > b.second;
	};

	std::priority_queue<std::pair<uint64_t, size_t>, std::vector<std::pair<uint64_t, size_t>>, decltype(sort)> queue(sort);
	for (const auto& p : sizes)
		queue.push(p);

	LOG_USER(segmentcontrollerlog) << "showing " << k << " largest meshes..." << std::endl;

	for (size_t i = 0; i < k; i++) {

		auto p = queue.top();
		queue.pop();

		uint64_t id = p.first;
		send<sg_gui::ShowSegment>(id);
		_visibleSegments.insert(id);
	}
}
