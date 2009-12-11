/**
 * BlacklistSoap.java
 *
 * This file was auto-generated from WSDL
 * by the Apache Axis 1.4 Apr 22, 2006 (06:55:48 PDT) WSDL2Java emitter.
 */

package ru.sibinco.smsx.engine.soaphandler.blacklist;

public interface BlacklistSoap extends java.rmi.Remote {
  public boolean add(String msisdn) throws java.rmi.RemoteException;
  public boolean remove(String msisdn) throws java.rmi.RemoteException;
  public boolean check(String msisdn) throws java.rmi.RemoteException;
}
