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

/// \file TrivialVertexer.cxx
/// \brief Implementation of the ITS trivial vertex finder

#include <limits>

#include "TFile.h"
#include "TTree.h"

#include "FairMCEventHeader.h"
#include "FairLogger.h"

#include "ITSReconstruction/TrivialVertexer.h"
#include "DataFormatsITSMFT/CompCluster.h"
#include "SimulationDataFormat/MCCompLabel.h"
#include "SimulationDataFormat/MCTruthContainer.h"

using namespace o2::itsmft;
using namespace o2::its;

using Point3Df = o2::math_utils::Point3D<float>;

TrivialVertexer::TrivialVertexer() = default;

TrivialVertexer::~TrivialVertexer()
{
  if (mHeader)
    delete mHeader;
  if (mTree)
    delete mTree;
  if (mFile)
    delete mFile;
}

Bool_t TrivialVertexer::openInputFile(const Char_t* fname)
{
  mFile = TFile::Open(fname, "old");
  if (!mFile) {
    LOG(error) << "TrivialVertexer::openInputFile() : "
               << "Cannot open the input file !";
    return kFALSE;
  }
  mTree = (TTree*)mFile->Get("o2sim");
  if (!mTree) {
    LOG(error) << "TrivialVertexer::openInputFile() : "
               << "Cannot get the input tree !";
    return kFALSE;
  }
  Int_t rc = mTree->SetBranchAddress("MCEventHeader.", &mHeader);
  if (rc != 0) {
    LOG(error) << "TrivialVertexer::openInputFile() : "
               << "Cannot get the input branch ! rc=" << rc;
    return kFALSE;
  }
  return kTRUE;
}

void TrivialVertexer::process(const std::vector<CompCluster>& clusters, std::vector<std::array<Double_t, 3>>& vertices)
{
  if (mClsLabels == nullptr) {
    LOG(info) << "TrivialVertexer::process() : "
              << "No cluster labels available ! Running with a default MC vertex...";
    vertices.emplace_back(std::array<Double_t, 3>{0., 0., 0.});
    return;
  }

  if (mTree == nullptr) {
    LOG(info) << "TrivialVertexer::process() : "
              << "No MC information available ! Running with a default MC vertex...";
    vertices.emplace_back(std::array<Double_t, 3>{0., 0., 0.});
    return;
  }

  Int_t lastEventID = 0;
  Int_t firstEventID = std::numeric_limits<Int_t>::max();

  // Find the first and last MC event within this TF
  for (Int_t i = 0; i < clusters.size(); ++i) {
    auto mclab = (mClsLabels->getLabels(i))[0];
    if (mclab.getTrackID() == -1)
      continue; // noise
    auto id = mclab.getEventID();
    if (id < firstEventID)
      firstEventID = id;
    if (id > lastEventID)
      lastEventID = id;
  }

  for (Int_t mcEv = firstEventID; mcEv <= lastEventID; ++mcEv) {
    mTree->GetEvent(mcEv);
    Double_t vx = mHeader->GetX();
    Double_t vy = mHeader->GetY();
    Double_t vz = mHeader->GetZ();
    vertices.emplace_back(std::array<Double_t, 3>{vx, vy, vz});
    LOG(info) << "TrivialVertexer::process() : "
              << "MC event #" << mcEv << " with vertex (" << vx << ',' << vy << ',' << vz << ')';
  }
}
