package mobi.eyeline.dcpgw.journal;

import test.TestConfigurationManager;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;

public class JournalTest extends test.Test {

    private static Journal journal;
    private static Data expected;
    private static String dir;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        dir = "D:\\work\\dcpgw\\config\\config.properties";
        journal = new Journal(dir, 1);
        registerTestObject(journal);

        expected = TestConfigurationManager.getData();
        registerTestObject(expected);
    }

    @Test
    public void writeTest() throws Exception {
        journal.write(expected);
        File file = new File(dir);
        BufferedReader br = new BufferedReader(new FileReader(file));
        String line = br.readLine();
        Assert.assertNotNull("Journal file is empty.", line);
        Data data = Data.parse(line, ";");
        Assert.assertEquals("Journal write doesn't work.", expected, data);
    }

    @AfterClass
    public static void setUpAfterClass(){
        clean();
    }
}
