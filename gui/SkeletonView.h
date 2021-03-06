#ifndef HOST_TUBES_GUI_SKELETON_VIEW_H__
#define HOST_TUBES_GUI_SKELETON_VIEW_H__

#include <FTGL/ftgl.h>
#include <scopegraph/Agent.h>
#include <imageprocessing/Skeletons.h>
#include <imageprocessing/SkeletonEdgeMatchScores.h>
#include <sg_gui/GuiSignals.h>
#include <sg_gui/MouseSignals.h>
#include <sg_gui/KeySignals.h>
#include <sg_gui/SegmentSignals.h>
#include <sg_gui/RecordableView.h>
#include <sg_gui/Sphere.h>

class SetSkeletons : public sg_gui::SetContent {

public:

	typedef sg_gui::SetContent parent_type;

	SetSkeletons(std::shared_ptr<Skeletons> skeletons) :
			_skeletons(skeletons) {}

	std::shared_ptr<Skeletons> getSkeletons() { return _skeletons; }

private:

	std::shared_ptr<Skeletons> _skeletons;
};

class SkeletonView :
		public sg::Agent<
				SkeletonView,
				sg::Accepts<
						sg_gui::Draw,
						sg_gui::DrawTranslucent,
						sg_gui::QuerySize,
						sg_gui::MouseDown,
						sg_gui::KeyDown,
						SetSkeletons,
						sg_gui::ShowSegment,
						sg_gui::HideSegment
				>,
				sg::Provides<
						sg_gui::ContentChanged
				>
		>,
		public sg_gui::RecordableView {

public:

	SkeletonView();

	void setSkeletons(std::shared_ptr<Skeletons> skeletons);

	void setEdgeMatchScores(std::vector<std::shared_ptr<SkeletonEdgeMatchScores>> scores) {

		_edgeMatchScores = scores;
		_currentScoreIndex = 0;
		updateRecording();
		send<sg_gui::ContentChanged>();
	}

	void onSignal(sg_gui::Draw& draw);

	void onSignal(sg_gui::DrawTranslucent& draw);

	void onSignal(sg_gui::QuerySize& signal);

	void onSignal(sg_gui::MouseDown& signal);

	void onSignal(sg_gui::KeyDown& signal);

	void onSignal(SetSkeletons& signal);

	void onSignal(sg_gui::ShowSegment& signal);

	void onSignal(sg_gui::HideSegment& signal);

private:

	void updateRecording();

	void drawSkeleton(const Skeleton& skeleton);

	void drawEdgeMatchScores(const SkeletonEdgeMatchScores& scores);

	void drawSphere(const util::point<float,3>& center, float diameter);

	void findClosestEdge(const util::ray<float,3>& ray);

	std::shared_ptr<Skeletons> _skeletons;
	std::shared_ptr<Skeletons> _visibleSkeletons;

	std::vector<std::shared_ptr<SkeletonEdgeMatchScores>> _edgeMatchScores;

	int  _currentScoreIndex;
	bool _invertScores;

	bool                      _showFocus;
	std::shared_ptr<Skeleton> _focusSkeleton;
	Skeleton::Graph::Edge     _focusEdge;

	bool _showNumbers;
	bool _showZeroLines;

	sg_gui::Sphere _sphere;
	bool           _showSpheres;
	float          _sphereScale;

	FTTextureFont _ftfont;
};

#endif // HOST_TUBES_GUI_SKELETON_VIEW_H__

