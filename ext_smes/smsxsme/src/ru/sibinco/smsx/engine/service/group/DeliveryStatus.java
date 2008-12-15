package ru.sibinco.smsx.engine.service.group;

import java.util.Collection;

/**
 * User: artem
 * Date: 28.11.2008
 */
public class DeliveryStatus {

  public static final byte ACCEPTED = 0;
  public static final byte SENT = 1;
  public static final byte DELIVERED = 2;
  public static final byte NOT_DELIVERED = -1;

  private String[] members;
  private byte[] statuses;

  public DeliveryStatus(Collection<String> members) {
    this.members = new String[members.size()];
    this.statuses = new byte[members.size()];
    int i = 0;
    for (String m : members)
      this.members[i++] = m;
  }

  void setStatus(String member, int s) {
    for (int i=0; i < members.length; i++) {
      if (members[i].equals(member)) {
        statuses[i] = (byte)s;
        break;
      }
    }
  }

  public String[] members() {
    return members;
  }

  public byte[] statuses() {
    return statuses;
  }

  boolean isAllMessagesSent() {
    for (int i=0; i < members.length; i++) {
      if (statuses[i] == ACCEPTED)
        return false;
    }
    return true;
  }
}
