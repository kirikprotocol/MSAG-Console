#ifndef _PIPE_OBJECT_H
#define _PIPE_OBJECT_H

#define ACTION_SEND_SMS 0
#define ACTION_RECIEVE_SMS 1
#define ACTION_GETSTATUS_SEND 3

 

class Pipe
{
public:

    std::string GetText() { return m_text; }
    void SetText(std::string txt){m_text  = txt; }

    void SetAbsent(bool v) {absent = v;}
    bool GetAbsent(){return absent;}

    void SetProbability(int v) {probability = v;}
    int GetProbability(){return probability;}

    std::string GetDestinatorMsisdn() { return m_dst_msisdn; }
    void SetDestinatorMsisdn(std::string dstmsisdn){m_dst_msisdn  = dstmsisdn; }


    std::string GetImsi() { return m_imsi; }
    void SetImsi(std::string ims){m_imsi  = ims; }

    std::string GetMsisdn() { return m_msisdn; }
    void SetMsisdn(std::string msd){m_msisdn  = msd; }

 std::string GetMSCAddr() { return m_mscaddr; }
 void SetMSCAddr(std::string msca){m_mscaddr  = msca; }

 int GetSendSmsStatus(){return sendStatus;};
 int GetRecvSmsStatus(){return recieveStatus;};

 std::string GetErrorInfo() { return m_error_info; }
 void SetErrorInfo(std::string txt){m_error_info  = txt; }

 
 private:

    std::string m_text;
 std::string m_imsi;
 std::string m_msisdn;
 std::string m_mscaddr;
 std::string m_error_info;
 std::string m_dst_msisdn;

 int sendStatus; //0 - nothing, 1- ok other error
 int recieveStatus;//0 - nothing ,1 - ok other error
 
 int action;

 int dialogid;
 int finished;
 int probability;
 bool absent;
};

#endif