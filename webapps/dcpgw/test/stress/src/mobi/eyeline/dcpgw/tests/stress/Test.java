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
public class Test {

    private static Logger log = Logger.getLogger(Test.class);
    private static Client client;

    private static int speed;

    private static String source_address;

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
        config1.load(new FileInputStream(System.getProperty("user.dir") + File.separator + "conf" + File.separator + "config1.properties"));
        log.debug("Successfully load config1 properties.");

        source_address = config1.getProperty("source.address");

        client = new Client(config1, "c1");

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        String error_message;

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
                            submitSM.setConnectionName("con1");
                            submitSM.setSourceAddress(source_address);
                            submitSM.setMessage("message: con="+submitSM.getConnectionName()+", date="+df.format(date));
                            submitSM.setDestinationAddress("79139118729");
                            submitSM.setValidityPeriod(3600*1000);
                            submitSM.setSequenceNumber(ai.incrementAndGet());
                        } catch (InvalidAddressFormatException e){
                            log.error(e);
                        }
                        submitSM.setValidityPeriod(1000 * 3600);
                        client.handlePDU(submitSM);
                    }
                }

        }, 1, 1, TimeUnit.SECONDS);

        Thread.sleep(test_time * 1000);

        scheduler1.shutdown();

        Thread.sleep(check_delay * 1000);

        Set<Integer> s = client.getSubmitSMSequenceNumberSet();
        log.debug("sn set size: "+s.size());

        if (!s.isEmpty()) {
            error_message = "test1, Not all submitSM sequence numbers was received.";
            log.debug(error_message);
            System.exit(-1);
        }

        HashSet<Long> s2 = client.getMessageIdSet();
        log.debug("deliver sm size: "+s2.size());

        if (!s2.isEmpty()){
            error_message = "test1, Not all delivery receipt was received.";
            log.debug(error_message);
            System.exit(-1);
        }

        log.debug("Success");
        System.exit(0);

    }

}
