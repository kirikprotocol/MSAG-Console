#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_GLOBALTITLE_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_GLOBALTITLE_HPP__

# include <sys/types.h>
# include <string>

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

class GlobalTitle {
public:
  typedef enum { NATURE_OF_ADDRESS_TAKENOVER = 0x01, GT_INCLUDE_TRANSLATION_TYPE_ONLY = 0x02,
                 NUMPLAN_AND_TRNSLTYPE_TAKENOVER = 0x03, GT_INCLUDE_FULL_INFO = 0x04 } gti_t;
  typedef enum { UNKNOWN_PLAN=0, E164_NUMBERING_PLAN = 1, GENERIC_NUMBERING_PLAN = 2,
                 DATA_NUMBERING_PLAN = 3, TELEX_NUMBERING_PLAN = 4, MARITIME_MOBILE_NUMBERING_PLAN = 5,
                 LAND_MOBILE_NUMBERING_PLAN = 6, E214_NUMBERING_PLAN = 7, PRIVATE_NETWORK_PLAN = 14,
                 RESERVED_PLAN = 127 } numbering_plan_t;
  typedef enum { UNKNOWN_NATURE_OF_ADDRESS = 0, SUBSCRIBER_NUMBER_T = 1, RESERVED_FOR_NATIONAL_USE = 2,
                 NATIONAL_SIGNIFICANT_NUMBER = 3, INTERNATIONAL_NUMBER = 4 } nature_addr_t;

  typedef enum { TRANSLATION_TYPE_UNKNOWN=0, TRANSLATION_TYPE_INTERNATIONAL_SERVICE_1 } translation_type_t;

  GlobalTitle();

  GlobalTitle(const uint8_t* buf, size_t buf_sz);
  // GTI = 0x01 (nature of address is taken over; it is implicitly assumed that the translation type = Unknown,
  // Numbering Plan = E.164)
  GlobalTitle(nature_addr_t nature_of_address, const std::string& gt_digits);

  // GTI = 0x02 (global title includes translation type only)
  GlobalTitle(uint8_t translation_type, const std::string& gt_digits);

  // GTI = 0x03 (numbering plan and translation type are taken over; it is implicitly assumed 
  // that the Nature of Address = Unknown)
  GlobalTitle(uint8_t translation_type, numbering_plan_t num_plan, const std::string& gt_digits);
  
  // gti, translation type, numbering plan and nature of address is taken over in arguments
  GlobalTitle(uint8_t translation_type, numbering_plan_t num_plan, nature_addr_t addr_nature, const std::string& gt_digits);

  GlobalTitle(uint8_t nature_of_address, const uint8_t* packed_gt_digits, size_t packed_gt_digits_sz);

  GlobalTitle(uint8_t translation_type, uint8_t num_plan, uint8_t enc_scheme, const uint8_t* packed_gt_digits, size_t packed_gt_digits_sz);

  GlobalTitle(uint8_t translation_type, uint8_t num_plan, uint8_t enc_scheme, uint8_t nature_of_address, const uint8_t* packed_gt_digits, size_t packed_gt_digits_sz);

  uint8_t getGTI() const;
  uint8_t getTranslationType() const;
  uint8_t getNumberingPlan() const;
  uint8_t getNatureOfAddress() const;

  std::string getGlobalTitleDigits() const;
  size_t getNumOfGlobalTitleDigits() const;

  const uint8_t* getPackedGTDigits() const;
  size_t getPackedGTDigitsSize() const;

  const uint8_t* getValue() const;
  size_t getValueSz() const;

  std::string toString() const;
private:
  const static size_t MAX_NUM_OF_DIGITS = 32;
  const static size_t GT_HEADER_SZ = 5; // GTI (1 octet) + No. Digits (1 octet) + Trans. Type (1 octet) + Num. Plan (1 octet) + Nature of Address (1 octet)
  enum { GTI_IDX=0, DIGIT_NUM_IDX=1, TRANS_TYPE_IDX=2, NUM_PLAN_IDX=3, NATURE_OF_ADDR_IDX=4 };
  enum { ENCODING_BCD_ODD_NUMBER_DIGITS = 0x01, ENCODING_BCD_EVEN_NUMBER_DIGITS = 0x02 };
  struct gt_buf {
    uint8_t buf[GT_HEADER_SZ + MAX_NUM_OF_DIGITS/2];
    size_t packedDataSz;
  } _gt_buffer;

  size_t packGTDigits(const std::string& gtDigits);
};

}}}}}

#endif
