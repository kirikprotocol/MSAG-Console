package ru.novosoft.smsc.admin.cluster_controller;

import org.junit.Test;

import java.util.Locale;

/**
 * @author Artem Snopkov
 */
public class ClusterControllerExceptionTest {
  @Test
  public void testGetMessage() throws Exception {
    ClusterControllerException ex = new ClusterControllerException("interaction_error", new int[] {0,1,2}, new byte[] {3,2,1});
    System.out.println(ex.getMessage(new Locale("en")));
    System.out.println(ex.getMessage(new Locale("ru")));

  }
}
