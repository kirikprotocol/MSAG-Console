package ru.novosoft.smsc.admin.smsview;

import org.apache.log4j.Category;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;

public abstract class SmsSource {
  private static Category logger = Category.getInstance(SmsSource.class);

  private final static byte INT_TAG_TYPE = 0;
  private final static byte STR_TAG_TYPE = 1;
  private final static byte BIN_TAG_TYPE = 2;

  private final static short SMSC_BACKUP_SME_TAG = 0;
  private final static short SMPP_ESM_CLASS_TAG = 2;
  private final static short SMPP_DATA_CODING_TAG = 3;
  private final static short SMPP_SHORT_MESSAGE_TAG = 28;
  private final static short SMPP_MESSAGE_PAYLOAD_TAG = 29;
  private final static short SMPP_CONCAT_INFO_TAG = 40;
  private final static short SMSC_MERGE_CONCAT_TAG = 45;
  private final static short SMSC_DC_LIST_TAG = 52;

  private final static short DATA_CODING_DEFAULT = 0;  // 0
  private final static short DATA_CODING_LATIN1 = 3;  // 11
  private final static short DATA_CODING_BINARY = 4;  // 10
  private final static short DATA_CODING_UCS2 = 8;  // 100

  public static String getHexString(byte val[]) {
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

  public static void parseBody(InputStream is, SmsRow row) {
    int textEncoding = DATA_CODING_DEFAULT;
    int esmClass = 0;
    int textLen = 0;
    byte text[] = null;
    byte concatInfo[] = null;
    byte partsEncoding[] = null;
    boolean isMergeConcat = false;

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

        switch (tag) {
          case SMSC_BACKUP_SME_TAG: {
            byte msgText[] = new byte[textLen = len];
            stream.read(msgText, 0, textLen);
            text = msgText;
            row.addBodyParameter(tag, msgText);
            break;
          }
          case SMPP_SHORT_MESSAGE_TAG:
          case SMPP_MESSAGE_PAYLOAD_TAG: {
            byte msgText[] = new byte[textLen = len];
            stream.read(msgText, 0, textLen);
            text = msgText;
            row.addBodyParameter(tag, msgText);
            break;
          }
          case SMSC_MERGE_CONCAT_TAG: {
            isMergeConcat = true;
            int mcValue = stream.readInt();
            row.addBodyParameter(tag, new Integer(mcValue));
            break;
          }
          case SMSC_DC_LIST_TAG: {
            partsEncoding = new byte[len];
            stream.read(partsEncoding, 0, len);
            row.addBodyParameter(tag, partsEncoding);
            break;
          }
          case SMPP_CONCAT_INFO_TAG: {
            concatInfo = new byte[len];
            stream.read(concatInfo, 0, len);
            row.addBodyParameter(tag, concatInfo);
            break;
          }
          case SMPP_DATA_CODING_TAG: {
            textEncoding = stream.readInt();
            row.addBodyParameter(tag, new Integer(textEncoding));
            break;
          }
          case SMPP_ESM_CLASS_TAG: {
            esmClass = stream.readInt();
            row.addBodyParameter(tag, new Integer(esmClass));
            break;
          }
          default: {
            switch (type) {
              case INT_TAG_TYPE: {
                int val = stream.readInt();
                row.addBodyParameter(tag, new Integer(val));
                break;
              }
              case STR_TAG_TYPE: {
                byte val[] = new byte[len];
                stream.read(val, 0, len);
                row.addBodyParameter(tag, new String(val));
                break;
              }
              case BIN_TAG_TYPE: {
                byte val[] = new byte[len];
                stream.read(val, 0, len);
                row.addBodyParameter(tag, val);
                break;
              }
              default: {
                byte val[] = new byte[len];
                stream.read(val, 0, len);
                row.addBodyParameter(tag, "invalid tag type: " + getHexString(val));
                break;
              }
            }
            break;
          }
        } // switch (tag)
      } // while (stream.available() > 0)
      stream.close();

      if (text == null || text.length <= 0) return;

      StringBuffer textBuffer = new StringBuffer(text.length);
      StringBuffer originalTextBuffer = new StringBuffer(text.length);
      if (isMergeConcat) {
        if (concatInfo != null) {
          int partsCount = concatInfo[0];
          StringBuffer sb = new StringBuffer();
          for (int i = 0; i < concatInfo.length; i++) {
            sb.append(Integer.toHexString(((int) concatInfo[i]) & 0xFF)).append(' ');
          }
          if (partsEncoding != null && partsCount != partsEncoding.length) {
            IOException exc = new IOException("Invalid partsEncoding count dc:" + partsEncoding.length +
                    " ci:" + partsCount);
            row.setText(exc.getMessage());
            throw exc;
          }

          for (int i = 0; i < partsCount; i++) {
            int offset = ((((int) concatInfo[i * 2 + 1]) & 0xFF) << 8) | (((int) concatInfo[i * 2 + 2]) & 0xFF);
            int len = text.length - offset;
            if (i < partsCount - 1) {
              int offset_next = ((((int) concatInfo[(i + 1) * 2 + 1]) & 0xFF) << 8) | (((int) concatInfo[(i + 1) * 2 + 2]) & 0xFF);
              len = offset_next - offset;
            }
            convertMessage(textBuffer, originalTextBuffer, text, offset, len, true,
                    (partsEncoding != null) ? partsEncoding[i] : textEncoding);
          }
        } else
          convertMessage(textBuffer, originalTextBuffer, text, 0, text.length, true, textEncoding);
      } else
        convertMessage(textBuffer, originalTextBuffer, text, 0, text.length, ((esmClass & 0x40) == 0x40), textEncoding);

      row.setTextEncoded(textEncoding == DATA_CODING_UCS2);
      row.setText(textBuffer.toString());
      row.setOriginalText(originalTextBuffer.toString());
    } catch (IOException exc) {
      logger.warn("SMS Body parsing failed", exc);
    }
    //System.out.println("SMS body parsed.");
  }

  private static void convertMessage(StringBuffer sb, StringBuffer orgsb, byte text[], int start, int len, boolean udh, int encoding) throws UnsupportedEncodingException {
    if (udh) {
      int headerLen = ((int) text[start]) & 0xff;  // convert negative byte to int
      if (headerLen > len - 1) {
        sb.append("<< UDH len greater then message len " + headerLen + "/" + (len - 1) + ">>");
        orgsb.append("<< UDH len greater then message len " + headerLen + "/" + (len - 1) + ">>");
      } else {
        sb.append("<< UDH " + headerLen + " bytes >> ");
        orgsb.append("<< UDH " + headerLen + " bytes >> ");
        int textLen = len - headerLen - 1;
        if (textLen > 0) {
          byte msgText[] = new byte[textLen];
          System.arraycopy(text, start + headerLen + 1, msgText, 0, textLen);
          sb.append(decodeMessage(msgText, textLen, encoding));
          orgsb.append(decodeOriginalMessage(msgText, textLen, encoding));
        }
      }
    } else {
      byte msgText[] = new byte[len];
      System.arraycopy(text, start, msgText, 0, len);
      sb.append(decodeMessage(msgText, len, encoding));
      orgsb.append(decodeOriginalMessage(msgText, len, encoding));
    }
  }


  private static String decodeMessage(byte text[], int len, int encoding)
          throws UnsupportedEncodingException {
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
    return message;
  }

  private static String decodeOriginalMessage(byte text[], int len, int encoding)
          throws UnsupportedEncodingException {
    String message = "";
    if (encoding == DATA_CODING_LATIN1) {           // ISO-LATIN-1
      message = new String(text, 0, len, "ISO-8859-1");
    } else if (encoding == DATA_CODING_DEFAULT) {   // GSM7
      message = ru.novosoft.smsc.util.GSM7CharSet.decode(text);
    } else if (encoding == DATA_CODING_UCS2) {      // UTF-16
      StringBuffer sb = new StringBuffer();
      for (int i = 0; i < len / 2; ++i) {
        int x = ((((int) text[i * 2]) & 0x0ff) << 8) | ((((int) text[i * 2 + 1]) & 0x0ff));
        sb.append((char) x);
      }
      message = sb.toString();
    } else if (encoding == DATA_CODING_BINARY) { // DATA_CODING_BINARY
      message = "<< Binary data >>";
    } else {
      message = "<< Unsupported encoding (" + encoding + ") ! >>";
    }
    return message;
  }
}
