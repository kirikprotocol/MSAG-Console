<%!
String getTriggerParamName(String trigger_name)
{
  return "trigger_" + trigger_name;
}

boolean set_trigger_by_request_param(String trigger_name, boolean default_value, HttpSession session, HttpServletRequest request)
{
  Boolean trigger = null;
  String trigger_string = request.getParameter(getTriggerParamName(trigger_name));
  if (trigger_string != null)
    trigger = Boolean.valueOf(trigger_string);
  else 
    trigger = new Boolean(default_value);
    
  session.setAttribute(getTriggerParamName(trigger_name), trigger);

  return trigger.booleanValue();
}


boolean process_trigger(String trigger_name, HttpSession session, HttpServletRequest request)
{
  Boolean trigger = (Boolean) session.getAttribute(getTriggerParamName(trigger_name));
  if (trigger == null)
    trigger = new Boolean(false);
    
  String trigger_string = request.getParameter(getTriggerParamName(trigger_name));
  if (trigger_string != null)
    trigger = Boolean.valueOf(trigger_string);
    
  session.setAttribute(getTriggerParamName(trigger_name), trigger);

  return trigger.booleanValue();
}

String switch_trigger(String trigger_url, String trigger_name, String trigger_to_true, String trigger_to_false, HttpSession session, HttpServletRequest request)
{
  return "<a href=\"" + trigger_url + getTriggerParamName(trigger_name)+ "=" + (process_trigger(trigger_name, session, request)
            ? ("false\">" + StringEncoderDecoder.encode(trigger_to_false))
            : ("true\">" + StringEncoderDecoder.encode(trigger_to_true))) + "</a>";
}

String show_set_combo(String name, Set values)
{
  String result = "<select name=\""+StringEncoderDecoder.encode(name)+"\">";
  for (Iterator j = values.iterator(); j.hasNext(); )
  {
    result += "<option>" + StringEncoderDecoder.encode((String) j.next()) + "</option>";
  }
  result += "</select>";
  return result;
}

String maskStrings2String(Set maskStrings)
{
  String masks = "";
  for (Iterator i = maskStrings.iterator(); i.hasNext(); )
  {
    masks += (String) i.next();
    if (i.hasNext())
      masks += '\n';
  }
  return masks;
}

int getIntegerParam(HttpServletRequest request, String paramName)
{
	String paramStr = request.getParameter(paramName);
	if (paramStr == null)
		return Integer.MIN_VALUE;
	
	try {
		return Integer.decode(paramStr).intValue();
	} catch (Throwable t) {
		return Integer.MIN_VALUE;
	}
}
%>