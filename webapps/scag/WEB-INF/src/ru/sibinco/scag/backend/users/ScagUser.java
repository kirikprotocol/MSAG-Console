package ru.sibinco.scag.backend.users;

import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.Iterator;
import java.util.Map;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 20.12.2006
 * Time: 12:42:35
 * To change this template use File | Settings | File Templates.
 */
public class ScagUser extends User {
  private UserPreferences prefs = null;

  public ScagUser(final Element userElem) {
    super(userElem);
    NodeList prefsList = userElem.getElementsByTagName("pref");
    prefs = new UserPreferences(prefsList);
  }

  public ScagUser(final String login, final String password, final String[] roles, final String firstName, final String lastName, final String dept,
              final String workPhone, final String homePhone, final String cellPhone, final String email, final long providerId, final Map params) {
    super(login, password, roles, firstName, lastName, dept, workPhone, homePhone, cellPhone, email, providerId);
    prefs = new UserPreferences(params);
  }

  public UserPreferences getPrefs() {
    return prefs;
  }

  public String getXmlText()
  {
    String result = "\t<user";
    result += " login=\"" + StringEncoderDecoder.encode(getLogin()) + '"';
    result += " password=\"" + StringEncoderDecoder.encode(getPassword()) + '"';
    result += " firstName=\"" + StringEncoderDecoder.encode(getFirstName()) + '"';
    result += " lastName=\"" + StringEncoderDecoder.encode(getLastName()) + '"';
    result += " dept=\"" + StringEncoderDecoder.encode(getDept()) + '"';
    result += " workPhone=\"" + StringEncoderDecoder.encode(getWorkPhone()) + '"';
    result += " homePhone=\"" + StringEncoderDecoder.encode(getHomePhone()) + '"';
    result += " cellPhone=\"" + StringEncoderDecoder.encode(getCellPhone()) + '"';
    result += " email=\"" + StringEncoderDecoder.encode(getEmail()) + '"';
    result += " providerId=\"" + getProviderId() + '"';
    result += ">\n";
    for (Iterator i = getRoles().iterator(); i.hasNext();) {
      final String role = (String) i.next();
      result += "\t\t<role name=\"" + StringEncoderDecoder.encode(role) + "\"/>\n";
    }
    result += prefs.getXmlText();
    result += "\t</user>\n";

    return result;
  }

}
