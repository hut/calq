/** @file SAMRecord.h
 *  @brief This file constains the definition of the SAMRecord struct.
 *  @author Jan Voges (voges)
 *  @bug No known bugs
 */

#ifndef SAMRECORD_H
#define SAMRECORD_H

#include "constants.h"
#include <stdint.h>

/** @brief Struct: SAMRecord
 * 
 *  Structure of a SAM alignment line: The 11 mandatory fields are
 *  TAB-delimited. Optional information is stored as 12th field.
 *  Data types have been selected according to the SAM format specification.
 */
typedef struct {
    char     line[8*MB];

    char     *qname; // Query template NAME
    uint16_t flag;   // bitwise FLAG (uint16_t)
    char     *rname; // Reference sequence NAME
    uint32_t pos;    // 1-based leftmost mapping POSition (uint32_t)
    uint8_t  mapq;   // MAPping Quality (uint8_t)
    char     *cigar; // CIGAR string
    char     *rnext; // Ref. name of the mate/NEXT read
    uint32_t pnext;  // Position of the mate/NEXT read (uint32_t)
    int64_t  tlen;   // observed Template LENgth (int64_t)
    char     *seq;   // segment SEQuence
    char     *qual;  // QUALity scores
    char     *opt;   // OPTional information
} SAMRecord;

#endif // SAMRECORD_H

