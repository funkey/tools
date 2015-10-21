/**
 * This programs visualizes a volume.
 */

#include <util/ProgramOptions.h>
#include <imageprocessing/ExplicitVolume.h>
#include <gui/OverlayView.h>
#include <gui/MeshViewController.h>
#include <sg_gui/MeshView.h>
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

util::ProgramOption optionResX(
		util::_long_name        = "resX",
		util::_description_text = "x resolution of the volume.",
		util::_default_value    = 1.0);
util::ProgramOption optionResY(
		util::_long_name        = "resY",
		util::_description_text = "y resolution of the volume.",
		util::_default_value    = 1.0);
util::ProgramOption optionResZ(
		util::_long_name        = "resZ",
		util::_description_text = "z resolution of the volume.",
		util::_default_value    = 1.0);

int main(int argc, char** argv) {

	try {

		util::ProgramOptions::init(argc, argv);
		logger::LogManager::init();

		// read volume and overlay

		std::vector<std::string> files = getImageFiles(optionVolume.as<std::string>());
		auto volume = std::make_shared<ExplicitVolume<float>>();
		*volume = readVolume<float>(files);
		volume->setResolution(util::point<float, 3>(optionResX, optionResY, optionResZ));

		auto overlay = std::make_shared<ExplicitVolume<float>>();
		if (optionOverlay) {

			files = getImageFiles(optionOverlay.as<std::string>());
			*overlay = readVolume<float>(files);
			overlay->setResolution(util::point<float, 3>(optionResX, optionResY, optionResZ));
		}

		// visualize

		auto overlayView        = std::make_shared<OverlayView>();
		auto meshView           = std::make_shared<MeshView>();
		auto meshViewController = std::make_shared<MeshViewController>(overlay);
		auto rotateView         = std::make_shared<RotateView>();
		auto zoomView           = std::make_shared<ZoomView>(true);
		auto window             = std::make_shared<sg_gui::Window>("volume_viewer");

		window->add(zoomView);
		zoomView->add(rotateView);
		rotateView->add(overlayView);

		overlayView->setRawVolume(volume);
		overlayView->setLabelsVolume(overlay);
		overlayView->add(meshView);
		overlayView->add(meshViewController);

		window->processEvents();

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}

