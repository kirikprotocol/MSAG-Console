package ru.novosoft.smsc.topmon;

import ru.novosoft.smsc.util.SnapBufferReader;

import java.io.IOException;

/**
 * Created by: Serge Lugovoy
 * Date: 14.05.2004
 * Time: 11:56:51
 */
public class ErrorSnap {
  public int code;
  public short count;
  public short avgCount;

  public ErrorSnap() {
  }

  public ErrorSnap( ErrorSnap snap) {
    code = snap.code;
    count = snap.count;
    avgCount = snap.avgCount;
  }

  public void init( SnapBufferReader in ) throws IOException {
    code = in.readNetworkInt();
    count = in.readNetworkShort();
    avgCount = in.readNetworkShort();
  }
  public void write(java.io.DataOutputStream out) throws IOException {
    out.writeInt(code);
    out.writeShort(count);
    out.writeShort(avgCount);
  }

  public void read(java.io.DataInputStream in) throws IOException {
    code = in.readInt();
    count = in.readShort();
    avgCount = in.readShort();
  }

}
