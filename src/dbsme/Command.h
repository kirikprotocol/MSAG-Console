#ifndef SMSC_DBSME_COMMAND
#define SMSC_DBSME_COMMAND

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <util/debug.h>
#include <sms/sms.h>

namespace smsc { namespace dbsme 
{
    using smsc::sms::Address;
    
    class Command
    {
    protected:

        Address from;
        Address to;

        char*   jobName;
        char*   inData;
        char*   outData;

        inline void setString(char*& str, const char* name) 
        {
            if (str) delete str;
            str = new char[strlen(name)+1];
            strcpy(str, name);
        };

    public:

        Command() : jobName(0), inData(0), outData(0) {};
        virtual ~Command() 
        {
            if (jobName) delete jobName;
            if (inData) delete inData;
            if (outData) delete outData;
        };

        inline void setFromAddress(const Address& from) {
            this->from = from;
        };
        inline const Address& getFromAddress() const {
            return from; 
        };
        inline Address& getFromAddress() {
            return from; 
        };
        
        inline void setToAddress(const Address& to) {
            this->to = to;
        };
        inline const Address& getToAddress() const {
            return to; 
        };
        inline Address& getToAddress() {
            return to; 
        };

        inline void setJobName(const char* name) {
            setString(jobName, name);
        };
        inline const char* getJobName() const {
            return jobName;
        };
        
        inline void setInData(const char* data) {
            setString(inData, data);
        };
        inline const char* getInData() const {
            return inData;
        };
        
        inline void setOutData(const char* data) {
            setString(outData, data);
        };
        inline const char* getOutData() const {
            return outData;
        };
    };

}}

#endif
