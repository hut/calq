/** @file CalqCodec.h
 *  @brief This file contains the definitions of the CalqEncoder and
 *         CalqDecoder classes.
 *  @author Jan Voges (voges)
 *  @bug No known bugs
 */

/*
 *  Changelog
 *  YYYY-MM-DD: what (who)
 */

#ifndef CALQCODEC_H
#define CALQCODEC_H

#include "bitstream.h"
#include "Codecs/QualCodec.h"
#include "Parsers/FASTAParser.h"
#include "Parsers/SAMParser.h"
#include <fstream>
#include <string>
#include <vector>

/** @brief Class: CalqCodec
*
*  The CalqEncoder and the CalqDecoder inherit common methods from this class.
*/
class CalqCodec {
public:
    CalqCodec(const std::string &inFileName,
              const std::string &outFileName,
              const std::vector<std::string> &fastaFileNames);
    virtual ~CalqCodec(void);

    void readFastaReferences(void);

protected:
    const std::vector<std::string> fastaFileNames;
    std::vector<FASTAReference> fastaReferences;
    const std::string inFileName;
    const std::string outFileName;

private:
    FASTAParser fastaParser;
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
                const std::vector<std::string> &fastaFileNames);
    ~CalqEncoder(void);

    void encode(void);

private:
    ofbitstream ofbs;
    QualEncoder qualEncoder;
    SAMParser samParser;
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
                const std::vector<std::string> &fastaFileNames);
    ~CalqDecoder(void);

    void decode(void);
 
private:
    ifbitstream ifbs;
    std::ofstream ofs;
    QualDecoder qualDecoder;
};

#endif // CALQCODEC_H
