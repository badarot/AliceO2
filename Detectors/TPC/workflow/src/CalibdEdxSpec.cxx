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
#include "CCDB/CcdbApi.h"
#include "CCDB/CcdbObjectInfo.h"
#include "DataFormatsTPC/TrackTPC.h"
#include "DetectorsCalibration/Utils.h"
#include "Framework/Task.h"
#include "Framework/DataProcessorSpec.h"
#include "Framework/ConfigParamRegistry.h"
#include "TPCCalibration/CalibdEdx.h"

using namespace o2::framework;

namespace o2
{
namespace tpc
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
    sendOutput(pc.outputs());

    const auto& infoVec = mCalibrator->getInfoVector();
    LOG(INFO) << "Created " << infoVec.size() << " objects for TF " << tfcounter;
  }

  void endOfStream(EndOfStreamContext& eos) final
  {
    LOG(INFO) << "Finalizing calibration";
    // FIXME: not sure about this
    constexpr uint64_t INFINITE_TF = 0xffffffffffffffff;
    mCalibrator->checkSlotsToFinalize(INFINITE_TF);
    sendOutput(eos.outputs());
    mCalibrator->dumpToFile("mip_position.root");
  }

 private:
  void sendOutput(DataAllocator& output)
  {
    // extract CCDB infos and calibration objects, convert it to TMemFile and send them to the output
    // TODO in principle, this routine is generic, can be moved to Utils.h
    using clbUtils = o2::calibration::Utils;
    const auto& payloadVec = mCalibrator->getMIPVector();
    auto& infoVec = mCalibrator->getInfoVector(); // use non-const version as we update it
    assert(payloadVec.size() == infoVec.size());

    // FIXME: not sure about this
    for (uint32_t i = 0; i < payloadVec.size(); i++) {
      auto& entry = infoVec[i];
      auto image = o2::ccdb::CcdbApi::createObjectImage(&payloadVec[i], &entry);
      LOG(INFO) << "Sending object " << entry.getPath() << "/" << entry.getFileName() << " of size " << image->size()
                << " bytes, valid for " << entry.getStartValidityTimestamp() << " : " << entry.getEndValidityTimestamp();
      output.snapshot(Output{o2::calibration::Utils::gDataOriginCDBPayload, "TPC_MIPposition", i}, *image.get()); // vector<char>
      output.snapshot(Output{o2::calibration::Utils::gDataOriginCDBWrapper, "TPC_MIPposition", i}, entry);        // root-serialized
    }
    if (payloadVec.size()) {
      mCalibrator->initOutput(); // reset the outputs once they are already sent
    }
  }

  std::unique_ptr<CalibratordEdx> mCalibrator;
};

DataProcessorSpec getCalibdEdxSpec()
{
  using device = o2::tpc::CalibdEdxDevice;
  using clbUtils = o2::calibration::Utils;

  std::vector<OutputSpec> outputs;

  // FIXME: not sure about this
  outputs.emplace_back(ConcreteDataTypeMatcher{o2::calibration::Utils::gDataOriginCDBPayload, "TPC_MIPposition"});
  outputs.emplace_back(ConcreteDataTypeMatcher{o2::calibration::Utils::gDataOriginCDBWrapper, "TPC_MIPposition"});

  return DataProcessorSpec{
    "tpc-calib-dEdx",
    // select("tracks:TPC/TRACKS"),
    Inputs{
      InputSpec{"tracks", "TPC", "TRACKS"},
    },
    outputs,
    adaptFromTask<device>(),
    Options{
      {"tf-per-slot", VariantType::Int, 100, {"number of TFs per calibration time slot"}},
      {"max-delay", VariantType::Int, 3, {"number of slots in past to consider"}},
      {"min-entries", VariantType::Int, 100, {"minimum number of entries to fit single time slot"}},
      {"min-momentum", VariantType::Double, 0.4, {"minimum momentum cut"}},
      {"max-momentum", VariantType::Double, 0.6, {"maximum momentum cut"}},
      {"min-clusters", VariantType::Int, 60, {"minimum number of clusters in a track"}},
      {"nbins", VariantType::Int, 200, {"number of bins for stored"}}}};
}

} // namespace tpc
} // namespace o2
