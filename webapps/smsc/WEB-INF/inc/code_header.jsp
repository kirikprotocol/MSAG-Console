<%@ page import="java.util.*,
                 java.net.URLEncoder,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.admin.Constants"
%>
<%@ page import="java.io.IOException"%>
<%@
page pageEncoding="windows-1251"%><%@
page contentType="text/html; charset=windows-1251"%><%@
page buffer="64kb"%><%!
private String TITLE = getLocString("common.mainTitle");
private String BROWSER_TITLE = null;
private String FORM_METHOD = "POST";
private String FORM_URI = "";
private String MENU0_SELECTION = "MENU0_NONE";
private String FORM_ENCTYPE = "application/x-www-form-urlencoded";
private String CPATH = "/smsc";
private String ServiceIDForShowStatus = null;
private boolean isServiceStatusColored = false;
private boolean isServiceStatusNeeded = false;
private java.util.List errorMessages = new ArrayList();
private final int BROWSER_TYPE_UNKNOWN=0;
private final int BROWSER_TYPE_MSIE=1;
private final int BROWSER_TYPE_MOZILLA=2;
private final int BROWSER_TYPE_SAFARI=3;
private       int browserType = -1;
public int getBrowserType(HttpServletRequest request) {
  String _browserType = request.getHeader("User-Agent");
  if( _browserType.indexOf("Safari") != -1 ) return BROWSER_TYPE_SAFARI;
  else if( _browserType.indexOf("MSIE") != -1 ) {
    return BROWSER_TYPE_MSIE;
  } else if( _browserType.indexOf("Mozilla/") != -1 ) return BROWSER_TYPE_MOZILLA;
  else return BROWSER_TYPE_UNKNOWN;
}

public String makeHref(String action) throws IOException {
  if( browserType == BROWSER_TYPE_MSIE ) {
    if( action.indexOf("return") != -1 ) {
      return " href=\"#\" onClick=\""+action+"\" ";
    } else {
      return " href=\"#\" onClick=\"return "+action+"\" ";
    }
  } else {
    return " href=\"javascript:"+action+"\" ";
  }
}


public boolean isBrowserMSIE(HttpServletRequest request) {
  return browserType == BROWSER_TYPE_MSIE;
}
%><%@include file="/WEB-INF/inc/localization.jsp"%><%
browserType = getBrowserType(request);
errorMessages.clear();
request.setAttribute(Constants.SMSC_ERROR_MESSAGES_ATTRIBUTE_NAME, errorMessages);
FORM_URI = (String)request.getAttribute("requestURI");
if (FORM_URI == null)
	FORM_URI = request.getRequestURI();
CPATH = request.getContextPath() + "/smsc";
{
%>