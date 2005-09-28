<%@ include file="/WEB-INF/inc/header.jspf" %>

<sm:page>
    <jsp:attribute name="title">Create new rule
    </jsp:attribute>
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbNext" value="Next>>>" title="Create rule"/>
            <sm-pm:item name="mbCancel" value="Cancel" title="Cancel add a new rule" onclick="clickCancel()"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>
    <jsp:body>
        <sm-ep:properties title="">
            <sm-ep:list title="Transport" name="transportId"
                        values="${fn:join(bean.transpotIds, ',')}"
                        valueTitles="${fn:join(bean.transpotTitles, ',')}"/>
        </sm-ep:properties>
    </jsp:body>
</sm:page>

