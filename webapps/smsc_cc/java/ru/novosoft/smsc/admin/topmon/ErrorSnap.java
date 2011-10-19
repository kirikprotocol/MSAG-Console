package ru.novosoft.smsc.admin.topmon;

import ru.novosoft.smsc.util.applet.SnapBufferReader;

import java.io.IOException;


public class ErrorSnap {
  public int code;
  public short count;
  public short avgCount;

  public ErrorSnap() {
  }

  public ErrorSnap(ErrorSnap snap) {
    code = snap.code;
    count = snap.count;
    avgCount = snap.avgCount;
  }

  public void init(SnapBufferReader in) throws IOException {
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
