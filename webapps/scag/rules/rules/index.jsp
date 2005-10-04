<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page title="Rules">
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbAdd" value="Add" title="Add new rule" onclick="clearForm()"/>
            <sm-pm:item name="mbDelete" value="Delete" title="Delete rule" isCheckSensible="true"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>

    <jsp:body>
        <sm:table columns="checkbox,id,name,providerName,transport" names="c,id,name,provider,transport"
                  widths="1,15,28,28,28" edit="id" goal="jedit"/>

    </jsp:body>
</sm:page>