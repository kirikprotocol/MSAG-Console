package ru.sibinco.ss7.logparser;

import java.text.ParseException;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 16:17:53
 */
public class Util {
  public static String hex( byte data[], int start, int len ) {
    StringBuffer hex = new StringBuffer(len*3);
    int end = start+len;
    for( int i = start; i < end; i++ ) {
      String s = Integer.toHexString(((int)data[i])&0xFF).toUpperCase();
      if( s.length() == 1 ) {
        hex.append('0');
      }
      hex.append(s).append(',');
    }
    return hex.toString();
  }

  public static int unsigned( byte data ) {
    return ((int)data)&0xFF;
  }

  public static int unsigned( short data ) {
    return ((int)data)&0xFFFF;
  }

  public static String hex( byte data ) {
    return Integer.toHexString( ((int)data)&0xFF ).toUpperCase();
  }

  public static String hex( short data ) {
    return Integer.toHexString( ((int)data)&0xFFFF ).toUpperCase();
  }

  static byte parseByte( String s ) throws ParseException {
    if( s.length() > 2 ) throw new ParseException("Invalid hex string: "+s, 0);
    if( s.length() == 2 ) {
      return (byte)((parseSemiOctet(s.charAt(0))<<4)|parseSemiOctet(s.charAt(1)));
    }
    return (byte)parseSemiOctet(s.charAt(0));
  }

  static int parseSemiOctet( char c ) throws ParseException {
    if( c >= 'A' && c <= 'F' ) return 10+(c-'A');
    if( c >= 'a' && c <= 'f' ) return 10+(c-'a');
    if( c >= '0' && c <= '9' ) return c - '0';
    throw new ParseException("Invalid hex char: "+c, 0);
  }
}
