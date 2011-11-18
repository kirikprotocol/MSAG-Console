package mobi.eyeline.dcpgw.smpp;

import static org.mockito.Mockito.*;

import junit.framework.Assert;
import mobi.eyeline.dcpgw.Config;
import mobi.eyeline.dcpgw.dcp.DcpConnectionImpl;
import mobi.eyeline.dcpgw.model.Delivery;
import mobi.eyeline.dcpgw.model.Provider;
import mobi.eyeline.smpp.api.pdu.SubmitSM;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import test.T;

import java.io.IOException;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 15.11.11
 * Time: 16:48
 */
public class PDUListenerImplTest extends T{

    private static Config config;

    @BeforeClass
    public static void setUpBeforeClass() throws IOException, InvalidAddressFormatException {

        config = mock(Config.class);

        when(config.getInitialMessageIdRang()).thenReturn((long) 0);

        when(config.getRang()).thenReturn(10000);

        Provider provider = new Provider(PROVIDER_NAME);

        Delivery delivery = new Delivery();
        delivery.setId(DELIVERY_ID);
        delivery.setUser(INFORMER_USER);
        String[] ar = new String[1]; ar[0] = SERVICE_NUMBER;
        delivery.setServicesNumbers(ar);
        provider.addDelivery(delivery);

        when(config.getProvider(CONNECTION_NAME)).thenReturn(provider);

        DcpConnectionImpl dcpConnection = mock(DcpConnectionImpl.class);

        when(config.getDCPConnection(INFORMER_USER)).thenReturn(dcpConnection);


    }

    @Test
    public void get01handlePDUTest() throws Exception {
        SubmitSM submitSM = getSubmitSMWithSimpleConfiguration();
        submitSM.setSourceAddress(SERVICE_NUMBER);

        PDUListenerImpl pduListener = new PDUListenerImpl(config);
        Assert.assertTrue("PDU listener couldn't handle SubmitSM.", pduListener.handlePDU(submitSM) );
    }

    /*@Test
    public void get02handlePDUTest() throws Exception {
        SubmitSM submitSM = getSubmitSMWithSimpleConfiguration();
        submitSM.setSourceAddress(WRONG_SERVICE_NUMBER);

        PDUListenerImpl pduListener = new PDUListenerImpl(config);
        Assert.assertTrue("PDU listener couldn't handle SubmitSM.", !pduListener.handlePDU(submitSM) );
    } */

    @AfterClass
    public static void setUpAfterClass(){

    }

}
