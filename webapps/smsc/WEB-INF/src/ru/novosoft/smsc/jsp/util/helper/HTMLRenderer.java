package ru.novosoft.smsc.jsp.util.helper;

import ru.novosoft.smsc.util.LocaleMessages;

import java.util.ResourceBundle;
import java.util.Locale;
import java.util.MissingResourceException;

/**
 * User: artem
 * Date: 17.01.2007
 */

public class HTMLRenderer {

  // Input =============================================================================================================

  protected static String textInput(String className, String name, String id, String value, boolean readOnly, int maxLength, String size, Validation validation, String additional) {
    return input("txt", className, name, id, value, readOnly, false, maxLength, size, null, validation, additional);
  }

  protected static String check(String name, boolean checked) {
    return input("checkBox", null, name, name, null, false, checked, -1, null, null, null, null);
  }

  private static String input(String type, String className, String name, String id, String value, boolean readOnly, boolean checked, int maxLength, String size, String src, Validation validation, String additional) {
    final StringBuffer buffer = new StringBuffer();
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
      buffer.append(" maxLength=\"").append(maxLength).append("\"");

    if (size !=null)
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

    return buffer.toString();
  }


  // Image =============================================================================================================

  protected static String image(String src, String onClick, String title) {
    return image(src, onClick, title, null);
  }

  protected static String image(String src, String onClick, String title, String className) {
    final StringBuffer buffer = new StringBuffer();
    buffer.append("<img src=\"").append(src).append("\"");

    if (onClick != null)
      buffer.append(" onclick=\"").append(onClick).append("\"");

    if (title != null)
      buffer.append(" title=\"").append(title).append("\"");

    if (className != null)
      buffer.append(" class=\"").append(className).append("\"");

    buffer.append(">");

    return buffer.toString();
  }


  // TR ================================================================================================================

  protected static String trStart(String className, String id) {
    final StringBuffer buffer = new StringBuffer();
    buffer.append("<tr");

    if (className != null)
      buffer.append(" class=\"").append(className).append("\"");

    if (id != null)
      buffer.append(" id=\"").append(id).append("\"");

    buffer.append(">\n");

    return buffer.toString();
  }

  protected static String trEnd() {
    return "</tr>\n";
  }


  // TD ================================================================================================================

  protected static String tdStart(int width) {
    return tdStart(null, String.valueOf(width));
  }

  protected static String tdStart() {
    return tdStart(null, null);
  }

  protected static String tdStart(String width) {
    return tdStart(null, width);
  }

  protected static String tdStart(String className, String width) {
    final StringBuffer buffer = new StringBuffer();
    buffer.append("<td align=center");

    if (className != null)
      buffer.append(" class=\"").append(className).append("\"");
    if (width != null)
      buffer.append(" width=\"").append(width).append("\"");

    buffer.append(">");

    return buffer.toString();
  }

  protected static String tdEnd() {
    return "</td>\n";
  }


  // a =================================================================================================================

  protected static String aStart(String href, String onClick) {
    final StringBuffer buffer = new StringBuffer();
    buffer.append("<a href=\"").append(href).append("\"");

    if (onClick != null)
      buffer.append(" onClick=\"").append(onClick).append("\"");

    buffer.append(">");

    return buffer.toString();
  }

  protected static String aEnd() {
    return "</a>";
  }


  // select ============================================================================================================


  protected static String select(String className, String name, String id, String[] values, int defaultValueId, boolean readOnly, Validation validation) {
    final StringBuffer buffer = new StringBuffer();
    buffer.append("<select ");

    if (className != null)
      buffer.append(" class=\"").append(className).append("\"");

    if (name != null)
      buffer.append(" name=\"").append(name).append("\"");

    if (id != null)
      buffer.append(" id=\"").append(id).append("\"");

    if (validation != null)
      buffer.append(" validation=\"").append(validation.getValidation()).append("\"");

    System.out.println("ReadOnly: " + readOnly);
    if (readOnly)
      buffer.append(" readOnly");

    buffer.append(">");

    for (int i = 0; i< values.length; i++)
      buffer.append("<option value=\"").append(i).append("\"").append(defaultValueId == i ? "SELECTED" : "").append(">").append(values[i]).append("</option>");

    buffer.append("</select>");
    return buffer.toString();
  }


  // Local Message =====================================================================================================

  protected static String getLocalMessage(String key) {
    try {
      ResourceBundle bundle = ResourceBundle.getBundle(LocaleMessages.SMSC_BUNDLE_NAME, new Locale(LocaleMessages.DEFAULT_PREFERRED_LANGUAGE));
      return bundle.getString(key);
    } catch (MissingResourceException e) {
    }
    return key;
  }
}
