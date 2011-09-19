package test;

import mobi.eyeline.dcpgw.FinalMessageState;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.dcpgw.journal.Status;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;

import java.util.Calendar;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 17.09.11
 * Time: 21:20
 */
public class TestConfigurationManager {

    public static Data getData(){
        Data data = new Data();
        data.setConnectionName("systemId");

        try {
            data.setDestinationAddress(new Address("+79139118729"));
            data.setSourceAddress(new Address("11111"));
        } catch (InvalidAddressFormatException e) {
            e.printStackTrace();
        }

        Calendar cal = Calendar.getInstance();
        data.setDoneDate(cal.getTime());
        data.setSubmitDate(cal.getTime());
        long time = System.currentTimeMillis();
        data.setFirstSendingTime(time);
        data.setLastResendTime(time);
        data.setStatus(Status.SEND);
        data.setNsms(1);
        data.setSequenceNumber(1);
        data.setFinalMessageState(FinalMessageState.DELIVRD);
        long systemId = Long.valueOf("1111111111");
        data.setMessageId(systemId);
        return data;
    }
}
