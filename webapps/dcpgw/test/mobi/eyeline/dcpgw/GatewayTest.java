package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.dcp.DcpConnection;
import mobi.eyeline.dcpgw.dcp.PseudoDcpConnectionImpl;
import mobi.eyeline.smpp.api.pdu.SubmitSM;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import mobi.eyeline.smpp.api.types.RegDeliveryReceipt;
import org.junit.BeforeClass;
import org.junit.Test;
import test.T;

import java.text.DateFormat;
import java.util.Calendar;
import java.util.Date;

public class GatewayTest extends T{

    private static Gateway gateway;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
        gateway = new Gateway();
        DcpConnection dcpConnection = new PseudoDcpConnectionImpl();
        //gateway.setDcpConnection("stepanovd", dcpConnection);
    }

    @Test
    public void sendSubmitSMTest() throws Exception {
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


}