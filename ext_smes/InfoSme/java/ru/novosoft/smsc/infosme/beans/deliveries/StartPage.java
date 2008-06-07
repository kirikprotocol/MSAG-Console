package ru.novosoft.smsc.infosme.beans.deliveries;

import javax.servlet.http.HttpServletRequest;

/**
 * User: artem
 * Date: 04.06.2008
 */

public class StartPage extends DeliveriesPage {

  public StartPage(DeliveriesPageData pageData) {
    super(pageData);
    pageData.clear();
  }  

  public DeliveriesPage mbNext(HttpServletRequest request) {
    pageData.clear();
    return new LoadFilePage(pageData);
  }

  public DeliveriesPage mbCancel(HttpServletRequest request) {
    return this;
  }

  public DeliveriesPage mbUpdate(HttpServletRequest request) {
    return this;
  }

  public int getId() {
    return START_PAGE;
  }
}
