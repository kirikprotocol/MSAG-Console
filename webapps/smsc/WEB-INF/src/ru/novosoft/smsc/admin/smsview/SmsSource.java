package ru.novosoft.smsc.admin.smsview;

import ru.novosoft.smsc.util.StringEncoderDecoder;

import java.io.UnsupportedEncodingException;
import java.io.InputStream;
import java.io.DataInputStream;
import java.io.IOException;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 27.01.2004
 * Time: 16:30:52
 * To change this template use Options | File Templates.
 */
public abstract class SmsSource
{
  //private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(Class.class);

  private static byte INT_TAG_TYPE = 0;
  private static byte STR_TAG_TYPE = 1;
  private static byte BIN_TAG_TYPE = 2;
  private static short SMPP_ESM_CLASS_TAG = 2;
  private static short SMPP_DATA_CODING_TAG = 3;
  private static short SMPP_SHORT_MESSAGE_TAG = 28;
  private static short SMPP_MESSAGE_PAYLOAD_TAG = 29;

  private static short DATA_CODING_DEFAULT = 0;    // 0
  private static short DATA_CODING_LATIN1  = 3;    // 11
  private static short DATA_CODING_BINARY  = 4;    // BIT(2)
  private static short DATA_CODING_UCS2    = 8;    // BIT(3)

  public static String getHexString(byte val[])
  {
    StringBuffer sb = new StringBuffer();
    for (int i = 0; i < val.length; i++) {
      sb.append(' ');
      int k = (((int) val[i]) >> 4) & 0xf;
      sb.append((char) (k < 10 ? k + 48 : k + 55));
      k = ((int) val[i]) & 0xf;
      sb.append((char) (k < 10 ? k + 48 : k + 55));
    }
    return sb.toString().toUpperCase();
  }

  public static void parseBody(InputStream is, SmsRow row)
  {
    int textEncoding = DATA_CODING_DEFAULT;
    int esmClass = 0;
    int textLen = 0;
    byte text[] = null;

    //System.out.println("Parsing SMS body ...");
    try {
      DataInputStream stream = new DataInputStream(is);
      while (stream.available() > 0) {
        short tag = stream.readShort();
        byte type = (byte) ((tag & (short) 0xff00) >> 8);
        type &= (short) 0x00ff;
        tag &= (short) 0x00ff;

        int len = (type == INT_TAG_TYPE) ? 4 : stream.readInt();

        //System.out.println("Tag: "+tag+" Type: "+type+" Len: "+len);
        if (tag == SMPP_SHORT_MESSAGE_TAG || tag == SMPP_MESSAGE_PAYLOAD_TAG) {
          byte msgText[] = new byte[textLen = len];
          stream.read(msgText, 0, textLen);
          text = msgText;
          row.addBodyParameter(tag, msgText);
        } else if (tag == SMPP_DATA_CODING_TAG) {
          textEncoding = stream.readInt();
          row.addBodyParameter(tag, new Integer(textEncoding));
        } else if (tag == SMPP_ESM_CLASS_TAG) {
          esmClass = stream.readInt();
          row.addBodyParameter(tag, new Integer(esmClass));
        } else {
          if (type == INT_TAG_TYPE) {
            int val = stream.readInt();
            row.addBodyParameter(tag, new Integer(val));
          } else if (type == STR_TAG_TYPE) {
            byte val[] = new byte[len];
            stream.read(val, 0, len);
            row.addBodyParameter(tag, new String(val));
          } else if (type == BIN_TAG_TYPE) {
            byte val[] = new byte[len];
            stream.read(val, 0, len);
            row.addBodyParameter(tag, val);
          } else {
            byte val[] = new byte[len];
            stream.read(val, 0, len);
            row.addBodyParameter(tag, "invalid tag type: " + getHexString(val));
          }
          //stream.skip(len);
        }
      }
      stream.close();

      String messagePrefix = "";
      if (text != null && text.length>0 && (esmClass & 0x40) == 0x40)
      {
        int headerLen = ((int)text[0])&0xff;  // convert negative byte to int
        if( headerLen > textLen-1 ) {
          text = new String("UDH len greater then message len "+headerLen+"/"+(textLen-1)).getBytes();
          textEncoding = DATA_CODING_LATIN1;
          textLen = text.length;
        } else {
          textLen = textLen-headerLen-1;
          if( textLen > 0 ) {
            byte msgText[] = new byte[textLen];
            System.arraycopy(text,  headerLen+1, msgText, 0, textLen);
            text = msgText;
          } else {
            text = null;
          }
          messagePrefix += "<< UDH "+headerLen+" bytes >> ";
        }
      }

      if (textEncoding == DATA_CODING_UCS2)
        messagePrefix = StringEncoderDecoder.encode(messagePrefix);

      row.setTextEncoded(textEncoding == DATA_CODING_UCS2);
      row.setText(messagePrefix + ((text!=null)?decodeMessage(text, textLen, textEncoding):""));
    }
    catch (IOException exc) {
      System.out.println("SMS Body parsing failed !");
      exc.printStackTrace();
    }
    //System.out.println("SMS body parsed.");
  }

  private static String decodeMessage(byte text[], int len, int encoding)
      throws UnsupportedEncodingException
  {
    String message = "";
    if (encoding == DATA_CODING_LATIN1) {           // ISO-LATIN-1
      message = new String(text, 0, len, "ISO-8859-1");
    } else if (encoding == DATA_CODING_DEFAULT) {   // GSM7
      message = ru.novosoft.smsc.util.GSM7CharSet.decode(text);
    } else if (encoding == DATA_CODING_UCS2) {      // UTF-16
      StringBuffer sb = new StringBuffer();
      for (int i = 0; i < len / 2; ++i) {
        int x = ((((int) text[i * 2]) & 0x0ff) << 8) | ((((int) text[i * 2 + 1]) & 0x0ff));
        sb.append("&#").append(x).append(';');
      }
      message = sb.toString();
    } else if (encoding == DATA_CODING_BINARY) { // DATA_CODING_BINARY
      message = "<< Binary data >>";
    } else {
      message = "<< Unsupported encoding (" + encoding + ") ! >>";
    }
    //System.out.println("Msg: "+message);
    return message;
  }

}
