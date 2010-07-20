package ru.novosoft.smsc.admin.cluster_controller;

import ru.novosoft.smsc.admin.AdminException;

import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

/**
 * @author Artem Snopkov
 */
public class ClusterControllerException extends AdminException {

  private int[] errorsPerNode;
  private byte[] nodes;

  protected ClusterControllerException(String key, Throwable cause) {
    super(key, cause);
  }

  protected ClusterControllerException(String key) {
    super(key);
  }

  protected ClusterControllerException(String key, String causeMessage) {
    super(key, causeMessage);
  }

  protected ClusterControllerException(String key, int[] errorsPerNode, byte[] nodes) {
    super(key);
    this.errorsPerNode = errorsPerNode;
    this.nodes = nodes;
  }

  public String getMessage(Locale locale) {
    if (errorsPerNode == null)
      return super.getMessage(locale);

    ResourceBundle r = ResourceBundle.getBundle(ClusterControllerException.class.getName(), locale);
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
