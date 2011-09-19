package mobi.eyeline.dcpgw.journal;

import test.TestConfigurationManager;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import test.Test;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 17.09.11
 * Time: 20:11
 */
public class DataTest extends Test {

    private static Data d;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        d = TestConfigurationManager.getData();
    }

    @org.junit.Test
    public void writeTest() throws Exception {
        Assert.assertEquals("Data.equals() doesn't work", d, d);
    }

    @AfterClass
    public static void setUpAfterClass(){
        clean();
    }
}
