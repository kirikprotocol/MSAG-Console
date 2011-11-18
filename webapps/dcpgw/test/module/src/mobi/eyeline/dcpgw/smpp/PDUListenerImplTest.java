package mobi.eyeline.dcpgw.smpp;

import static org.mockito.Mockito.*;

import junit.framework.Assert;
import mobi.eyeline.dcpgw.Config;
import mobi.eyeline.dcpgw.dcp.DcpConnectionImpl;
import mobi.eyeline.dcpgw.model.Provider;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.DataSM;
import mobi.eyeline.smpp.api.pdu.SubmitSM;
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
    private static Server server;

    @BeforeClass
    public static void setUpBeforeClass() throws IOException, SmppException {

        config = mock(Config.class);

        when(config.getInitialMessageIdRang()).thenReturn((long) 0);

        when(config.getRang()).thenReturn(10000);

        Provider provider = new Provider(PROVIDER_NAME);

        provider.addDelivery(getDeliveryWithSimpleConfiguration());

        when(config.getProvider(CONNECTION_NAME)).thenReturn(provider);

        DcpConnectionImpl dcpConnection = mock(DcpConnectionImpl.class);

        when(config.getDCPConnection(INFORMER_USER)).thenReturn(dcpConnection);

        server = mock(Server.class);

    }

    @Test
    public void handlePDUTest() throws Exception {
        PDUListenerImpl pduListener = new PDUListenerImpl(config, server);
        
        SubmitSM submitSM = getSubmitSMWithSimpleConfiguration();
        
        submitSM.setSourceAddress(SERVICE_NUMBER);        
        Assert.assertTrue("PDU listener couldn't handle SubmitSM.", pduListener.handlePDU(submitSM) );
        
        submitSM.setSourceAddress(WRONG_SERVICE_NUMBER);
        Assert.assertTrue("PDU listener couldn't handle SubmitSM with wrong service number", pduListener.handlePDU(submitSM) );

        DataSM dataSM = getDataSMWithSimpleConfiguration();

        dataSM.setSourceAddress(SERVICE_NUMBER);
        Assert.assertTrue("PDU listener couldn't handle DataSM.", pduListener.handlePDU(dataSM) );

        dataSM.setSourceAddress(WRONG_SERVICE_NUMBER);
        Assert.assertTrue("PDU listener couldn't handle DataSM with wrong service number", pduListener.handlePDU(dataSM) );
    }

    @AfterClass
    public static void setUpAfterClass(){

    }

}
