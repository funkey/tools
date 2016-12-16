#include <boost/filesystem.hpp>
#include <util/Logger.h>
#include "volumes.h"

std::vector<std::string>
getImageFiles(std::string path) {

	std::vector<std::string> filenames;

	boost::filesystem::path p(path);

	if (boost::filesystem::is_directory(p)) {

		for (boost::filesystem::directory_iterator i(p); i != boost::filesystem::directory_iterator(); i++)
			if (!boost::filesystem::is_directory(*i) && (
			    i->path().extension() == ".png" ||
			    i->path().extension() == ".tif" ||
			    i->path().extension() == ".tiff"
			))
				filenames.push_back(i->path().native());

		std::sort(filenames.begin(), filenames.end());

	} else if (boost::filesystem::is_regular_file(p)) {

		filenames.push_back(path);

	} else if (boost::filesystem::is_symlink(p)) {

		return getImageFiles(boost::filesystem::read_symlink(p).string());

	} else {

		LOG_ERROR(logger::out) << "no such file or directory: " << path << std::endl;
	}

	return filenames;
}
