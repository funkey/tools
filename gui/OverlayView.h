#ifndef TOOLS_GUI_OVERLAY_VIEW_H__
#define TOOLS_GUI_OVERLAY_VIEW_H__

#include <scopegraph/Scope.h>
#include <sg_gui/VolumeView.h>
#include <sg_gui/KeySignals.h>

class OverlayView :
		public sg::Scope<
				OverlayView,
				sg::Accepts<
						sg_gui::KeyDown
				>,
				sg::Provides<
						sg_gui::ContentChanged
				>,
				sg::ProvidesInner<
						sg_gui::ChangeAlpha
				>,
				sg::PassesUp<
						sg_gui::ContentChanged,
						sg_gui::VolumePointSelected
				>
		> {

public:

	OverlayView();

	void setRawVolume(std::shared_ptr<ExplicitVolume<float>> volume);

	void setLabelsVolume(std::shared_ptr<ExplicitVolume<float>> volume);

	void onSignal(sg_gui::KeyDown& signal);

private:

	/**
	 * Scope preventing change alpha signals to get to raw images.
	 */
	class RawScope : public sg::Scope<
			RawScope,
			sg::FiltersDown<
					sg_gui::ChangeAlpha,
					sg_gui::DrawOpaque
			>,
			sg::PassesUp<
					sg_gui::ContentChanged
			>
	> {

	public:

		RawScope() : _zBufferWrites(false) {}

		bool filterDown(sg_gui::ChangeAlpha&) { return false; }
		void unfilterDown(sg_gui::ChangeAlpha&) {}

		// disable z-write for the raw image
		bool filterDown(sg_gui::DrawOpaque&) {

			if (!_zBufferWrites) {

				glGetBooleanv(GL_DEPTH_WRITEMASK, &_prevDepthMask);
				glDepthMask(GL_FALSE);
			}

			return true;
		}

		void unfilterDown(sg_gui::DrawOpaque&) {

			if (!_zBufferWrites)
				glDepthMask(_prevDepthMask);
		}

		void toggleZBufferWrites() { _zBufferWrites = !_zBufferWrites; }

	private:

		bool _zBufferWrites;
		GLboolean _prevDepthMask;
	};

	/**
	 * Scope preventing change alpha signals to get to label images, also 
	 * ignores depth buffer for drawing on top of raw images.
	 */
	class LabelsScope : public sg::Scope<
			LabelsScope,
			sg::FiltersDown<
					sg_gui::DrawOpaque,
					sg_gui::DrawTranslucent,
					sg_gui::ChangeAlpha
			>,
			sg::AcceptsInner<
					sg::AgentAdded
			>,
			sg::ProvidesInner<
					sg_gui::ChangeAlpha,
					sg_gui::DrawTranslucent
			>,
			sg::PassesUp<
					sg_gui::ContentChanged,
					sg_gui::VolumePointSelected
			>
	> {

	public:

		LabelsScope() : _visible(true) {}

		void onInnerSignal(sg::AgentAdded&) {

			sendInner<sg_gui::ChangeAlpha>(0.5);
		}

		// stop the translucent draw -- we take care of it in opaque draw
		bool filterDown(sg_gui::DrawTranslucent&) { return false; }
		void unfilterDown(sg_gui::DrawTranslucent&) {}

		// convert opaque draw into translucent draw
		bool filterDown(sg_gui::DrawOpaque& s) {

				if (!_visible)
					return false;

				glEnable(GL_BLEND);
				sg_gui::DrawTranslucent signal;
				signal.roi() = s.roi();
				signal.resolution() = s.resolution();
				sendInner(signal);
				glDisable(GL_BLEND);
				
				return false;
		}
		void unfilterDown(sg_gui::DrawOpaque&) {}

		bool filterDown(sg_gui::ChangeAlpha&) { return false; }
		void unfilterDown(sg_gui::ChangeAlpha&) {}

		void toggleVisibility() {

			_visible = !_visible;
		}

	private:

		bool _visible;
	};

	std::shared_ptr<RawScope>           _rawScope;
	std::shared_ptr<LabelsScope>        _labelsScope;
	std::shared_ptr<sg_gui::VolumeView> _rawView;
	std::shared_ptr<sg_gui::VolumeView> _labelsView;

	double _alpha;
};

#endif // TOOLS_GUI_OVERLAY_VIEW_H__

