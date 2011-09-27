package mobi.eyeline.dcpgw.tests.stress;

import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.SubmitSM;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import org.apache.log4j.Logger;
import org.junit.*;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.HashSet;
import java.util.Properties;
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
public class StressTest {

    private static Logger log = Logger.getLogger(StressTest.class);
    private static Client client;

    private static int speed, check_delay, test_time;

    private static String dest_address, source_address;
    private static long validity_period;

    private static AtomicInteger ai = new AtomicInteger(0);

    private static SimpleDateFormat sdf = new SimpleDateFormat("ddHHmmss");
    private static Calendar cal = Calendar.getInstance();

    private ScheduledExecutorService scheduler;


    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        log.debug("before class");
        Properties config = new Properties();
        try {
            config.load(new FileInputStream(System.getProperty("user.dir")+ File.separator + "conf" + File.separator + "config.properties"));
            log.debug("Successfully load config properties.");
        } catch (IOException e) {
            throw new SmppException("Could not initialize", e);
        }

        speed = Integer.parseInt(config.getProperty("speed"));
        check_delay = Integer.parseInt(config.getProperty("check.delay.sec"));
        test_time = Integer.parseInt(config.getProperty("test.time.sec"));


        source_address = config.getProperty("source.address");
        dest_address = config.getProperty("dest.address");
        validity_period = Long.parseLong(config.getProperty("validity.period"));

        client = new Client(config);
    }

    @Before
    public void setUp() throws Exception {
        log.debug("before");
        scheduler = Executors.newSingleThreadScheduledExecutor();
    }

    @Test
    public void test1() throws Exception {
        log.debug("test");
        scheduler.scheduleAtFixedRate(new Runnable() {

                @Override
                public void run() {

                    for(int i=0; i<speed;i++){
                        log.debug("Try to send SubmitSM ...");
                        SubmitSM submitSM = new SubmitSM();

                        Date date = cal.getTime();
                        int sn = Integer.parseInt(sdf.format(date)) + ai.incrementAndGet();
                        submitSM.setRegDeliveryReceipt(RegDeliveryReceipt.SuccessOrFailure);

                        submitSM.setSequenceNumber(sn);
                        submitSM.setConnectionName("con1");

                        DateFormat df = DateFormat.getDateTimeInstance();

                        submitSM.setMessage("message: con="+submitSM.getConnectionName()+", date="+df.format(date));

                        try{
                            submitSM.setSourceAddress(source_address);
                            submitSM.setDestinationAddress(dest_address);
                            submitSM.setValidityPeriod(validity_period*1000);
                        } catch (InvalidAddressFormatException e){
                            log.error(e);
                        }
                        submitSM.setValidityPeriod(1000 * 3600);
                        client.handlePDU(submitSM);
                    }
                }

        }, 1, 1, TimeUnit.SECONDS);

        Thread.sleep(test_time * 1000);

        scheduler.shutdown();
        log.debug("Shutdown scheduler.");

        Thread.sleep(check_delay * 1000);

        HashSet<Integer> s = client.getSubmitSMSequenceNumberSet();
        log.debug("sn set size: "+s.size());

        boolean empty = s.isEmpty();

        Assert.assertTrue("Not all submitSM sequence numbers was received.", empty);

        HashSet<Long> s2 = client.getMessageIdSet();
        log.debug("deliver sm size: "+s2.size());
        empty = s2.isEmpty();

        Assert.assertTrue("Not all delivery receipt was received.", empty);
    }

    @After
    public void tearDown() throws Exception {
        log.debug("after");
        scheduler.shutdown();
    }

    @AfterClass
    public static void tearDownAfterClass(){
        log.debug("after class");
        client.shutdown();
    }

}
