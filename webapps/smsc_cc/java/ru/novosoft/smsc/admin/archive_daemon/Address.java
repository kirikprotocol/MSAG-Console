/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:37:28
 */
package ru.novosoft.smsc.admin.archive_daemon;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.MaskUtils;
import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.PrintWriter;

/**
 * ����� ��������� � ���������� ArchiveDemon
 *
 * @author Aleksandr Khalitov
 */

public class Address {
  private byte tone = 0;
  private byte npi = 0;
  private String mask = null;                //todo ������������

  /**
   * ������ �����
   *
   * @param tone tone
   * @param npi  npi
   * @param mask �����
   */
  public Address(byte tone, byte npi, String mask) {
    this.tone = tone;
    this.npi = npi;
    this.mask = mask;
  }

  /**
   * ������ ����� �� ������
   *
   * @param address - ����� � ���� ������
   * @throws AdminException ������������ ������ ������
   */
  public Address(String address) throws AdminException {
    String tmpAddress = address.trim();
    if (tmpAddress == null)
      throw new NullPointerException("Address string is null");
    if (!MaskUtils.isMaskValid(tmpAddress))
      throw new AdminException("Address \"" + tmpAddress + "\" is not valid");
    MaskUtils.MaskInfo mi = MaskUtils.parseMask(tmpAddress);
    this.tone = mi.getTone();
    this.npi = mi.getNpi();
    this.mask = mi.getMask();
  }

  /**
   * ���������� �����
   *
   * @return �����
   */
  public String getMask() {
    return mask;
  }

  /**
   * ���������� ����� � ���� ������ (� �������: +79139489906, .5.0.somethink)
   *
   * @return ����� � ���� ������
   */
  public String getSimpleAdress() {
    return MaskUtils.getMask(tone, npi, mask);
  }

  /**
   * ���������� ��������������� ����� � ���� ������: 'tone'.'npi'.'mask'
   *
   * @return ����� � ���� ������
   */
  public String getNormalizedAddress() {
    return MaskUtils.getNormalizedMask(tone, npi, mask);
  }

  public boolean equals(Object obj) {
    if (obj instanceof Address) {
      Address m = (Address) obj;
      return m.tone == this.tone && m.npi == this.npi && m.mask.equals(this.mask);
    } else
      return false;
  }

  /**
   * ���������� ������ � �����
   *
   * @param out �����
   * @return �����
   */
  public PrintWriter store(PrintWriter out) {
    out.println("       <adress value=\"" + StringEncoderDecoder.encode(getNormalizedAddress()) + "\"/>");
    return out;
  }

  /**
   * ���������� tone
   *
   * @return tone
   */
  public byte getTone() {
    return tone;
  }

  /**
   * ���������� npi
   *
   * @return npi
   */
  public byte getNpi() {
    return npi;
  }

}