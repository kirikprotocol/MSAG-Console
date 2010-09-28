package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.admin.AdminException;

import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

/**
 * @author Aleksandr Khalitov
 */
public class InfosmeException extends AdminException {

  private int[] errorsPerNode;
  private byte[] nodes;

  public InfosmeException(String key, Throwable cause) {
    super(key, cause);
  }

  public InfosmeException(String key) {
    super(key);
  }

  public InfosmeException(String key, String... args) {
    super(key, args);
  }

  protected InfosmeException(String key, int[] errorsPerNode, byte[] nodes) {
    super(key);
    this.errorsPerNode = errorsPerNode;
    this.nodes = nodes;
  }

  public String getMessage(Locale locale) {
    if (errorsPerNode == null)
      return super.getMessage(locale);

    ResourceBundle r = ResourceBundle.getBundle(InfosmeException.class.getName(), locale);
    String result = r.getString(key);
    result+=". ";
    result+= r.getString("error_nodes");
    result += ":\n";

    for (int i=0; i<errorsPerNode.length; i++) {
      if (errorsPerNode[i] != 0) {
        result += nodes[i];
        result += " : ";
        String errorDescription;
        try {
          errorDescription = r.getString("error_" + errorsPerNode[i]);
        } catch (MissingResourceException e) {
          errorDescription = r.getString("unknown_error") + " " + errorsPerNode[i];
        }
        result += errorDescription + "\n";
      }
    }
    return result;
  }  
}
