#ifndef CANDIDATE_MC_IO_HDF5_VOLUME_READER_H__
#define CANDIDATE_MC_IO_HDF5_VOLUME_READER_H__

#include <string>
#include <vigra/hdf5impex.hxx>
#include <imageprocessing/ExplicitVolume.h>

class Hdf5VolumeReader {

public:

	Hdf5VolumeReader(vigra::HDF5File& hdfFile) :
		_hdfFile(hdfFile) {}

	template <typename ValueType>
	void readVolume(ExplicitVolume<ValueType>& volume, std::string dataset, bool onlyGeometry = false) {

		// the volume
		if (!onlyGeometry)
			_hdfFile.readAndResize(dataset, volume.data());

		vigra::MultiArray<1, float> p(3);

		// resolution
		p[0] = p[1] = p[2] = 1.0;
		if (_hdfFile.existsAttribute(dataset, "resolution"))
			_hdfFile.readAttribute(
					dataset,
					"resolution",
					p);
		// resolution is stored as (z,y,x) to conform to how dataset is stored
		volume.setResolution(p[2], p[1], p[0]);

		// offset
		p[0] = p[1] = p[2] = 0.0;
		if (_hdfFile.existsAttribute(dataset, "offset"))
			_hdfFile.readAttribute(
					dataset,
					"offset",
					p);
		// offset is stored as (z,y,x) to conform to how dataset is stored
		volume.setOffset(p[2], p[1], p[0]);
	}

private:

	vigra::HDF5File& _hdfFile;
};

#endif // CANDIDATE_MC_IO_HDF5_VOLUME_READER_H__

