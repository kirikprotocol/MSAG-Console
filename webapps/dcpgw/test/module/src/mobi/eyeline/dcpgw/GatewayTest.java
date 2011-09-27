package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.dcp.DcpConnection;
import mobi.eyeline.dcpgw.dcp.DcpConnectionStub;
import mobi.eyeline.smpp.api.pdu.SubmitSM;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import org.junit.BeforeClass;
import org.junit.Test;
import test.T;

import java.text.DateFormat;
import java.util.Calendar;
import java.util.Date;

public class GatewayTest extends T{

    private static Gateway gateway;
    //private static Client client;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        String user_dir = System.getProperty("user.dir");
        System.out.println("user.dir="+user_dir);

        gateway = createGateway();

        DcpConnection stub = new DcpConnectionStub();
        gateway.setDcpConnection("test", stub);

        //client = createClient();


    }

    @Test
    public void sendSubmitSMTest() throws Exception {
        for(int i=0; i<1000; i++){
            SubmitSM submitSM = new SubmitSM();
            submitSM.setRegDeliveryReceipt(RegDeliveryReceipt.SuccessOrFailure);
            submitSM.setConnectionName("con1");

            DateFormat df = DateFormat.getDateTimeInstance();
            Calendar cal = Calendar.getInstance();
            Date date = cal.getTime();

            submitSM.setMessage("message from "+submitSM.getConnectionName()+", "+df.format(date));
            submitSM.setSourceAddress("11111");
            submitSM.setDestinationAddress("79139118729");

            submitSM.setValidityPeriod(1000 * 3600);

            gateway.handlePDU(submitSM);
        }

        wait(10000);

    }


}