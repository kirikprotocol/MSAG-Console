/*
 * Created by igork
 * Date: Mar 20, 2002
 * Time: 5:07:12 PM
 */
package ru.novosoft.smsc.util.xml;

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

public class Utils
{
  static public String getNodeText(final Node node)
  {
    String result = "";
    NodeList list = node.getChildNodes();
    for (int i = 0; i < list.getLength(); i++) {
      if (list.item(i).getNodeType() == Node.TEXT_NODE)
        result += list.item(i).getNodeValue();
    }
    return result;
  }
}
