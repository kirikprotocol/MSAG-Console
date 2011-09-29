package test;

//import mobi.eyeline.dcpgw.Client;
import mobi.eyeline.dcpgw.FinalMessageState;
import mobi.eyeline.dcpgw.Gateway;
import mobi.eyeline.dcpgw.model.Provider;
import mobi.eyeline.dcpgw.exeptions.InitializationException;
import mobi.eyeline.dcpgw.journal.Data;
import mobi.eyeline.dcpgw.journal.Status;
import mobi.eyeline.smpp.api.SmppException;
import mobi.eyeline.smpp.api.pdu.data.Address;
import mobi.eyeline.smpp.api.pdu.data.InvalidAddressFormatException;
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

    protected static Comparator forward() {
        return new Comparator() {

            public int compare(Object o1, Object o2) {
                Description d1 = (Description) o1;
                Description d2 = (Description) o2;
                return d1.getDisplayName().compareTo(d2.getDisplayName());
            }
        };
    }

    /*public static Gateway createGateway() throws InitializationException, SmppException {
        Properties properties = new Properties();
        properties.setProperty("smpp.lastActivityTimeout","300000");
        properties.setProperty("smpp.inactivityTimeout","60000");
        properties.setProperty("smpp.connectRetryInterval","5000");
        properties.setProperty("smpp.bindTimeout","30000");
        properties.setProperty("smpp.server.port","9012");
        properties.setProperty("informer.host","silverstone");
        properties.setProperty("informer.port","9573");
        properties.setProperty("informer.messages.list.capacity","100");
        properties.setProperty("sending.timeout.mls","30000");
        properties.setProperty("file.monitor.interval.mls","30000");
        properties.setProperty("max.journal.size.mb","10");
        properties.setProperty("clean.journal.timeout.msl","30000");
        properties.setProperty("resend.receipts.interval.sec","30");
        properties.setProperty("resend.receipts.timeout.sec","30");
        properties.setProperty("resend.receipts.max.timeout.min","720");
        properties.setProperty("update.config.server.port","9876");
        properties.setProperty("final.log.dir",System.getProperty("user.dir")+File.separator+".build"+File.separator+"final_log");

        Hashtable<String, String> user_password_table = new Hashtable<String, String>();
        Hashtable<String, Provider> connection_provider_table = new Hashtable<String, Provider>();

        Gateway gateway = new Gateway();

        return gateway;
    }

    public static Client createClient() throws SmppException {
        Properties properties = new Properties();
        properties.setProperty("smpp.lastActivityTimeout","300000");
        properties.setProperty("smpp.inactivityTimeout","90000");
        properties.setProperty("smpp.connectRetryInterval","5000");
        properties.setProperty("smpp.bindTimeout","30000");
        properties.setProperty("smpp.connector.con1.transformer","mobi.eyeline.smpp.api.transformers.MultipartAssemblingTransformer,mobi.eyeline.smpp.api.transformers.Latin1UnsupportedTransformer");
        properties.setProperty("smpp.connector.con1.smsc","localhost:9012");
        properties.setProperty("smpp.connector.con1.systemId","systemId1");
        properties.setProperty("smpp.connector.con1.password","p1");
        properties.setProperty("smpp.connector.con1.interfaceVersion","v50");

        return new Client(properties);
    }*/


}
