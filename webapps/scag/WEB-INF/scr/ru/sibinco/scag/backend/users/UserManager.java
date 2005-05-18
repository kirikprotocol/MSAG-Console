package ru.sibinco.scag.backend.users;

import org.w3c.dom.*;
import org.xml.sax.SAXException;
import ru.sibinco.lib.backend.users.User;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.tomcat_auth.XmlAuthenticator;

import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;


/**
 * Created by igork Date: 03.03.2004 Time: 19:20:19
 */
public class UserManager
{
  private Map users = Collections.synchronizedMap(new HashMap());
  private final File configFile;

  public UserManager(String config) throws IOException, ParserConfigurationException, SAXException
  {
    this.configFile = new File(config);
    load();
  }

  private void load() throws IOException, ParserConfigurationException, SAXException
  {
    Document document = Utils.parse(new FileReader(configFile));
    NodeList userNodes = document.getElementsByTagName("user");
    for (int i = 0; i < userNodes.getLength(); i++) {
      User user = new User((Element) userNodes.item(i));
      users.put(user.getLogin(), user);
    }
  }

  public synchronized void apply() throws IOException, ParserConfigurationException, SAXException
  {
    store();
    XmlAuthenticator.init(configFile);
  }

  protected void store() throws IOException
  {
    File configNew = Functions.createNewFilenameForSave(configFile);

    PrintWriter out = new PrintWriter(new OutputStreamWriter(new FileOutputStream(configNew), Functions.getLocaleEncoding()));
    Functions.storeConfigHeader(out, "users", "users.dtd", System.getProperty("file.encoding"));
    for (Iterator i = new SortedList(users.keySet()).iterator(); i.hasNext();) {
      String userLogin = (String) i.next();
      User user = (User) users.get(userLogin);
      out.print(user.getXmlText());
    }
    Functions.storeConfigFooter(out, "users");
    out.flush();
    out.close();

    Functions.renameNewSavedFileToOriginal(configNew, configFile);
  }

  public synchronized Map getUsers()
  {
    return users;
  }
}
