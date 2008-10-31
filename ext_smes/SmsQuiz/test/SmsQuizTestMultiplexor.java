
import ru.aurorisoft.smpp.PDU;
import ru.aurorisoft.smpp.Message;
import ru.aurorisoft.smpp.SMPPException;

import java.util.Random;

import com.logica.smpp.Data;
import com.eyeline.sme.smpp.test.TestMultiplexor;
import com.eyeline.sme.smpp.test.SimpleResponse;
import org.apache.log4j.Logger;


/**
 * author: alkhal
 */
public class SmsQuizTestMultiplexor extends TestMultiplexor {

  private String[] answers ={"yes","no"};
  private Random random = new Random();

  private static Logger logger = Logger.getLogger(SmsQuizTestMultiplexor.class);



  public void sendResponse(PDU response){
  }

  protected void _sendMessage(Message message) throws SMPPException {
    String oa = message.getSourceAddress();
    String da = message.getDestinationAddress();
    handleResponse(new SimpleResponse(message));
    if(!message.getMessageString().equals("Thanks")) {
      message.setDestinationAddress(oa);
      message.setSourceAddress(da);
      message.setMessageString(answers[random.nextInt(2)]);
      handleMessage(message);
    }
  }



}
