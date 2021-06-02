// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "TPCWorkflow/CalibdEdxSpec.h"

#include <vector>
#include <memory>

//o2 includes
#include "Framework/Task.h"
#include "Framework/DataProcessorSpec.h"
#include "Framework/ConfigParamRegistry.h"
#include "DataFormatsTPC/TrackTPC.h"
#include "DetectorsCalibration/Utils.h"
#include "TPCCalibration/CalibdEdx.h"

using namespace o2::framework;
namespace o2
{
namespace calibration
{

class CalibdEdxDevice : public Task
{
 public:
  void init(framework::InitContext& ic) final
  {
    int slot_length = ic.options().get<int>("tf-per-slot");
    int max_delay = ic.options().get<int>("max-delay");
    int min_entries = std::max(50, ic.options().get<int>("min-entries"));
    double min_p = std::min(0.49, ic.options().get<double>("min-momentum"));
    double max_p = std::max(0.51, ic.options().get<double>("max-momentum"));
    int min_clusters = std::max(15, ic.options().get<int>("min-clusters"));
    int nbins = std::max(50, ic.options().get<int>("nbins"));

    mCalibrator = std::make_unique<tpc::CalibratordEdx>(min_entries, min_p, max_p, min_clusters, nbins);
    mCalibrator->setSlotLength(slot_length);
    mCalibrator->setMaxSlotsDelay(max_delay);
  }

  void run(ProcessingContext& pc) final
  {
    auto tfcounter = o2::header::get<DataProcessingHeader*>(pc.inputs().get("tracks").header)->startTime;
    auto tracks = pc.inputs().get<gsl::span<tpc::TrackTPC>>("tracks");

    LOG(INFO) << "Processing TF " << tfcounter << " with " << tracks.size() << " tracks";

    mCalibrator->process(tfcounter, tracks);


  }

  void endOfStream(EndOfStreamContext& eos) final
  {
    // mCalibrator->dumpToFile("test.root");
  }

 private:
  std::unique_ptr<tpc::CalibratordEdx> mCalibrator;
};

} // namespace calibration

namespace framework {
DataProcessorSpec getCalibdEdxSpec()
{
  return DataProcessorSpec{
    "tpc-calib-dEdx",
    // select("tracks:TPC/TRACKS"),
    Inputs{
      InputSpec{"tracks", "TPC", "TRACKS"},
    },
    Outputs{},
    adaptFromTask<calibration::CalibdEdxDevice>(),
    Options{
      {"tf-per-slot", VariantType::Int, 100, {"number of TFs per calibration time slot"}},
      {"max-delay", VariantType::Int, 3, {"number of slots in past to consider"}},
      {"min-entries", VariantType::Int, 100, {"minimum number of entries to fit single time slot"}},
      {"min-momentum", VariantType::Double, 0.4, {"minimum momentum cut"}},
      {"max-momentum", VariantType::Double, 0.6, {"maximum momentum cut"}},
      {"min-clusters", VariantType::Int, 60, {"minimum number of clusters in a track"}},
      {"nbins", VariantType::Int, 200, {"number of bins for stored"}}}};
}

} // namespace framwork
} // namespace o2
