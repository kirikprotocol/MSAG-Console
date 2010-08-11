package ru.novosoft.smsc.web.components;

import java.io.IOException;
import java.io.Writer;
import java.util.List;

/**
 * @author Artem Snopkov
 */
public class HTMLRenderer {
  // Input =============================================================================================================

  public static void textInput(Writer w, String className, String name, String id, String value, boolean readOnly, int maxLength, String size, Validation validation, String additional) throws IOException {
    input(w, "txt", className, name, id, value, readOnly, false, maxLength, size, null, validation, additional);
  }

  public static void check(Writer w, String name, boolean checked) throws IOException {
    input(w, "checkBox", null, name, name, null, false, checked, -1, null, null, null, null);
  }

  private static void input(Writer buffer, String type, String className, String name, String id, String value, boolean readOnly, boolean checked, int maxLength, String size, String src, Validation validation, String additional) throws IOException {
    buffer.append("<input type=\"").append(type).append("\"");

    if (className != null)
      buffer.append(" class=\"").append(className).append("\"");

    if (name != null)
      buffer.append(" name=\"").append(name).append("\"");

    if (id != null)
      buffer.append(" id=\"").append(id).append("\"");

    if (value != null)
      buffer.append(" value=\"").append(value).append("\"");

    if (readOnly)
      buffer.append(" readOnly");

    if (checked)
      buffer.append(" checked");

    if (maxLength >= 0)
      buffer.append(" maxLength=\"").append(String.valueOf(maxLength)).append("\"");

    if (size != null)
      buffer.append(" size=\"").append(size).append("\"");

    if (src != null)
      buffer.append(" src=\"").append(src).append("\"");

    if (validation != null) {
      buffer.append(" validation=\"").append(validation.getValidation()).append("\"");
//      buffer.append(" onChange=\"resetValidation(this)\"");
    }

    if (additional != null)
      buffer.append(additional);

    buffer.append(">");
  }


  // Image =============================================================================================================

  public static void image(Writer w, String src, String onClick, String title) throws IOException {
    image(w, src, onClick, title, null);
  }

  public static void image(Writer buffer, String src, String onClick, String title, String className) throws IOException {
    buffer.append("<img src=\"").append(src).append("\"");

    if (onClick != null)
      buffer.append(" onclick=\"").append(onClick).append("\"");

    if (title != null)
      buffer.append(" title=\"").append(title).append("\"");

    if (className != null)
      buffer.append(" class=\"").append(className).append("\"");

    buffer.append(">");
  }


  // TR ================================================================================================================

  public static void trStart(Writer buffer, String className, String id) throws IOException {
    buffer.append("<tr");

    if (className != null)
      buffer.append(" class=\"").append(className).append("\"");

    if (id != null)
      buffer.append(" id=\"").append(id).append("\"");

    buffer.append(">\n");
  }

  public static void trEnd(Writer w) throws IOException {
    w.append("</tr>\n");
  }


  // TD ================================================================================================================

  public static void tdStart(Writer w, int width) throws IOException {
    tdStart(w, null, String.valueOf(width));
  }

  public static void tdStart(Writer w) throws IOException {
    tdStart(w, null, null);
  }

  public static void tdStart(Writer w, String width) throws IOException {
    tdStart(w, null, width);
  }

  public static void tdStart(Writer buffer, String className, String width) throws IOException {
    buffer.append("<td align=center");

    if (className != null)
      buffer.append(" class=\"").append(className).append("\"");
    if (width != null)
      buffer.append(" width=\"").append(width).append("\"");

    buffer.append(">");
  }

  public static void tdEnd(Writer w) throws IOException {
    w.append("</td>\n");
  }


  // a =================================================================================================================

  public static void aStart(Writer buffer, String href, String onClick) throws IOException {
    buffer.append("<a href=\"").append(href).append("\"");

    if (onClick != null)
      buffer.append(" onClick=\"").append(onClick).append("\"");

    buffer.append(">");

  }

  public static void aEnd(Writer w) throws IOException {
    w.append("</a>");
  }


  // select ============================================================================================================


  public static void select(Writer buffer, String className, String name, String id, List values, int defaultValueId, boolean readOnly, Validation validation) throws IOException {
    buffer.append("<select ");

    if (className != null)
      buffer.append(" class=\"").append(className).append("\"");

    if (name != null)
      buffer.append(" name=\"").append(name).append("\"");

    if (id != null)
      buffer.append(" id=\"").append(id).append("\"");

    if (validation != null)
      buffer.append(" validation=\"").append(validation.getValidation()).append("\"");

    if (readOnly)
      buffer.append(" readOnly");

    buffer.append(">");

    for (int i = 0; i < values.size(); i++)
      buffer.append("<option value=\"").append(String.valueOf(i)).append("\"").append(defaultValueId == i ? "SELECTED" : "").append(">").append(values.get(i).toString()).append("</option>");

    buffer.append("</select>");
  }


  // Local Message =====================================================================================================

//  public static String getLocalMessage(String key, Locale locale) {
//    try {
//      if (locale == null)
//        locale = new Locale(LocaleMessages.DEFAULT_PREFERRED_LANGUAGE);
//      ResourceBundle bundle = ResourceBundle.getBundle(LocaleMessages.SMSC_BUNDLE_NAME, locale);
//      return bundle.getString(key);
//    } catch (MissingResourceException e) {
//    }
//    return key;
//  }
}
