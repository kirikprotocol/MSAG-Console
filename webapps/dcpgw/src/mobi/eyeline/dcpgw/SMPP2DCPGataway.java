package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.informer.admin.delivery.DeliveryManager;
import mobi.eyeline.smpp.api.pdu.Request;
import mobi.eyeline.smpp.api.processing.ProcessingQueue;
import mobi.eyeline.smpp.api.processing.QueueException;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import org.apache.log4j.Logger;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Properties;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicLong;

import mobi.eyeline.smpp.api.SmppServer;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.PDUListener;
import mobi.eyeline.smpp.api.types.Status;
import mobi.eyeline.smpp.api.types.EsmMessageType;
import mobi.eyeline.smpp.api.pdu.PDU;
import mobi.eyeline.smpp.api.pdu.Message;

/**
 * Created by Serge Lugovoy
 * Date: Jan 22, 2010
 * Time: 5:36:58 PM
 */
public class SMPP2DCPGataway extends Thread implements PDUListener {

    private static Logger log = Logger.getLogger(SMPP2DCPGataway.class);

    private Properties config;
    private SmppServer smppServer;
    private AtomicLong msgid = new AtomicLong(0);
    private SimpleDateFormat sdf = new SimpleDateFormat("yyMMddHHmm");
    private ProcessingQueue procQueue;

    private DeliveryManager deliveryManager;

    private String informer_host;
    private int informer_port;
    private String informer_user_login;
    private String informer_user_password;

    public SMPP2DCPGataway() throws SmppException, InitializationException{

        this.config = new Properties();

        String userDir = System.getProperty("user.dir");
        String filename = userDir+"/config/dcpgw.properties";

        try {
            config.load(new FileInputStream(filename));
        } catch (IOException e) {
            log.debug("Couldn't load properties file.");
            throw new InitializationException(e);
        }

        String s = config.getProperty("informer.host");
        if (s != null && !s.isEmpty()){
            informer_host = s;
            log.debug("Set informer host: "+informer_host);
        } else {
            log.error("informer.host property is invalid or not specified in config");
            throw new InitializationException("informer.host property is invalid or not specified in config");
        }

        s = config.getProperty("informer.port");
        if (s != null && !s.isEmpty()){
            informer_port = Integer.parseInt(s);
            log.debug("Set informer port: "+informer_port);
        } else {
            log.error("informer.port property is invalid or not specified in config");
            throw new InitializationException("informer.port property is invalid or not specified in config");
        }

        s = config.getProperty("informer.user.login");
        if (s != null && !s.isEmpty()){
            informer_user_login = s;
            log.debug("Set informer user login: "+informer_user_login);
        } else {
            log.error("informer.user.login property is invalid or not specified in config");
            throw new InitializationException("informer.user.login property is invalid or not specified in config");
        }

        s = config.getProperty("informer.user.password");
        if (s != null && !s.isEmpty()){
            informer_user_password = s;
            log.debug("Set informer user password: "+informer_user_password);
        } else {
            log.error("informer.user.password property is invalid or not specified in config");
            throw new InitializationException("informer.user.password property is invalid or not specified in config");
        }

        log.debug("Try to initialize delivery manager ...");
        deliveryManager = new DeliveryManager(informer_host, informer_port);
        log.debug("Successfully initialize delivery manager.");

        Runtime.getRuntime().addShutdownHook(this);

        procQueue = new ProcessingQueue(config,

            new PDUListener() {

                public boolean handlePDU(PDU pdu) {

                    switch (pdu.getType()) {

                        case SubmitSM:

                        case DataSM:

                            try {
                                Message request = (Message) pdu;
                                Message msg = request.revert();
                                try {
                                  smppServer.send(msg); // send message to destination
                                } catch (SmppException e) {
                                  log.error("Could not process message "+e.getMessage());
                                  smppServer.send(request.getResponse(e.getStatus()));
                                  return true;
                                }
                                smppServer.send(request.getResponse(Status.OK));
                                if( request.getRegDeliveryReceipt() != RegDeliveryReceipt.None ) {
                                  Message rcpt = request.getAnswer();
                                  rcpt.setEsmMessageType(EsmMessageType.DeliveryReceipt);
                                  String dt = sdf.format(new Date());
                                  rcpt.setMessage("id:" + msgid.incrementAndGet() + " sub:001 dlvrd:001 submit date:" + dt + " done date:" + dt + " stat:DELIVRD err:000 Text:");
                                  smppServer.send(rcpt);
                                }
                                return true;
                            } catch (SmppException e) {
                                log.error("Could not send response to client", e);
                            }

                    }

                    return false;
                }
            }

        , null);

        smppServer = new SmppServer(config, this);
    }

  public boolean handlePDU(PDU pdu) {
    try {
      procQueue.add(pdu, pdu.isMessage());
    } catch (QueueException e) {
      try {
        smppServer.send(((Request)pdu).getResponse(e.getStatus()));
      } catch (SmppException e1) {
        log.error("Could not send resp", e1);
      }
    }
    return true;
  }

  @Override
  public void run() {
    if (smppServer != null) smppServer.shutdown();
  }

    public static void main(String args[]){

        try {
            new SMPP2DCPGataway();
        } catch (SmppException e) {
            log.error(e);
        } catch (InitializationException e) {
            log.error("Couldn't initialize gateway.", e);
        }

    }


}
