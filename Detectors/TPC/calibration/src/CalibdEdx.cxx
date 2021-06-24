// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "TPCCalibration/CalibdEdx.h"

#include <array>
#include <cstddef>
#include <utility>

//o2 includes
#include "DataFormatsTPC/TrackTPC.h"
#include "DataFormatsTPC/TrackCuts.h"
#include "Framework/Logger.h"
#include "TPCCalibration/FastHisto.h"

//root includes
#include "TFile.h"

using namespace o2::tpc;

CalibdEdx::CalibdEdx(TrackCuts cuts, unsigned int nBins)
  : mCuts{std::move(cuts)}, mHist{{{nBins, 0, static_cast<float>(nBins)}, {nBins, 0, static_cast<float>(nBins)}}}
{
}

CalibdEdx::CalibdEdx(double minP, double maxP, int minClusters, unsigned int nBins)
  : CalibdEdx(TrackCuts(minP, maxP, minClusters), nBins) {}

CalibdEdx::CalibdEdx(unsigned int nBins)
  : CalibdEdx({}, nBins)
{
  mApplyCuts = false;
}

void CalibdEdx::fill(const gsl::span<const TrackTPC> tracks)
{
  for (const auto& track : tracks) {
    const auto p = track.getP();
    const int cluster_count = track.getNClusters();

    // applying cut
    if (!mApplyCuts || mCuts.goodTrack(track)) {
      // filling histogram
      if (track.hasASideClustersOnly()) {
        mEntries[0]++;
        mHist[0].fill(track.getdEdx().dEdxTotTPC);
      } else if (track.hasCSideClustersOnly()) {
        mEntries[1]++;
        mHist[1].fill(track.getdEdx().dEdxTotTPC);
      }
    }
  }
}

void CalibdEdx::merge(const CalibdEdx* prev)
{
  for (size_t i = 0; i < mHist.size(); i++) {
    for (size_t bin = 0; bin < mHist[0].getNBins(); bin++) {
      float bin_content = prev->getHist()[i].getBinContent(bin);
      mHist[i].fillBin(bin, bin_content);
    }
  }
}

void CalibdEdx::print() const
{
  LOG(INFO) << "Total number of entries: " << mEntries[0] << " in A side, " << mEntries[1] << " in C side";
}

void CalibdEdx::dumpToFile(const std::string& file_name) const
{
  TFile file(file_name.c_str(), "recreate");
  file.WriteObject(&mHist[0], "dEdxTotTPC A side");
  file.WriteObject(&mHist[1], "dEdxTotTPC C side");

  file.Close();
}
