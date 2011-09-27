package mobi.eyeline.dcpgw.tests.stress;


import mobi.eyeline.smpp.api.pdu.*;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import org.apache.log4j.Logger;
import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.IOException;
import java.io.InputStream;
import java.text.DateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Properties;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class StressTest{

    private static Logger log = Logger.getLogger(StressTest.class);

    private Client client;

    private static final ScheduledExecutorService scheduler = Executors.newSingleThreadScheduledExecutor();

    @Before
    public void setUp() throws Exception {
        client = new Client();
    }

    @Test
    public void stressTest(){
        scheduler.scheduleAtFixedRate(new Runnable() {

                @Override
                public void run() {
                    SubmitSM submitSM = new SubmitSM();
                    submitSM.setRegDeliveryReceipt(RegDeliveryReceipt.SuccessOrFailure);

                    submitSM.setConnectionName("con1");

                    DateFormat df = DateFormat.getDateTimeInstance();
                    Calendar cal = Calendar.getInstance();
                    Date date = cal.getTime();

                    submitSM.setMessage("message from "+submitSM.getConnectionName()+", "+df.format(date));

                    try{
                        submitSM.setSourceAddress("10001");
                        submitSM.setDestinationAddress("+79139118729");
                    } catch (InvalidAddressFormatException e){
                        log.error(e);
                    }
                    submitSM.setValidityPeriod(1000 * 3600);
                    client.handlePDU(submitSM);
                }

            }, 5, 5, TimeUnit.SECONDS);
    }

    @After
    public void tearDown() throws Exception {
        client.shutdown();
    }


}