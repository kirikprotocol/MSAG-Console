package ru.sibinco.ss7.logparser;

import java.util.ArrayList;
import java.util.Locale;
import java.util.StringTokenizer;
import java.util.Date;
import java.io.PrintWriter;
import java.io.File;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * Created by: Serge Lugovoy
 * Date: 18.10.2004
 * Time: 10:36:51
 */
public class LogBlock implements Comparable {
  static SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy MMM dd  HH:mm:ss:SSS", Locale.ENGLISH);
  public static final String[] userNames = {
    "UNKNOWN",
    "MTPL2",
    "MTPL3",
    "SCCP",
    "TCAP",
    "ETSIMAP",
    "MGMT",
    "ISUP",
    "USER01",
    "USER02",
    "USER03",
    "USER04",
    "USER05",
    "USER06",
    "USER07",
    "USER08",
    "USER09"
  };

  public static final int USER_UNKNOWN = 0;
  public static final int USER_MTPL2 = 1;
  public static final int USER_MTPL3 = 2;
  public static final int USER_SCCP = 3;
  public static final int USER_TCAP = 4;
  public static final int USER_ETSIMAP = 5;
  public static final int USER_MGMT = 6;
  public static final int USER_ISUP = 7;
  public static final int USER_USER01 = 8;
  public static final int USER_USER02 = 9;
  public static final int USER_USER03 = 10;
  public static final int USER_USER04 = 11;
  public static final int USER_USER05 = 12;
  public static final int USER_USER06 = 13;
  public static final int USER_USER07 = 14;
  public static final int USER_USER08 = 15;
  public static final int USER_USER09 = 16;
  // change length if added new user
  public static final int USER_LENGTH = 17;

  public static final String[] typeNames = {
    "UNKNOWN",
    "MTPL2",
    "MTPL3",
    "SCCP",
    "TCAP",
    "MAP",
    "MGMT",
    "ISUP"
  };
  public static final int TYPE_UNKNOWN = 0;
  public static final int TYPE_MTPL2 = 1;
  public static final int TYPE_MTPL3 = 2;
  public static final int TYPE_SCCP = 3;
  public static final int TYPE_TCAP = 4;
  public static final int TYPE_MAP = 5;
  public static final int TYPE_MGMT = 6;
  public static final int TYPE_ISUP = 7;

  public static int packetTypes[][]; // sender/receiver
  static {
    packetTypes = new int[USER_LENGTH][USER_LENGTH];
    for( int i = 0; i < USER_LENGTH; i++ )
      for( int j = 0; j < USER_LENGTH; j++ )
        packetTypes[i][j] = 0;
    packetTypes[USER_ETSIMAP][USER_USER01] = TYPE_MAP;
    packetTypes[USER_ETSIMAP][USER_USER02] = TYPE_MAP;
    packetTypes[USER_ETSIMAP][USER_USER03] = TYPE_MAP;
    packetTypes[USER_ETSIMAP][USER_USER04] = TYPE_MAP;
    packetTypes[USER_ETSIMAP][USER_USER05] = TYPE_MAP;
    packetTypes[USER_ETSIMAP][USER_USER06] = TYPE_MAP;
    packetTypes[USER_ETSIMAP][USER_USER07] = TYPE_MAP;
    packetTypes[USER_ETSIMAP][USER_USER08] = TYPE_MAP;
    packetTypes[USER_ETSIMAP][USER_USER09] = TYPE_MAP;
    packetTypes[USER_USER01][USER_ETSIMAP] = TYPE_MAP;
    packetTypes[USER_USER02][USER_ETSIMAP] = TYPE_MAP;
    packetTypes[USER_USER03][USER_ETSIMAP] = TYPE_MAP;
    packetTypes[USER_USER04][USER_ETSIMAP] = TYPE_MAP;
    packetTypes[USER_USER05][USER_ETSIMAP] = TYPE_MAP;
    packetTypes[USER_USER06][USER_ETSIMAP] = TYPE_MAP;
    packetTypes[USER_USER07][USER_ETSIMAP] = TYPE_MAP;
    packetTypes[USER_USER08][USER_ETSIMAP] = TYPE_MAP;
    packetTypes[USER_USER09][USER_ETSIMAP] = TYPE_MAP;
    packetTypes[USER_TCAP][USER_ETSIMAP] = TYPE_TCAP;
    packetTypes[USER_ETSIMAP][USER_TCAP] = TYPE_TCAP;
    packetTypes[USER_TCAP][USER_SCCP] = TYPE_SCCP;
    packetTypes[USER_SCCP][USER_TCAP] = TYPE_SCCP;
  }

  public static String lineSeparator;
  {
    lineSeparator = (String) java.security.AccessController.doPrivileged(
               new sun.security.action.GetPropertyAction("line.separator"));
  }

  int type;
  long time;
  boolean sent = false;
  int sender;
  int receiver;
  int primitive;
  int size;
  byte data[];
  String errors;

  public void init(  int type, long time, boolean sent, int sender, int receiver, int primitive, int size, byte data[] ) {
    this.type = type;
    this.time = time;
    this.sent = sent;
    this.sender = sender;
    this.receiver = receiver;
    this.primitive = primitive;
    this.size = size;
    this.data = data;
  }

  public void addError( String line ) {
    if( errors == null ) errors = line;
    else errors = errors+lineSeparator+line;
  }

  public static LogBlock createInstance(ArrayList lines) throws ParseException {
    int curline = 0;
    String line = (String) lines.get(curline++);
    long tm = 0;
    boolean sentFlag = false;
    if( line.startsWith("RECEIVED") ) {
      tm = dateFormat.parse(line.substring("RECEIVED:".length()).trim()).getTime();
    } else if( line.startsWith("SENT") ) {
      tm = dateFormat.parse(line.substring("SENT:".length()).trim()).getTime();
      sentFlag = true;
    } else {
      throw new ParseException("Invalid bloc start: "+line, 0);
    }
    String sender = parseSender( (String) lines.get(curline++));
    String receiver = parseReceiver((String) lines.get(curline++));
    int receiverId = lookupUser(receiver);
    if( receiverId == 0 ) throw new ParseException("Can't determine packet type: "+sender+" -> "+receiver, 0);
    int senderId = lookupUser(sender);
    int type = packetTypes[senderId][receiverId];
    int primitive = parsePrimitive((String) lines.get(curline++));
    int size = parseSize((String) lines.get(curline++));
    byte data[] = new byte[size];
    curline = parseData(lines, curline, data);
    LogBlock block = null;
    switch( type ) {
      case TYPE_MAP:
        block = new ETSIMapBlock();
        break;
      case TYPE_TCAP:
        block = new TCAPBlock();
        break;
      case TYPE_SCCP:
        block = new SCCPBlock();
        break;
      default:
        block = new LogBlock();
        break;
    }
    block.init(type, tm, sentFlag, senderId, receiverId, primitive, size, data );
    for(; curline < lines.size(); curline++) {
      line = (String) lines.get(curline);
      if( line.startsWith("****") ) {
        block.addError(line);
      }
    }
    return block;
  }

  static String parseSender( String line ) throws ParseException {
    if( !line.startsWith("Sender:") ) throw new ParseException("Sender line expected: "+line, 0);
    String s = line.substring("Sender:".length()).trim();
    int pos = s.indexOf(':');
    if( pos != -1 ) return s.substring(0, pos);
    else return s;
  }

  static String parseReceiver( String line ) throws ParseException {
    if( !line.startsWith("Receiver:") ) throw new ParseException("Receiver line expected: "+line, 0);
    String s = line.substring("Receiver:".length()).trim();
    int pos = s.indexOf(':');
    if( pos != -1 ) return s.substring(0, pos);
    else return s;
  }

  static int parsePrimitive( String line ) throws ParseException {
    if( !line.startsWith("Primitive:") ) throw new ParseException("Primitive line expected: "+line, 0);
    String s = line.substring("Primitive:".length()).trim();
    try {
      return Integer.valueOf(s).intValue();
    } catch (NumberFormatException e) {
      throw new ParseException("Invalid priomitive number in: "+line, 0);
    }
  }

  static int parseSize( String line ) throws ParseException {
    if( !line.startsWith("Size:") ) throw new ParseException("Size line expected: "+line, 0);
    String s = line.substring("Size:".length()).trim();
    try {
      return Integer.valueOf(s).intValue();
    } catch (NumberFormatException e) {
      throw new ParseException("Invalid size value in: "+line, 0);
    }
  }

  static int lookupUser( String uname ) {
    for( int i = 0; i < userNames.length; i++ ) {
      if( uname.equals(userNames[i]) ) return i;
    }
    return 0;
  }

  static int lookupType( String tname ) {
    for( int i = 0; i < typeNames.length; i++ ) {
      if( tname.equals(typeNames[i]) ) return i;
    }
    return 0;
  }

  static int parseData( ArrayList lines, int curline, byte data[] ) throws ParseException {
    int bytes = 0;
    try {
      while( bytes < data.length ) {
        String line = (String) lines.get( curline++ );
        StringTokenizer stk = new StringTokenizer(line, ",");
        while( stk.hasMoreTokens() && bytes < data.length ) {
          String tk = stk.nextToken().trim();
          if( tk.length() == 0 ) continue;
          data[bytes++] = Util.parseByte(tk);
        }
      }
    } catch (IndexOutOfBoundsException ex) {
    }
    return curline;
  }

  static SimpleDateFormat printDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:SSS", Locale.ENGLISH);
  public void write( PrintWriter pw ) {
    pw.print(sent?'S':'R');
    pw.print(' ');
    pw.print(printDateFormat.format(new Date(time)));
    pw.print(' ');
    pw.print(userNames[sender]);
    pw.print(" -> ");
    pw.println(userNames[receiver]);
    writePrimitiveInfo(pw);
    pw.print("Size: ");
    pw.println(size);
    String shex = Util.hex(data, 0, size);
    for( int i = 0; i*20 < size; i++ ) {
      int shpos = 60*i;
      if( (i+1)*20 < size ) {
        pw.println(shex.substring(shpos,shpos+60));
      } else {
        pw.println(shex.substring(shpos));
      }
    }
    if( errors != null ) {
      pw.println(errors);
    }
    pw.println();
  }

  public void writePrimitiveInfo( PrintWriter pw ) {
    pw.print("Primitive: ");
    pw.println(primitive);
  }
  public int compareTo(Object o) {
    return (int)(time - ((LogBlock)o).time);
  }
}
