package ru.novosoft.smsc.topmon;

import ru.novosoft.smsc.util.SnapBufferReader;

import java.io.IOException;

/**
 * Created by: Serge Lugovoy
 * Date: 14.05.2004
 * Time: 11:55:54
 */
public class SmeSnap {
  public static final int COUNTERS_NUM = 6;
  public static final int SP_ACCEPTED = 0;
  public static final int SP_REJECTED = 1;
  public static final int SP_DELIVERED = 2;
  public static final int SP_RESCHEDULED = 3;
  public static final int SP_TEMPERROR = 4;
  public static final int SP_PERMERROR = 5;

         static final int MAX_SME_STR_LEN = 16;

  public String smeId;
  public short  speed[] = new short[COUNTERS_NUM];
  public short  avgSpeed[] = new short[COUNTERS_NUM];

  public SmeSnap() {
  }

  public SmeSnap(SmeSnap snap) {
    smeId = new String(snap.smeId);
    System.arraycopy(snap.speed, 0, speed, 0, COUNTERS_NUM);
    System.arraycopy(snap.avgSpeed, 0, avgSpeed, 0, COUNTERS_NUM);
  }

  public void init( SnapBufferReader in ) throws IOException {
    smeId = in.readNetworkCFixedString(MAX_SME_STR_LEN);
    for( int i = 0; i < COUNTERS_NUM; i++) {
      speed[i] = in.readNetworkShort();
      avgSpeed[i] = in.readNetworkShort();
    }
  }

  public void write(java.io.DataOutputStream out) throws IOException {
    out.writeUTF(smeId);
    for( int i = 0; i < COUNTERS_NUM; i++) {
      out.writeShort(speed[i]);
      out.writeShort(avgSpeed[i]);
    }
  }

  public void read(java.io.DataInputStream in) throws IOException {
    smeId = in.readUTF();
    for( int i = 0; i < COUNTERS_NUM; i++) {
      speed[i] = in.readShort();
      avgSpeed[i] = in.readShort();
    }
  }

}
