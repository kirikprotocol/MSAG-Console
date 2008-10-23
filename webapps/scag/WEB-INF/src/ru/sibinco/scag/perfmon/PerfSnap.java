package ru.sibinco.scag.perfmon;

import ru.sibinco.scag.perfmon.applet.PerfMon;
import ru.sibinco.lib.backend.util.SnapBufferReader;

import java.util.*;
import java.io.*;

public class PerfSnap {

    public int uptime;
    public int sctime;
    
    public int sessionCount;
    public int sessionLockedCount;
    
    public int smppReqQueueLen;
    public int smppResQueueLen;
    public int smppLCMQueueLen;

    public int httpReqQueueLen;
    public int httpResQueueLen;
    public int httpLCMQueueLen;

    public SMPPSnap smppSnap;
    public HTTPSnap httpSnap;

    public String strUptime;
    public String strSctime;
    
    public String strSessionCount ="";
    public String strSessionLockedCount ="";
    
    public String strSmppReqQueueLen ="";
    public String strSmppResQueueLen ="";
    public String strSmppLCMQueueLen ="";
    
    public String strHttpReqQueueLen ="";
    public String strHttpResQueueLen ="";
    public String strHttpLCMQueueLen ="";

    public class SMPPSnap{
      public long last[] = {0, 0, 0, 0, 0};
      public long avg[] = {0, 0, 0, 0, 0};
      public long total[] = {0, 0, 0, 0, 0};

      public final static int IDX_ACCEPTED = 0;
      public final static int IDX_REJECTED = 1;
      public final static int IDX_DELIVERED = 2;
      public final static int IDX_GW_REJECTED = 3;
      public final static int IDX_FAILED = 4;
    }

    public class HTTPSnap{
      public long last[] = {0, 0, 0, 0, 0, 0};
      public long avg[] = {0, 0, 0, 0, 0, 0};
      public long total[] = {0, 0, 0, 0, 0, 0};

      public final static int IDX_REQUEST = 0;
      public final static int IDX_REQUEST_REJECTED = 1;
      public final static int IDX_RESPONSE = 2;
      public final static int IDX_RESPONSE_REJECTED = 3;
      public final static int IDX_DELIVERED = 4;
      public final static int IDX_FAILED = 5;
    }

    public PerfSnap() {
      smppSnap = new SMPPSnap();
      httpSnap = new HTTPSnap();

    }

    public PerfSnap(PerfSnap snap) {
        this();
        System.arraycopy(snap.smppSnap.last, 0, smppSnap.last, 0, snap.smppSnap.last.length);
        System.arraycopy(snap.smppSnap.avg, 0, smppSnap.avg, 0, snap.smppSnap.avg.length);
        System.arraycopy(snap.smppSnap.total, 0, smppSnap.total, 0, snap.smppSnap.total.length);
        System.arraycopy(snap.httpSnap.last, 0, httpSnap.last, 0, snap.httpSnap.last.length);
        System.arraycopy(snap.httpSnap.avg, 0, httpSnap.avg, 0, snap.httpSnap.avg.length);
        System.arraycopy(snap.httpSnap.total, 0, httpSnap.total, 0, snap.httpSnap.total.length);

        uptime = snap.uptime;
        sctime = snap.sctime;
	
        sessionCount = snap.sessionCount;
	    sessionLockedCount = snap.sessionLockedCount;
	
	    smppReqQueueLen = snap.smppReqQueueLen;
	    smppResQueueLen = snap.smppResQueueLen;
	    smppLCMQueueLen = snap.smppLCMQueueLen;
	
        httpReqQueueLen = snap.httpReqQueueLen;
	    httpResQueueLen = snap.httpResQueueLen;
	    httpLCMQueueLen = snap.httpLCMQueueLen;
	
        strSctime = new String(snap.strSctime);
        strUptime = new String(snap.strUptime);
	
        strSessionCount = new String(snap.strSessionCount);
	    strSessionLockedCount = new String(snap.strSessionLockedCount);
	
        strSmppReqQueueLen = new String(snap.strSmppReqQueueLen);
	    strSmppResQueueLen = new String(snap.strSmppResQueueLen);
	    strSmppLCMQueueLen = new String(snap.strSmppLCMQueueLen);
	
        strHttpReqQueueLen = new String(snap.strHttpReqQueueLen);
	    strHttpResQueueLen = new String(snap.strHttpResQueueLen);
	    strHttpLCMQueueLen = new String(snap.strHttpLCMQueueLen);
    }

    protected String verbDigit(String key, int number) {
        if (number >= 10 && number < 20) return PerfMon.localeText.getString(key + "5");
        int i = number % 10;
        if (i == 0) {
            return PerfMon.localeText.getString(key + "5");
        } else if (i == 1) {
            return PerfMon.localeText.getString(key + "1");
        } else if (i < 5) {
            return PerfMon.localeText.getString(key + "2");
        } else {
            return PerfMon.localeText.getString(key + "5");
        }
    }

    public void calc() {
        {
            StringBuffer sb = new StringBuffer(128);
            boolean include = false;
            int days = uptime / (3600 * 24);
            if (days > 0) {
                sb.append(days);
                sb.append(' ');
                sb.append(verbDigit("uptime.days", days));
                sb.append(' ');
                include = true;
            }
            int hours = (uptime - (days * 3600 * 24)) / 3600;
            if (include || hours > 0) {
                sb.append(hours);
                sb.append(' ');
                sb.append(verbDigit("uptime.hours", hours));
                sb.append(' ');
                include = true;
            }
            int minutes = (uptime % 3600) / 60;
            if (include || minutes > 0) {
                sb.append(minutes);
                sb.append(' ');
                sb.append(verbDigit("uptime.minutes", minutes));
                sb.append(' ');
            }
            int seconds = uptime % 60;
            if (seconds < 10) sb.append('0');
            sb.append(seconds);
            sb.append(' ');
            sb.append(verbDigit("uptime.seconds", seconds));
            strUptime = sb.toString();
        }
        {
            strSctime = PerfMon.dateFormat.format(new Date(((long) sctime) * 1000));
            
	        strSessionCount = new Integer(sessionCount).toString();
	        strSessionLockedCount = new Integer(sessionLockedCount).toString();
	    
            strSmppReqQueueLen = new Integer(smppReqQueueLen).toString();
	        strSmppResQueueLen = new Integer(smppResQueueLen).toString();
	        strSmppLCMQueueLen = new Integer(smppLCMQueueLen).toString();
            
	        strHttpReqQueueLen = new Integer(httpReqQueueLen).toString();
	        strHttpResQueueLen = new Integer(httpResQueueLen).toString();
	        strHttpLCMQueueLen = new Integer(httpLCMQueueLen).toString();
        }
    }

    public void write(java.io.DataOutputStream out) throws IOException {
        out.writeInt(sessionCount);
	    out.writeInt(sessionLockedCount);
	
        out.writeInt(smppReqQueueLen);
	    out.writeInt(smppResQueueLen);
	    out.writeInt(smppLCMQueueLen);
	
        out.writeInt(httpReqQueueLen);
	    out.writeInt(httpResQueueLen);
	    out.writeInt(httpLCMQueueLen);
        
	    out.writeInt(uptime);
        out.writeInt(sctime);
	
        writeArray(out, smppSnap.last);
        writeArray(out, smppSnap.avg);
        writeArray(out, smppSnap.total);
        writeArray(out, httpSnap.last);
        writeArray(out, httpSnap.avg);
        writeArray(out, httpSnap.total);
    }

    void writeArray(java.io.DataOutputStream out, long arrray[]) throws IOException {
        for (int i = 0; i < arrray.length; i++) {
            out.writeLong(arrray[i]);
        }
    }

    public void read(java.io.DataInputStream in) throws IOException {
        sessionCount = in.readInt();
	    sessionLockedCount = in.readInt();
        System.out.println("PerfSnap.read()\nsessionCount=" + sessionCount + "\nsessionLockedCount=" + sessionLockedCount + "\n---------------------");

        smppReqQueueLen = in.readInt();
	    smppResQueueLen = in.readInt();
	    smppLCMQueueLen = in.readInt();
	
        httpReqQueueLen = in.readInt();
	    httpResQueueLen = in.readInt();
	    httpLCMQueueLen = in.readInt();

        uptime = in.readInt();
        sctime = in.readInt();

        readArray(in, smppSnap.last);
        readArray(in, smppSnap.avg);
        readArray(in, smppSnap.total);
        readArray(in, httpSnap.last);
        readArray(in, httpSnap.avg);
        readArray(in, httpSnap.total);
    }

    void readArray(java.io.DataInputStream in, long array[]) throws IOException {
        for (int i = 0; i < array.length; i++) {
            array[i] = in.readLong();
        }
    }


    public void init(SnapBufferReader in) throws IOException {

        sessionCount = in.readNetworkInt();
	    sessionLockedCount = in.readNetworkInt();
        System.out.println("PerfSnap.init()\nsessionCount=" + sessionCount + "\nsessionLockedCount=" + sessionLockedCount + "\n---------------------");

        smppReqQueueLen = in.readNetworkInt();
	    smppResQueueLen = in.readNetworkInt();
	    smppLCMQueueLen = in.readNetworkInt();
	
        httpReqQueueLen = in.readNetworkInt();
	    httpResQueueLen = in.readNetworkInt();
	    httpLCMQueueLen = in.readNetworkInt();

        uptime = in.readNetworkInt();
        sctime = in.readNetworkInt();

        //smpp counter
        smppSnap.last[PerfSnap.SMPPSnap.IDX_ACCEPTED] = (long) in.readNetworkInt();
        smppSnap.avg[PerfSnap.SMPPSnap.IDX_ACCEPTED] = (long) in.readNetworkInt();
        smppSnap.total[PerfSnap.SMPPSnap.IDX_ACCEPTED] =  in.readNetworkLong();

        smppSnap.last[PerfSnap.SMPPSnap.IDX_REJECTED] = (long) in.readNetworkInt();
        smppSnap.avg[PerfSnap.SMPPSnap.IDX_REJECTED] = (long) in.readNetworkInt();
        smppSnap.total[PerfSnap.SMPPSnap.IDX_REJECTED] = in.readNetworkLong();

        smppSnap.last[PerfSnap.SMPPSnap.IDX_DELIVERED] = (long) in.readNetworkInt();
        smppSnap.avg[PerfSnap.SMPPSnap.IDX_DELIVERED] =  (long) in.readNetworkInt();
        smppSnap.total[PerfSnap.SMPPSnap.IDX_DELIVERED] = in.readNetworkLong();

        smppSnap.last[PerfSnap.SMPPSnap.IDX_GW_REJECTED] = (long) in.readNetworkInt();
        smppSnap.avg[PerfSnap.SMPPSnap.IDX_GW_REJECTED] = (long) in.readNetworkInt();
        smppSnap.total[PerfSnap.SMPPSnap.IDX_GW_REJECTED] = in.readNetworkLong();

        smppSnap.last[PerfSnap.SMPPSnap.IDX_FAILED] = (long) in.readNetworkInt();
        smppSnap.avg[PerfSnap.SMPPSnap.IDX_FAILED] = (long) in.readNetworkInt();
        smppSnap.total[PerfSnap.SMPPSnap.IDX_FAILED] = in.readNetworkLong();

       //http counter
        httpSnap.last[PerfSnap.HTTPSnap.IDX_REQUEST] = (long) in.readNetworkInt();
        httpSnap.avg[PerfSnap.HTTPSnap.IDX_REQUEST] = (long) in.readNetworkInt();
        httpSnap.total[PerfSnap.HTTPSnap.IDX_REQUEST] = in.readNetworkLong();

        httpSnap.last[PerfSnap.HTTPSnap.IDX_REQUEST_REJECTED] = (long) in.readNetworkInt();
        httpSnap.avg[PerfSnap.HTTPSnap.IDX_REQUEST_REJECTED] = (long) in.readNetworkInt();
        httpSnap.total[PerfSnap.HTTPSnap.IDX_REQUEST_REJECTED] = in.readNetworkLong();

        httpSnap.last[PerfSnap.HTTPSnap.IDX_RESPONSE] = (long) in.readNetworkInt();
        httpSnap.avg[PerfSnap.HTTPSnap.IDX_RESPONSE] = (long) in.readNetworkInt();
        httpSnap.total[PerfSnap.HTTPSnap.IDX_RESPONSE] = in.readNetworkLong();

        httpSnap.last[PerfSnap.HTTPSnap.IDX_RESPONSE_REJECTED] = (long) in.readNetworkInt();
        httpSnap.avg[PerfSnap.HTTPSnap.IDX_RESPONSE_REJECTED] = (long) in.readNetworkInt();
        httpSnap.total[PerfSnap.HTTPSnap.IDX_RESPONSE_REJECTED] = in.readNetworkLong();

        httpSnap.last[PerfSnap.HTTPSnap.IDX_DELIVERED] = (long) in.readNetworkInt();
        httpSnap.avg[PerfSnap.HTTPSnap.IDX_DELIVERED] = (long) in.readNetworkInt();
        httpSnap.total[PerfSnap.HTTPSnap.IDX_DELIVERED] = in.readNetworkLong();

        httpSnap.last[PerfSnap.HTTPSnap.IDX_FAILED] = (long) in.readNetworkInt();
        httpSnap.avg[PerfSnap.HTTPSnap.IDX_FAILED] = (long) in.readNetworkInt();
        httpSnap.total[PerfSnap.HTTPSnap.IDX_FAILED] = in.readNetworkLong();
    }
}