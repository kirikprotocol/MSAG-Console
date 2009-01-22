package mobi.eyeline.smsquiz;

import ru.novosoft.smsc.admin.service.Service;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.Type;
import ru.novosoft.smsc.admin.AdminException;

import java.util.Map;
import java.util.HashMap;
import java.util.List;

/**
 * author: alkhal
 * Date: 21.01.2009
 */
public class SmsQuiz extends Service {

  private static final String METHOD_QUIZCHANGED = "quizchanged";

  private static final String METHOD_GETSTATUS = "getstatus";

  private static final String METHOD_GETDISTRID = "getdistrid";

  private static final String COMPONENT = "smsquiz";


  public SmsQuiz(ServiceInfo info, String host, int port) {
    super(info, port);
    this.host = host;
  }

  public String quizChanged(String quizId) throws AdminException {
    Map args = new HashMap();
    args.put("id",quizId);
    Object result = callMethod(METHOD_QUIZCHANGED, Type.Types[Type.StringType], args);
    if (result instanceof String)
      return (String) result;
    else
      throw new AdminException("isTaskProcessorRuning: Incorrect return type \"" + result.getClass().getName() + "\"");
  }

  public String getStatus(String quizId) throws AdminException{
    Map args = new HashMap();
    args.put("id",quizId);
    Object result = callMethod(METHOD_GETSTATUS, Type.Types[Type.StringType], args);
    if (result instanceof String)
      return (String)result;
    else
      throw new AdminException("isTaskProcessorRuning: Incorrect return type \"" + result.getClass().getName() + "\"");
  }

  public String getDistrId(String quizId) throws AdminException{
    Map args = new HashMap();
    args.put("id",quizId);
    Object result = callMethod(METHOD_GETDISTRID, Type.Types[Type.StringType], args);
    if (result instanceof String)
      return (String)result;
    else
      throw new AdminException("isTaskProcessorRuning: Incorrect return type \"" + result.getClass().getName() + "\"");
  }

  private Object callMethod(String methodId, Type type, Map args) throws AdminException{
    return call(COMPONENT, methodId, type, args);
  }



}
