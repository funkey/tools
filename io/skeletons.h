#ifndef TOOLS_IO_SKELETONS_H__
#define TOOLS_IO_SKELETONS_H__

#include <fstream>
#include <boost/filesystem.hpp>
#include <imageprocessing/Skeleton.h>
#include <util/Logger.h>

// returns the minumal number of times to multiply the given values by 10 such that all of them are integer
int getMaxPrecision(const std::vector<float>& values) {

	int maxPrecision = 0;

	for (float v : values) {

		int precision = 0;
		double _;
		while (modf(v, &_) > 0.0001) {

			precision++;
			v *= 10;
		}

		maxPrecision = std::max(precision, maxPrecision);
	}

	return maxPrecision;
}

void readSkeleton(const std::string& filename, Skeleton& skeleton) {

	std::ifstream file(filename);

	std::string token;
	int numNodes = 0;

	while (file.good()) {

		file >> token;
		if (!file.good())
			break;

		LOG_ALL(logger::out) << "read token " << token << std::endl;

		if (token == "POINTS") {

			file >> numNodes;

			std::string type;
			file >> type;

			// read the real-valued coordinates
			std::vector<float> xs, ys, zs;
			for (int i = 0; i < numNodes; i++) {

				float x, y, z;
				file >> x;
				file >> y;
				file >> z;

				xs.push_back(x);
				ys.push_back(y);
				zs.push_back(z);
			}

			// get the maximal precision of each axis
			int precisionX = getMaxPrecision(xs);
			int precisionY = getMaxPrecision(ys);
			int precisionZ = getMaxPrecision(zs);

			int fx = pow(10, precisionX);
			int fy = pow(10, precisionY);
			int fz = pow(10, precisionZ);

			skeleton.setResolution(1.0/fx, 1.0/fy, 1.0/fz);

			// get minimal point
			float minX = *std::min_element(xs.begin(), xs.end());
			float minY = *std::min_element(ys.begin(), ys.end());
			float minZ = *std::min_element(zs.begin(), zs.end());

			skeleton.setOffset(minX, minY, minZ);

			for (int i = 0; i < numNodes; i++) {

				auto n = skeleton.graph().addNode();
				skeleton.positions()[n] = util::point<unsigned int,3>((xs[i] - minX)*fx, (ys[i] - minY)*fy, (zs[i] - minZ)*fz);

				LOG_ALL(logger::out) << "setting position of node " << i << " to " << util::point<unsigned int,3>(xs[i]*fx, ys[i]*fy, zs[i]*fz) << std::endl;
			}
		}

		if (token == "EDGES") {

			int numEdges;
			file >> numEdges;

			for (int i = 0; i < numEdges; i++) {

				int u, v;
				file >> u;
				file >> v;

				skeleton.graph().addEdge(skeleton.graph().nodeFromId(u), skeleton.graph().nodeFromId(v));

				LOG_ALL(logger::out) << "adding edge between " << u << " and " << v << std::endl;
			}
		}

		if (token == "diameters") {

			int _; std::string type;
			file >> _;
			file >> _;
			file >> type;

			for (int i = 0; i < numNodes; i++) {

				double diameter;
				file >> diameter;
				skeleton.diameters()[skeleton.graph().nodeFromId(i)] = diameter;

				LOG_ALL(logger::out) << "setting diameter of node " << i << " to " << diameter << std::endl;
			}
		}
	}

	LOG_USER(logger::out) << "read skeleton with " << numNodes << " nodes" << std::endl;
}

std::vector<std::shared_ptr<SkeletonEdgeMatchScores>>
readEdgeMatchScores(const std::vector<std::string>& filenames) {

	std::vector<std::shared_ptr<SkeletonEdgeMatchScores>> allScores;

	for (auto filename : filenames) {

		auto scores = std::make_shared<SkeletonEdgeMatchScores>(filename);

		std::ifstream file(filename);
		std::string token;

		while (file.good()) {

			unsigned int e, f;
			double score;

			file >> e;
			file >> f;
			file >> score;

			if (!file.good())
				break;

			scores->setScore(e, f, score);
		}

		allScores.push_back(scores);
	}

	return allScores;
}

std::vector<std::shared_ptr<SkeletonEdgeMatchScores>>
readEdgeMatchScores(const std::string& path) {

	std::vector<std::string> filenames;

	boost::filesystem::path p(path);
	if (!boost::filesystem::is_directory(p)) {

		filenames.push_back(path);

	} else {

		for (boost::filesystem::directory_iterator i(p); i != boost::filesystem::directory_iterator(); i++)
			if (!boost::filesystem::is_directory(*i) && i->path().extension() == ".dat")
				filenames.push_back(i->path().native());

		std::sort(filenames.begin(), filenames.end());
	}

	return readEdgeMatchScores(filenames);
}

#endif // TOOLS_IO_SKELETONS_H__

