package ru.novosoft.smsc.jsp.smsc.localeResources;

import ru.novosoft.smsc.admin.resources.ResourcesManager;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.util.jsp.MultipartDataSource;
import ru.novosoft.util.jsp.MultipartServletRequest;

import javax.servlet.http.HttpServletRequest;
import java.util.*;

/**
 * Created by igork
 * Date: Feb 18, 2003
 * Time: 4:39:40 PM
 */
public class LocaleResourcesAdd extends PageBean
{
  private String mbCancel = null;;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    return RESULT_OK;
  }

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbCancel != null)
      return RESULT_DONE;

    MultipartServletRequest multi = (MultipartServletRequest)request.getAttribute("multipart.request");
    if (multi != null) {
      MultipartDataSource dataFile = null;
      try {
        dataFile = multi.getMultipartDataSource("resourceFile");
        if (dataFile == null)
          return error(SMSCErrors.error.localeResources.fileNotAttached);

        if (dataFile.getContentType().equals("text/xml")) {
          String name = dataFile.getName();
          int pos = name.lastIndexOf('/');
          if (pos >= 0)
            name = name.substring(pos + 1);
          pos = name.lastIndexOf('\\');
          if (pos >= 0)
            name = name.substring(pos + 1);

          if (!name.matches(ResourcesManager.RESOURCE_FILENAME_PATTERN))
            return error(SMSCErrors.error.localeResources.wrongFileName);

          final String localeName = name.substring(ResourcesManager.RESOURCE_FILENAME_PREFIX_LENGTH, ResourcesManager.RESOURCE_FILENAME_PREFIX_LENGTH + ResourcesManager.RESOURCE_FILENAME_BODY_LENGTH);
          appContext.getResourcesManager().add(localeName, dataFile.getInputStream());

          dataFile.close();
          dataFile = null;
          journalAppend(SubjectTypes.TYPE_locale, localeName, Actions.ACTION_ADD);
          return RESULT_DONE;
        } else
          return error(SMSCErrors.error.localeResources.wrongFileType);
      } catch (Throwable t) {
        logger.debug("Couldn't receive file", t);
        return error(SMSCErrors.error.localeResources.couldntReceiveFile, t);
      } finally {
        if (dataFile != null) {
          dataFile.close();
          dataFile = null;
        }
      }
    } else
      return RESULT_OK;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }
}
