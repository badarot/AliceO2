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

#ifndef ALICEO2_TFIDINFO_H
#define ALICEO2_TFIDINFO_H

#include <Rtypes.h>

namespace o2::dataformats
{
struct TFIDInfo { // helper info to patch DataHeader

  uint32_t firstTForbit = -1;
  uint32_t tfCounter = -1;
  uint32_t runNumber = -1;

  bool isDummy() { return tfCounter == -1; }
  ClassDefNV(TFIDInfo, 1);
};
} // namespace o2::dataformats

#endif
