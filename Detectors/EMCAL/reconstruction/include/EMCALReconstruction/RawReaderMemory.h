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
#ifndef ALICEO2_EMCAL_RAWREADERMEMORY_H
#define ALICEO2_EMCAL_RAWREADERMEMORY_H

#include <gsl/span>
#include <Rtypes.h>

#include "EMCALBase/RCUTrailer.h"
#include "EMCALReconstruction/RawBuffer.h"
#include "EMCALReconstruction/RawPayload.h"
#include "Headers/RAWDataHeader.h"
#include "Headers/RDHAny.h"

namespace o2
{

namespace emcal
{

/// \class RawReaderMemory
/// \brief Reader for raw data produced by the Readout application in in-memory format
/// \ingroup EMCALreconstruction
/// \author Markus Fasel <markus.fasel@cern.ch>, Oak Ridge National Laboratory
/// \since Nov. 14, 2019
///
///
class RawReaderMemory
{
 public:
  /// \brief Constructor
  RawReaderMemory(const gsl::span<const char> rawmemory);

  /// \brief Destructor
  ~RawReaderMemory() = default;

  /// \brief set new raw memory chunk
  /// \param rawmemory New raw memory chunk
  void setRawMemory(const gsl::span<const char> rawmemory);

  /// \brief Set range for DDLs from SRU (for RCU trailer merging)
  /// \param minDDL Min DDL of the SRU DDL range
  /// \param maxDDL Max DDL of the SRU DDL range
  void setRangeSRUDDLs(uint16_t minDDL, uint16_t maxDDL)
  {
    mMinSRUDDL = minDDL;
    mMaxSRUDDL = maxDDL;
  }

  /// \brief Read next payload from the stream
  ///
  /// Read the next pages until the stop bit is found.
  void
    next();

  /// \brief Read the next page from the stream (single DMA page)
  /// \param resetPayload If true the raw payload is reset
  /// \throw Error if the page cannot be read or header or payload cannot be deocded
  ///
  /// Function reading a single DMA page from the stream. It is called
  /// inside the next() function for reading payload from multiple DMA
  /// pages. As the function cannot handle payload from multiple pages
  /// it should not be called directly by the user.
  void nextPage(bool resetPayload = true);

  /// \brief access to the raw header of the current page
  /// \return Raw header of the current page
  /// \throw RawDecodingError with HEADER_INVALID if the header was not decoded
  const o2::header::RDHAny& getRawHeader() const;

  /// \brief access to the raw buffer (single DMA page)
  /// \return Raw buffer of the current page
  /// \throw Error with PAYLOAD_INCALID if payload was not decoded
  const RawBuffer& getRawBuffer() const;

  /// \brief access to the full raw payload (single or multiple DMA pages)
  /// \return Raw Payload of the data until the stop bit is received.
  const RawPayload& getPayload() const { return mRawPayload; }

  /// \brief Return size of the payload
  /// \return size of the payload
  int getPayloadSize() const { return mRawPayload.getPayloadSize(); }

  /// \brief get the size of the file in bytes
  /// \return size of the file in byte
  int getFileSize() const noexcept { return mRawMemoryBuffer.size(); }

  /// \brief check if more pages are available in the raw file
  /// \return true if there is a next page
  bool hasNext() const { return mCurrentPosition < mRawMemoryBuffer.size(); }

 protected:
  /// \brief Initialize the raw stream
  ///
  /// Rewind stream to the first entry
  void init();

  /// \brief Check whether the current page is accepted
  /// \param page Raw page to check
  /// \return True if the page is accepted, false otherwise
  bool acceptPage(const char* page) const;

  o2::header::RDHAny decodeRawHeader(const void* headerwords);

 private:
  gsl::span<const char> mRawMemoryBuffer; ///< Memory block with multiple DMA pages
  RawBuffer mRawBuffer;                   ///< Raw buffer
  o2::header::RDHAny mRawHeader;          ///< Raw header
  RawPayload mRawPayload;                 ///< Raw payload (can consist of multiple pages)
  RCUTrailer mCurrentTrailer;             ///< RCU trailer
  uint64_t mTrailerPayloadWords = 0;      ///< Payload words in common trailer
  uint16_t mMinSRUDDL = 0;                ///< Min. range of SRU DDLs (for RCU trailer merging)
  uint16_t mMaxSRUDDL = 39;               ///< Max. range of SRU DDls (for RCU trailer merging)
  int mCurrentPosition = 0;               ///< Current page in file
  int mCurrentFEE = -1;                   ///< Current FEE in the data stream
  bool mRawHeaderInitialized = false;     ///< RDH for current page initialized
  bool mPayloadInitialized = false;       ///< Payload for current page initialized

  ClassDefNV(RawReaderMemory, 1);
};

} // namespace emcal

} // namespace o2

#endif