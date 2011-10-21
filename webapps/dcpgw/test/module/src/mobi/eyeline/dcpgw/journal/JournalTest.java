package mobi.eyeline.dcpgw.journal;

import org.junit.*;
import org.junit.runner.JUnitCore;
import org.junit.runner.Request;
import test.T;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.Hashtable;
import java.util.concurrent.LinkedBlockingQueue;

import test.CalcListener;

public class JournalTest extends T {

    private static Journal journal = Journal.getInstance();
    private static File j1;
    private static File j2;
    private static File j2t;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        String user_dir = System.getProperty("user.dir");
        System.out.println("user.dir="+user_dir);

        File dir = new File(user_dir + File.separator+".build"+File.separator+"journal");
        registerTestObject(dir);

        j1 = new File(user_dir +"/.build/journal/j1.csv");
        registerTestObject(j1);
        j1.delete();

        j2 = new File(user_dir +"/.build/journal/j2.csv");
        registerTestObject(j2);
        j2.delete();

        j2t = new File(user_dir +"/.build/journal/j2t.csv");
        registerTestObject(j2t);
        j2t.delete();

        journal.init(dir, 1, 30000000, 100);
        registerTestObject(journal);
    }

    @Test
    public void getO1writeTest() throws Exception {
        DeliveryReceiptData e = createStaticData();
        journal.write(e);

        BufferedReader br = new BufferedReader(new FileReader(j1));
        String line = br.readLine();
        br.close();

        Assert.assertNotNull("Journal file is empty.", line);
        DeliveryReceiptData d = DeliveryReceiptData.parse(line);

        boolean isEquals = e.equals(d);
        Assert.assertTrue("Journal write doesn't work.", isEquals);
    }

    @Test
    public void get020loadTest() throws Exception {
        Hashtable<Integer, DeliveryReceiptData> t1 = new Hashtable<Integer, DeliveryReceiptData>();
        for(int i=0; i<100; i++){
            DeliveryReceiptData d = createData();
            registerTestObject(d);
            t1.put(d.getSequenceNumber(), d);
            journal.write(d);
        }

        journal.load();

        Hashtable<Integer, DeliveryReceiptData> sn_data_store = journal.getDataTable(con);

        for(Integer sn: t1.keySet()) {
            DeliveryReceiptData expected = t1.get(sn);
            DeliveryReceiptData loaded = sn_data_store.get(sn);
            Assert.assertNotNull("Loaded data is null.", loaded);

            boolean isEquals = expected.equals(loaded);
            Assert.assertTrue("Expected and loaded data objects differ.", isEquals);
        }
    }

    @Test
    public void get021loadTest() throws Exception {
        DeliveryReceiptData d = createData();
        journal.write(d);

        d.setStatus(DeliveryReceiptData.Status.EXPIRED_TIMEOUT);
        journal.write(d);

        DeliveryReceiptData d1 = createData();
        journal.write(d1);

        d1.setStatus(DeliveryReceiptData.Status.EXPIRED_MAX_TIMEOUT);
        journal.write(d1);

        DeliveryReceiptData d2 = createData();
        journal.write(d2);

        d2.setStatus(DeliveryReceiptData.Status.DONE);
        journal.write(d2);

        DeliveryReceiptData d3 = createData();
        journal.write(d3);

        d3.setStatus(DeliveryReceiptData.Status.DELETED);
        journal.write(d3);

        journal.load();

        Hashtable<Integer, DeliveryReceiptData> sn_data_store = journal.getDataTable(con);

        Assert.assertTrue("Not all objects was removed.", sn_data_store.size() == 0);
    }

    @Test
    public void get022loadTest() throws Exception {
        String con = "con";
        DeliveryReceiptData expected = createInitialData();
        journal.write(expected);

        journal.load();

        LinkedBlockingQueue<DeliveryReceiptData> queue = journal.getDataQueue(con);
        DeliveryReceiptData loaded = queue.poll();

        boolean isEquals = expected.equals(loaded);
        Assert.assertTrue("Expected and loaded data objects differ.", isEquals);
    }

    @Test
    public void get03cleanTest() throws Exception{
        DeliveryReceiptData d = createUniqueData(DeliveryReceiptData.Status.SEND);
        journal.write(d);
        d.setStatus(DeliveryReceiptData.Status.DONE);
        journal.write(d);

        DeliveryReceiptData d1 = createUniqueData(DeliveryReceiptData.Status.SEND);
        journal.write(d1);
        d1.setStatus(DeliveryReceiptData.Status.EXPIRED_TIMEOUT);
        journal.write(d1);

        DeliveryReceiptData d2 = createUniqueData(DeliveryReceiptData.Status.SEND);
        journal.write(d2);
        d2.setStatus(DeliveryReceiptData.Status.EXPIRED_MAX_TIMEOUT);
        journal.write(d2);

        journal.clean();

        boolean empty1 = j1.length() == 0;
        Assert.assertTrue("First journal not empty.", empty1);

        boolean empty2 = j2.length() == 0;
        Assert.assertTrue("Second journal not empty.", empty2);
    }

    @Test
    public void get04cleanTest() throws Exception{
        DeliveryReceiptData expected1 = createUniqueData(DeliveryReceiptData.Status.SEND);
        journal.write(expected1);

        DeliveryReceiptData expected2 = createUniqueData(DeliveryReceiptData.Status.NOT_SEND);
        journal.write(expected2);

        journal.clean();

        boolean empty = j1.length() == 0;
        Assert.assertTrue("First journal not empty.", empty);

        BufferedReader br = new BufferedReader(new FileReader(j2));
        String line1 = br.readLine();
        String line2 = br.readLine();
        br.close();
        Assert.assertNotNull("Couldn't first read line.", line1);
        Assert.assertNotNull("Couldn't second read line.", line2);

        DeliveryReceiptData loaded1 = DeliveryReceiptData.parse(line1);
        boolean isEquals = expected1.equals(loaded1);
        Assert.assertTrue("First data object wrong.", isEquals);

        DeliveryReceiptData loaded2 = DeliveryReceiptData.parse(line2);
        isEquals = expected2.equals(loaded2);
        Assert.assertTrue("Second data object wrong.", isEquals);
    }

    @Test
    public void get05cleanTest() throws Exception{
        DeliveryReceiptData d = createUniqueData(DeliveryReceiptData.Status.SEND);
        journal.write(d);

        d.setStatus(DeliveryReceiptData.Status.EXPIRED_TIMEOUT);
        journal.write(d);

        DeliveryReceiptData expected = createUniqueData(DeliveryReceiptData.Status.SEND);
        journal.write(expected);

        journal.clean();

        boolean empty = j1.length() == 0;
        Assert.assertTrue("First journal not empty.", empty);

        BufferedReader br = new BufferedReader(new FileReader(j2));
        String line = br.readLine();
        br.close();
        Assert.assertNotNull("Couldn't read first line.", line);

        d = DeliveryReceiptData.parse(line);
        boolean isEquals = expected.equals(d);
        Assert.assertTrue("DeliveryReceiptData object wrong.", isEquals);
    }

    @Test
    public void get06cleanTest() throws Exception{
        DeliveryReceiptData d = createUniqueData(DeliveryReceiptData.Status.INIT);
        journal.write(d);

        d.setStatus(DeliveryReceiptData.Status.SEND);
        journal.write(d);

        d.setStatus(DeliveryReceiptData.Status.DONE);
        journal.write(d);

        journal.clean();

        boolean empty = j1.length() == 0;
        Assert.assertTrue("First journal not empty.", empty);

        empty = j2.length() == 0;
        Assert.assertTrue("Second journal not empty.", empty);
    }

    @Test
    public void get07cleanTest() throws Exception{
        DeliveryReceiptData d = createUniqueData(DeliveryReceiptData.Status.INIT);
        journal.write(d);

        d.setStatus(DeliveryReceiptData.Status.SEND);
        journal.write(d);

        journal.clean();

        boolean empty = j1.length() == 0;
        Assert.assertTrue("First journal not empty.", empty);

        BufferedReader br = new BufferedReader(new FileReader(j2));
        String line = br.readLine();
        br.close();
        Assert.assertNotNull("Couldn't read first line.", line);

        DeliveryReceiptData loaded = DeliveryReceiptData.parse(line);
        boolean isEquals = d.equals(loaded);
        Assert.assertTrue("DeliveryReceiptData object wrong.", isEquals);
    }

    @After
    public void tearDown(){
        if (journal.getDataQueue(con) != null) journal.getDataQueue(con).clear();
        if (journal.getDataTable(con) != null) journal.getDataTable(con).clear();
        j1.delete();
        j2.delete();
        j2t.delete();
    }

    @AfterClass
    public static void setUpAfterClass(){
        clean();
    }

    public static void main(String[] args) {
        JUnitCore core = new JUnitCore();
        core.addListener(new CalcListener());
        core.run(Request.aClass(JournalTest.class).sortWith(forward()));
    }

}
