#ifndef __SMSC_MCISME_ADVERT_BANNERRESPONSELISTENER_HPP__
# define __SMSC_MCISME_ADVERT_BANNERRESPONSELISTENER_HPP__

# include <set>
# include "logger/Logger.h"
# include "core/threads/ThreadGroup.hpp"
# include "mcisme/TaskProcessor.h"
# include "scag/util/encodings/Encodings.h"

namespace smsc {
namespace mcisme {

struct BannerInfo {
  BannerResponseTrace bannerRespTrace;
  std::string bannerString;
  MCEventOut* mcEventOut;
  uint32_t    charSet;

  BannerInfo(const BannerResponseTrace& b_resp_trace, const std::string& banner_string,
             MCEventOut* mc_event_out, uint32_t char_set)
  : bannerRespTrace(b_resp_trace), bannerString(banner_string),
    mcEventOut(mc_event_out), charSet(char_set)
  {}

  ~BannerInfo() { delete mcEventOut; }
};

class BannerResponseListener : public core::threads::ThreadGroup<BannerInfo> {
public:
  BannerResponseListener(TaskProcessor& proc, int arg_disp_threads_count)
  : ThreadGroup<BannerInfo>(arg_disp_threads_count),
    _processor(proc), _isRunning(true),
    _logger(logger::Logger::getInstance("advert"))
  {}

  void handleEvent(BannerInfo* b_info)
  {
    publishEvent(b_info);
  }

  void stop()
  {
    _isRunning=false;
  }

protected:
  virtual bool isRunning()
  {
    return _isRunning;
  }

  virtual void processEvent(BannerInfo* b_info)
  {
    try {
      smsc_log_debug(_logger, "BannerResponseListener::processEvent::: handle bannerReqInfo={subscriber=%s,bannerId=%u,ownerId=%u,rotatorId=%u,notification=%d}",
                      b_info->mcEventOut->msg.called_abonent.getText().c_str(),
                      b_info->bannerRespTrace.bannerId,
                      b_info->bannerRespTrace.ownerId, b_info->bannerRespTrace.rotatorId,
                      b_info->mcEventOut->msg.notification);

      if ( b_info->charSet == UTF16BE && !b_info->bannerString.empty()) {
        std::string banner, tmpBanner;
        try {
          scag::util::encodings::Convertor::convert("UTF-16BE", "UTF-8", b_info->bannerString.c_str(), b_info->bannerString.length(), tmpBanner);
          scag::util::encodings::Convertor::convert("UTF-8", "CP1251", tmpBanner.c_str(), tmpBanner.length(), banner);
          addBanner(b_info->mcEventOut->msg.message, banner);
        } catch(scag::exceptions::SCAGException e) {
          smsc_log_error(_logger, "BannerResponseListener::processEvent::: banner converting exception='%s'", e.what());
        }
      } else
        addBanner(b_info->mcEventOut->msg.message, b_info->bannerString);

      if (b_info->mcEventOut->msg.notification)
        _processor.processNotificationMessage(b_info->bannerRespTrace,
                                              b_info->mcEventOut->msg);
      else
        _processor.sendMessage(*b_info->mcEventOut, b_info->bannerRespTrace);

    } catch (std::exception& e) {
      smsc_log_warn(_logger, "BannerResponseListener::processEvent::: banner handling exception:%s", e.what());
    } catch (...) {
      smsc_log_warn(_logger, "BannerResponseListener::processEvent::: caught unexpected exception");
    }
    delete b_info;
  }

private:
  TaskProcessor& _processor;
  bool _isRunning;
  logger::Logger* _logger;
};

}}

#endif
