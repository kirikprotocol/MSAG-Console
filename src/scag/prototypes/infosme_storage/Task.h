#ifndef _SCAG_PROTOTYPES_INFOSME_TASK_H
#define _SCAG_PROTOTYPES_INFOSME_TASK_H

namespace scag2 {
namespace prototypes {
namespace infosme {

class TaskInfo;

class Task
{
public:
    Task( StoreLog& alog );

    taskid_type getId() const;

    void setInfo( const TaskInfo& info );
    const TaskInfo& getInfo() const;

    unsigned getRequestNewLimit() const;
    unsigned getRequestNewCount() const;
    // msgtime_type getMinRetryTime() const;

    /// get the next message.
    bool getNextMessage( regionid_type regionId,
                         msgtime_type  currentTime,
                         Message&      msg );

    /// request to load count messages for region regionId.
    void requestNewMessages(regionid_type regionId, unsigned count);

private:
    MessageCache         cache_;
    StoreLog*            alog_;
    MessageSource*       source_;
};

}
}
}

#endif /* !_SCAG_PROTOTYPES_INFOSME_TASK_H */
