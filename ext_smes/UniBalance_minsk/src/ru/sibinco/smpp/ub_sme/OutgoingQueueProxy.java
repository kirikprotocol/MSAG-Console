package ru.sibinco.smpp.ub_sme;

public interface OutgoingQueueProxy {
  public static int OUTGOING_OBJECT_UPDATED=0;
  public static int OUTGOING_OBJECT_REMOVED_BY_ERROR=1;
  public static int OUTGOING_OBJECT_REMOVED_BY_MAX_ATTEMPS_REACHED =2;
  public static int OUTGOING_OBJECT_NOT_FOUND=3;

  public void addOutgoingObject(Response response);
  public boolean registerMessageStatusListener(MessageStatusListener listener);
}
