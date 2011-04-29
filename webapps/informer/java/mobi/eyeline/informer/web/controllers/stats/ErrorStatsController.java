package mobi.eyeline.informer.web.controllers.stats;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.delivery.Message;
import mobi.eyeline.informer.admin.delivery.MessageFilter;
import mobi.eyeline.informer.admin.delivery.Visitor;
import mobi.eyeline.informer.admin.users.User;
import mobi.eyeline.informer.web.components.data_table.model.DataTableModel;
import mobi.eyeline.informer.web.components.data_table.model.DataTableSortOrder;
import mobi.eyeline.informer.web.components.data_table.model.EmptyDataTableModel;
import mobi.eyeline.informer.web.config.Configuration;
import mobi.eyeline.informer.web.controllers.LongOperationController;

import javax.faces.model.SelectItem;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * @author Aleksandr Khalitov
 */
public class ErrorStatsController extends LongOperationController{

  private Integer deliveryId;

  private User user;

  private Delivery delivery;

  private AggregatedErrorStatsStrategy.GroupType groupType;

  private ErrorStatsStrategy statsStrategy;

  private InformerStrategy informerStrategy;

  private Configuration config;

  public static final String COME_BACK_PARAMS = "errors_stats_come_back_params";
  public static final String COME_BACK = "errors_stats_come_back";

  public ErrorStatsController() {
    config = getConfig();
    informerStrategy = new CommonInformerStrategy(config);
    statsStrategy = new SimpleErrorStatsStrategy(informerStrategy);
  }

  public boolean isAggregated() {
    return groupType != null;
  }

  @Override
  public void execute(Configuration config, Locale locale) throws Exception {
    if(groupType == null) {
      statsStrategy = new SimpleErrorStatsStrategy(informerStrategy);
    }else {
      statsStrategy = new AggregatedErrorStatsStrategy(groupType, informerStrategy);
    }
    ErrorStatsStrategy.ProgressListener pL = new ProgressListenerImpl();
    statsStrategy.execute(delivery, user, pL);
  }

  @Override
  public String start() {
    if(deliveryId == null) {
      return null;
    }
    reset();
    user = config.getUser(getUserName());
    if(user == null) {
      return null;
    }
    try {
      delivery = informerStrategy.getDelivery(user.getLogin(), user.getPassword(), deliveryId);
    } catch (AdminException e) {
      addError(e);
      return null;
    }
    if(delivery == null) {
      return null;
    }
    return super.start();
  }


  private String backParams;
  private String backAction;

  public String setDeliveryParam() {

    backParams = getRequestParameter(COME_BACK_PARAMS);
    backAction = getRequestParameter(COME_BACK);

    if(Boolean.valueOf(getRequestParameter("archive")) || Boolean.valueOf((String)getRequest().get("archive"))) {
      informerStrategy = new ArchiveInformerStrategy(config);
    }else {
      informerStrategy = new CommonInformerStrategy(config);
    }


    Integer s = (Integer)getRequest().get("delivery");
    if (s != null) {
      deliveryId = s;
      start();
      return "MESS_ERRORS";
    }
    return null;
  }

  public String getBackParams() {
    return backParams;
  }

  public String backAction() {
    return getBackAction();
  }

  public void setBackParams(String backParams) {
    this.backParams = backParams;
  }

  public String getBackAction() {
    return backAction;
  }

  public void setBackAction(String backAction) {
    this.backAction = backAction;
  }

  public DataTableModel getRecords() {

    if (getState() != 2)
      return new EmptyDataTableModel();

    return new DataTableModel() {

      public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {
        return statsStrategy.getRows(startPos, count, sortOrder);
      }

      public int getRowsCount() {
        return statsStrategy.getRowsCount();
      }
    };
  }

  public String getDeliveryName() {
    return delivery == null ? null : delivery.getName();
  }


  public Integer getDeliveryId() {
    return deliveryId;
  }

  public void setDeliveryId(Integer deliveryId) {
    this.deliveryId = deliveryId;
  }

  @Override
  protected void _download(PrintWriter writer) throws IOException {
    statsStrategy.download(writer);
  }

  public String clearFilter() {
    groupType = null;
    return start();
  }

  public String getGroupType() {
    return groupType == null ? "" : groupType.toString();
  }

  public void setGroupType(String groupType) {
    this.groupType = groupType == null || groupType.length() == 0 ? null : AggregatedErrorStatsStrategy.GroupType.valueOf(groupType);
  }

  public List<SelectItem> getGroupTypes() {
    Locale l = getLocale();
    ResourceBundle bundle = ResourceBundle.getBundle("mobi.eyeline.informer.web.resources.Informer", l);
    AggregatedErrorStatsStrategy.GroupType[] vs = AggregatedErrorStatsStrategy.GroupType.values();
    List<SelectItem> result = new ArrayList<SelectItem>(vs.length);
    for(AggregatedErrorStatsStrategy.GroupType v : vs) {
      String s = v.toString();
      result.add(new SelectItem(s, bundle.getString("informer.stats.by.errors.group."+s)));
    }
    return result;
  }

  private class ProgressListenerImpl implements ErrorStatsStrategy.ProgressListener {
    @Override
    public void incrementCurrent() {
      current++;
    }
    @Override
    public void setTotal(int t) {
      total = t;
    }
  }


  static class ArchiveInformerStrategy implements InformerStrategy {

    private final Configuration config;

    ArchiveInformerStrategy(Configuration config) {
      this.config = config;
    }

    @Override
    public int countMessages(String user, String password, MessageFilter filter) throws AdminException{
      return config.countArchiveMessages(user, filter);
    }

    @Override
    public void getMessagesStates(String user, String password, MessageFilter filter, int pieceSize, Visitor<Message> visitor) throws AdminException{
      config.getArchiveMessages(user, filter, pieceSize, visitor);
    }

    @Override
    public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException {
      return config.getArchiveDelivery(login, deliveryId);
    }
  }

  static class CommonInformerStrategy implements InformerStrategy {

    private final Configuration config;

    CommonInformerStrategy(Configuration config) {
      this.config = config;
    }

    @Override
    public int countMessages(String user, String password, MessageFilter filter) throws AdminException{
      return config.countMessages(user, password, filter);
    }

    @Override
    public void getMessagesStates(String user, String password, MessageFilter filter, int pieceSize, Visitor<Message> visitor) throws AdminException{
      config.getMessagesStates(user, password, filter, pieceSize, visitor);
    }

    @Override
    public Delivery getDelivery(String login, String password, int deliveryId) throws AdminException {
      return config.getDelivery(login, password, deliveryId);
    }
  }




}
