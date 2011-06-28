import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.SmppClient;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.Message;
import mobi.eyeline.smpp.api.pdu.PDU;
import mobi.eyeline.smpp.api.pdu.SubmitSM;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.pdu.tlv.TLVByte;
import mobi.eyeline.smpp.api.pdu.tlv.TLVString;
import mobi.eyeline.smpp.api.types.Encoding;
import mobi.eyeline.smpp.api.types.EsmGsm;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import mobi.eyeline.smpp.api.types.Status;
import mobi.eyeline.smpp.api.types.eyeline.Charging;
import mobi.eyeline.smpp.api.types.eyeline.TransportType;
import org.apache.log4j.Logger;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 28.02.11
 * Time: 10:30
 */
public class Client extends Thread implements PDUListener {

    static Logger logger = Logger.getLogger(Client.class);
    Properties config;
    SmppClient smppClient;

    private static String dest_address;
    private static long validity_period;
    private static String message;


    public Client() throws SmppException {
        this.config = new Properties();
        try {
          InputStream is = this.getClass().getResourceAsStream("/config.properties");
          config.load(is);
          logger.debug("Successfully load config properties.");
        } catch (IOException e) {
          throw new SmppException("Could not initialize", e);
        }
        logger.debug("Try to add shutdown hook ...");
        Runtime.getRuntime().addShutdownHook(this);
        logger.debug("Try to initialize smpp client ...");
        smppClient = new SmppClient(config, this);
        logger.debug("Try to start smpp client ...");
        smppClient.start();

        String source_address = config.getProperty("source.address");
        dest_address = config.getProperty("dest.address");
        validity_period = Long.parseLong(config.getProperty("validity.period"));

        message = config.getProperty("message");

        logger.debug("Start smpp client.");
    }

    public boolean handlePDU(PDU pdu) {
        logger.debug("Handle pdu with '"+pdu.getType()+"' type.");
        switch (pdu.getType()) {
            case SubmitSM:
                try {
                    Message request = (Message)pdu;
                    smppClient.send(request);
                    logger.debug("Send SubmitSM request.");
                    return true;
                } catch (SmppException e) {
                    logger.error("", e);
                }
            case DeliverSM:
                logger.debug("Handle DeliverSM pdu.");
                return true;
        }
        return false;
    }

    public static void main(String args[]){
        logger.debug("Start smpp client.");
        try{
            Client client = new Client();
            for(int i=0; i<5; i++){
                SubmitSM submitSM = new SubmitSM();
                submitSM.setRegDeliveryReceipt(RegDeliveryReceipt.SuccessOrFailure);
                submitSM.setConnectionName("env.client");

                submitSM.setMessage(message+" "+i);
                submitSM.setSourceAddress("10001");
                submitSM.setDestinationAddress(dest_address);
                submitSM.setValidityPeriod(1000*validity_period);
                logger.debug("Try send submitSM ...");
                client.handlePDU(submitSM);
            }

            for(int i=5; i<10; i++){
                SubmitSM submitSM = new SubmitSM();
                submitSM.setRegDeliveryReceipt(RegDeliveryReceipt.SuccessOrFailure);
                submitSM.setConnectionName("env.client");

                submitSM.setMessage(message+" "+i);
                submitSM.setSourceAddress("10003");
                submitSM.setDestinationAddress(dest_address);
                submitSM.setValidityPeriod(1000*validity_period);
                logger.debug("Try send submitSM ...");
                client.handlePDU(submitSM);
            }
        } catch (SmppException e) {
            logger.error("", e);
        }
    }

    @Override
    public void run() {
      if( smppClient != null ) smppClient.shutdown();
    }
}
