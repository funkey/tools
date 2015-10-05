#include "OverlayView.h"
#include <util/ProgramOptions.h>

util::ProgramOption optionShowNormals(
		util::_long_name        = "showNormals",
		util::_description_text = "Show the mesh normals.");

OverlayView::OverlayView() :
	_rawScope(std::make_shared<RawScope>()),
	_labelsScope(std::make_shared<LabelsScope>()),
	_rawView(std::make_shared<sg_gui::VolumeView>()),
	_labelsView(std::make_shared<sg_gui::VolumeView>()),
	_alpha(1.0) {

	_rawScope->add(_rawView);
	_labelsScope->add(_labelsView);

	add(_rawScope);
	add(_labelsScope);
}

void
OverlayView::setRawVolume(std::shared_ptr<ExplicitVolume<float>> volume) {

	_rawView->setVolume(volume);
}

void
OverlayView::setLabelsVolume(std::shared_ptr<ExplicitVolume<float>> volume) {

	_labelsView->setVolume(volume);
}

void
OverlayView::onSignal(sg_gui::KeyDown& signal) {

	if (signal.key == sg_gui::keys::Tab) {

		_alpha += 0.5;
		if (_alpha > 1.0)
			_alpha = 0;

		sendInner<sg_gui::ChangeAlpha>(_alpha);
	}

	if (signal.key == sg_gui::keys::L) {

		_rawScope->toggleZBufferWrites();
		_labelsScope->toggleVisibility();
		send<sg_gui::ContentChanged>();
	}
}
