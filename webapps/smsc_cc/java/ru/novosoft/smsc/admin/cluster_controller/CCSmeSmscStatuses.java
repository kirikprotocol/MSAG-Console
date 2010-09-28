package ru.novosoft.smsc.admin.cluster_controller;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class CCSmeSmscStatuses {

  private String smeId;
  private int connectType;
  private List<ConnectStatus> connectStatuses;

  public CCSmeSmscStatuses(String smeId, int connectType) {
    this.smeId = smeId;
    this.connectType = connectType;
    this.connectStatuses = new ArrayList<ConnectStatus>();
  }

  void addConnectStatus(int nodeIndex, int status, Integer bindMode, String peerIn, String peerOut) {
    connectStatuses.add(new ConnectStatus(nodeIndex, status, bindMode, peerIn, peerOut));
  }

  public String getSmeId() {
    return smeId;
  }

  public int getConnectType() {
    return connectType;
  }

  public List<ConnectStatus> getConnectStatuses() {
    return Collections.unmodifiableList(connectStatuses);
  }

  public static class ConnectStatus {
    int nodeIndex;
    int status;
    Integer bindMode;
    String peerIn;
    String peerOut;

    private ConnectStatus(int nodeIndex, int status, Integer bindMode, String peerIn, String peerOut) {
      this.nodeIndex = nodeIndex;
      this.status = status;
      this.bindMode = bindMode;
      this.peerIn = peerIn;
      this.peerOut = peerOut;
    }

    public int getNodeIndex() {
      return nodeIndex;
    }

    public int getStatus() {
      return status;
    }

    public Integer getBindMode() {
      return bindMode;
    }

    public String getPeerIn() {
      return peerIn;
    }

    public String getPeerOut() {
      return peerOut;
    }
  }
}
