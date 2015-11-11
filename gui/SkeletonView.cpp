#include "SkeletonView.h"
#include <sg_gui/OpenGl.h>
#include <sg_gui/Colors.h>
#include <util/ProgramOptions.h>
#include <util/Logger.h>
#include <util/geometry.hpp>

util::ProgramOption optionSkeletonSphereScale(
		util::_module           = "gui",
		util::_long_name        = "skeletonSphereScale",
		util::_description_text = "The initial scale of the skeleton spheres to show. Default is 1.",
		util::_default_value    = 1.0);

SkeletonView::SkeletonView() :
	_currentScoreIndex(0),
	_invertScores(false),
	_showFocus(false),
	_sphere(10),
	_showSpheres(false),
	_sphereScale(optionSkeletonSphereScale),
	_ftfont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") {

	_ftfont.FaceSize(100);
	_ftfont.CharMap(ft_encoding_unicode);
}

void
SkeletonView::setSkeletons(std::shared_ptr<Skeletons> skeletons) {

	_skeletons = skeletons;

	updateRecording();
	send<sg_gui::ContentChanged>();
}

void
SkeletonView::onSignal(sg_gui::Draw& /*draw*/) {

	if (!_skeletons)
		return;

	draw();
}

void
SkeletonView::onSignal(sg_gui::DrawTranslucent& /*draw*/) {

	if (!_skeletons)
		return;

	drawTranslucent();
}

void
SkeletonView::onSignal(sg_gui::QuerySize& signal) {

	if (!_skeletons)
		return;

	signal.setSize(_skeletons->getBoundingBox());
}

void
SkeletonView::onSignal(sg_gui::MouseDown& signal) {

	if (signal.modifiers & sg_gui::keys::ShiftDown) {

		if (signal.button == sg_gui::buttons::WheelUp) {

			_sphereScale *= 1.1;
			signal.processed = true;
		}

		if (signal.button == sg_gui::buttons::WheelDown) {

			_sphereScale = std::max(0.1, _sphereScale*(1.0/1.1));
			signal.processed = true;
		}

		updateRecording();
		send<sg_gui::ContentChanged>();
	}

	if (signal.button == sg_gui::buttons::Right) {

		findClosestEdge(signal.ray);

		updateRecording();
		send<sg_gui::ContentChanged>();
	}
}

void
SkeletonView::onSignal(sg_gui::KeyDown& signal) {

	if (signal.key == sg_gui::keys::S) {

		_showSpheres = !_showSpheres;
		updateRecording();
		send<sg_gui::ContentChanged>();
	}

	if (signal.key == sg_gui::keys::A) {

		_currentScoreIndex = std::max(0, _currentScoreIndex - 1);
		updateRecording();
		send<sg_gui::ContentChanged>();
	}

	if (signal.key == sg_gui::keys::D) {

		_currentScoreIndex = std::min((int)_edgeMatchScores.size() - 1, _currentScoreIndex + 1);
		updateRecording();
		send<sg_gui::ContentChanged>();
	}

	if (signal.key == sg_gui::keys::I) {

		_invertScores = !_invertScores;
		updateRecording();
		send<sg_gui::ContentChanged>();
	}
}

void
SkeletonView::onSignal(SetSkeletons& signal) {

	setSkeletons(signal.getSkeletons());
}

void
SkeletonView::updateRecording() {

	sg_gui::OpenGl::Guard guard;

	startRecording();

	for (unsigned int id : _skeletons->getSkeletonIds()) {

		auto skeleton = _skeletons->get(id);
		unsigned char r, g, b;
		sg_gui::idToRgb(id, r, g, b);
		glColor4f(
				static_cast<float>(r)/200.0,
				static_cast<float>(g)/200.0,
				static_cast<float>(b)/200.0,
				1.0);

		drawSkeleton(*skeleton);
	}

	stopRecording();

	startRecordingTranslucent();

	if (_edgeMatchScores.size() > 0)
		drawEdgeMatchScores(*_edgeMatchScores[_currentScoreIndex]);

	stopRecording();
}

void
SkeletonView::drawSkeleton(const Skeleton& skeleton) {

	glLineWidth(2.0);
	glEnable(GL_LINE_SMOOTH);

	GLfloat specular[] = {0.2, 0.2, 0.2, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	GLfloat shininess[] = {0.2, 0.2, 0.2, 1.0};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

	for (Skeleton::Graph::EdgeIt e(skeleton.graph()); e != lemon::INVALID; ++e) {

		Skeleton::Node u = skeleton.graph().u(e);
		Skeleton::Node v = skeleton.graph().v(e);

		Skeleton::Position pu = skeleton.positions()[u];
		Skeleton::Position pv = skeleton.positions()[v];

		util::point<float,3> ru;
		util::point<float,3> rv;
		skeleton.getRealLocation(pu, ru);
		skeleton.getRealLocation(pv, rv);

		if (_showSpheres) {

			float diameter = skeleton.diameters()[u];
			drawSphere(ru, diameter*_sphereScale);
		}

		glBegin(GL_LINES);
		glVertex3d(ru.x(), ru.y(), ru.z());
		glVertex3d(rv.x(), rv.y(), rv.z());
		glEnd();
	}
}

void
SkeletonView::drawEdgeMatchScores(const SkeletonEdgeMatchScores& scores) {

	LOG_USER(logger::out) << "showing matching scores " << scores.getName() << std::endl;

	const Skeleton& a = *scores.getSource();
	const Skeleton& b = *scores.getTarget();

	double maxScore = scores.getMaxScore();

	for (Skeleton::Graph::EdgeIt e(a.graph()); e != lemon::INVALID; ++e) {

		if (_showFocus)
			if (scores.getSource() == _focusSkeleton && e != _focusEdge)
				continue;

		Skeleton::Node au = a.graph().u(e);
		Skeleton::Node av = a.graph().v(e);

		Skeleton::Position apu = a.positions()[au];
		Skeleton::Position apv = a.positions()[av];

		util::point<float,3> aru;
		util::point<float,3> arv;
		a.getRealLocation(apu, aru);
		a.getRealLocation(apv, arv);

		util::point<float,3> acenter = (aru + arv)/2.0;

		for (Skeleton::Graph::EdgeIt f(b.graph()); f != lemon::INVALID; ++f) {

			if (_showFocus)
				if (scores.getTarget() == _focusSkeleton && f != _focusEdge)
					continue;

			Skeleton::Node bu = b.graph().u(f);
			Skeleton::Node bv = b.graph().v(f);

			Skeleton::Position bpu = b.positions()[bu];
			Skeleton::Position bpv = b.positions()[bv];

			util::point<float,3> bru;
			util::point<float,3> brv;
			b.getRealLocation(bpu, bru);
			b.getRealLocation(bpv, brv);

			util::point<float,3> bcenter = (bru + brv)/2.0;

			double score = scores.getScore(a.graph().id(e), b.graph().id(f));
			double s = score;
			if (_invertScores)
				s = maxScore - score;

			glLineWidth(std::max(1.0, 10*s/maxScore));
			glEnable(GL_LINE_SMOOTH);
			glBegin(GL_LINES);
			glColor4f(0, 0, 0, 0.25 + 0.5*s/maxScore);
			glVertex3d(acenter.x(), acenter.y(), acenter.z());
			glVertex3d(bcenter.x(), bcenter.y(), bcenter.z());
			glEnd();

			glColor4f(0.5, 0.5, 1, 1);
			util::point<float,3> middle = (acenter + bcenter)/2.0;
			glPushMatrix();
			glTranslatef(middle.x(), middle.y(), middle.z());
			glScalef(0.01, -0.01, 0.01);
			_ftfont.Render(boost::lexical_cast<std::string>(score).c_str());
			glPopMatrix();
		}
	}
}

void
SkeletonView::drawSphere(const util::point<float,3>& center, float diameter) {

	glPushMatrix();
	glTranslatef(center.x(), center.y(), center.z());
	glScalef(diameter, diameter, diameter);

	const std::vector<sg_gui::Triangle>& triangles = _sphere.getTriangles();

	glBegin(GL_TRIANGLES);
	for (const sg_gui::Triangle& triangle : triangles) {

		const sg_gui::Point3d&  v0 = _sphere.getVertex(triangle.v0);
		const sg_gui::Point3d&  v1 = _sphere.getVertex(triangle.v1);
		const sg_gui::Point3d&  v2 = _sphere.getVertex(triangle.v2);
		const sg_gui::Vector3d& n0 = _sphere.getNormal(triangle.v0);
		const sg_gui::Vector3d& n1 = _sphere.getNormal(triangle.v1);
		const sg_gui::Vector3d& n2 = _sphere.getNormal(triangle.v2);

		glNormal3f(n0.x(), n0.y(), n0.z()); glVertex3f(v0.x(), v0.y(), v0.z());
		glNormal3f(n1.x(), n1.y(), n1.z()); glVertex3f(v1.x(), v1.y(), v1.z());
		glNormal3f(n2.x(), n2.y(), n2.z()); glVertex3f(v2.x(), v2.y(), v2.z());
	}
	glEnd();

	glPopMatrix();
}

void
SkeletonView::findClosestEdge(const util::ray<float,3>& ray) {

	std::shared_ptr<Skeleton> closestSkeleton;
	unsigned int closestSkeletonId;
	Skeleton::Graph::Edge closestEdge;

	float minSkeletonDistance = std::numeric_limits<float>::max();

	for (unsigned int id : _skeletons->getSkeletonIds()) {

		const Skeleton& skeleton = *_skeletons->get(id);

		float minDistance = std::numeric_limits<float>::max();
		Skeleton::Graph::Edge bestEdge;

		for (Skeleton::Graph::EdgeIt e(skeleton.graph()); e != lemon::INVALID; ++e) {

			Skeleton::Node su = skeleton.graph().u(e);
			Skeleton::Node sv = skeleton.graph().v(e);

			Skeleton::Position spu = skeleton.positions()[su];
			Skeleton::Position spv = skeleton.positions()[sv];

			util::point<float,3> sru;
			util::point<float,3> srv;
			skeleton.getRealLocation(spu, sru);
			skeleton.getRealLocation(spv, srv);

			util::ray<float,3> edgeRay(sru, srv - sru);

			float s, t;
			float dist = distance(ray, edgeRay, s, t);
			std::cout << t << std::endl;
			if (t >= 0 && t <= 1 && dist < minDistance) {

				minDistance = dist;
				bestEdge = e;
			}
		}

		if (minDistance < minSkeletonDistance) {

			minSkeletonDistance = minDistance;
			closestSkeleton = _skeletons->get(id);
			closestSkeletonId = id;
			closestEdge = bestEdge;
		}
	}

	_focusSkeleton = closestSkeleton;
	_focusEdge = closestEdge;
	_showFocus = (minSkeletonDistance < std::numeric_limits<float>::max());

	if (_showFocus)
		LOG_USER(logger::out) << "[SkeletonView] showing edge " << closestSkeleton->graph().id(_focusEdge) << " of skeleton " << closestSkeletonId << std::endl;
}
