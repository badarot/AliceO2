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

// o2 includes
#include "CCDB/CcdbObjectInfo.h"
#include "DetectorsCalibration/TimeSlotCalibration.h"
#include "DetectorsCalibration/TimeSlot.h"
#include "TPCCalibration/FastHisto.h"

namespace o2
{
namespace tpc
{

class TrackTPC;

class CalibdEdx
{
  using Hist = FastHisto<float>;

 public:
  CalibdEdx(double min_p, double max_p, int min_clusters, unsigned int n_bins)
    : mMinClusters{min_clusters}, mMinP{min_p}, mMaxP{max_p}
    // oh my god this is so ugly
    , mHist{{{n_bins, 0, static_cast<float>(n_bins)}, {n_bins, 0, static_cast<float>(n_bins)}}}
  {
  }

  void fill(const gsl::span<const TrackTPC> tracks);
  void merge(const CalibdEdx* prev);
  void print() const;

  double getASideEntries() const { return mHistEntries[0]; }
  double getCSideEntries() const { return mHistEntries[1]; }

  void dumpToFile(const std::string& file_name) const;
  const std::array<Hist, 2>& getHist() const { return mHist; }

 private:
  int mMinClusters;
  double mMinP, mMaxP;
  std::array<float, 2> mHistEntries{0, 0};
  std::array<Hist, 2> mHist;

  // ClassDefNV(CalibdEdx, 1);
};

class CalibratordEdx final : public o2::calibration::TimeSlotCalibration<TrackTPC, CalibdEdx>
{
  using Slot = o2::calibration::TimeSlot<CalibdEdx>;
  using CcdbObjectInfo = o2::ccdb::CcdbObjectInfo;
  using CcdbObjectInfoVector = std::vector<CcdbObjectInfo>;

 public:
  CalibratordEdx(int min_entries = 100, double min_p = 0.4, double max_p = 0.6, int min_clusters = 60, int n_bins = 200)
    : mMinEntries(min_entries), mMinClusters{min_clusters}, mMinP{min_p}, mMaxP{max_p}, mNBins{n_bins}
  {
  }

  bool hasEnoughData(const Slot& slot) const final {
    return slot.getContainer()->getASideEntries() >= mMinEntries && slot.getContainer()->getCSideEntries() >= mMinEntries;}

  void initOutput() final;
  void finalizeSlot(Slot& slot) final;
  Slot& emplaceNewSlot(bool front, uint64_t tstart, uint64_t tend) final;

 private:
  int mMinEntries;
  int mMinClusters;
  double mMinP, mMaxP;
  int mNBins;

  // ClassDefOverride(CalibratordEdx, 1);
};

} // namespace tpc
} // namespace o2
#endif
