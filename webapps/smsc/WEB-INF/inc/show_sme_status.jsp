<%@ page import="ru.novosoft.smsc.admin.route.SmeStatus,
					  ru.novosoft.smsc.admin.route.SME"%><%!
String showSmeStatus(SmeStatus status)
{
	String result = "";
	if (status != null)
	{
		if (status.isInternal())
		{
			result += " class=C008>internal</span>";
		}
		else
		{
			if (!status.isConnected())
			{
				result += " class=C800>disconnected</span>";
			}
			else
			{
				switch (status.getBindMode())
				{
					case SME.MODE_TX:
						result += " class=C080>TX&nbsp;" + status.getInAddress() + "</span>";
						break;
					case SME.MODE_RX:
						result += " class=C080>RX&nbsp;" + status.getInAddress() + "</span>";
						break;
					case SME.MODE_TRX:
						result += " class=C080>TRX&nbsp;" + status.getInAddress() + "</span>";
						break;
					default:
						result += " class=C080>unknown&nbsp;" + status.getInAddress() + '/' + status.getOutAddress() + "</span>";
				}
			}
		}
	}
	else
	{
		result += " class=C000>unknown</span>";
	}
	return result;
}
%>