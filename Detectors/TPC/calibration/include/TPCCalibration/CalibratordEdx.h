// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#ifndef ALICEO2_TPC_CALIBRATORDEDX_H_
#define ALICEO2_TPC_CALIBRATORDEDX_H_

#include <string>
#include <array>

// o2 includes
#include "DataFormatsTPC/TrackTPC.h"
#include "CCDB/CcdbObjectInfo.h"
#include "DetectorsCalibration/TimeSlotCalibration.h"
#include "DetectorsCalibration/TimeSlot.h"
#include "TPCCalibration/CalibdEdx.h"
#include "TPCCalibration/FastHisto.h"

namespace o2::tpc
{

// FIXME: not sure about this
struct CalibMIP {
  uint64_t timeFrame;
  std::vector<double> mip;
};

class CalibratordEdx final : public o2::calibration::TimeSlotCalibration<o2::tpc::TrackTPC, o2::tpc::CalibdEdx>
{
  using Slot = o2::calibration::TimeSlot<CalibdEdx>;
  using CcdbObjectInfoVector = std::vector<o2::ccdb::CcdbObjectInfo>;
  using MIPVector = std::vector<CalibMIP>;

 public:
  CalibratordEdx(int minEntries = 100, double minP = 0.4, double maxP = 0.6, int minClusters = 60, int nBins = 200)
    : mMinEntries(minEntries), mMinClusters{minClusters}, mMinP{minP}, mMaxP{maxP}, mNBins{nBins}
  {
  }
  ~CalibratordEdx() final = default;

  bool hasEnoughData(const Slot& slot) const final
  {
    return slot.getContainer()->getASideEntries() >= mMinEntries && slot.getContainer()->getCSideEntries() >= mMinEntries;
  }

  void initOutput() final;
  void finalizeSlot(Slot& slot) final;
  Slot& emplaceNewSlot(bool front, uint64_t tstart, uint64_t tend) final;

  const MIPVector& getMIPVector() const { return mMIPVector; }
  const CcdbObjectInfoVector& getInfoVector() const { return mInfoVector; }
  CcdbObjectInfoVector& getInfoVector() { return mInfoVector; }
  void dumpToFile(const std::string& fileName) const;

 private:
  int mMinEntries{};  ///< Minimum amount of tracks in each time slot, to get enough statics
  int mMinClusters{}; ///< Minimum number of clusters in a track
  double mMinP{};     ///< Minimum track momentum
  double mMaxP{};     ///< Maximum track momentum
  int mNBins{};       ///< Number of bins in each time slot histogram

  CcdbObjectInfoVector mInfoVector; ///< vector of CCDB Infos, each element is filled with the CCDB description of the accompanying MIP positions
  MIPVector mMIPVector;             ///< vector of MIP positions, each element is filled in "process" when we finalize one slot (multiple can be finalized during the same "process", which is why we have a vector. Each element is to be considered the output of the device, and will go to the CCDB

  ClassDefOverride(CalibratordEdx, 1);
};

} // namespace o2::tpc
#endif
