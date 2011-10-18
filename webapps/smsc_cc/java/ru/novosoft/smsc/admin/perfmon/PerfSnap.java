package ru.novosoft.smsc.admin.perfmon;

import ru.novosoft.smsc.util.applet.SnapBufferReader;

import java.io.IOException;
import java.util.Date;

public class PerfSnap {
  public final long[] last = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  public final long[] avg = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  public final long[] total = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  public long uptime;
  public long sctime;
  public int queueSize;
  public int processingSize;
  public int schedulerSize;
  public int dpfSize;

  public Time strUptime = new Time();
  public Date strSctime = new Date();

  public final static int IDX_SUBMIT = 0;
  public final static int IDX_SUBMITERR = 1;
  public final static int IDX_RETRY = 2;
  public final static int IDX_DELIVER = 3;
  public final static int IDX_DELIVERERR = 4;
  public final static int IDX_TEMPERR = 5;
  private final static int IDX_MSU_SUBMIT = 6 + IDX_SUBMIT;
  private final static int IDX_MSU_SUBMITERR = 6 + IDX_SUBMITERR;
  private final static int IDX_MSU_RETRY = 6 + IDX_RETRY;
  private final static int IDX_MSU_DELIVER = 6 + IDX_DELIVER;
  private final static int IDX_MSU_DELIVERERR = 6 + IDX_DELIVERERR;
  private final static int IDX_MSU_TEMPERR = 6 + IDX_TEMPERR;

  private final boolean support64Bit;

  public PerfSnap(boolean support64Bit) {
    this.support64Bit = support64Bit;
  }

  public PerfSnap(PerfSnap snap) {
    System.arraycopy(snap.last, 0, last, 0, snap.last.length);
    System.arraycopy(snap.avg, 0, avg, 0, snap.avg.length);
    System.arraycopy(snap.total, 0, total, 0, snap.total.length);
    uptime = snap.uptime;
    sctime = snap.sctime;
    queueSize = snap.queueSize;
    processingSize = snap.processingSize;
    schedulerSize = snap.schedulerSize;
    dpfSize = snap.dpfSize;
    strSctime = snap.strSctime;
    strUptime = snap.strUptime;
    this.support64Bit = snap.support64Bit;
  }

  public void calc() {
    boolean include = false;
    int days = (int) (uptime / (3600 * 24));
    strUptime = new Time();
    if (days > 0) {
      strUptime.days = days;
      include = true;
    }
    int hours = (int) ((uptime - (days * 3600 * 24)) / 3600);
    if (include || hours > 0) {
      strUptime.hours = hours;
      include = true;
    }
    int minutes = (int) ((uptime % 3600) / 60);
    if (include || minutes > 0) {
      strUptime.minutes = minutes;
    }
    strUptime.seconds = (int) (uptime % 60);


    strSctime = new Date(sctime * 1000);

  }

  public void write(java.io.DataOutputStream out)
      throws IOException {
    if (support64Bit) {
      out.writeLong(uptime);
      out.writeLong(sctime);
    } else {
      out.writeInt((int) uptime);
      out.writeInt((int) sctime);
    }
    out.writeInt(queueSize);
    out.writeInt(processingSize);
    out.writeInt(schedulerSize);
    out.writeInt(dpfSize);
    for (long aLast : last) out.writeLong(aLast);
    for (long anAvg : avg) out.writeLong(anAvg);
    for (long aTotal : total) out.writeLong(aTotal);
  }

  public void read(java.io.DataInputStream in)
      throws IOException {
    if (support64Bit) {
      uptime = in.readLong();
      sctime = in.readLong();
    } else {
      uptime = in.readInt();
      sctime = in.readInt();
    }
    queueSize = in.readInt();
    processingSize = in.readInt();
    schedulerSize = in.readInt();
    dpfSize = in.readInt();
    for (int i = 0; i < last.length; i++)
      last[i] = in.readLong();
    for (int i = 0; i < avg.length; i++)
      avg[i] = in.readLong();
    for (int i = 0; i < total.length; i++)
      total[i] = in.readLong();

  }

  public void init(SnapBufferReader in) throws IOException {
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

    last[PerfSnap.IDX_MSU_SUBMIT] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_MSU_SUBMIT] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_MSU_SUBMIT] = in.readNetworkLong();

    last[PerfSnap.IDX_MSU_SUBMITERR] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_MSU_SUBMITERR] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_MSU_SUBMITERR] = in.readNetworkLong();

    last[PerfSnap.IDX_MSU_DELIVER] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_MSU_DELIVER] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_MSU_DELIVER] = in.readNetworkLong();

    last[PerfSnap.IDX_MSU_TEMPERR] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_MSU_TEMPERR] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_MSU_TEMPERR] = in.readNetworkLong();

    last[PerfSnap.IDX_MSU_DELIVERERR] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_MSU_DELIVERERR] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_MSU_DELIVERERR] = in.readNetworkLong();

    last[PerfSnap.IDX_MSU_RETRY] = (long) in.readNetworkInt();
    avg[PerfSnap.IDX_MSU_RETRY] = (long) in.readNetworkInt();
    total[PerfSnap.IDX_MSU_RETRY] = in.readNetworkLong();

    if (support64Bit) {
      uptime = in.readNetworkLong();
      sctime = in.readNetworkLong();
    } else {
      uptime = in.readNetworkInt();
      sctime = in.readNetworkInt();
    }
    queueSize = in.readNetworkInt();
    processingSize = in.readNetworkInt();
    schedulerSize = in.readNetworkInt();
    try {
      dpfSize = in.readNetworkInt();
    } catch (Throwable e) {
      dpfSize = 0;
    }

  }

  public static class Time {
    private int days;
    private int hours;
    private int minutes;
    private int seconds;

    public Time() {
    }

    public Time(Time t) {
      this.days = t.days;
      this.hours = t.hours;
      this.minutes = t.minutes;
      this.seconds = t.seconds;
    }

    public int getDays() {
      return days;
    }

    public int getHours() {
      return hours;
    }

    public int getMinutes() {
      return minutes;
    }

    public int getSeconds() {
      return seconds;
    }
  }
}