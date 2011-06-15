/*
 * Author: igork
 * Date: 14.05.2002
 * Time: 13:37:28
 */
package mobi.eyeline.informer.util;

import java.io.Serializable;
import java.util.regex.Pattern;

/**
 * Структура для хранения SMPP адресов. Может быть использована в качестве ключа в хеш-таблицах.
 *
 * @author Aleksandr Khalitov
 */

public class Address implements Serializable {

  private static final String pattern_header = "^((\\.[0-6]\\.(0|1|3|4|6|8|9|10|14|18)\\.)|(\\+))?";
  private static final Pattern pattern1 = Pattern.compile(pattern_header + "\\d{0,15}\\?{0,15}$");
  private static final Pattern pattern2 = Pattern.compile(pattern_header + "(\\d|\\?){1,15}$");
  private static final String special_pattern_header = "^\\.5\\.[0-6]\\.";
  private static final Pattern special_pattern1 = Pattern.compile(special_pattern_header + "[ _@\\-:\\.\\,0-9A-Za-z]{0,15}\\?{0,11}$");
  private static final Pattern special_pattern2 = Pattern.compile(special_pattern_header + "([ _@\\-:\\.\\,0-9A-Za-z]|\\?){1,11}$");
  private static final Pattern special_pattern3 = Pattern.compile(special_pattern_header + "ussd:([ _@\\-:\\.\\,0-9A-Za-z]|\\?){1,11}$");

  private byte tone = 0;
  private byte npi = 0;
  private String address = null;

  /**
   * Конструктор копирования
   *
   * @param addr адрес, из которого происходит копирование
   */
  public Address(Address addr) {
    this.tone = addr.tone;
    this.npi = addr.npi;
    this.address = addr.address;
  }


  /**
   * Создаёт адрес из строки
   *
   * @param address - адрес в виде строки
   */
  public Address(String address) {

    if (address == null)
      throw new NullPointerException("Address string is null");

    address = address.trim();

    if (!validate(address))
      throw new IllegalArgumentException("Address \"" + address + "\" is not valid");

    if (address.startsWith(".")) {
      int dp = address.indexOf('.', 1);
      int dp2 = address.indexOf('.', dp + 1);

      String toneStr = address.substring(1, dp);
      tone = Byte.decode(toneStr);
      String npiStr = address.substring(dp + 1, dp2);
      npi = Byte.decode(npiStr);
      this.address = address.substring(dp2 + 1);

    } else {
      if (address.startsWith("+")) {
        address = address.substring(1);
        tone = 1;
      }else {
        tone = 0;
      }
      if(address.startsWith("7") && address.length() == 11) {
        this.tone = 1;
      }else if(address.startsWith("8") && address.length() == 11) {
        address =  new StringBuffer(address.length()).append('7').append(address.substring(1)).toString();
        this.tone = 1;
      }

      this.npi = 1;
      this.address = address;
    }

  }

  /**
   * Возвращает true, если адрес имеет корректный формат
   *
   * @param address проверяемый адрес
   * @return true, если адрес имеет корректный формат. Иначе - false.
   */
  public synchronized static boolean validate(String address) {

    return address != null && address.trim().length() > 0
        && ((pattern1.matcher(address).matches() && pattern2.matcher(address).matches())
        || (special_pattern1.matcher(address).matches() && special_pattern2.matcher(address).matches())
        || special_pattern3.matcher(address).matches());
  }

  /**
   * Преобразует адрес в строку формата .tone.npi.address
   *
   * @param tone    tone
   * @param npi     npi
   * @param address address
   * @return строку формата .tone.npi.address
   */
  public static String toNormalizedAddress(int tone, int npi, String address) {
    return "." + tone + "." + npi + "." + address;
  }

  /**
   * Возвращает адрес
   *
   * @return адрес
   */
  public String getAddress() {
    return address;
  }

  /**
   * Возвращает адрес в виде строки (К примеру: +79139489906, .5.0.somethink)
   *
   * @return адрес в виде строки
   */
  public String getSimpleAddress() {
    if (tone == 1 && npi == 1)
      return "+" + address;
    else if (tone == 0 && npi == 1)
      return (address);
    else
      return getNormalizedAddress();
  }

  /**
   * Возвращает нормализованный адрес в виде строки: 'tone'.'npi'.'address'
   *
   * @return адрес в виде строки
   */
  public String getNormalizedAddress() {
    return toNormalizedAddress(tone, npi, address);
  }

  public boolean equals(Object obj) {
    if (obj instanceof Address) {
      Address m = (Address) obj;
      return m.tone == this.tone && m.npi == this.npi && m.address.equals(this.address);
    } else
      return false;
  }

  public int hashCode() {
    return getNormalizedAddress().hashCode();
  }

  /**
   * Возвращает tone
   *
   * @return tone
   */
  public byte getTone() {
    return tone;
  }

  /**
   * Возвращает npi
   *
   * @return npi
   */
  public byte getNpi() {
    return npi;
  }

  @Override
  public String toString() {
    return getNormalizedAddress();
  }
}