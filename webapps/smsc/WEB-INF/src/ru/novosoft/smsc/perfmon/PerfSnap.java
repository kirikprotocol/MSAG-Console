package ru.novosoft.smsc.perfmon;

import ru.novosoft.smsc.perfmon.applet.PerfMon;
import ru.novosoft.smsc.util.SnapBufferReader;
import ru.novosoft.smsc.util.Support64Bit;

import java.io.IOException;
import java.util.Date;

public class PerfSnap
{
  public long last[] = {0, 0, 0, 0, 0, 0};
  public long avg[] = {0, 0, 0, 0, 0, 0};
  public long total[] = {0, 0, 0, 0, 0, 0};

  public long uptime;
  public long sctime;
  public int queueSize;
  public int processingSize;
  public int schedulerSize;

  public String strUptime;
  public String strSctime;

  public final static int IDX_SUBMIT = 0;
  public final static int IDX_SUBMITERR = 1;
  public final static int IDX_RETRY = 2;
  public final static int IDX_DELIVER = 3;
  public final static int IDX_DELIVERERR = 4;
  public final static int IDX_TEMPERR = 5;

  public PerfSnap()
  {
  }

  public PerfSnap(PerfSnap snap)
  {
    System.arraycopy(snap.last, 0, last, 0, snap.last.length);
    System.arraycopy(snap.avg, 0, avg, 0, snap.avg.length);
    System.arraycopy(snap.total, 0, total, 0, snap.total.length);
    uptime = snap.uptime;
    sctime = snap.sctime;
    queueSize = snap.queueSize;
    processingSize = snap.processingSize;
    schedulerSize = snap.schedulerSize;
    strSctime = new String(snap.strSctime);
    strUptime = new String(snap.strUptime);
  }

  protected String verbDigit(String key, int number)
  {
    if (number >= 10 && number < 20) return PerfMon.localeText.getString(key + "5");
    int i = number % 10;
    if (i == 0) {
      return PerfMon.localeText.getString(key + "5");
    }
    else if (i == 1) {
      return PerfMon.localeText.getString(key + "1");
    }
    else if (i < 5) {
      return PerfMon.localeText.getString(key + "2");
    }
    else {
      return PerfMon.localeText.getString(key + "5");
    }
  }

  public void calc()
  {
    {
      StringBuffer sb = new StringBuffer(128);
      boolean include = false;
      int days = (int)(uptime / (3600 * 24));
      if (days > 0) {
        sb.append(days);
        sb.append(' ');
        sb.append(verbDigit("uptime.days", days));
        sb.append(' ');
        include = true;
      }
      int hours = (int)((uptime - (days * 3600 * 24)) / 3600);
      if (include || hours > 0) {
        sb.append(hours);
        sb.append(' ');
        sb.append(verbDigit("uptime.hours", hours));
        sb.append(' ');
        include = true;
      }
      int minutes = (int)((uptime % 3600) / 60);
      if (include || minutes > 0) {
        sb.append(minutes);
        sb.append(' ');
        sb.append(verbDigit("uptime.minutes", minutes));
        sb.append(' ');
      }
      int seconds = (int)(uptime % 60);
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
          throws IOException
  {
    if( Support64Bit.enabled ) {
      out.writeLong(uptime);
      out.writeLong(sctime);
    } else {
      out.writeInt((int)uptime);
      out.writeInt((int)sctime);
    }
    out.writeInt(queueSize);
    out.writeInt(processingSize);
    out.writeInt(schedulerSize);
    out.writeLong(last[IDX_DELIVER]);
    out.writeLong(last[IDX_DELIVERERR]);
    out.writeLong(last[IDX_TEMPERR]);
    out.writeLong(last[IDX_SUBMIT]);
    out.writeLong(last[IDX_SUBMITERR]);
    out.writeLong(last[IDX_RETRY]);
    out.writeLong(avg[IDX_DELIVER]);
    out.writeLong(avg[IDX_DELIVERERR]);
    out.writeLong(avg[IDX_TEMPERR]);
    out.writeLong(avg[IDX_SUBMIT]);
    out.writeLong(avg[IDX_SUBMITERR]);
    out.writeLong(avg[IDX_RETRY]);
    out.writeLong(total[IDX_DELIVER]);
    out.writeLong(total[IDX_DELIVERERR]);
    out.writeLong(total[IDX_TEMPERR]);
    out.writeLong(total[IDX_SUBMIT]);
    out.writeLong(total[IDX_SUBMITERR]);
    out.writeLong(total[IDX_RETRY]);
  }

  public void read(java.io.DataInputStream in)
          throws IOException
  {
    if( Support64Bit.enabled ) {
      uptime = in.readLong();
      sctime = in.readLong();
    } else {
      uptime = in.readInt();
      sctime = in.readInt();
    }
    queueSize = in.readInt();
    processingSize = in.readInt();
    schedulerSize = in.readInt();
    last[IDX_DELIVER] = in.readLong();
    last[IDX_DELIVERERR] = in.readLong();
    last[IDX_TEMPERR] = in.readLong();
    last[IDX_SUBMIT] = in.readLong();
    last[IDX_SUBMITERR] = in.readLong();
    last[IDX_RETRY] = in.readLong();
    avg[IDX_DELIVER] = in.readLong();
    avg[IDX_DELIVERERR] = in.readLong();
    avg[IDX_TEMPERR] = in.readLong();
    avg[IDX_SUBMIT] = in.readLong();
    avg[IDX_SUBMITERR] = in.readLong();
    avg[IDX_RETRY] = in.readLong();
    total[IDX_DELIVER] = in.readLong();
    total[IDX_DELIVERERR] = in.readLong();
    total[IDX_TEMPERR] = in.readLong();
    total[IDX_SUBMIT] = in.readLong();
    total[IDX_SUBMITERR] = in.readLong();
    total[IDX_RETRY] = in.readLong();
  }

  public void init(SnapBufferReader in) throws IOException
  {
    in.readNetworkInt(); // read num of counters
    last[PerfSnap.IDX_SUBMIT] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_SUBMIT] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_SUBMIT] = in.readNetworkLong();

    last[PerfSnap.IDX_SUBMITERR] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_SUBMITERR] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_SUBMITERR] = in.readNetworkLong();

    last[PerfSnap.IDX_DELIVER] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_DELIVER] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_DELIVER] = in.readNetworkLong();

    last[PerfSnap.IDX_TEMPERR] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_TEMPERR] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_TEMPERR] = in.readNetworkLong();

    last[PerfSnap.IDX_DELIVERERR] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_DELIVERERR] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_DELIVERERR] = in.readNetworkLong();

    last[PerfSnap.IDX_RETRY] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_RETRY] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_RETRY] = in.readNetworkLong();

    if( Support64Bit.enabled ) {
      uptime = in.readNetworkLong();
      sctime = in.readNetworkLong();
    } else {
      uptime = in.readNetworkInt();
      sctime = in.readNetworkInt();
    }
    queueSize = in.readNetworkInt();
    processingSize = in.readNetworkInt();
    schedulerSize = in.readNetworkInt();
  }
}