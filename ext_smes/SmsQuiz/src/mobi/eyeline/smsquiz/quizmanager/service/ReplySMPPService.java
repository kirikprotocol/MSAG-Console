package mobi.eyeline.smsquiz.quizmanager.service;

import mobi.eyeline.smsquiz.quizmanager.QuizManager;
import mobi.eyeline.smsquiz.quizmanager.QuizException;
import mobi.eyeline.smsquiz.quizmanager.Result;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import com.eyeline.sme.handler.SMPPRequest;
import com.eyeline.sme.handler.SMPPServiceException;
import com.eyeline.sme.handler.services.BasicService;
import com.eyeline.sme.smpp.ShutdownedException;
import com.logica.smpp.Data;

import java.util.Properties;

import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.SMPPException;
import org.apache.log4j.Logger;

public class ReplySMPPService extends BasicService {

  private static final Logger logger = Logger.getLogger(ReplySMPPService.class);

  private ReplyStatsDataSource replyStatsDataSource;
  private QuizManager quizManager;

  public boolean serve(SMPPRequest smppRequest) {
    try {
      final Message reqMsg = smppRequest.getInObj().getMessage();
      String oa = reqMsg.getSourceAddress();
      String da = reqMsg.getDestinationAddress();
      String text = reqMsg.getMessageString();
      Result result;
      try {
        result = quizManager.handleSms(da, oa, text);
      } catch (QuizException e) {
        logger.error("Unable to handle sms");
        return false;
      }

      smppRequest.getInObj().respond(Data.ESME_ROK);
      if (result != null) {
        Result.ReplyRull replyRull = result.getReplyRull();
        if ((replyRull.equals(Result.ReplyRull.OK)) || (replyRull.equals(Result.ReplyRull.REPEAT))) {
          Message respMsg = new Message();

          respMsg.setSourceAddress(result.getSourceAddress());
          respMsg.setDestinationAddress(oa);
          respMsg.setMessageString(result.getText());
          try {
            send(respMsg);
          } catch (ShutdownedException e) {
            logger.error("Shutdowned.", e);
          }
        }
      }
    } catch (SMPPException e) {
      logger.error(e, e);
    }
    return true;
  }

  protected void init(Properties properties) throws SMPPServiceException {
    quizManager = QuizManager.getInstance();
  }
}
 
