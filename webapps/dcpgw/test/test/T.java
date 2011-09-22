package test;

import mobi.eyeline.dcpgw.FinalMessageState;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.dcpgw.journal.Status;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import org.apache.log4j.Logger;

import java.io.File;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.Iterator;

public abstract class T {

    public static ArrayList testObjects = new ArrayList();

    public static void registerTestObject(Object object){
        testObjects.add(object);
    }

    public static void clean(){
        for (Object testObject : testObjects) {
            try {
                deleteObject(testObject);
            } catch (RuntimeException e) {
                System.out.println(e);
            }
        }
    }

    private static void deleteObject(Object o){
        if (o instanceof Data){
            o = null;
        } else if (o instanceof File){
            File f = (File) o;
            if (f.isFile()){
                f.delete();
            }
        }
    }

    private static Date date = Calendar.getInstance().getTime();
    private static long time = System.currentTimeMillis();

    public static Data createStaticData(){
        Data data = new Data();
        data.setConnectionName("systemId");

        try {
            data.setDestinationAddress(new Address("79139118729"));
            data.setSourceAddress(new Address("11111"));
        } catch (InvalidAddressFormatException e) {
            e.printStackTrace();
        }

        data.setDoneDate(date);
        data.setSubmitDate(date);
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
