package mobi.eyeline.dcpgw.journal;

import org.junit.*;
import org.junit.runner.JUnitCore;
import org.junit.runner.Request;
import test.T;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.Hashtable;
import test.CalcListener;

public class JournalTest extends T {

    private static Journal journal;
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

        journal = new Journal(dir, 1);
        registerTestObject(journal);
    }

    @Test
    public void getO1writeTest() throws Exception {
        Data e = createStaticData();
        journal.write(e);

        BufferedReader br = new BufferedReader(new FileReader(j1));
        String line = br.readLine();
        br.close();

        Assert.assertNotNull("Journal file is empty.", line);
        Data d = Data.parse(line);

        boolean isEquals = e.equals(d);
        Assert.assertTrue("Journal write doesn't work.", isEquals);

        j1.delete();
    }

    @Test
    public void get020loadTest() throws Exception {
        Hashtable<Integer, Data> t1 = new Hashtable<Integer, Data>();
        for(int i=0; i<100; i++){
            Data d = createUniqueData();
            registerTestObject(d);
            t1.put(d.getSequenceNumber(), d);
            journal.write(d);
        }

        Hashtable<Integer, Data> t2 = journal.load();
        for(Integer sn: t1.keySet()) {
            Data expected = t1.get(sn);
            Data loaded = t2.get(sn);
            Assert.assertNotNull("Loaded data is null.", loaded);

            boolean isEquals = expected.equals(loaded);
            Assert.assertTrue("Expected and loaded data objects differ.", isEquals);
        }

        System.out.println(j1.delete());
    }

    @Test
    public void get021loadTest() throws Exception {
        Data d = createUniqueData();
        journal.write(d);

        d.setStatus(Status.EXPIRED_TIMEOUT);
        journal.write(d);

        Data d1 = createUniqueData();
        journal.write(d1);

        d1.setStatus(Status.EXPIRED_MAX_TIMEOUT);
        journal.write(d1);

        Data d2 = createUniqueData();
        journal.write(d2);

        d2.setStatus(Status.DONE);
        journal.write(d2);

        Data expected = createUniqueData();
        journal.write(expected);

        Hashtable<Integer, Data> t = journal.load();
        Data loaded = t.get(expected.getSequenceNumber());
        boolean isEquals = expected.equals(loaded);
        Assert.assertTrue("Data object wrong.", isEquals);

        j1.delete();
    }

    /*@Test
    public void get022loadTest() throws Exception {

    }*/

    @Test
    public void get03cleanTest() throws Exception{
        Data d = createUniqueData(Status.SEND);
        journal.write(d);
        d.setStatus(Status.DONE);
        journal.write(d);

        Data d1 = createUniqueData(Status.SEND);
        journal.write(d1);
        d1.setStatus(Status.EXPIRED_TIMEOUT);
        journal.write(d1);

        Data d2 = createUniqueData(Status.SEND);
        journal.write(d2);
        d2.setStatus(Status.EXPIRED_MAX_TIMEOUT);
        journal.write(d2);

        journal.clean();

        boolean empty1 = j1.length() == 0;
        Assert.assertTrue("First journal not empty.", empty1);

        boolean empty2 = j2.length() == 0;
        Assert.assertTrue("Second journal not empty.", empty2);

        j1.delete();
        j2.delete();
    }

    @Test
    public void get04cleanTest() throws Exception{
        Data expected1 = createUniqueData(Status.SEND);
        journal.write(expected1);

        Data expected2 = createUniqueData(Status.NOT_SEND);
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

        Data loaded1 = Data.parse(line1);
        boolean isEquals = expected1.equals(loaded1);
        Assert.assertTrue("First data object wrong.", isEquals);

        Data loaded2 = Data.parse(line2);
        isEquals = expected2.equals(loaded2);
        Assert.assertTrue("Second data object wrong.", isEquals);

        j1.delete();
        j2.delete();
    }

    @Test
    public void get05cleanTest() throws Exception{
        Data d = createUniqueData(Status.SEND);
        journal.write(d);

        d.setStatus(Status.EXPIRED_TIMEOUT);
        journal.write(d);

        Data expected = createUniqueData(Status.SEND);
        journal.write(expected);

        journal.clean();

        boolean empty = j1.length() == 0;
        Assert.assertTrue("First journal not empty.", empty);

        BufferedReader br = new BufferedReader(new FileReader(j2));
        String line = br.readLine();
        br.close();
        Assert.assertNotNull("Couldn't read first line.", line);

        d = Data.parse(line);
        boolean isEquals = expected.equals(d);
        Assert.assertTrue("Data object wrong.", isEquals);

        j1.delete();
        j2.delete();
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
