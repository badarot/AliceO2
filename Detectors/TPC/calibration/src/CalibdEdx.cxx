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

#include <cstddef>
#include <iostream>
#include <array>

//o2 includes
#include "CommonUtils/MemFileHelper.h"
#include "CCDB/CcdbApi.h"
#include "DataFormatsTPC/dEdxInfo.h"
#include "DataFormatsTPC/TrackTPC.h"
#include "DetectorsCalibration/Utils.h"
#include "Framework/Logger.h"
#include "TPCCalibration/FastHisto.h"

//root includes
#include "TFile.h"
#include "TTree.h"

using namespace o2::tpc;

void CalibdEdx::fill(const gsl::span<const TrackTPC> tracks)
{
  for (const auto& track : tracks) {
    const auto p = track.getP();
    const int cluster_count = track.getNClusters();

    // applying cut
    if (mMinP < p && p < mMaxP || cluster_count >= mMinClusters) {
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
  LOG(INFO) << mEntries[0] << " A side entries and " << mEntries[1] << " C side entries";
}

void CalibdEdx::dumpToFile(const std::string& file_name) const
{
  TFile file(file_name.c_str(), "recreate");
  file.WriteObject(&mHist[0], "dEdxTotTPC A side");
  file.WriteObject(&mHist[1], "dEdxTotTPC C side");

  file.Close();
}
