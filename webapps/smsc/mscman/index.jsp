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
    TITLE="Commutators lock status";
    MENU0_SELECTION = "MENU0_SMSSTAT";

    int beanResult = mscManagerFormBean.RESULT_OK;
    switch(beanResult = bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
    {
        case mscManagerFormBean.RESULT_DONE:
            response.sendRedirect("index.jsp");
            return;
        case mscManagerFormBean.RESULT_FILTER:
        case mscManagerFormBean.RESULT_OK:
            STATUS.append("Ok");
            break;
        case mscManagerFormBean.RESULT_ERROR:
            STATUS.append("<span class=CF00>Error</span>");
            break;
        default:
            STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
            errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
    }
%>
<%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
    List mscList = bean.getMscs();
    if (mscList.size() <= 0) {
    %><div align=left>No commutators defined.</div><%
    } else { int posIdx = 0;%>
<table class=secRep cellspacing=1 width="100%">
<tr>
    <td colspan=4><div class=secView>Commutators status</div></td>
</tr>
<tr class=row<%= (posIdx++)%2%>>
    <th width="20%"><div align=right>Commutator</div></th>
    <th width="25%"><div align=right>Status</div></th>
    <th width="10%"><div align=right>Failures</div></th>
    <th width="45%"><div align=right>Action</div></th>
</tr><%
        for (int i=0; i<mscList.size(); i++)
        {
            Object obj = mscList.get(i);
            if (obj != null && obj instanceof MscInfo) {
                MscInfo info = (MscInfo)obj; %>
<tr class=row<%= (posIdx++)%2%>>
    <td><div align=right><%= info.getMscNum()%></div></td>
    <td><div align=right><%= info.getLockString()%></div></td>
    <td><div align=right><%= info.getfCount()%></div></td>
    <td>
    <div class=secButtons align=right>
        <input class=btn type="submit" name="mbBlock" value="Lock" onclick="document.forms[0].mscNum.value='<%= info.getMscNum()%>'">
        <input class=btn type="submit" name="mbClear" value="UnLock" onclick="document.forms[0].mscNum.value='<%= info.getMscNum()%>'">
        <input class=btn type="submit" name="mbUnregister" value="UnRegister" onclick="document.forms[0].mscNum.value='<%= info.getMscNum()%>'">
    </div>
    </td>
</tr><%     }
        }%>
</table>
<%  }%>

<div class=secQuestion>Register new commutator</div>
<table class=secRep cellspacing=1 width="100%">
<tr class=row0>
     <td width="55%">
        <input class=txtW type="text" id="mscNumId" name="mscNum" value="<%=bean.getMscNum()%>" size=21 maxlength=21>
    </td>
<td width="45%"><div class=secButtons>
        <input class=btn type="submit" name="mbRegister" value="Ok">
    </div></td>
</tr>
</table>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

