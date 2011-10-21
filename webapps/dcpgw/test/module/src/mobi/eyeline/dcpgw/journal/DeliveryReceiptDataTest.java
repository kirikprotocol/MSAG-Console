package mobi.eyeline.dcpgw.journal;

import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import test.T;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 17.09.11
 * Time: 20:11
 */
public class DeliveryReceiptDataTest extends T {

    private static DeliveryReceiptData d1, d2;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        d1 = createStaticData();
        d2 = createStaticData();
    }

    @org.junit.Test
    public void writeTest() throws Exception {
        Assert.assertTrue(d1.equals(d2));
    }

    @AfterClass
    public static void setUpAfterClass(){
        clean();
    }
}
