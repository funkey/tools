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
#include <io/Hdf5VolumeReader.h>

using namespace sg_gui;

util::ProgramOption optionVolume(
		util::_long_name        = "volume",
		util::_description_text = "The volume to show.",
		util::_is_positional    = true);

util::ProgramOption optionNormalizeVolume(
		util::_long_name        = "normalize",
		util::_description_text = "Normalize the intensities of the volume to show. Does not change the overlay.");

util::ProgramOption optionTranspose(
		util::_long_name        = "transpose",
		util::_description_text = "Invert the order of the axises, applies to both the volume and overlay.");

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

void readVolumeFromOption(ExplicitVolume<float>& volume, std::string option) {

	// hdf file given?
	size_t sepPos = option.find_first_of(":");
	if (sepPos != std::string::npos) {

		std::string hdfFileName = option.substr(0, sepPos);
		std::string dataset     = option.substr(sepPos + 1);

		vigra::HDF5File file(hdfFileName, vigra::HDF5File::OpenMode::ReadOnly);
		Hdf5VolumeReader hdfReader(file);
		hdfReader.readVolume(volume, dataset);

	// read volume from set of images
	} else {

		std::vector<std::string> files = getImageFiles(option);
		volume = readVolume<float>(files);
		volume.setResolution(util::point<float, 3>(optionResX, optionResY, optionResZ));
	}
}

int main(int argc, char** argv) {

	try {

		util::ProgramOptions::init(argc, argv);
		logger::LogManager::init();

		// read volume and overlay

		auto volume  = std::make_shared<ExplicitVolume<float>>();
		auto overlay = std::make_shared<ExplicitVolume<float>>();

		readVolumeFromOption(*volume, optionVolume);

		if (optionNormalizeVolume)
			volume->normalize();

		if (optionOverlay)
			readVolumeFromOption(*overlay, optionOverlay);

		if (optionTranspose) {

			volume->transpose();
			if (optionOverlay)
				overlay->transpose();
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

