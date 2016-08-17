/** @file CalqCodec.h
 *  @brief This file contains the definitions of the CalqEncoder and
 *         CalqDecoder classes.
 *  @author Jan Voges (voges)
 *  @bug No known bugs
 */

/*
 *  Changelog
 *  YYYY-MM-DD: What (who)
 */

#ifndef CALQCODEC_H
#define CALQCODEC_H

#include "Common/File.h"
#include "Parsers/FASTAParser.h"
#include "Parsers/SAMParser.h"
#include "QualCodec.h"
#include <fstream>
#include <string>
#include <vector>

/** @brief Class: CalqCodec
 *
 *  The CalqEncoder and the CalqDecoder classes inherit common methods from
 *  this class.
 */
class CalqCodec {
public:
    /** @brief Constructor: CalqCodec
     *
     *  Initializes a new CalqCodec instance and reads the reference
     *  sequence(s) from all provided FASTA files. The reference
     *  sequence(s) are stored in 'fastaReferences'.
     *
     *  @param inFileName File name of the input file to be en- or decoded
     *  @param outfileName File name of the output file
     *  @param fastaFileNames Vector containing the file names of the FASTA
     *         which contain the reference sequence(s) used during en- or
     *         decoding
     */
    CalqCodec(const std::string &inFileName,
              const std::string &outFileName,
              const std::vector<std::string> &fastaFileNames);

    /** @brief Destructor: CalqCodec
     *
     *  Destructs a CalqCodec instance.
     */
    virtual ~CalqCodec(void);

protected:
    std::vector<FASTAReference> fastaReferences;
    const std::string inFileName;
    const std::string outFileName;
};

/** @brief Class: CalqEncoder
*
*  The CalqEncoder provides only one interface to the outside which is the
*  member function encode; it encodes the SAM file with the name infileName
*  with the specified blockSize and writes the encoded bitstream to the CQ
*  file with the name outfileName.
*/
class CalqEncoder: public CalqCodec {
public:
    CalqEncoder(const std::string &samFileName,
                const std::string &cqFileName,
                const std::vector<std::string> &fastaFileNames,
                const unsigned int &blockSize,
                const unsigned int &polyploidy,
                const int &qvMin,
                const int &qvMax);
    ~CalqEncoder(void);

    void encode(void);

private:
    unsigned int blockSize;
    File cqFile;
    unsigned int polyploidy;
    QualEncoder qualEncoder;
    SAMParser samParser;

    size_t writeFileHeader(void);
};

/** @brief Class: CalqDecoder
 *
 *  The CalqDecoder provides only one interface to the outside which is the
 *  member function decode; it decodes the CQ file with the name infileName
 *  and writes the decoded quality scores to the file with the name outfileName.
 */
class CalqDecoder: public CalqCodec {
public:
    CalqDecoder(const std::string &cqFileName,
                const std::string &qualFileName,
                const std::string &samFileName,
                const std::vector<std::string> &fastaFileNames);
    ~CalqDecoder(void);

    void decode(void);

private:
    File cqFile;
    File qualFile;
    QualDecoder qualDecoder;
    SAMParser samParser;

    size_t readFileHeader(void);
};

#endif // CALQCODEC_H

