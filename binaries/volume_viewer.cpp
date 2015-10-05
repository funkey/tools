/**
 * This programs visualizes a volume.
 */

#include <util/ProgramOptions.h>
#include <imageprocessing/ExplicitVolume.h>
#include <gui/OverlayView.h>
#include <sg_gui/RotateView.h>
#include <sg_gui/ZoomView.h>
#include <sg_gui/Window.h>
#include <io/volumes.h>

using namespace sg_gui;

util::ProgramOption optionVolume(
		util::_long_name        = "volume",
		util::_description_text = "The volume to show.",
		util::_is_positional    = true);

util::ProgramOption optionOverlay(
		util::_long_name        = "overlay",
		util::_short_name       = "o",
		util::_description_text = "A volume containing integer values to be shown as a label overlay.");

int main(int argc, char** argv) {

	try {

		util::ProgramOptions::init(argc, argv);
		logger::LogManager::init();

		// read volume and overlay

		std::vector<std::string> files = getImageFiles(optionVolume.as<std::string>());
		auto volume = std::make_shared<ExplicitVolume<float>>();
		*volume = readVolume<float>(files);

		auto overlay = std::make_shared<ExplicitVolume<float>>();
		if (optionOverlay) {

			files = getImageFiles(optionOverlay.as<std::string>());
			*overlay = readVolume<float>(files);
		}

		// visualize

		auto overlayView    = std::make_shared<OverlayView>();
		auto rotateView     = std::make_shared<RotateView>();
		auto zoomView       = std::make_shared<ZoomView>(true);
		auto window         = std::make_shared<sg_gui::Window>("volume_viewer");

		window->add(zoomView);
		zoomView->add(rotateView);
		rotateView->add(overlayView);

		overlayView->setRawVolume(volume);
		overlayView->setLabelsVolume(overlay);

		window->processEvents();

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}

