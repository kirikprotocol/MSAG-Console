package mobi.eyeline.smsquiz.quizmanager;

import org.apache.log4j.Logger;
import ru.sibinco.smsc.utils.admin.server.Command;
import ru.sibinco.smsc.utils.admin.server.Service;
import ru.sibinco.smsc.utils.admin.server.model.*;

/**
 * author: alkhal
 */
public class SmsQuizService implements Service {

  private QuizManager manager;

  private Description description;

  private static final Logger logger = Logger.getLogger(SmsQuizService.class);

  private static final String METHOD_QUIZCHANGED = "quizchanged";

  private static final String METHOD_GETSTATUS = "getstatus";

  private static final String METHOD_GETDISTRID = "getdistrid";

  private static final String COMPONENT = "smsquiz";

  private final String name = "SmsQuiz";

  public SmsQuizService(QuizManager manager) {
    if (manager == null) {
      logger.error("Some arguments are null");
      throw new IllegalArgumentException("Some arguments are null");
    }
    this.manager = manager;
    description = new Description();
    createDescription();
  }

  private void createDescription() {
    Component c = new Component(COMPONENT);
    Method m1 = new Method(METHOD_QUIZCHANGED, Type.STRING);
    Method m2 = new Method(METHOD_GETSTATUS, Type.STRING);
    Method m3 = new Method(METHOD_GETDISTRID, Type.STRING);
    m1.addParameter(new Parameter("id", Type.STRING));
    m2.addParameter(new Parameter("id", Type.STRING));
    m3.addParameter(new Parameter("id", Type.STRING));
    c.addMethod(m1);
    c.addMethod(m2);
    c.addMethod(m3);
    description.addComponent(c);
  }

  public Response executeCommand(Command command) {
    Response response;
    try {
      if (command.getComponentName().equalsIgnoreCase(COMPONENT)) {
        if (command.getMethodName().equalsIgnoreCase(METHOD_QUIZCHANGED)) {

          Parameter param = command.getParameter("id");
          try{
            manager.refreshQuiz((String) param.getValue());
            response = new Response(Type.STRING, "");
          }catch (Throwable e) {
            logger.error(e,e);
            response = new Response(Type.STRING, e.getMessage());
            response.setStatus(Response.Status.ERROR);
          }
          return response;

        } else if (command.getMethodName().equalsIgnoreCase(METHOD_GETSTATUS)) {

          Parameter param = command.getParameter("id");
          String value = manager.getStatus((String) param.getValue());
          if (value == null) {
            response = new Response(null, null);
            response.setError("Quiz not found");
            response.setStatus(Response.Status.ERROR);
            return response;
          } else {
            return new Response(Type.STRING, value);
          }
        } else if (command.getMethodName().equalsIgnoreCase(METHOD_GETDISTRID)) {

          Parameter param = command.getParameter("id");
          String value = manager.getDistrId((String) param.getValue());
          if (value == null) {
            response = new Response(null, null);
            response.setError("Quiz not found");
            response.setStatus(Response.Status.ERROR);
            return response;
          } else {
            return new Response(Type.STRING, value);
          }
        }
      }
      throw new IllegalAccessException("No such COMPONENT or method");
    } catch (Exception e) {
      response = new Response(null, null);
      response.setError(e.getMessage());
      response.setStatus(Response.Status.ERROR);
      return response;
    }
  }

  public String getName() {
    return name;
  }

  public Description getDescription() {
    return description;
  }
}
