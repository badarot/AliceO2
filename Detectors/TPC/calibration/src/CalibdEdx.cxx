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
        mHistEntries[0]++;
        mHist[0].fill(track.getdEdx().dEdxTotTPC);
      } else if (track.hasCSideClustersOnly()) {
        mHistEntries[1]++;
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
  LOG(INFO) << mHistEntries[0] << " A side entries and " << mHistEntries[1] << " C side entries";
}

void CalibdEdx::dumpToFile(const std::string& file_name) const
{
  TFile file(file_name.c_str(), "recreate");
  file.WriteObject(&mHist[0], "dEdxTotTPC A side");
  file.WriteObject(&mHist[1], "dEdxTotTPC C side");

  file.Close();
}

void CalibratordEdx::initOutput()
{

  // Here we initialize the vector of our output objects
  mInfoVector.clear();
  mMIPVector.clear();
  return;
}

void CalibratordEdx::finalizeSlot(Slot& slot)
{
  CalibdEdx* container = slot.getContainer();
  LOG(INFO) << "Finalizing slot " << slot.getTFStart() << " <= TF <= " << slot.getTFEnd();

  auto statsASide = container->getHist()[0].getStatisticsData();
  auto statsCSide = container->getHist()[1].getStatisticsData();

  LOG(INFO) << "A side: Mean = " << statsASide.mCOG << ", StdDev = " << statsCSide.mStdDev << ", Entries = " << statsASide.mSum;
  LOG(INFO) << "C side: Mean = " << statsCSide.mCOG << ", StdDev = " << statsCSide.mStdDev << ", Entries = " << statsCSide.mSum;

  // FIXME: not sure about this

  // TODO: the timestamp is now given with the TF index, but it will have
  // to become an absolute time. This is true both for the lhc phase object itself
  // and the CCDB entry
  MIPposition mip{slot.getTFStart(), {statsASide.mCOG, statsCSide.mCOG}};

  auto clName = o2::utils::MemFileHelper::getClassName(mip);
  auto flName = o2::ccdb::CcdbApi::generateFileName(clName);
  std::map<std::string, std::string> md;
  mInfoVector.emplace_back("TPC/MIPposition", clName, flName, md, slot.getTFStart(), 99999999999999);
  mMIPVector.emplace_back(mip);

  slot.print();
}

CalibratordEdx::Slot& CalibratordEdx::emplaceNewSlot(bool front, uint64_t tstart, uint64_t tend)
{
  auto& cont = getSlots();
  auto& slot = front ? cont.emplace_front(tstart, tend) : cont.emplace_back(tstart, tend);
  slot.setContainer(std::make_unique<CalibdEdx>(mMinClusters, mMinP, mMaxP, mNBins));
  return slot;
}

void CalibratordEdx::dumpToFile(const std::string& file_name) const
{
  TFile file(file_name.c_str(), "recreate");
  file.WriteObject(&mMIPVector, "MIPposition");

  file.Close();
}
