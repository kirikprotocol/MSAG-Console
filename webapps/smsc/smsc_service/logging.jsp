<%@
 include file="/WEB-INF/inc/code_header.jsp"%><%@
 page import="ru.novosoft.smsc.jsp.smsc.smsc_service.Index,
				  ru.novosoft.smsc.admin.service.ServiceInfo,
				  ru.novosoft.smsc.admin.Constants,
				  java.io.IOException,
				  ru.novosoft.smsc.util.StringEncoderDecoder,
				  java.util.Iterator,
				  ru.novosoft.smsc.jsp.SMSCErrors,
				  ru.novosoft.smsc.jsp.SMSCJspException,
              ru.novosoft.smsc.jsp.smsc.smsc_service.Logging,
              java.util.Collection,
              java.util.Arrays"
%><jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.smsc_service.Logging"
/><jsp:setProperty name="bean" property="*"/><%
  //todo ??????? ??????
  FORM_METHOD = "POST";
	TITLE = "SMSC";
	MENU0_SELECTION = "MENU0_SMSC_LOGGING";
	switch (bean.process(appContext, errorMessages, loginedUserPrincipal, request.getParameterMap()))
	{
		case Index.RESULT_DONE:
			response.sendRedirect(CPATH+"/index.jsp");
			return;
		case Index.RESULT_OK:
			STATUS.append("Ok");
			break;
		case Index.RESULT_ERROR:
			STATUS.append("<span class=CF00>Error</span>");
			break;
		default:
			STATUS.append("<span class=CF00>Error</span>");
			errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
	}
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/collapsing_tree.jsp"%>
<%
  page_menu_begin(out);
  page_menu_button(out, "mbSave",  "Save",  "Save user info");
  page_menu_button(out, "mbCancel", "Cancel", "Cancel user editing", "clickCancel()");
  page_menu_space(out);
  page_menu_end(out);
%><div class=content><%!
  private static final String[] PRIORITIES = {"FATAL", "ALERT", "CRIT", "ERROR", "WARN", "NOTICE", "INFO", "DEBUG", "NOTSET"};

  private static final StringBuffer createOption(StringBuffer result, final String priority, final String catPriority)
  {
    return result.append("<option value=\"").append(priority).append('"').append(priority.equals(catPriority) ? " selected>" : ">").append(priority).append("</option>");
  }
  private static final String createSelection(String prefix, String catName, String catPriority)
  {
    StringBuffer result = new StringBuffer();
    result.append("<select name=\"").append(prefix).append(catName).append("\" style=\"font-size:80%;\" onClick=\"window.event.cancelBubble = true;\">");
    for (int i = 0; i < PRIORITIES.length; i++) {
      createOption(result, PRIORITIES[i], catPriority);
    }
    return result.append("</select>").toString();
  }

  private final void printCategory(JspWriter out, Logging.LoggerCategoryInfo category) throws IOException
  {
    startSection(out, category.getFullName(), category.getName(), category.isRoot(), createSelection(Logging.catParamNamePrefix, category.getFullName(), category.getPriority()));
    {
      { // print childs that has not subchilds
        boolean paramsStarted = false;
        for (Iterator i = category.getChilds().values().iterator(); i.hasNext();) {
          Logging.LoggerCategoryInfo child = (Logging.LoggerCategoryInfo) i.next();
          if (!child.hasChilds())
          {
            if (!paramsStarted) {
              startParams(out);
              paramsStarted = true;
            }
            paramSelect(out, child.getName(), Logging.catParamNamePrefix + child.getFullName(), Arrays.asList(PRIORITIES), child.getPriority(), null, "font-size:80%;");
          }
        }
        if (paramsStarted)
          finishParams(out);
      }
      { // print childs with subchilds
        for (Iterator i = category.getChilds().values().iterator(); i.hasNext();) {
          Logging.LoggerCategoryInfo child = (Logging.LoggerCategoryInfo) i.next();
          if (child.hasChilds())
            printCategory(out, child);
        }
      }
    }
    finishSection(out);
  }
%><%
  printCategory(out, bean.getRootCategory());
%></div><%
page_menu_begin(out);
page_menu_button(out, "mbSave",  "Save",  "Save user info");
page_menu_button(out, "mbCancel", "Cancel", "Cancel user editing", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><%@
 include file="/WEB-INF/inc/html_3_footer.jsp"%><%@
 include file="/WEB-INF/inc/code_footer.jsp"%>