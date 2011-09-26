package mobi.eyeline.dcpgw.dcp;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.DeliveryState;
import mobi.eyeline.informer.admin.delivery.DeliveryStatistics;
import mobi.eyeline.informer.admin.delivery.DeliveryStatus;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.informer.util.Functions;
import sun.org.mozilla.javascript.internal.Function;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 25.09.11
 * Time: 18:20
 */
public class DcpConnectionStub implements DcpConnection{

    private File dir;

    private static final TimeZone STAT_TIMEZONE=TimeZone.getTimeZone("UTC");
    private static final TimeZone LOCAL_TIMEZONE=TimeZone.getDefault();

    private static SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMddHHmm");
    private static SimpleDateFormat sdf2 = new SimpleDateFormat("ss");

    public DcpConnectionStub(){
        dir = new File(System.getProperty("user.dir")+File.separator+".build"+File.separator+"final_log");
    }

    @Override
    public long[] addDeliveryMessages(int deliveryId, List<Message> messages) throws AdminException {

        for(Message m: messages){
            Date date = Functions.convertTime(new Date(System.currentTimeMillis()), LOCAL_TIMEZONE, STAT_TIMEZONE);
            File f = new File(dir, sdf.format(date)+".csv" );

            if (f.length() == 0){
                try {
                    BufferedWriter bw = new BufferedWriter(new FileWriter(f, true));
                    bw.write("#1 SEC,DLVID,USERID,RECTYPE,MSGID,STATE,SMPP,SUBSCRIBER,NSMS,USERDATA\n");
                    bw.flush();
                    bw.close();
                } catch (IOException e) {
                    System.out.println(e);
                }
            }

            try {
                BufferedWriter bw = new BufferedWriter(new FileWriter(f, true));
                bw.write(sdf2.format(date)+","+deliveryId+",test,0,"+m.getId()+",D,0,"+m.getAbonent().getAddress()+","+m.getProperties()+"\n");
                System.out.println(sdf2.format(date)+","+deliveryId+",test,0,"+m.getId()+",D,0,"+m.getAbonent().getAddress()+","+m.getProperties()+"\n");
                bw.flush();
                bw.close();
            } catch (IOException e) {
                System.out.println(e);
            }
        }


        return new long[0];
    }

    @Override
    public void changeDeliveryState(int deliveryId, DeliveryState state) throws AdminException {

    }

    @Override
    public DeliveryStatistics getDeliveryState(int deliveryId) throws AdminException {
        DeliveryStatistics deliveryStatistics = new DeliveryStatistics();
        DeliveryState deliveryState = new DeliveryState();
        deliveryStatistics.setDeliveryState(deliveryState);
        deliveryState.setStatus(DeliveryStatus.Finished);
        return deliveryStatistics;
    }

    @Override
    public void close() {

    }
}
