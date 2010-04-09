<%@ include file="/WEB-INF/inc/sme_menu.jsp"%>
<%
	  sme_menu_begin(out);
    sme_menu_button(out, "mbMenu",  "options",   getLocString( "welcomesms.label.options"),    getLocString("welcomesms.label.options.info"));
    sme_menu_button(out, "mbMenu",  "zones",      getLocString("welcomesms.label.zones"),   getLocString("welcomesms.label.zones.info"));
    sme_menu_button(out, "mbMenu",  "networks",   getLocString("welcomesms.label.networks"),  getLocString("welcomesms.label.networks.info"));
    sme_menu_button(out, "mbMenu",  "upload",      getLocString("welcomesms.label.upload"),      getLocString("welcomesms.label.upload.info"));
    sme_menu_space(out);
    sme_menu_end(out);
%>