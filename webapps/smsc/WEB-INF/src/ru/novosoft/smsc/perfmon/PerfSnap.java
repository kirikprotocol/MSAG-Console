package ru.novosoft.smsc.perfmon;

import ru.novosoft.smsc.perfmon.applet.PerfMon;

import java.util.*;
import java.text.*;
import java.io.*;

public class PerfSnap {
  public long last[] = {0,0,0,0};
  public long avg[] = {0,0,0,0};
  public long total[] = {0,0,0,0};

  public int uptime;
  public int sctime;

  public String strUptime;
  public String strSctime;

  public final static int IDX_SUCCESS = 1;
  public final static int IDX_ERROR = 2;
  public final static int IDX_RETRY = 3;


  public PerfSnap() {
  }

  public PerfSnap( PerfSnap snap ) {
    System.arraycopy( snap.last, 0, last, 0, snap.last.length );
    System.arraycopy( snap.avg, 0, avg, 0, snap.avg.length );
    System.arraycopy( snap.total, 0, total, 0, snap.total.length );
    uptime = snap.uptime;
    sctime = snap.sctime;
    strSctime = new String( snap.strSctime );
    strUptime = new String( snap.strUptime );
  }

  protected String verbDigit( String key, int number ) {
    if( number >= 10 && number < 20 ) return PerfMon.localeText.getString( key+"5" );
    int i = number%10;
    if( i == 0 ) {
      return PerfMon.localeText.getString( key+"5" );
    } else if( i == 1 ) {
      return PerfMon.localeText.getString( key+"1" );
    } else if( i < 5 ) {
      return PerfMon.localeText.getString( key+"2" );
    } else {
      return PerfMon.localeText.getString( key+"5" );
    }
  }

  public void calc() {
    last[0] = last[1]+last[2]+last[3];
    avg[0] = avg[1]+avg[2]+avg[3];
    total[0] = total[1]+total[2]+total[3];

    {
      StringBuffer sb = new StringBuffer( 128 );
      boolean include = false;
      int days = uptime/(3600*24);
      if( days > 0 ) {
        sb.append( days );
        sb.append( ' ' );
        sb.append( verbDigit( "uptime.days", days ) );
        sb.append( ' ' );
        include = true;
      }
      int hours = (uptime-(days*3600*24))/3600;
      if( include || hours > 0 ) {
        sb.append( hours );
        sb.append( ' ' );
        sb.append( verbDigit( "uptime.hours", hours ) );
        sb.append( ' ' );
        include = true;
      }
      int minutes = (uptime%3600)/60;
      if( include || minutes > 0 ) {
        sb.append( minutes );
        sb.append( ' ' );
        sb.append( verbDigit( "uptime.minutes", minutes ) );
        sb.append( ' ' );
      }
      int seconds = uptime%60;
      if( seconds < 10 ) sb.append( '0' );
      sb.append( seconds );
      sb.append( ' ' );
      sb.append( verbDigit( "uptime.seconds", seconds ) );
      strUptime = sb.toString();
    }

    {
      strSctime = PerfMon.dateFormat.format( new Date( ((long)sctime)*1000 ) );
    }

  }

  public void write(java.io.DataOutputStream out)
  throws IOException
  {
    out.writeInt( uptime );
    out.writeInt( sctime );
    out.writeLong( last[IDX_SUCCESS] );
    out.writeLong( last[IDX_ERROR] );
    out.writeLong( last[IDX_RETRY] );
    out.writeLong( avg[IDX_SUCCESS] );
    out.writeLong( avg[IDX_ERROR] );
    out.writeLong( avg[IDX_RETRY] );
    out.writeLong( total[IDX_SUCCESS] );
    out.writeLong( total[IDX_ERROR] );
    out.writeLong( total[IDX_RETRY] );
  }

  public void read(java.io.DataInputStream in)
  throws IOException
  {
    uptime = in.readInt();
    sctime = in.readInt();
    last[IDX_SUCCESS] = in.readLong();
    last[IDX_ERROR] = in.readLong();
    last[IDX_RETRY] = in.readLong();
    avg[IDX_SUCCESS] = in.readLong();
    avg[IDX_ERROR] = in.readLong();
    avg[IDX_RETRY] = in.readLong();
    total[IDX_SUCCESS] = in.readLong();
    total[IDX_ERROR] = in.readLong();
    total[IDX_RETRY] = in.readLong();
  }
  
}
