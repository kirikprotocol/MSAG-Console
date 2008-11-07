<%@ page import="ru.novosoft.smsc.infosme.beans.InfoSmeBean,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.PageBean"%>
<%@ page import="mobi.eyeline.smsquiz.beans.SmsQuizBean"%>
<%
  boolean defaultError = false;
  switch(beanResult)
  {
    case SmsQuizBean.RESULT_OK:
    case SmsQuizBean.RESULT_ERROR:
      break;
    case SmsQuizBean.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case SmsQuizBean.RESULT_OPTIONS:
      response.sendRedirect("options.jsp");
      return;
    case SmsQuizBean.RESULT_RESULTS:
      response.sendRedirect("results.jsp");
      return;
    case SmsQuizBean.RESULT_REPLYSTATS:
      response.sendRedirect("reply_stats.jsp");
      return;
    case SmsQuizBean.RESULT_QUIZES:
      response.sendRedirect("quizes.jsp");
      return;
    default:
      defaultError = true;
  }
%>