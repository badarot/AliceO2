// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "TPCCalibration/CalibratordEdx.h"

#include <cstddef>
#include <array>

//o2 includes
#include "CommonUtils/MemFileHelper.h"
#include "CCDB/CcdbApi.h"
#include "DetectorsCalibration/Utils.h"
#include "Framework/Logger.h"
#include "TPCCalibration/CalibdEdx.h"

//root includes
#include "TFile.h"
#include "TTree.h"

using namespace o2::tpc;

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

  slot.print();
  LOG(INFO) << "A side, truncated mean statistics: Mean = " << statsASide.mCOG << ", StdDev = " << statsCSide.mStdDev << ", Entries = " << statsASide.mSum;
  LOG(INFO) << "C side, truncated mean statistics: Mean = " << statsCSide.mCOG << ", StdDev = " << statsCSide.mStdDev << ", Entries = " << statsCSide.mSum;

  // FIXME: not sure about this

  // TODO: the timestamp is now given with the TF index, but it will have
  // to become an absolute time. This is true both for the lhc phase object itself
  // and the CCDB entry
  CalibMIP mip{slot.getTFStart(), {statsASide.mCOG, statsCSide.mCOG}};

  auto clName = o2::utils::MemFileHelper::getClassName(mip);
  auto flName = o2::ccdb::CcdbApi::generateFileName(clName);
  std::map<std::string, std::string> md;
  mInfoVector.emplace_back("TPC/Calib/MIPS", clName, flName, md, slot.getTFStart(), 99999999999999);
  mMIPVector.push_back(mip);
}

CalibratordEdx::Slot& CalibratordEdx::emplaceNewSlot(bool front, uint64_t tstart, uint64_t tend)
{
  auto& cont = getSlots();
  auto& slot = front ? cont.emplace_front(tstart, tend) : cont.emplace_back(tstart, tend);
  if (mApplyCuts) {
    slot.setContainer(std::make_unique<CalibdEdx>(mMinP, mMaxP, mMinClusters, mNBins));
  } else {
    slot.setContainer(std::make_unique<CalibdEdx>(mNBins));
  }
  return slot;
}

void CalibratordEdx::dumpToFile(const std::string& file_name) const
{
  LOG(INFO) << "Writing mip positions to file";
  TFile file(file_name.c_str(), "recreate");
  CalibMIP data;

  TTree tree("mip_position", "MIP position over time");
  tree.Branch("mip", &data);

  LOG(INFO) << "Writing " << mMIPVector.size() << " entries";
  for (const auto& x : mMIPVector) {
    data = x;
    tree.Fill();
  }

  file.Write();
  file.Close();
}
