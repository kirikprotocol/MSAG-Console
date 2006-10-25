package ru.sibinco.calendarsme.network;

public class SyncObject {

  private boolean syncMode = false;
  private int connId = -1;
  private int seqNum = -1;
  private boolean locked = false;
  private int status = -1;

  public boolean isSyncMode() {
    return syncMode;
  }

  protected void setSyncMode(boolean syncMode) {
    this.syncMode = syncMode;
  }

  public int getConnId() {
    return connId;
  }

  protected void setConnId(int connId) {
    this.connId = connId;
  }

  public int getSeqNum() {
    return seqNum;
  }

  protected void setSeqNum(int seqNum) {
    this.seqNum = seqNum;
  }

  public boolean isLocked() {
    return locked;
  }

  protected void setLocked(boolean locked) {
    this.locked = locked;
  }

  public int getStatus() {
    return status;
  }

  protected void setStatus(int status) {
    this.status = status;
  }

  public void clean() {
    setConnId(-1);
    setSeqNum(-1);
    setLocked(false);
    setStatus(-1);
  }
}
