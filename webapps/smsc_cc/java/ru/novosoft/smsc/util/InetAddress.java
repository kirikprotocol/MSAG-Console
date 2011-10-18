package ru.novosoft.smsc.util;

/**
 * author: Aleksandr Khalitov
 */
public class InetAddress {

  private final String host;
  private final int port;

  public InetAddress(InetAddress address) {
    this.host = address.host;
    this.port = address.port;
  }

  public InetAddress(String host, int port) {
    this.host = host;
    this.port = port;
  }

  public String getHost() {
    return host;
  }

  public int getPort() {
    return port;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    InetAddress address = (InetAddress) o;

    if (port != address.port) return false;
    if (host != null ? !host.equals(address.host) : address.host != null) return false;

    return true;
  }

  @Override
  public int hashCode() {
    int result = host != null ? host.hashCode() : 0;
    result = 31 * result + port;
    return result;
  }

  @Override
  public String toString() {
    return new StringBuilder(host.length()+6).append(host).append(':').append(port).toString();
  }
}
