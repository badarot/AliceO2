// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef ALICEO2_TPC_CALIBDEDX_H_
#define ALICEO2_TPC_CALIBDEDX_H_

#include <string>
#include <array>
#include <gsl/span>

// o2 includes
#include "TPCCalibration/FastHisto.h"
#include "DataFormatsTPC/TrackCuts.h"

namespace o2::tpc
{

// forward declaration
class TrackTPC;

class CalibdEdx
{
  using Hist = FastHisto<float>;

 public:
  CalibdEdx(TrackCuts cuts, unsigned int nBins);
  CalibdEdx(double minP, double maxP, int minClusters, unsigned int nBins);
  CalibdEdx(unsigned int nBins);

  void fill(const gsl::span<const TrackTPC> tracks);
  void merge(const CalibdEdx* prev);
  void print() const;

  double getASideEntries() const { return mEntries[0]; }
  double getCSideEntries() const { return mEntries[1]; }

  void dumpToFile(const std::string& file_name) const;
  const std::array<Hist, 2>& getHist() const { return mHist; }

 private:
  bool mApplyCuts{true};
  TrackCuts mCuts;

  std::array<float, 2> mEntries{0, 0}; ///< Number of entries in each histogram
  std::array<Hist, 2> mHist;           ///< MIP position histograms, for TPC's A and C sides

  ClassDefNV(CalibdEdx, 1);
};

} // namespace o2::tpc
#endif
