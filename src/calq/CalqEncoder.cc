/** @file CalqEncoder.cc
 *  @brief This file contains the implementation of the CalqEncoder class.
 */

// Copyright 2015-2017 Leibniz Universitaet Hannover

#include "CalqEncoder.h"

#include <chrono>
#include <limits>

#include "Common/constants.h"
#include "Common/Exceptions.h"
#include "Common/log.h"
#include "config.h"
#include "IO/FASTA/FASTAFile.h"
#include "QualCodec/QualEncoder.h"

namespace calq {

CalqEncoder::CalqEncoder(const Options &options)
    : blockSize_(options.blockSize),
      cqFile_(options.outputFileName, CQFile::MODE_WRITE),
      inputFileName_(options.inputFileName),
      polyploidy_(options.polyploidy),
      qualityValueMin_(options.qualityValueMin),
      qualityValueMax_(options.qualityValueMax),
      qualityValueOffset_(options.qualityValueOffset),
      referenceFileNames_(options.referenceFileNames),
      samFile_(options.inputFileName) {
    if (options.blockSize < 1) {
        throwErrorException("blockSize must be greater than zero");
    }
    if (options.inputFileName.empty() == true) {
        throwErrorException("inputFileName is empty");
    }
    if (options.outputFileName.empty() == true) {
        throwErrorException("outputFileName is empty");
    }
    if (options.polyploidy < 1) {
        throwErrorException("polyploidy must be greater than zero");
    }
    if (options.qualityValueMin < 0) {
        throwErrorException("qualityValueMin must be zero or greater");
    }
    if (options.qualityValueMax < 0) {
        throwErrorException("qualityValueMax must be zero or greater");
    }
    if (options.qualityValueOffset < 1) {
        throwErrorException("qualityValueOffset must be greater than zero");
    }
//     if (referenceFileNames.empty() == true) {
//        throwErrorException("referenceFileNames is empty");
//     }

    // Check and, in case they are provided, get reference sequences
    if (referenceFileNames_.empty() == true) {
        CALQ_LOG("No reference file name(s) given - operating without reference sequence(s)");
    } else {
        CALQ_LOG("Looking in %zu reference file(s) for reference sequence(s)", referenceFileNames_.size());
        for (auto const &referenceFileName : referenceFileNames_) {
            CALQ_LOG("Parsing reference file: %s", referenceFileName.c_str());
            FASTAFile fastaFile(referenceFileName);
            CALQ_LOG("Found %zu reference(s):", fastaFile.references.size());
            for (auto const &reference : fastaFile.references) {
                CALQ_LOG("  %s (length: %zu)", reference.first.c_str(), reference.second.length());
            }
        }
    }
}

CalqEncoder::~CalqEncoder(void) {}

void CalqEncoder::encode(void) {
    size_t compressedMappedQualSize = 0;
    size_t compressedUnmappedQualSize = 0;
    size_t uncompressedMappedQualSize = 0;
    size_t uncompressedUnmappedQualSize = 0;

    // Take time
    auto startTime = std::chrono::steady_clock::now();

    // Write CQ file header
    CALQ_LOG("Writing CQ file header");
    cqFile_.writeHeader(blockSize_);

    while (samFile_.readBlock(blockSize_) != 0) {
//         CALQ_LOG("Processing block %zu", samFile_.nrBlocksRead()-1);

        // Check quality value range
        for (auto const &samRecord : samFile_.currentBlock.records) {
            if (samRecord.isMapped() == true) {
                for (auto const &q : samRecord.qual) {
                    if (((int)q-qualityValueOffset_) < qualityValueMin_) {
                        throwErrorException("Quality value too small");
                    }
                    if (((int)q-qualityValueOffset_) > qualityValueMax_) {
                        throwErrorException("Quality value too large");
                    }
                }
            }
        }

        // Encode the quality values
        QualEncoder qualEncoder(polyploidy_, qualityValueMax_, qualityValueMin_, qualityValueOffset_);
        for (auto const &samRecord : samFile_.currentBlock.records) {
            if (samRecord.isMapped() == true) {
                qualEncoder.addMappedRecordToBlock(samRecord);
            } else {
                qualEncoder.addUnmappedRecordToBlock(samRecord);
            }
        }
        qualEncoder.finishBlock();
        qualEncoder.writeBlock(&cqFile_);

        // Update statistics
        compressedMappedQualSize += qualEncoder.compressedMappedQualSize();
        compressedUnmappedQualSize += qualEncoder.compressedUnmappedQualSize();
        uncompressedMappedQualSize += qualEncoder.uncompressedMappedQualSize();
        uncompressedUnmappedQualSize += qualEncoder.uncompressedUnmappedQualSize();
    }

    auto stopTime = std::chrono::steady_clock::now();
    auto diffTime = stopTime - startTime;
    auto diffTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(diffTime).count();
    auto diffTimeS = std::chrono::duration_cast<std::chrono::seconds>(diffTime).count();
    auto diffTimeM = std::chrono::duration_cast<std::chrono::minutes>(diffTime).count();
    auto diffTimeH = std::chrono::duration_cast<std::chrono::hours>(diffTime).count();

    CALQ_LOG("COMPRESSION STATISTICS");
    CALQ_LOG("  Took %d ms ~= %d s ~= %d m ~= %d h", (int)diffTimeMs, (int)diffTimeS, (int)diffTimeM, (int)diffTimeH);
    CALQ_LOG("  Speed (uncompressed size/time): %.2f MB/s", ((double)((double)(uncompressedMappedQualSize+uncompressedUnmappedQualSize)/(double)MB))/((double)diffTimeS));
    CALQ_LOG("  Wrote %zu block(s)", samFile_.nrBlocksRead());
    CALQ_LOG("  Record(s):  %12zu", samFile_.nrRecordsRead());
    CALQ_LOG("    Mapped:   %12zu", samFile_.nrMappedRecordsRead());
    CALQ_LOG("    Unmapped: %12zu", samFile_.nrUnmappedRecordsRead());
    CALQ_LOG("  Uncompressed size: %12zu", uncompressedMappedQualSize+uncompressedUnmappedQualSize);
    CALQ_LOG("    Mapped:          %12zu", uncompressedMappedQualSize);
    CALQ_LOG("    Unmapped:        %12zu", uncompressedUnmappedQualSize);
    CALQ_LOG("  Compressed size: %12zu", cqFile_.nrWrittenBytes());
    CALQ_LOG("    File format:   %12zu", cqFile_.nrWrittenFileFormatBytes());
    CALQ_LOG("    Mapped:        %12zu", compressedMappedQualSize);
    CALQ_LOG("    Unmapped:      %12zu", compressedUnmappedQualSize);
    CALQ_LOG("  Compression ratio: %4.2f%%", (double)cqFile_.nrWrittenBytes()*100/(double)(uncompressedMappedQualSize+uncompressedUnmappedQualSize));
    CALQ_LOG("    Mapped:          %4.2f%%", (double)compressedMappedQualSize*100/(double)(uncompressedMappedQualSize));
    CALQ_LOG("    Unmapped:        %4.2f%%", (double)compressedUnmappedQualSize*100/(double)(uncompressedUnmappedQualSize));
    CALQ_LOG("  Compression factor: %4.2f", (double)(uncompressedMappedQualSize+uncompressedUnmappedQualSize)/(double)cqFile_.nrWrittenBytes());
    CALQ_LOG("    Mapped:           %4.2f", (double)(uncompressedMappedQualSize)/(double)compressedMappedQualSize);
    CALQ_LOG("    Unmapped:         %4.2f", (double)(uncompressedUnmappedQualSize)/(double)compressedUnmappedQualSize);
    CALQ_LOG("  Bits per quality value: %2.4f", ((double)cqFile_.nrWrittenBytes() * 8)/(double)(uncompressedMappedQualSize+uncompressedUnmappedQualSize));
    CALQ_LOG("    Mapped:               %2.4f", ((double)compressedMappedQualSize * 8)/(double)(uncompressedMappedQualSize));
    CALQ_LOG("    Unmapped:             %2.4f", ((double)compressedUnmappedQualSize * 8)/(double)(uncompressedUnmappedQualSize));
}

}  // namespace calq

