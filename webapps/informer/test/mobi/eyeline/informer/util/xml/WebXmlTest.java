package mobi.eyeline.informer.util.xml;

import org.junit.BeforeClass;
import org.junit.Test;
import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.TransformerConfigurationException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Collection;
import java.util.Iterator;
import java.util.Set;

import static org.junit.Assert.assertEquals;

/**
 * @author Aleksandr Khalitov
 */

public class WebXmlTest {

  private static WebXml webXml;

  @BeforeClass
  public static void init() throws IOException, TransformerConfigurationException, SAXException, ParserConfigurationException {
    InputStream is = null;
    try{
      is = WebXmlTest.class.getResourceAsStream("web.xml");
      webXml = new WebXml(is);
    }finally {
      if(is != null) {
        try{
          is.close();
        }catch (IOException e){}
      }
    }
  }

  @Test
  public void rolesList() {
    Set<String> roles = webXml.getRoles();
    assertEquals(roles.size(), 2);
    Iterator<String> i = roles.iterator();
    while(i.hasNext()) {
      String n = i.next();
      if(n.equals("admin") || n.equals("user")) {
        i.remove();
      }
    }
    assertEquals(roles.size(), 0);
  }

  @Test
  public void rolesByFullUrl() {
    Collection<String> roles = webXml.getRoles("/index.faces");
    assertEquals(roles.size(), 2);
    Iterator<String> i = roles.iterator();
    while(i.hasNext()) {
      String n = i.next();
      if(n.equals("admin") || n.equals("user")) {
        i.remove();
      }
    }
    assertEquals(roles.size(), 0);
  }

  @Test
  public void rolesByUserContext() {
    Collection<String> roles = webXml.getRoles("/user/edit.faces");
    assertEquals(roles.size(), 2);
    Iterator<String> i = roles.iterator();
    while(i.hasNext()) {
      String n = i.next();
      if(n.equals("admin") || n.equals("user")) {
        i.remove();
      }
    }
    assertEquals(roles.size(), 0);
  }

  @Test
  public void rolesByAdminContext2() {
    Collection<String> roles = webXml.getRoles("/admin/edit.faces");
    assertEquals(roles.size(), 1);
    Iterator<String> i = roles.iterator();
    while(i.hasNext()) {
      String n = i.next();
      if(n.equals("admin")) {
        i.remove();
      }
    }
    assertEquals(roles.size(), 0);
  }

  @Test
  public void rolesByAdminFullUrl() {
    Collection<String> roles = webXml.getRoles("/user/admin1.jsp");
    assertEquals(roles.size(), 1);
    Iterator<String> i = roles.iterator();
    while(i.hasNext()) {
      String n = i.next();
      if(n.equals("admin")) {
        i.remove();
      }
    }
    assertEquals(roles.size(), 0);
  }

  @Test
  public void rolesByExt() {
    Collection<String> roles = webXml.getRoles("/user/edit.xhtml");
    assertEquals(roles.size(), 2);
    Iterator<String> i = roles.iterator();
    while(i.hasNext()) {
      String n = i.next();
      if(n.equals("admin") || n.equals("user")) {
        i.remove();
      }
    }
    assertEquals(roles.size(), 0);
  }

  @Test
  public void rolesByAdminExt() {
    Collection<String> roles = webXml.getRoles("/req/edit.xhtml");
    assertEquals(roles.size(), 1);
    Iterator<String> i = roles.iterator();
    while(i.hasNext()) {
      String n = i.next();
      if(n.equals("admin")) {
        i.remove();
      }
    }
    assertEquals(roles.size(), 0);
  }

}
