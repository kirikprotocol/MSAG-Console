<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.admin.dl.*,
                 ru.novosoft.smsc.jsp.smsview.SmsViewFormBean,
                 ru.novosoft.smsc.jsp.smsc.IndexBean"%>
<%@ page import="ru.novosoft.smsc.jsp.dl.*"%>
<jsp:useBean id="dlBean" scope="page" class="ru.novosoft.smsc.jsp.dl.DistributionListAdminFormBean" />
<%
    DistributionListAdminFormBean bean = dlBean;
%>
<jsp:setProperty name="dlBean" property="*"/>
<%
    TITLE="Distribution lists";
    //MENU0_SELECTION = "MENU0_DLSET";

    int beanResult = IndexBean.RESULT_OK;
    switch(beanResult = bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
    {
        case IndexBean.RESULT_DONE:
            response.sendRedirect("index.jsp");
            return;
        case IndexBean.RESULT_FILTER:
        case IndexBean.RESULT_OK:
            STATUS.append("Ok");
            break;
        case IndexBean.RESULT_ERROR:
            STATUS.append("<span class=CF00>Error</span>");
            break;
        default:
            STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
            errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
    <h1>Not implemented yet.</h1>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>


