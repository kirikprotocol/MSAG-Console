<%@ include file="/WEB-INF/inc/code_header.jsp"%>
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
<%  List mscList = bean.getMscs();
    if (mscList == null || mscList.size() <= 0) {%>
<tr class=row0>
    <td colspan=4><div align=left>No Mobile Switching Centers defined</div><td>
</tr>
<%  } else {
        int posIdx = 0;%>
<tr class=row<%= (posIdx++)%2%>>
    <th width="20%"><div align=right>Commutator</div></th>
    <th width="25%"><div align=right>Status</div></th>
    <th width="10%"><div align=right>Failures</div></th>
    <th width="45%"><div align=right>Actions</div></th>
</tr><%
        for (int i=0; i<mscList.size(); i++)
        {
            Object obj = mscList.get(i);
            if (obj != null && obj instanceof MscInfo) {
                MscInfo info = (MscInfo)obj; %>
<tr class=row<%= (posIdx++)%2%>>
    <td><div align=right><%= StringEncoderDecoder.encode(info.getMscNum())%></div></td>
    <td><div align=right><%= StringEncoderDecoder.encode(info.getLockString())%></div></td>
    <td><div align=right><%= info.getfCount()%></div></td>
    <td>
    <div align=right>
        <input class=btn type="submit" name="mbBlock" value="Lock" onclick="this.form.mscKey.value='<%= info.getMscNum()%>'" <%= (info.ismLock()) ? "disabled":""%>>
        <input class=btn type="submit" name="mbClear" value="UnLock" onclick="this.form.mscKey.value='<%= info.getMscNum()%>'" <%= (info.isaLock() || info.ismLock()) ? "":"disabled"%>>
        <input class=btn type="submit" name="mbUnregister" value="UnRegister" onclick="this.form.mscKey.value='<%= info.getMscNum()%>'">
    </div>
    </td>
</tr><%     }
        }
    }%>
</table>


<div class=page_subtitle>Register new Mobile Switching Center</div>
<table class=list cellspacing=1 width="100%">
<tr class=row0>
     <td width="55%">
        <input type="hidden" name="mscKey" value="none">
        <input class=txt type="text" name="mscNum" value="<%=bean.getMscNum()%>" size=21 maxlength=21>
    </td>
<td width="45%"><input class=btn type="submit" name="mbRegister" value="Register"></td>
</tr>
</table>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

