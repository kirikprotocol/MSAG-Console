<%@ tag body-content="scriptless"%>
</div>
<jsp:doBody/>
<%
    boolean isNotNull = false;

    if(session.getAttribute("BROWSER_TYPE") != null){
        isNotNull = true;
    }

    if( isNotNull && session.getAttribute("BROWSER_TYPE").equals("IE") ){
//        if(session.getAttribute("BROWSER_TYPE").equals("IE")){
%>
            <div class=content>
<%
    } else {
%>
            <div class=contentFF>
<%
    }
%>