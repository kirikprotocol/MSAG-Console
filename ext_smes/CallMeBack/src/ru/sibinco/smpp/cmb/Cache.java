package ru.sibinco.smpp.cmb;

/**
 * Created by IntelliJ IDEA.
 * User: kpv
 * Date: Feb 1, 2007
 * Time: 11:57:27 AM
 */
public interface Cache {
  public short getCache(int index) throws IndexOutOfBoundsException;
  public void setCache(int index, short data) throws IndexOutOfBoundsException;
}
