package ru.novosoft.smsc.topmon;

import ru.novosoft.smsc.util.SnapBufferReader;

import java.io.IOException;
import java.util.Comparator;
import java.util.Arrays;

import org.apache.log4j.Category;

/**
 * Created by: Serge Lugovoy
 * Date: 14.05.2004
 * Time: 11:53:41
 */
public class TopSnap {
  private static Category logger = Category.getInstance(TopSnap.class);
  public int     smeCount;
  public SmeSnap smeSnap[];
  public int     errCount;
  public ErrorSnap errSnap[];

  public TopSnap() {
    smeCount = 0;
    smeSnap = null;
    errCount = 0;
    errSnap = null;
  }

  public TopSnap(TopSnap snap) {
    smeCount = snap.smeCount;
    smeSnap = new SmeSnap[smeCount];
    for( int i=0; i < smeCount; i++) {
      smeSnap[i] = new SmeSnap(snap.smeSnap[i]);
    }
    errCount = snap.errCount;
    errSnap = new ErrorSnap[errCount];
    for( int i=0; i < errCount; i++) {
      errSnap[i] = new ErrorSnap(snap.errSnap[i]);
    }
  }
  public void init( SnapBufferReader in ) throws IOException {
    smeCount = ((int)in.readNetworkShort())&0xFFFF;
    if( smeSnap == null || smeSnap.length < smeCount) {
      smeSnap = new SmeSnap[smeCount];
      for( int i = 0; i < smeCount; i++) {
        smeSnap[i] = new SmeSnap();
      }
    }
    logger.debug("SmeSnap array = "+smeSnap+" count = "+smeCount);
    for( int i = 0; i < smeCount; i++) {
      smeSnap[i].init(in);
    }
    errCount = ((int)in.readNetworkShort())&0xFFFF;
    if( errSnap == null || errSnap.length < errCount) {
      errSnap = new ErrorSnap[errCount];
      for( int i = 0; i < errCount; i++) {
        errSnap[i] = new ErrorSnap();
      }
    }
    for( int i = 0; i < errCount; i++) {
      errSnap[i].init(in);
    }
  }

  public void write(java.io.DataOutputStream out) throws IOException {
    out.writeInt(smeCount);
    for( int i = 0; i < smeCount; i++) {
      smeSnap[i].write(out);
    }
    out.writeInt(errCount);
    for( int i = 0; i < errCount; i++) {
      errSnap[i].write(out);
    }
  }

  public void read(java.io.DataInputStream in) throws IOException {
    smeCount = in.readInt();
    if( smeSnap == null || smeSnap.length < smeCount) {
      smeSnap = new SmeSnap[smeCount];
      for( int i = 0; i < smeCount; i++) {
        smeSnap[i] = new SmeSnap();
      }
    }
    for( int i = 0; i < smeCount; i++) {
      smeSnap[i].read(in);
    }
    errCount = in.readInt();
    if( errSnap == null || errSnap.length < errCount) {
      errSnap = new ErrorSnap[errCount];
      for( int i = 0; i < errCount; i++) {
        errSnap[i] = new ErrorSnap();
      }
    }
    for( int i = 0; i < errCount; i++) {
      errSnap[i].read(in);
    }
  }

  public void sortSme(Comparator c) {
    Arrays.sort(smeSnap, c);
  }

  public void sortErr(Comparator c) {
    Arrays.sort(errSnap, c);
  }
}
