/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: Nov 28, 2002
 * Time: 7:01:17 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.users;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;
import ru.novosoft.smsc.jsp.util.tables.DataSource;
import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.user.UserDataItem;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.SortedList;
import ru.novosoft.smsc.util.auth.XmlAuthenticator;
import ru.novosoft.smsc.util.xml.Utils;

import javax.xml.parsers.ParserConfigurationException;
import java.io.*;
import java.util.*;
import java.security.Principal;

public class UserManager implements DataSource
{
	private File configFile = null;
	private Map users = new HashMap();

	private Principal loginedPrincipal = null;
	private User loginedUser = null;

  public UserManager(File configFile)
      throws Exception, SAXException, IOException
  {
    this.configFile = configFile;
    load(this.configFile);
    XmlAuthenticator.init(configFile);
  }

  public synchronized QueryResultSet query(Query query_to_run)
  {
    String sort = (String) query_to_run.getSortOrder().get(0);
    boolean isNegativeSort = false;
    if (sort.charAt(0) == '-') {
      sort = sort.substring(1);
      isNegativeSort = true;
    }

    QueryResultSetImpl result = new QueryResultSetImpl(User.columnNames, query_to_run.getSortOrder());

    List filteredUsers = filterUsers(query_to_run.getFilter());
    sortUsers(sort, isNegativeSort, filteredUsers);
    result.setLast(true);
    int lastpos = Math.min(query_to_run.getStartPosition() + query_to_run.getExpectedResultsQuantity(), filteredUsers.size());
    for (int i = query_to_run.getStartPosition(); i < lastpos; i++) {
      result.add((UserDataItem) filteredUsers.get(i));
      result.setLast(i == (filteredUsers.size() - 1));
    }
    result.setTotalSize(filteredUsers.size());

    return result;
  }

  private void sortUsers(final String sort, final boolean isNegativeSort, List filteredUsers)
  {
    Collections.sort(filteredUsers, new Comparator()
    {
      public int compare(Object o1, Object o2)
      {
        if (o1 instanceof UserDataItem && o2 instanceof UserDataItem) {
          UserDataItem u1 = (UserDataItem) o1;
          UserDataItem u2 = (UserDataItem) o2;
          int result = ((String) u1.getValue(sort)).compareToIgnoreCase((String) u2.getValue(sort));
          return isNegativeSort
                 ? -result
                 : result;
        } else
          return 0;
      }
    });
  }

  private List filterUsers(Filter filter)
  {
    List result = new LinkedList();
    for (Iterator i = users.values().iterator(); i.hasNext();) {
      User user = (User) i.next();
      UserDataItem userItem = new UserDataItem(user);
      if (filter.isItemAllowed(userItem))
        result.add(userItem);
    }
    return result;
  }

  protected void store()
      throws IOException
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

  protected void load(File file)
      throws IOException, SAXException, ParserConfigurationException
  {
    Document document = Utils.parse(new FileReader(file));
    NodeList userNodes = document.getElementsByTagName("user");
    for (int i = 0; i < userNodes.getLength(); i++) {
      User user = new User((Element) userNodes.item(i));
      users.put(user.getLogin(), user);
    }
  }

  public synchronized User getUser(String login)
  {
    return (User) users.get(login);
  }

  public synchronized boolean addUser(User user)
  {
    if (!users.containsKey(user.getLogin())) {
      users.put(user.getLogin(), user);
      return true;
    } else
      return false;
  }

  public synchronized void apply()
      throws Exception
  {
    store();
    XmlAuthenticator.init(configFile);
  }

  public User removeUser(String userLogin)
  {
    return (User) users.remove(userLogin);
  }

  public void removeRole(String roleName)
  {
    for (Iterator i = users.values().iterator(); i.hasNext();) {
      User user = (User) i.next();
      user.revokeRole(roleName);
    }
  }
	public Principal getLoginedPrincipal ()
	{
		return loginedPrincipal;
	}

	public User getLoginedUser()
	{
		return loginedUser;
	}

	public void setLoginedPrincipal (Principal principal)
	{
		if (principal != null)
			{
			Object o = users.get(principal.getName());
			if (o != null)
				{
				loginedPrincipal = principal;
				loginedUser = (User) o;
				}
			}
		else
		{
			loginedPrincipal = null;
			loginedUser = null;
		}
	}
}
