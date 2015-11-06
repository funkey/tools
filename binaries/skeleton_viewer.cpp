/**
 * This programs visualizes a skeleton.
 */

#include <util/ProgramOptions.h>
#include <imageprocessing/ExplicitVolume.h>
#include <imageprocessing/Skeleton.h>
#include <imageprocessing/Skeletons.h>
#include <gui/OverlayView.h>
#include <gui/MeshViewController.h>
#include <gui/SkeletonView.h>
#include <sg_gui/RotateView.h>
#include <sg_gui/ZoomView.h>
#include <sg_gui/Window.h>
#include <io/volumes.h>
#include <io/skeletons.h>

using namespace sg_gui;

util::ProgramOption optionSkeleton(
		util::_long_name        = "skeleton",
		util::_description_text = "The skeleton to show.");

util::ProgramOption optionCompareSkeleton(
		util::_long_name        = "compare",
		util::_description_text = "Another skeleton to show for comparison.");

util::ProgramOption optionEdgeMatchScores(
		util::_long_name        = "edgeMatchScores",
		util::_description_text = "A file containing edge match scores between the two skeletons as lines of <e> <f> <score>. "
		                          "If a directory is provided, load all score files in there.");

util::ProgramOption optionVolume(
		util::_long_name        = "volume",
		util::_description_text = "The volume to show around the skeleton.");

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

		// read volume

		std::shared_ptr<ExplicitVolume<float>> volume;
		if (optionVolume) {

			std::vector<std::string> files = getImageFiles(optionVolume.as<std::string>());
			auto volume = std::make_shared<ExplicitVolume<float>>();
			*volume = readVolume<float>(files);
			volume->setResolution(util::point<float, 3>(optionResX, optionResY, optionResZ));
		}

		// read skeleton

		std::shared_ptr<Skeleton> skeleton = std::make_shared<Skeleton>();
		readSkeleton(optionSkeleton, *skeleton);

		std::shared_ptr<Skeletons> skeletons = std::make_shared<Skeletons>();
		skeletons->add(1, skeleton);

		if (optionCompareSkeleton) {

			std::shared_ptr<Skeleton> compare = std::make_shared<Skeleton>();
			readSkeleton(optionCompareSkeleton, *compare);
			skeletons->add(2, compare);
		}

		// visualize

		auto skeletonView = std::make_shared<SkeletonView>();
		auto rotateView   = std::make_shared<RotateView>();
		auto zoomView     = std::make_shared<ZoomView>(true);
		auto window       = std::make_shared<sg_gui::Window>("volume_viewer");

		window->add(zoomView);
		zoomView->add(rotateView);
		rotateView->add(skeletonView);

		skeletonView->setSkeletons(skeletons);

		if (optionEdgeMatchScores) {

			LOG_USER(logger::out) << "reading edge match scores from " << optionEdgeMatchScores.as<std::string>() << std::endl;

			std::vector<std::shared_ptr<SkeletonEdgeMatchScores>> scores = readEdgeMatchScores(optionEdgeMatchScores.as<std::string>());
			for (auto s : scores) {
				s->setSource(skeletons->get(1));
				s->setTarget(skeletons->get(2));
			}
			skeletonView->setEdgeMatchScores(scores);
		}

		if (volume) {

			auto overlayView = std::make_shared<OverlayView>();
			overlayView->setRawVolume(volume);
			rotateView->add(overlayView);
		}

		window->processEvents();

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}

