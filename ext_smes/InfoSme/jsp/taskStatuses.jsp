<%@ page import="java.util.*,
                 ru.novosoft.smsc.util.StringEncoderDecoder"%><jsp:useBean id="bean" class="ru.novosoft.smsc.infosme.beans.TaskStatuses"/><jsp:setProperty name="bean" property="*"/><%
/*try
{*/
//String CPATH = request.getContextPath() + "/smsc";
bean.process(request);
List c = new ArrayList(bean.getTaskIds());
for (Iterator i = c.iterator(); i.hasNext(); )
{
  final String taskIdHex = StringEncoderDecoder.encodeHEX((String) i.next());
  out.print("\"gen" + taskIdHex + "\", \"prc" + taskIdHex + '"');
	if (i.hasNext())
		out.print(", ");
}
out.println();
for (Iterator i = c.iterator(); i.hasNext(); )
{
	String taskId = (String) i.next();
  if (bean.isTaskGenerating(taskId)){
    out.print("\"<span><img src='/images/ic_running.gif' title='generating'></span>\"");
    //out.print("generating");
  }else{
    out.print("\"<span><img src='/images/ic_stopped.gif' title='idle'></span>\"");
    //out.print("notGenerating");
  }

  out.print(", ");
  if (bean.isTaskProcessing(taskId)) {
    out.print("\"<span><img src='/images/ic_running.gif' title='processing'><span>\"");
    //out.print("processing");
  }else{
    out.print("\"<span><img src='/images/ic_stopped.gif' title='idle'></span>\"");
    //out.print("notProcessing");
  }
	if (i.hasNext())
		out.print(", ");
}
/*} catch (Throwable t)
{
	t.printStackTrace(new java.io.PrintWriter(out));
}*/
%>