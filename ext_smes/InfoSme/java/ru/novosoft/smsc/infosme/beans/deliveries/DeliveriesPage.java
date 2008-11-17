package ru.novosoft.smsc.infosme.beans.deliveries;

import ru.novosoft.smsc.infosme.beans.InfoSmeBean;
import ru.novosoft.smsc.admin.AdminException;

import javax.servlet.http.HttpServletRequest;

import org.apache.log4j.Category;

/**
 * User: artem
 * Date: 04.06.2008
 */

public abstract class DeliveriesPage  {

  protected static final Category log = Category.getInstance(DeliveriesPage.class);

  public static final int LOAD_FILE_PAGE = 1001;
  public static final int PROCESS_FILE_PAGE = 1002;
  public static final int EDIT_TASK_PAGE = 1003;
  public static final int GEN_TASK_PAGE = 1004;
  public static final int FINISH_TASK_PAGE = 1005;
  public static final int START_PAGE = 1006;

  protected final DeliveriesPageData pageData;

  protected DeliveriesPage(DeliveriesPageData pageData) {
    this.pageData = pageData;
  }

  public abstract DeliveriesPage mbNext(HttpServletRequest request) throws AdminException;
  public abstract DeliveriesPage mbCancel(HttpServletRequest request) throws AdminException;
  public abstract DeliveriesPage mbUpdate(HttpServletRequest request) throws AdminException;

  public abstract int getId();
}
