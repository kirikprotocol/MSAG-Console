package mobi.eyeline.dcpgw.journal;

import org.junit.*;
import test.T;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;

public class JournalTest extends T {

    private static Journal journal;
    private static Data expected_data;
    private static File dir, j1;
    private static String user_dir;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        user_dir = System.getProperty("user.dir");
        dir = new File(user_dir+"/.build/journal");
        registerTestObject(dir);

        journal = new Journal(dir, 1);
        registerTestObject(journal);
    }

    @Before
    public void beforeWriteMethod(){
        j1 = new File(user_dir+"/.build/journal/j1.csv");
        registerTestObject(j1);
        expected_data = createStaticData();
        registerTestObject(expected_data);
    }

    @Test
    public void writeTest() throws Exception {
        journal.write(expected_data);
        BufferedReader br = new BufferedReader(new FileReader(j1));
        String line = br.readLine();

        Assert.assertNotNull("Journal file is empty.", line);
        Data data = Data.parse(line, ";");

        boolean isEquals = expected_data.equals(data);
        Assert.assertTrue("Journal write doesn't work.", isEquals);
    }

    @After
    public void afterWriteMethod(){
        clean();
    }

    @Before
    public void beforeLoadMethod(){

    }

    @Test
    public void loadTest() throws Exception {

    }

    @After
    public void afterLoadMethod(){
        clean();
    }

    @AfterClass
    public static void setUpAfterClass(){
        clean();
    }
}
