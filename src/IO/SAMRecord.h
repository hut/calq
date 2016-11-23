/** @file SAMRecord.h
 *  @brief This file contains the definition of the SAMRecord class.
 *  @author Jan Voges (voges)
 *  @bug No known bugs
 */

/*
 *  Changelog
 *  YYYY-MM-DD: What (who)
 */

#ifndef SAMRECORD_H
#define SAMRECORD_H

#include <string>

class SAMRecord {
public:
    static const unsigned int NUM_FIELDS = 12;

public:
    SAMRecord(char *fields[SAMRecord::NUM_FIELDS]);
    ~SAMRecord(void);

    bool isMapped(void) const;
    void print(void) const;

private:
    void check(void);

public:
    std::string qname; // Query template NAME
    uint16_t    flag;  // bitwise FLAG (uint16_t)
    std::string rname; // Reference sequence NAME
    uint32_t    pos;   // 1-based leftmost mapping POSition (uint32_t)
    uint8_t     mapq;  // MAPping Quality (uint8_t)
    std::string cigar; // CIGAR string
    std::string rnext; // Ref. name of the mate/NEXT read
    uint32_t    pnext; // Position of the mate/NEXT read (uint32_t)
    int64_t     tlen;  // observed Template LENgth (int64_t)
    std::string seq;   // segment SEQuence
    std::string qual;  // QUALity scores
    std::string opt;   // OPTional information

    uint32_t posMin; // 0-based leftmost mapping position
    uint32_t posMax; // 0-based rightmost mapping position

private:
    bool mapped;
};

#endif // SAMRECORD_H
