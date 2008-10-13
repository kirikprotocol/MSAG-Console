package mobi.eyeline.smsquiz.quizmanager.service;

import mobi.eyeline.smsquiz.quizmanager.QuizManager;
import mobi.eyeline.smsquiz.replystats.datasource.ReplyStatsDataSource;
import com.eyeline.sme.handler.SMPPService;
import com.eyeline.sme.handler.SMPPRequest;
import com.eyeline.sme.handler.SMPPServiceException;
import com.eyeline.sme.smpp.OutgoingQueue;

import java.util.Properties;

public class ReplySMPPService implements SMPPService {
 
	private ReplyStatsDataSource replyStatsDataSource;
	 
	 
	private QuizManager quizManager;

    public boolean serve(SMPPRequest smppRequest) {
        return false;
    }

    public void init(OutgoingQueue outgoingQueue, Properties properties) throws SMPPServiceException {
        
    }
}
 
