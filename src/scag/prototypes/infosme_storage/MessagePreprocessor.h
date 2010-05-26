#ifndef _SCAG_PROTOTYPES_INFOSME_IOPROCESSOR_H
#define _SCAG_PROTOTYPES_INFOSME_IOPROCESSOR_H

namespace scag2 {
namespace prototypes {
namespace infosme {

class IOTask;

class IOProcessor
{
public:
    void startTask( taskreg_type taskReg, IOTask* task ) = 0;

private:
    /// tasks to process
    smsc::core::threads::ThreadPool pool_;
};

}
}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_MESSAGEPREPROCESSOR_H */
