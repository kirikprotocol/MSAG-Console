package mobi.eyeline.dcpgw.tests.stress;

import mobi.eyeline.smpp.api.pdu.SubmitSM;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FileInputStream;
import java.text.DateFormat;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 27.09.11
 * Time: 14:26
 */
public class Test2 {

    private static Logger log = Logger.getLogger(Test.class);
    private static Client client1, client2;

    private static int speed;

    private static String source_address1, source_address2;
    private static String con1, con2;

    private static AtomicInteger ai = new AtomicInteger(0);

    private static Calendar cal = Calendar.getInstance();

    public static void main(String args[]) throws Exception {

        Properties config = new Properties();
        config.load(new FileInputStream(System.getProperty("user.dir") + File.separator + "conf" + File.separator + "config.properties"));
        log.debug("Successfully load config properties.");

        speed = Integer.parseInt(config.getProperty("speed"));
        int check_delay = Integer.parseInt(config.getProperty("check.delay.sec"));
        int test_time = Integer.parseInt(config.getProperty("test.time.sec"));

        Properties config1 = new Properties();
        config1.load(new FileInputStream(System.getProperty("user.dir") + File.separator + "conf" + File.separator + args[0]));
        log.debug("Successfully load config1 properties.");

        source_address1 = config1.getProperty("source.address");
        con1 = config1.getProperty("con");

        client1 = new Client(config1,"c1");

        Properties config2 = new Properties();
        config2.load(new FileInputStream(System.getProperty("user.dir") + File.separator + "conf" + File.separator + args[1]));
        log.debug("Successfully load config2 properties.");

        source_address2 = config2.getProperty("source.address");
        con2 = config2.getProperty("con");

        client2 = new Client(config2, "c2");

        ScheduledExecutorService scheduler1 = Executors.newSingleThreadScheduledExecutor();
        scheduler1.scheduleAtFixedRate(new Runnable() {

                @Override
                public void run() {

                    for(int i=0; i<speed;i++){

                        SubmitSM submitSM = new SubmitSM();

                        Date date = cal.getTime();
                        submitSM.setRegDeliveryReceipt(RegDeliveryReceipt.SuccessOrFailure);
                        DateFormat df = DateFormat.getDateTimeInstance();

                        try{
                            submitSM.setConnectionName(con1);
                            submitSM.setSourceAddress(source_address1);
                            submitSM.setMessage("message: con="+submitSM.getConnectionName()+", date="+df.format(date));
                            submitSM.setDestinationAddress("79139118729");
                            submitSM.setValidityPeriod(3600*1000);
                            submitSM.setSequenceNumber(ai.incrementAndGet());
                        } catch (InvalidAddressFormatException e){
                            log.error(e);
                        }
                        submitSM.setValidityPeriod(1000 * 3600);
                        client1.handlePDU(submitSM);
                    }
                }

        }, 1, 1, TimeUnit.SECONDS);

        ScheduledExecutorService scheduler2 = Executors.newSingleThreadScheduledExecutor();
        scheduler2.scheduleAtFixedRate(new Runnable() {

                @Override
                public void run() {

                    for(int i=0; i<speed;i++){
                        //log.debug("Try to send SubmitSM ...");
                        SubmitSM submitSM = new SubmitSM();

                        Date date = cal.getTime();
                        submitSM.setRegDeliveryReceipt(RegDeliveryReceipt.SuccessOrFailure);
                        DateFormat df = DateFormat.getDateTimeInstance();

                        try{
                            submitSM.setConnectionName(con2);
                            submitSM.setSourceAddress(source_address2);
                            submitSM.setMessage("message: con="+submitSM.getConnectionName()+", date="+df.format(date));
                            submitSM.setDestinationAddress("79139118729");
                            submitSM.setValidityPeriod(3600*1000);
                            submitSM.setSequenceNumber(ai.incrementAndGet());
                        } catch (InvalidAddressFormatException e){
                            log.error(e);
                        }
                        submitSM.setValidityPeriod(1000 * 3600);
                        client2.handlePDU(submitSM);
                    }
                }

        }, 1, 1, TimeUnit.SECONDS);

        Thread.sleep(test_time * 1000);

        scheduler1.shutdown();
        scheduler2.shutdown();

        Thread.sleep(check_delay * 1000);

        Set<Integer> s = client1.getSubmitSMSequenceNumberSet();
        log.debug("sn set size: "+ s.size());

        String error_message;

        if (!s.isEmpty()) {
            error_message = "Not all submitSM sequence numbers was received.";
            log.debug(error_message);
            System.exit(-1);
        }

        HashSet<Long> s2 = client1.getMessageIdSet();
        log.debug("deliver sm size: "+ s2.size());

        if (!s2.isEmpty()){
            error_message = "Not all delivery receipt was received.";
            log.debug(error_message);
            System.exit(-1);
        }

        s = client2.getSubmitSMSequenceNumberSet();
        log.debug("sn set size: "+ s.size());

        if (!s.isEmpty()) {
            error_message = "Not all submitSM sequence numbers was received.";
            log.debug(error_message);
            System.exit(-2);
        }

        s2 = client2.getMessageIdSet();
        log.debug("deliver sm size: "+ s2.size());

        if (!s2.isEmpty()){
            error_message = "Not all delivery receipt was received.";
            log.debug(error_message);
            System.exit(-2);
        }

        log.debug("Success");
        System.exit(0);

    }

}