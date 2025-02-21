// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "PHOSCalibWorkflow/PHOSPedestalCalibDevice.h"
#include "PHOSCalibWorkflow/PHOSHGLGRatioCalibDevice.h"
#include "PHOSCalibWorkflow/PHOSEnergyCalibDevice.h"
#include "PHOSCalibWorkflow/PHOSTurnonCalibDevice.h"
#include "PHOSCalibWorkflow/PHOSRunbyrunCalibDevice.h"
#include "PHOSCalibWorkflow/PHOSBadMapCalibDevice.h"
#include "Framework/DataProcessorSpec.h"
#include "CommonUtils/ConfigurableParam.h"
#include "CommonUtils/NameConf.h"

using namespace o2::framework;

// // we need to add workflow options before including Framework/runDataProcessing
void customize(std::vector<o2::framework::ConfigParamSpec>& workflowOptions)
{
  // option allowing to set parameters
  // which method should be called
  workflowOptions.push_back(ConfigParamSpec{"pedestals", o2::framework::VariantType::Bool, false, {"do pedestal calculation"}});
  workflowOptions.push_back(ConfigParamSpec{"hglgratio", o2::framework::VariantType::Bool, false, {"do HG/LG ratio calculation"}});
  workflowOptions.push_back(ConfigParamSpec{"turnon", o2::framework::VariantType::Bool, false, {"scan trigger turn-on curves"}});
  workflowOptions.push_back(ConfigParamSpec{"runbyrun", o2::framework::VariantType::Bool, false, {"do run by run correction calculation"}});
  workflowOptions.push_back(ConfigParamSpec{"energy", o2::framework::VariantType::Bool, false, {"collect tree for E calib"}});
  workflowOptions.push_back(ConfigParamSpec{"badmap", o2::framework::VariantType::Bool, false, {"do bad map calculation"}});

  //
  workflowOptions.push_back(ConfigParamSpec{"not-use-ccdb", o2::framework::VariantType::Bool, false, {"enable access to ccdb phos calibration objects"}});
  workflowOptions.push_back(ConfigParamSpec{"forceupdate", o2::framework::VariantType::Bool, false, {"update ccdb even difference to previous object large"}});

  workflowOptions.push_back(ConfigParamSpec{"ptminmgg", o2::framework::VariantType::Float, 1.5f, {"minimal pt to fill mgg calib histos"}});
  workflowOptions.push_back(ConfigParamSpec{"eminhgtime", o2::framework::VariantType::Float, 1.5f, {"minimal E (GeV) to fill HG time calib histos"}});
  workflowOptions.push_back(ConfigParamSpec{"eminlgtime", o2::framework::VariantType::Float, 5.f, {"minimal E (GeV) to fill LG time calib histos"}});
  workflowOptions.push_back(ConfigParamSpec{"ecalibdigitmin", o2::framework::VariantType::Float, 0.05f, {"minimal digtit E (GeV) to keep digit for calibration"}});
  workflowOptions.push_back(ConfigParamSpec{"ecalibclumin", o2::framework::VariantType::Float, 0.4f, {"minimal cluster E (GeV) to keep digit for calibration"}});

  // BadMap
  workflowOptions.push_back(ConfigParamSpec{"mode", o2::framework::VariantType::Int, 0, {"operation mode: 0: occupancy, 1: chi2, 2: pedestals"}});

  workflowOptions.push_back(ConfigParamSpec{"configKeyValues", VariantType::String, "", {"Semicolon separated key=value strings"}});
}

// ------------------------------------------------------------------
// we need to add workflow options before including Framework/runDataProcessing
#include "Framework/runDataProcessing.h"

WorkflowSpec defineDataProcessing(ConfigContext const& configcontext)
{
  WorkflowSpec specs;
  o2::conf::ConfigurableParam::updateFromString(configcontext.options().get<std::string>("configKeyValues"));
  auto doPedestals = configcontext.options().get<bool>("pedestals");
  auto doHgLgRatio = configcontext.options().get<bool>("hglgratio");
  auto doTurnOn = configcontext.options().get<bool>("turnon");
  auto doRunbyrun = configcontext.options().get<bool>("runbyrun");
  auto doEnergy = configcontext.options().get<bool>("energy");
  auto doBadMap = configcontext.options().get<bool>("badmap");
  auto useCCDB = !configcontext.options().get<bool>("not-use-ccdb");
  auto forceUpdate = configcontext.options().get<bool>("forceupdate");

  float ptMin = configcontext.options().get<float>("ptminmgg");
  float eMinHGTime = configcontext.options().get<float>("eminhgtime");
  float eMinLGTime = configcontext.options().get<float>("eminlgtime");
  float eCalibDigitMin = configcontext.options().get<float>("ecalibdigitmin");
  float eCalibCluMin = configcontext.options().get<float>("ecalibclumin");

  if (doPedestals && doHgLgRatio) {
    LOG(fatal) << "Can not run pedestal and HG/LG calibration simulteneously";
  }

  LOG(info) << "PHOS Calibration workflow: options";
  LOG(info) << "useCCDB = " << useCCDB;
  if (doPedestals) {
    LOG(info) << "pedestals ";
    specs.emplace_back(o2::phos::getPedestalCalibSpec(useCCDB, forceUpdate));
  } else {
    if (doHgLgRatio) {
      LOG(info) << "hglgratio ";
      specs.emplace_back(o2::phos::getHGLGRatioCalibSpec(useCCDB, forceUpdate));
    }
  }
  if (doEnergy) {
    LOG(info) << "Filling tree for energy and time calibration ";
    specs.emplace_back(o2::phos::getPHOSEnergyCalibDeviceSpec(useCCDB, ptMin, eMinHGTime, eMinLGTime, eCalibDigitMin, eCalibCluMin));
  }
  if (doTurnOn) {
    LOG(info) << "TurnOn curves calculation";
    specs.emplace_back(o2::phos::getPHOSTurnonCalibDeviceSpec(useCCDB));
  }
  if (doRunbyrun) {
    LOG(info) << "Run by run correction calculation on ";
    specs.emplace_back(o2::phos::getPHOSRunbyrunCalibDeviceSpec(useCCDB));
  }
  if (doBadMap) {
    LOG(info) << "bad map calculation ";
    int mode = configcontext.options().get<int>("mode");
    ;
    specs.emplace_back(o2::phos::getBadMapCalibSpec(mode));
  }
  return specs;
}
