package mobi.eyeline.dcpgw.smpp;

import static org.mockito.Mockito.*;

import mobi.eyeline.dcpgw.Config;
import mobi.eyeline.dcpgw.dcp.DcpConnection;
import mobi.eyeline.dcpgw.dcp.DcpConnectionImpl;
import mobi.eyeline.dcpgw.model.Delivery;
import mobi.eyeline.dcpgw.model.Provider;
import mobi.eyeline.smpp.api.pdu.SubmitSM;
import org.junit.BeforeClass;

import java.io.File;
import java.io.IOException;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 15.11.11
 * Time: 16:48
 */
public class PDUListenerImplTest {

    private static PDUListenerImpl pduListener;

    private static String SERVICE_NUMBER = "10001";
    private static String CONNECTION_NAME = "systemId1";
    private static String DESTINATION_ADDRESS = "79139118729";
    private static String INFORMER_USER = "informer_user1";
    private static int DELIVERY_ID = 1111;

    @BeforeClass
    public static void setUpBeforeClass() throws IOException {

        Config config = mock(Config.class);

        when(config.getInitialMessageIdRang()).thenReturn((long) 0);

        when(config.getRang()).thenReturn(10000);

        File file = new File(System.getProperty("user.dir")+ File.separator+".test"+File.separator+"message_id_rang");

        file.createNewFile();

        when(config.getMessageIdRangFile()).thenReturn(file);

        Provider provider = mock(Provider.class);

        when(config.getProvider(CONNECTION_NAME)).thenReturn(provider);

        Delivery delivery = mock(Delivery.class);

        when(provider.getDelivery(SERVICE_NUMBER)).thenReturn(delivery);

        when(delivery.getId()).thenReturn(DELIVERY_ID);

        when(delivery.getUser()).thenReturn(INFORMER_USER);

        DcpConnectionImpl dcpConnection = mock(DcpConnectionImpl.class);

        when(config.getDCPConnection(INFORMER_USER)).thenReturn(dcpConnection);

        pduListener = new PDUListenerImpl(config);
    }

    public void get01handlePDUTest() throws Exception {
        SubmitSM submitSM = new SubmitSM();
        submitSM.setConnectionName(CONNECTION_NAME);
        submitSM.setSourceAddress(SERVICE_NUMBER);
        submitSM.setDestinationAddress(DESTINATION_ADDRESS);

        pduListener.handlePDU(submitSM);

    }

}
