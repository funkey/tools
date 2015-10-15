/**
 * This programs shows individual images.
 */

#include <util/ProgramOptions.h>
#include <imageprocessing/ExplicitVolume.h>
#include <gui/OverlayView.h>
#include <sg_gui/RotateView.h>
#include <sg_gui/ZoomView.h>
#include <sg_gui/Window.h>
#include <io/volumes.h>

using namespace sg_gui;

util::ProgramOption optionImage(
		util::_long_name        = "image",
		util::_description_text = "A single image or a directory containing images.",
		util::_is_positional    = true);

int main(int argc, char** argv) {

	try {

		util::ProgramOptions::init(argc, argv);
		logger::LogManager::init();

		// read volume and overlay

		std::vector<std::string> files = getImageFiles(optionImage.as<std::string>());

		std::vector<std::shared_ptr<Image>> images;

		for (const std::string& file : files) {

			std::vector<std::string> single = {file};
			ExplicitVolume<float> v = readVolume<float>(single);
			auto image = std::make_shared<Image>();
			*image = v.data().bind<2>(0);
			image->setIdentifiyer(file);
			images.push_back(image);
		}

		// create a controller

		class Controller : public sg::Agent<
				Controller,
				sg::Provides<sg_gui::SetImage>,
				sg::Accepts<sg_gui::KeyDown>> {

		public:

			Controller() : _current(0) {}

			void setImages(const std::vector<std::shared_ptr<Image>>& images) {

				_images = images;

				if (_images.size() == 0)
					return;

				_current = 0;
				send<sg_gui::SetImage>(_images[_current]);
			}

			void onSignal(sg_gui::KeyDown& signal) {

				if (_images.size() == 0)
					return;

				if (signal.key == sg_gui::keys::D)
					_current = std::min((int)_images.size() - 1, _current + 1);

				if (signal.key == sg_gui::keys::A)
					_current = std::max(0, _current - 1);

				send<sg_gui::SetImage>(_images[_current]);
			}

		private:

			std::vector<std::shared_ptr<Image>> _images;
			int _current;
		};

		class Logger : public sg::Agent<Logger, sg::Accepts<sg_gui::SetImage>> {

		public:

			void onSignal(sg_gui::SetImage& signal) {

				std::cout << "showing image " << signal.getImage()->getIdentifier() << std::endl;
			}
		};

		// visualize

		auto controller = std::make_shared<Controller>();
		auto logger     = std::make_shared<Logger>();
		auto imageView  = std::make_shared<ImageView>();
		auto zoomView   = std::make_shared<ZoomView>(true);
		auto window     = std::make_shared<sg_gui::Window>("image_viewer");

		window->add(zoomView);
		zoomView->add(imageView);
		zoomView->add(controller);
		zoomView->add(logger);

		controller->setImages(images);

		window->processEvents();

	} catch (boost::exception& e) {

		handleException(e, std::cerr);
	}
}


