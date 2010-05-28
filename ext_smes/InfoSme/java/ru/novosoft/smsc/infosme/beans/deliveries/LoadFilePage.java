package ru.novosoft.smsc.infosme.beans.deliveries;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.util.jsp.MultipartDataSource;
import ru.novosoft.util.jsp.MultipartServletRequest;

import javax.servlet.http.HttpServletRequest;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;

/**
 * User: artem
 * Date: 04.06.2008
 */

public class LoadFilePage extends DeliveriesPage {
  protected LoadFilePage(DeliveriesPageData pageData) {
    super(pageData);
    pageData.clear();
  }

  private DeliveriesPage loadFile(HttpServletRequest request) throws AdminException {

    // Check request is multipart
    final MultipartServletRequest multi = (MultipartServletRequest)request.getAttribute("multipart.request");
    if (multi == null)
      return new LoadFilePage(pageData);

    if (multi.getParameter("splitDeliveriesFile") != null)
      pageData.setSplitDeliveriesFile(true);
    else
      pageData.setSplitDeliveriesFile(false);

    if (multi.getParameter("useSameSettingsForAllRegions") != null)
      pageData.setUseSameSettingsForAllRegions(true);
    else
      pageData.setUseSameSettingsForAllRegions(false);

    // Receive and analize file
    MultipartDataSource dataFile = null;
    InputStream is = null;
    try {

      dataFile = multi.getMultipartDataSource(DeliveriesPageData.ABONENTS_FILE_PARAM);
      if (dataFile == null)
        throw new AdminException("infosme.error.file_not_attached");

      is = dataFile.getInputStream();

      final File deliveriesFile = Functions.saveFileToTemp(is, new File(WebAppFolders.getWorkFolder(), "INFO_SME_abonents.list"));
      deliveriesFile.deleteOnExit();

      pageData.setDeliveriesFile(deliveriesFile);

      return new ProcessFilePage(pageData);

    } catch (Throwable t) {
      throw new AdminException(t.getMessage());
    } finally {
      if (dataFile != null)
        dataFile.close();
      try {
        if (is != null)
          is.close();
      } catch (IOException e) {
        log.error(e,e);
      }
    }
  }

  public DeliveriesPage mbNext(HttpServletRequest request) throws AdminException {
    return loadFile(request);
  }

  public DeliveriesPage mbCancel(HttpServletRequest request) throws AdminException {
    return new LoadFilePage(pageData);
  }

  public DeliveriesPage mbUpdate(HttpServletRequest request) throws AdminException {
    return this;
  }

  public int getId() {
    return LOAD_FILE_PAGE;
  }
}
