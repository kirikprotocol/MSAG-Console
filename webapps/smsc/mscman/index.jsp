<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ include file="/WEB-INF/inc/buttons.jsp"%>
<%@ page import="java.util.*,
                 ru.novosoft.smsc.jsp.SMSCAppContext,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%>
<%@ page import="ru.novosoft.smsc.admin.mscman.*"%>
<%@ page import="ru.novosoft.smsc.jsp.mscman.*"%>
<jsp:useBean id="mscManagerFormBean" scope="session" class="ru.novosoft.smsc.jsp.mscman.MscManagerFormBean" />
<%
	MscManagerFormBean bean = mscManagerFormBean;
%>
<jsp:setProperty name="mscManagerFormBean" property="*"/>
<%
    TITLE="Mobile Switching Centers lock status";
    MENU0_SELECTION = "MENU0_MSCMAN";

    int beanResult = mscManagerFormBean.RESULT_OK;
    switch(beanResult = bean.process(appContext, errorMessages, loginedUserPrincipal))
    {
        case MscManagerFormBean.RESULT_DONE:
            response.sendRedirect("index.jsp");
            return;
        case MscManagerFormBean.RESULT_FILTER:
        case MscManagerFormBean.RESULT_OK:
            STATUS.append("Ok");
            break;
        case MscManagerFormBean.RESULT_ERROR:
            STATUS.append("<span class=CF00>Error</span>");
            break;
        default:
            STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
            errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class=content>
<div class=page_subtitle>Commutators status</div>
<table class=list cellspacing=0>
<col width=20%>
<col width=25%>
<col width=10%>
<col width=45%>
<%  List mscList = bean.getMscs();
    if (mscList == null || mscList.size() <= 0) {%>
<tr class=row0>
    <td colspan=4>No Mobile Switching Centers defined<td>
</tr>
<%  } else {
        int posIdx = 0;%>
<tr class=row<%= (posIdx++)%2%>>
    <th>Commutator</th>
    <th>Status</th>
    <th>Failures</th>
    <th>Actions</th>
</tr><%
        for (int i=0; i<mscList.size(); i++)
        {
            Object obj = mscList.get(i);
            if (obj != null && obj instanceof MscInfo) {
                MscInfo info = (MscInfo)obj; %>
<tr class=row<%= (posIdx++)%2%>>
    <td><%= StringEncoderDecoder.encode(info.getMscNum())%></td>
    <td align=center><%= StringEncoderDecoder.encode(info.getLockString())%></td>
    <td align=center><%= info.getfCount()%></td>
    <td align=right>
		<%button(out, !info.ismLock() ? "but_lock.gif" : "but_lock_dis.gif",
					"mbBlock",      "Lock",       "Lock this MSC",       "opForm.mscKey.value='" + info.getMscNum() + "'");%>
		<%button(out, info.isaLock() || info.ismLock() ? "but_unlock.gif" : "but_unlock_dis.gif",
					"mbClear",      "UnLock",     "UnLock this MSC",     "opForm.mscKey.value='" + info.getMscNum() + "'");%>
		<%button(out, "but_unregister.gif", 
					"mbUnregister", "UnRegister", "Unregister this MSC", "opForm.mscKey.value='" + info.getMscNum() + "'");%>
    </td>
</tr><%     }
        }
    }%>
</table>


<div class=page_subtitle>Register new Mobile Switching Center</div>
<table class=list cellspacing=1 width="100%">
<tr class=row0>
     <td width="1%">
        <input type="hidden" name="mscKey" value="none">
        <input class=txt type="text" name="mscNum" value="<%=bean.getMscNum()%>" size=21 maxlength=21>
    </td>
<td width="1%"><%button(out, "but_register.gif", "mbRegister", "Register", "Register new MSC");%></td>
<td width="98%">&nbsp;</td>
</tr>
</table>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

