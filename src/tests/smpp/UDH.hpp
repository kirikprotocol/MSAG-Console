#ifndef ___USER_DATA_HEADER_CLASS___
#define ___USER_DATA_HEADER_CLASS___

#include <vector>
#include <string>
#include <sstream>

namespace smsc {
  namespace test {
    namespace smpp {

      class IE {// Information element
        uint8_t *IEI; //information element identifier
        uint8_t *IEDL;//information element data length
        uint8_t *IED; //information element data
      public:
        IE() {}
        IE(const char *buf) {
          IEI = (uint8_t*) buf;
          IEDL = (uint8_t*) buf + 1;
          if(getInformationElementDataLength() > 0) {
            IED = (uint8_t*) buf + 2;
          } else {
            IED = 0;
          }
        }
        uint8_t getInformationElementIdentifier() {
          return *IEI;
        }
        uint8_t getInformationElementDataLength() {
          return *IEDL;
        }
        uint8_t* getInformationElementData() {
          return IED;
        }
      };

      class UDH {//user data header
        uint8_t *length;
        IE  *elements;
        uint8_t elementsNumber;
      public:
        UDH(const char * buf) {
          length = (uint8_t*) buf;
          std::vector<IE> elems;
          uint8_t len = getLength();
          if(len > 0) {
            const char *ptr = buf + 1;
            while(len > 0) {
              IE ie(ptr);
              elems.push_back(ie);
              len -= 2 + ie.getInformationElementDataLength();
              ptr += 2 + ie.getInformationElementDataLength();
            }
            elementsNumber = elems.size();
            elements = new IE[elems.size()];
            for(int i=0; i<elems.size(); i++) {
              elements[i] = elems[i];
            }
          } else {
            elements = 0;
          }
        }
        uint8_t getLength() {
          return *length;
        }
        IE* getElements() {
          return elements;
        }
        uint8_t getElementsNumber() {
          return elementsNumber;
        }
        bool check() {
          //длина UDH должна совпадать с суммой длин IE
          uint8_t len = 0;
          for(int i=0; i<getElementsNumber(); i++) {
            len += 2 + elements[i].getInformationElementDataLength();
          }
          return len == getLength();
        }
        std::string toString() {
          return toString("  ");
        }
        std::string toString(std::string tab) {
          std::ostringstream sout;
          sout.fill('0');
          sout.width(2);

          sout << tab << "UDH {" << std::endl;
          sout << tab << tab << "length = " << (int) getLength() << std::endl;
          sout << tab << tab << "elementsNumber = " << (int) getElementsNumber() << std::endl;
          for(int i=0 ; i<getElementsNumber(); i++) {
            sout << tab << tab << "IE {" << std::endl;
            sout << tab << tab << tab << "IEI = " << (int) elements[i].getInformationElementIdentifier() << std::endl;
            sout << tab << tab << tab << "IEDL = " << (int) elements[i].getInformationElementDataLength() << std::endl;
            sout << tab << tab << tab << "IED = ";
            sout << std::hex;
            for(int j=0; j<elements[i].getInformationElementDataLength(); j++) {
              sout << (int) elements[i].getInformationElementData()[j] << " ";
            }
            sout << std::dec << std::endl;
            sout << tab << tab << "}//IE" << std::endl;
          }
          sout << tab << "}//UDH" << std::endl;

          return sout.str();
        }
        ~UDH() {
          if(elements) {
            delete[] elements;
          }
        }
      };

    }//namespace smpp
  }//namespace test
}//namespace smsc

#endif //___USER_DATA_HEADER_CLASS___
