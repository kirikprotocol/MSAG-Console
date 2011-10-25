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
        DeliveryData e = createStaticData();
        journal.write(e);

        BufferedReader br = new BufferedReader(new FileReader(j1));
        String line = br.readLine();
        br.close();

        Assert.assertNotNull("Journal file is empty.", line);
        DeliveryData d = DeliveryData.parse(line);

        boolean isEquals = e.equals(d);
        Assert.assertTrue("Journal write doesn't work.", isEquals);
    }

    @Test
    public void get020loadTest() throws Exception {
        Hashtable<Integer, DeliveryData> t1 = new Hashtable<Integer, DeliveryData>();
        for(int i=0; i<100; i++){
            DeliveryData d = createData();
            registerTestObject(d);
            t1.put(d.getSequenceNumber(), d);
            journal.write(d);
        }

        journal.load();

        Hashtable<Integer, DeliveryData> sn_data_store = journal.getDataTable(con);

        for(Integer sn: t1.keySet()) {
            DeliveryData expected = t1.get(sn);
            DeliveryData loaded = sn_data_store.get(sn);
            Assert.assertNotNull("Loaded data is null.", loaded);

            boolean isEquals = expected.equals(loaded);
            Assert.assertTrue("Expected and loaded data objects differ.", isEquals);
        }
    }

    @Test
    public void get021loadTest() throws Exception {
        DeliveryData d = createData();
        journal.write(d);

        d.setStatus(DeliveryData.Status.EXPIRED_TIMEOUT);
        journal.write(d);

        DeliveryData d1 = createData();
        journal.write(d1);

        d1.setStatus(DeliveryData.Status.EXPIRED_MAX_TIMEOUT);
        journal.write(d1);

        DeliveryData d2 = createData();
        journal.write(d2);

        d2.setStatus(DeliveryData.Status.DONE);
        journal.write(d2);

        DeliveryData d3 = createData();
        journal.write(d3);

        d3.setStatus(DeliveryData.Status.DELETED);
        journal.write(d3);

        journal.load();

        Hashtable<Integer, DeliveryData> sn_data_store = journal.getDataTable(con);

        Assert.assertTrue("Not all objects was removed.", sn_data_store.size() == 0);
    }

    @Test
    public void get022loadTest() throws Exception {
        String con = "con";
        DeliveryData expected = createInitialData();
        journal.write(expected);

        journal.load();

        LinkedBlockingQueue<DeliveryData> queue = journal.getDataQueue(con);
        DeliveryData loaded = queue.poll();

        boolean isEquals = expected.equals(loaded);
        Assert.assertTrue("Expected and loaded data objects differ.", isEquals);
    }

    @Test
    public void get03cleanTest() throws Exception{
        DeliveryData d = createUniqueData(DeliveryData.Status.SEND);
        journal.write(d);
        d.setStatus(DeliveryData.Status.DONE);
        journal.write(d);

        DeliveryData d1 = createUniqueData(DeliveryData.Status.SEND);
        journal.write(d1);
        d1.setStatus(DeliveryData.Status.EXPIRED_TIMEOUT);
        journal.write(d1);

        DeliveryData d2 = createUniqueData(DeliveryData.Status.SEND);
        journal.write(d2);
        d2.setStatus(DeliveryData.Status.EXPIRED_MAX_TIMEOUT);
        journal.write(d2);

        journal.cleanDeliveryJournal();

        boolean empty1 = j1.length() == 0;
        Assert.assertTrue("First journal not empty.", empty1);

        boolean empty2 = j2.length() == 0;
        Assert.assertTrue("Second journal not empty.", empty2);
    }

    @Test
    public void get04cleanTest() throws Exception{
        DeliveryData expected1 = createUniqueData(DeliveryData.Status.SEND);
        journal.write(expected1);

        DeliveryData expected2 = createUniqueData(DeliveryData.Status.NOT_SEND);
        journal.write(expected2);

        journal.cleanDeliveryJournal();

        boolean empty = j1.length() == 0;
        Assert.assertTrue("First journal not empty.", empty);

        BufferedReader br = new BufferedReader(new FileReader(j2));
        String line1 = br.readLine();
        String line2 = br.readLine();
        br.close();
        Assert.assertNotNull("Couldn't first read line.", line1);
        Assert.assertNotNull("Couldn't second read line.", line2);

        DeliveryData loaded1 = DeliveryData.parse(line1);
        boolean isEquals = expected1.equals(loaded1);
        Assert.assertTrue("First data object wrong.", isEquals);

        DeliveryData loaded2 = DeliveryData.parse(line2);
        isEquals = expected2.equals(loaded2);
        Assert.assertTrue("Second data object wrong.", isEquals);
    }

    @Test
    public void get05cleanTest() throws Exception{
        DeliveryData d = createUniqueData(DeliveryData.Status.SEND);
        journal.write(d);

        d.setStatus(DeliveryData.Status.EXPIRED_TIMEOUT);
        journal.write(d);

        DeliveryData expected = createUniqueData(DeliveryData.Status.SEND);
        journal.write(expected);

        journal.cleanDeliveryJournal();

        boolean empty = j1.length() == 0;
        Assert.assertTrue("First journal not empty.", empty);

        BufferedReader br = new BufferedReader(new FileReader(j2));
        String line = br.readLine();
        br.close();
        Assert.assertNotNull("Couldn't read first line.", line);

        d = DeliveryData.parse(line);
        boolean isEquals = expected.equals(d);
        Assert.assertTrue("DeliveryData object wrong.", isEquals);
    }

    @Test
    public void get06cleanTest() throws Exception{
        DeliveryData d = createUniqueData(DeliveryData.Status.INIT);
        journal.write(d);

        d.setStatus(DeliveryData.Status.SEND);
        journal.write(d);

        d.setStatus(DeliveryData.Status.DONE);
        journal.write(d);

        journal.cleanDeliveryJournal();

        boolean empty = j1.length() == 0;
        Assert.assertTrue("First journal not empty.", empty);

        empty = j2.length() == 0;
        Assert.assertTrue("Second journal not empty.", empty);
    }

    @Test
    public void get07cleanTest() throws Exception{
        DeliveryData d = createUniqueData(DeliveryData.Status.INIT);
        journal.write(d);

        d.setStatus(DeliveryData.Status.SEND);
        journal.write(d);

        journal.cleanDeliveryJournal();

        boolean empty = j1.length() == 0;
        Assert.assertTrue("First journal not empty.", empty);

        BufferedReader br = new BufferedReader(new FileReader(j2));
        String line = br.readLine();
        br.close();
        Assert.assertNotNull("Couldn't read first line.", line);

        DeliveryData loaded = DeliveryData.parse(line);
        boolean isEquals = d.equals(loaded);
        Assert.assertTrue("DeliveryData object wrong.", isEquals);
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
