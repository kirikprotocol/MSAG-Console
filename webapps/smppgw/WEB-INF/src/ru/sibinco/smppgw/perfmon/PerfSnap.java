package ru.sibinco.smppgw.perfmon;

import ru.sibinco.smppgw.perfmon.applet.PerfMon;
import ru.sibinco.lib.backend.util.SnapBufferReader;

import java.util.*;
import java.io.*;

public class PerfSnap {
  public long last[] = {0, 0, 0, 0, 0, 0};
  public long avg[] = {0, 0, 0, 0, 0, 0};
  public long total[] = {0, 0, 0, 0, 0, 0};

  public int uptime;
  public int sctime;
  public int queueSize;

  public String strUptime;
  public String strSctime;

  public final static int IDX_ACCEPTED = 0;
  public final static int IDX_REJECTED = 1;
  public final static int IDX_DELIVERED = 2;
  public final static int IDX_DELIVERERR = 3;
  public final static int IDX_TRANSOK = 4;
  public final static int IDX_TRANSFAILED = 5;

  public PerfSnap() {
  }

  public PerfSnap(PerfSnap snap) {
    System.arraycopy(snap.last, 0, last, 0, snap.last.length);
    System.arraycopy(snap.avg, 0, avg, 0, snap.avg.length);
    System.arraycopy(snap.total, 0, total, 0, snap.total.length);
    uptime = snap.uptime;
    sctime = snap.sctime;
    queueSize = snap.queueSize;
    strSctime = new String(snap.strSctime);
    strUptime = new String(snap.strUptime);
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
    }
  }

  public void write(java.io.DataOutputStream out)
      throws IOException {
    out.writeInt(uptime);
    out.writeInt(sctime);
    out.writeInt(queueSize);
    writeArray(out, last);
    writeArray(out, avg);
    writeArray(out, total);
  }

  void writeArray(java.io.DataOutputStream out, long arrray[]) throws IOException {
    for (int i = 0; i < arrray.length; i++) {
      out.writeLong(arrray[i]);
    }
  }

  public void read(java.io.DataInputStream in)
      throws IOException {
    uptime = in.readInt();
    sctime = in.readInt();
    queueSize = in.readInt();
    readArray(in, last);
    readArray(in, avg);
    readArray(in, total);
  }

  void readArray(java.io.DataInputStream in, long array[]) throws IOException {
    for (int i = 0; i < array.length; i++) {
      array[i] = in.readLong();
    }
  }


  public void init(SnapBufferReader in) throws IOException {
    queueSize = in.readNetworkInt();
    uptime = in.readNetworkInt();
    sctime = in.readNetworkInt();

    in.readNetworkInt(); // read num of counters
    last[PerfSnap.IDX_ACCEPTED] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_ACCEPTED] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_ACCEPTED] = in.readNetworkLong();

    last[PerfSnap.IDX_REJECTED] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_REJECTED] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_REJECTED] = in.readNetworkLong();

    last[PerfSnap.IDX_DELIVERED] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_DELIVERED] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_DELIVERED] = in.readNetworkLong();

    last[PerfSnap.IDX_DELIVERERR] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_DELIVERERR] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_DELIVERERR] = in.readNetworkLong();

    last[PerfSnap.IDX_TRANSOK] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_TRANSOK] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_TRANSOK] = in.readNetworkLong();

    last[PerfSnap.IDX_TRANSFAILED] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_TRANSFAILED] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_TRANSFAILED] = in.readNetworkLong();
  }
}