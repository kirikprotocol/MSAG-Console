package test;

import mobi.eyeline.dcpgw.FinalMessageState;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.dcpgw.journal.Status;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
import org.apache.log4j.Logger;
import org.junit.runner.Description;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

public abstract class T {

    public static ArrayList testObjects = new ArrayList();

    private static AtomicInteger ai = new AtomicInteger(0);
    private static SimpleDateFormat sdf2 = new SimpleDateFormat("ddHHmmss");

    private static AtomicLong al = new AtomicLong(0);

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

    public static Data createUniqueData(){
        Data data = new Data();
        data.setConnectionName("systemId");

        try {
            data.setDestinationAddress(new Address("79139118729"));
            data.setSourceAddress(new Address("11111"));
        } catch (InvalidAddressFormatException e) {
            e.printStackTrace();
        }

        Calendar cal = Calendar.getInstance();
        data.setSubmitDate(cal.getTime());
        data.setDoneDate(cal.getTime());
        long time = System.currentTimeMillis();
        data.setFirstSendingTime(time);
        data.setLastResendTime(time);
        data.setStatus(Status.SEND);
        data.setNsms(1);
        int sn = Integer.parseInt(sdf2.format(date)) + ai.incrementAndGet();
        data.setSequenceNumber(sn);
        data.setFinalMessageState(FinalMessageState.DELIVRD);
        long systemId = Long.valueOf("1111111111");
        data.setMessageId(systemId);
        return data;
    }

    public static Data createUniqueData(Status status){
        Data data = new Data();
        data.setConnectionName("systemId");

        try {
            data.setDestinationAddress(new Address("79139118729"));
            data.setSourceAddress(new Address("11111"));
        } catch (InvalidAddressFormatException e) {
            e.printStackTrace();
        }

        Calendar cal = Calendar.getInstance();
        data.setSubmitDate(cal.getTime());
        data.setDoneDate(cal.getTime());
        long time = System.currentTimeMillis();
        data.setFirstSendingTime(time);
        data.setLastResendTime(time);
        data.setStatus(status);
        data.setNsms(1);
        int sn = Integer.parseInt(sdf2.format(date)) + ai.incrementAndGet();
        data.setSequenceNumber(sn);
        data.setFinalMessageState(FinalMessageState.DELIVRD);
        long systemId = Long.valueOf("1111111111");

        long message_id = time + al.incrementAndGet();
        data.setMessageId(systemId);
        return data;
    }

    // Метод возвращает компаратор, который позволяет отсортировать методы в алфавитном порядке
    protected static Comparator forward() {
        return new Comparator() {

            public int compare(Object o1, Object o2) {
                Description d1 = (Description) o1;
                Description d2 = (Description) o2;
                return d1.getDisplayName().compareTo(d2.getDisplayName());
            }
        };
    }

}
