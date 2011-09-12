package mobi.eyeline.util.jsf.components;

import java.util.ArrayList;
import java.util.Collection;

/**
 * User: artem
 * Date: 05.09.11
 */
public class JsonUtils {

  public static String toJson(Collection<String> cl) {
    return toJson(new ArrayList<String>(cl).toArray(new String[cl.size()]));
  }


  public static String toJson(String[] strings) {
    StringBuilder res = new StringBuilder();
    res.append('[');
    if (strings != null) {
      boolean first = true;
      for (String s : strings) {
        if (!first) {
          res.append(',');
        } else {
          first = false;
        }
        res.append("&quot;").append(s).append("&quot;");
      }
    }
    res.append(']');
    return res.toString();
  }

  public static String[] toArray(String json) {
    if (json == null || json.length() <= 2) {
      return null;
    }
    json = json.substring(1, json.length() - 1); //remove []

    String[] ss = json.split(",");
    String[] res = new String[ss.length];

    int i = 0;
    for (String s : ss) {
      res[i] = s.substring(1, s.length() - 1); //remove ""
      i++;
    }
    return res;
  }


  public static String jsonEscape(String s) {
    StringBuilder sb = new StringBuilder();
    for (int i = 0; i < s.length(); i++) {
      char ch = s.charAt(i);
      switch (ch) {
        case '"':
          sb.append("\\\"");
          break;
        case '\\':
          sb.append("\\");
          break;
        case '\b':
          sb.append("\\b");
          break;
        case '\f':
          sb.append("\\f");
          break;
        case '\n':
//				sb.append("\\n");
          break;
        case '\r':
          sb.append("\\r");
          break;
        case '\t':
          sb.append("\\t");
          break;
        case '/':
          sb.append("/");
          break;
        default:
          if ((ch >= '\u0000' && ch <= '\u001F') || (ch >= '\u007F' && ch <= '\u009F') || (ch >= '\u2000' && ch <= '\u20FF')) {
            String ss = Integer.toHexString(ch);
            sb.append("\\u");
            for (int k = 0; k < 4 - ss.length(); k++) {
              sb.append('0');
            }
            sb.append(ss.toUpperCase());
          } else {
            sb.append(ch);
          }
      }
    }
    return sb.toString();
  }
}
