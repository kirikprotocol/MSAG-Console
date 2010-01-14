/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 17, 2003
 * Time: 1:43:21 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl;


import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.dl.exceptions.*;
import ru.novosoft.smsc.admin.service.Service;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.Type;
import ru.novosoft.smsc.jsp.util.tables.impl.dl.DlFilter;

import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: andrey
 * Date: 26.05.2005
 * Time: 18:24:47
 * To change this template use File | Settings | File Templates.
 */

public class DistributionListManager extends Service implements DistributionListAdmin {
  private static final String SMSC_COMPONENT_ID = "SMSC";

  private static final String GET_PRINCIPALS_METHOD_ID = "prc_list";
  private final static String ADD_PRINCIPAL_METHOD_ID = "prc_add";
  private final static String DELETE_PRINCIPAL_METHOD_ID = "prc_delete";
  private final static String GET_PRINCIPAL_METHOD_ID = "prc_get";
  private final static String ALT_PRINCIPAL_METHOD_ID = "prc_alter";
  private final static String ADD_MEMBER_METHOD_ID = "mem_add";
  private final static String DELETE_MEMBER_METHOD_ID = "mem_delete";
  private final static String GET_MEMBERS_METHOD_ID = "mem_get";
  private final static String ADD_SUBMITTER_METHOD_ID = "sbm_add";
  private final static String DELETE_SUBMITTER_METHOD_ID = "sbm_delete";
  private final static String SUBMITTERS_METHOD_ID = "sbm_list";
  private final static String ADD_DL_METHOD_ID = "dl_add";
  // private final static String DELETE_MEMBERS_METHOD_ID = "delete_members";
  private final static String DELETE_DL_METHOD_ID = "dl_delete";
  private final static String GET_DL_METHOD_ID = "dl_get";
  private final static String LIST_DL_METHOD_ID = "dl_list";
  private final static String ALT_DL_METHOD_ID = "dl_alter";

/*
private final static String GET_PRINCIPALS_SQL = "SELECT ADDRESS, MAX_LST, MAX_EL FROM DL_PRINCIPALS";
private final static String CHECK_PRINCIPAL_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_PRINCIPALS WHERE ADDRESS=?";
private final static String ADD_PRINCIPAL_SQL = "INSERT INTO DL_PRINCIPALS (ADDRESS, MAX_LST, MAX_EL) VALUES (?, ?, ?)";
private final static String CHECK_PRINCIPAL_SUB_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_SUBMITTERS WHERE ADDRESS=?";
private final static String DELETE_PRINCIPAL_SQL = "DELETE FROM DL_PRINCIPALS WHERE ADDRESS=?";
private final static String GET_PRINCIPAL_SQL = "SELECT ADDRESS, MAX_LST, MAX_EL FROM DL_PRINCIPALS WHERE ADDRESS=?";
private final static String ALT_PRINCIPAL_LIST_SQL = "UPDATE DL_PRINCIPALS SET MAX_LST=? WHERE ADDRESS=?";
private final static String ALT_PRINCIPAL_ELEM_SQL = "UPDATE DL_PRINCIPALS SET MAX_EL=? WHERE ADDRESS=?";
private final static String CHECK_MEMBER_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_MEMBERS WHERE LIST=? AND ADDRESS=?";
private final static String ADD_MEMBER_SQL = "INSERT INTO DL_MEMBERS (LIST, ADDRESS) VALUES (?, ?)";
private final static String DELETE_MEMBER_SQL = "DELETE FROM DL_MEMBERS WHERE LIST=? AND ADDRESS= ?";
private final static String GET_MEMBERS_SQL = "SELECT ADDRESS FROM DL_MEMBERS WHERE LIST=?";
private final static String CHECK_SUBMITTER_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_SUBMITTERS WHERE LIST=? AND ADDRESS=?";
private final static String ADD_SUBMITTER_SQL = "INSERT INTO DL_SUBMITTERS (LIST, ADDRESS) VALUES (?, ?)";
private final static String DELETE_SUBMITTER_SQL = "DELETE FROM DL_SUBMITTERS WHERE LIST=? AND ADDRESS=?";
private final static String GET_LIST_PRINCIPALS_SQL = "SELECT ADDRESS, MAX_LST, MAX_EL FROM DL_PRINCIPALS WHERE ADDRESS IN (SELECT ADDRESS FROM DL_SUBMITTERS WHERE LIST=?)";
private final static String CHECK_DL_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_SET WHERE LIST=?";
private final static String ADD_DL_SQL = "INSERT INTO DL_SET (LIST, MAX_EL, OWNER) VALUES (?, ?, ?)";
private final static String DELETE_MEMBERS_SQL = "DELETE FROM DL_MEMBERS WHERE LIST=?";
private final static String DELETE_DL_SUB_SQL = "DELETE FROM DL_SUBMITTERS WHERE LIST=?";
private final static String DELETE_DL_SQL = "DELETE FROM DL_SET WHERE LIST=?";
private final static String GET_DL_SQL = "SELECT LIST, OWNER, MAX_EL FROM DL_SET WHERE LIST=?";
private final static String LIST_DL_SQL = "SELECT LIST, OWNER, MAX_EL FROM DL_SET";
private final static String ALT_DL_SQL = "UPDATE DL_SET SET MAX_EL=? WHERE LIST=?";
private final static String GET_DL_SQL_FOR_UPDATE = "SELECT MAX_EL, OWNER FROM DL_SET WHERE LIST=?"; // FOR UPDATE
private final static String GET_DLMEMBERS_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_MEMBERS WHERE LIST=?";
private final static String GET_DLS_FOR_OWNER_SQL = "SELECT NVL(COUNT(*), 0) FROM DL_SET WHERE OWNER=?";
private final static String GET_MAX_MEMBERS_COUNT_FOR_OWNER_SQL = "SELECT NVL(MAX(COUNT(LIST)), 0) FROM DL_MEMBERS WHERE LIST IN (SELECT LIST FROM DL_SET WHERE OWNER=?) GROUP BY LIST";
*/

  // private DataSource ds;
  // private Category logger = Category.getInstance(this.getClass());

  public DistributionListManager(ServiceInfo info, int smscPort /*,DataSource ds*/) {
    super(info, smscPort);
    // this.ds = ds;
  }

  public synchronized void setInfo(ServiceInfo info) {
    super.setInfo(info);
  }

  private List parsePrincipalList(final List listStr) {
    if (null == listStr || 0 == listStr.size())
      return new LinkedList();
    // final StringBuffer buffer = new StringBuffer(listStr.size());
    final List result = new LinkedList();
    int i = 0;
    while (i + 2 < listStr.size()) {
      String address = (String) listStr.get(i++);
      int maxLists = Integer.parseInt((String) listStr.get(i++));
      int maxElements = Integer.parseInt((String) listStr.get(i++));
      result.add(new Principal(address, maxLists, maxElements));
    }
    return result;
  }

  public synchronized List principals() throws AdminException {
    List list;

    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("address", "");
    final Object res = _call(SMSC_COMPONENT_ID, GET_PRINCIPALS_METHOD_ID, Type.Types[Type.StringListType], args);

    list = res instanceof List ? (List) res : null;
    return parsePrincipalList(list);
  }

  public synchronized List principals(String address) throws AdminException {
    List list;
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("address", address);
    final Object res = _call(SMSC_COMPONENT_ID, GET_PRINCIPALS_METHOD_ID, Type.Types[Type.StringListType], args);

    list = res instanceof List ? (List) res : null;
    return parsePrincipalList(list);
  }

  public synchronized void addPrincipal(Principal prc) throws AdminException, PrincipalAlreadyExistsException {
    //for void no use call
    //  List list=principals(); if (list.contains(prc)) throw new PrincipalAlreadyExistsException(prc);
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("address", prc.getAddress());
    args.put("maxLists", prc.getMaxListsInteger());
    args.put("maxElements", prc.getMaxElementsInteger());
    _call(SMSC_COMPONENT_ID, ADD_PRINCIPAL_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized void deletePrincipal(String address) throws AdminException, PrincipalNotExistsException, PrincipalInUseException {
    /* List list=principals(); Principal prc=null;
    for (Iterator it = list.iterator(); it.hasNext();) {
    prc = (Principal) it.next();  if (prc.getAddress().equals(address))  break;
    }
    if (prc == null)  throw new PrincipalNotExistsException(address);
    */
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("address", address);
    _call(SMSC_COMPONENT_ID, DELETE_PRINCIPAL_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized Principal getPrincipal(String address) throws AdminException, PrincipalNotExistsException {
    List list;
    checkSmscIsRunning();

    final Map args = new HashMap();
    args.put("address", address);
    final Object res = _call(SMSC_COMPONENT_ID, GET_PRINCIPAL_METHOD_ID, Type.Types[Type.StringListType], args);
    list = res instanceof List ? (List) res : null;
    if (list == null) throw new AdminException("Unsupported return type for commmand " + GET_PRINCIPAL_METHOD_ID);
    String addr = (String) list.get(0);
    int maxLists = Integer.parseInt((String) list.get(1));
    int maxElements = Integer.parseInt((String) list.get(2));
    return new Principal(addr, maxLists, maxElements);
  }

  public synchronized void alterPrincipal(Principal prc, boolean altLists, boolean altElements)
          throws AdminException, PrincipalNotExistsException, ListsCountExceededException,
          MembersCountExceededForOwnerException {
    // List list=principals(); if (!list.contains(prc)) throw new PrincipalNotExistsException(prc.getAddress());
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("address", prc.getAddress());
    args.put("maxLists", prc.getMaxListsInteger());
    args.put("maxElements", prc.getMaxElementsInteger());
    args.put("altLists", new Boolean(altLists));
    args.put("altElements", new Boolean(altElements));
    _call(SMSC_COMPONENT_ID, ALT_PRINCIPAL_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized void addMember(String dlname, String address) throws AdminException, ListNotExistsException, MemberAlreadyExistsException {
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("address", address);
    args.put("dlname", dlname);
    _call(SMSC_COMPONENT_ID, ADD_MEMBER_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized void deleteMember(String dlname, String address) throws AdminException, ListNotExistsException, MemberNotExistsException {
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("address", address);
    args.put("dlname", dlname);
    _call(SMSC_COMPONENT_ID, DELETE_MEMBER_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized List members(String dlname) throws AdminException, ListNotExistsException {
    List list;
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("dlname", dlname);
    final Object res = _call(SMSC_COMPONENT_ID, GET_MEMBERS_METHOD_ID, Type.Types[Type.StringListType], args);
    list = res instanceof List ? (List) res : null;
    if (list == null) throw new AdminException("Unsupported return type for commmand " + GET_MEMBERS_METHOD_ID);
    return list;
  }

  public synchronized void grantPosting(String dlname, String submitter) throws AdminException, ListNotExistsException, PrincipalNotExistsException, SubmitterAlreadyExistsException {
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("dlname", dlname);
    args.put("address", submitter);
    try {
      _call(SMSC_COMPONENT_ID, ADD_SUBMITTER_METHOD_ID, Type.Types[Type.StringType], args);
    } catch (AdminException e) {
      if (e.getMessage().equals("SubmitterAlreadyExistsException")) {
        logger.warn("Submitter "+submitter+" already exists in list "+dlname);
      }
    }

  }

  public synchronized void revokePosting(String dlname, String submitter) throws AdminException, ListNotExistsException, SubmitterNotExistsException {
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("dlname", dlname);
    args.put("address", submitter);
    _call(SMSC_COMPONENT_ID, DELETE_SUBMITTER_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized List submitters(String dlname) throws AdminException, ListNotExistsException {
    List list;
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("dlname", dlname);
    final Object res = _call(SMSC_COMPONENT_ID, SUBMITTERS_METHOD_ID, Type.Types[Type.StringListType], args);
    list = res instanceof List ? (List) res : null;
    if (list == null) throw new AdminException("Unsupported return type for commmand " + SUBMITTERS_METHOD_ID);
    return parsePrincipalList(list);
  }

  public synchronized void addDistributionList(DistributionList dl) throws AdminException, ListAlreadyExistsException {
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("dlname", dl.getName());
    args.put("owner", dl.getOwner());
    args.put("maxElements", new Integer(dl.getMaxElements()));
    _call(SMSC_COMPONENT_ID, ADD_DL_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized void deleteDistributionList(String dlname) throws AdminException, ListNotExistsException {
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("dlname", dlname);
    _call(SMSC_COMPONENT_ID, DELETE_DL_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized DistributionList getDistributionList(String dlname) throws AdminException, ListNotExistsException {

    List list;
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("dlname", dlname);
    final Object res = _call(SMSC_COMPONENT_ID, GET_DL_METHOD_ID, Type.Types[Type.StringListType], args);
    list = res instanceof List ? (List) res : null;
    if (list == null) throw new AdminException("Unsupported return type for commmand " + GET_DL_METHOD_ID);
    String name = (String) list.get(0);
    String owner = (String) list.get(1);
    if (owner.equals("")) owner = null;
    int maxElements = Integer.parseInt((String) list.get(2));
    DistributionList dl = new DistributionList(name, owner, maxElements);
    return dl;
  }

  private List parseDistributionList(final List listStr /*, DlFilter filter*/) throws AdminException {
    if (null == listStr || 0 == listStr.size())
      return Collections.EMPTY_LIST;
    final List result = new ArrayList();
    int i = 0;
    while (i+2 < listStr.size()) {
      String name = (String) listStr.get(i++);
      String owner = (String) listStr.get(i++);
      if (owner.length() == 0) owner = null;
      int maxElements = Integer.parseInt((String) listStr.get(i++));
      final DistributionList dl = new DistributionList(name, owner, maxElements);
      // if (filter == null || filter.isItemAllowed(dl)) {
      result.add(dl);
      //  }
    }
    return result;
  }

  private List internal_list(DlFilter filter) throws AdminException {
    List list;
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("names", new LinkedList());
    args.put("owners", new LinkedList());
    if (filter != null) {
      if (!filter.isNamesContainsRegexp() && !filter.isOwnersContainsRegexp()) {
        List names = Arrays.asList(filter.getNames());
        List owners = Arrays.asList(filter.getOwners());
        args.put("names", names);
        args.put("owners", owners);
      }
    }
    final Object res = _call(SMSC_COMPONENT_ID, LIST_DL_METHOD_ID, Type.Types[Type.StringListType], args);
    list = res instanceof List ? (List) res : null;
    if (list == null) throw new AdminException("Unsupported return type for commmand " + LIST_DL_METHOD_ID);
    return parseDistributionList(list /*,filter*/);
    /*
 List list = new ArrayList();
 PreparedStatement stmt = null;
 Connection connection = null;
 ResultSet rs = null;
 try {
   connection = ds.getConnection();
   stmt = connection.prepareStatement(sql_statement);
   rs = stmt.executeQuery();
   while (rs.next()) {
     String name = rs.getString(1);
     String owner = rs.getString(2);
     if (rs.wasNull()) owner = null;
     int maxElements = rs.getInt(3);
     final DistributionList dl = new DistributionList(name, owner, maxElements);
     if (filter == null || filter.isItemAllowed(dl))
       list.add(dl);
   }
   rs.close();
   rs = null;
 } catch (Exception exc) {
   exc.printStackTrace();
   throw new AdminException(exc.getMessage());
 } finally {
   try {
     if (stmt != null) stmt.close();
     connection.close();
   } catch (Exception cexc) {
     cexc.printStackTrace();
     throw new AdminException(cexc.getMessage());
   }
 }
 return list;  */
  }

  public synchronized List list() throws AdminException {
    return internal_list(/*LIST_DL_SQL,*/ null);
  }

  public synchronized List list(DlFilter filter) throws AdminException {
    if (filter.isEmpty())
      return list();
    /*   String where_clause = "";
   if (!filter.isNamesContainsRegexp() && !filter.isOwnersContainsRegexp()) {
     for (int i = 0; i < filter.getNames().length; i++) {
       String name = filter.getNames()[i];
       where_clause += (i == 0 ? "" : " or ") + "(list like '" + name + "%')";
     }
     for (int i = 0; i < filter.getOwners().length; i++) {
       String owner = filter.getOwners()[i];
       where_clause += (i == 0 ? "" : " or ") + "(owner like '" + owner + "%')";
     }
     if (where_clause.length() > 0)
       where_clause = " where " + where_clause;
   } */

    return internal_list(/* LIST_DL_SQL + where_clause, */ filter);
  }

  public synchronized void alterDistributionList(String dlname, int maxElements) throws AdminException, ListNotExistsException {
    checkSmscIsRunning();
    final Map args = new HashMap();
    args.put("dlname", dlname);
    args.put("maxElements", new Integer(maxElements));
    _call(SMSC_COMPONENT_ID, ALT_DL_METHOD_ID, Type.Types[Type.StringType], args);
  }

  public synchronized void checkSmscIsRunning() throws AdminException {
//    if (!getInfo().isOnline())
//      throw new AdminException("SMSC is not running.");
    
  }

  private Object _call(String componentId, String methodId, Type returnType, Map arguments) throws AdminException {
    try {
      return call(componentId, methodId, returnType, arguments);
    } catch (AdminException e) {
      if (e.getMessage().indexOf("SubmitterAlreadyExistsException") > 0) {
        throw new SubmitterAlreadyExistsException(null,null);
      } else if (e.getMessage().indexOf("SubmitterNotExistsException") > 0) {
        throw new SubmitterNotExistsException(null,null);
      } else if (e.getMessage().indexOf("PrincipalAlreadyExistsException") > 0) {
        throw new PrincipalAlreadyExistsException(null);
      } else if (e.getMessage().indexOf("PrincipalNotExistsException") > 0) {
        throw new PrincipalNotExistsException(null);
      } else if (e.getMessage().indexOf("PrincipalInUseException") > 0) {
        throw new PrincipalInUseException(null);
      } else if (e.getMessage().indexOf("ListNotExistsException") > 0) {
        throw new ListNotExistsException(null);
      } else if (e.getMessage().indexOf("ListAlreadyExistsException") > 0) {
        throw new ListAlreadyExistsException(null);
      } else if (e.getMessage().indexOf("ListCountExceededException") > 0) {
        throw new ListsCountExceededException(null);
      } else if (e.getMessage().indexOf("MemberNotExistsException") > 0) {
        throw new MemberNotExistsException(null,null);
      } else if (e.getMessage().indexOf("MemberAlreadyExistsException") > 0) {
        throw new MemberAlreadyExistsException(null,null);
      } else if (e.getMessage().indexOf("MemberCountExceededException") > 0) {
        throw new MembersCountExceededException(null, 0, 0);
      }
      throw new AdminException(e.getMessage());
    }
  }

  ////////////////////////////////////////////////////
  // old implementation
  // work with base oracle
  /*
  public List principals() throws AdminException
  {

  List list = new ArrayList();
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(GET_PRINCIPALS_SQL);
  rs = stmt.executeQuery();
  while (rs.next()) {
  list.add(new Principal(rs.getString(1), rs.getInt(2), rs.getInt(3)));
  }
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  return list;
  }

  public void addPrincipal(Principal prc) throws AdminException, PrincipalAlreadyExistsException
  {
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(CHECK_PRINCIPAL_SQL);
  stmt.setString(1, prc.getAddress());
  rs = stmt.executeQuery();
  if (rs.next() && rs.getInt(1) != 0)
  throw new PrincipalAlreadyExistsException(prc);
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(ADD_PRINCIPAL_SQL);
  stmt.setString(1, prc.getAddress());
  stmt.setInt(2, prc.getMaxLists());
  stmt.setInt(3, prc.getMaxElements());
  stmt.executeUpdate();
  connection.commit();
  } catch (AdminException exc) {
  throw exc;
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  }

  public void deletePrincipal(String address) throws AdminException, PrincipalNotExistsException, PrincipalInUseException
  {
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(CHECK_PRINCIPAL_SQL);
  stmt.setString(1, address);
  rs = stmt.executeQuery();
  if (!rs.next() || rs.getInt(1) == 0)
  throw new PrincipalNotExistsException(address);
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(CHECK_PRINCIPAL_SUB_SQL);
  stmt.setString(1, address);
  rs = stmt.executeQuery();
  if (rs.next() && rs.getInt(1) != 0)
  throw new PrincipalInUseException(address);
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(DELETE_PRINCIPAL_SQL);
  stmt.setString(1, address);
  stmt.executeUpdate();
  connection.commit();
  } catch (AdminException exc) {
  throw exc;
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  }

  public Principal getPrincipal(String address) throws AdminException, PrincipalNotExistsException
  {
  Principal prc = null;
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(GET_PRINCIPAL_SQL);
  stmt.setString(1, address);
  rs = stmt.executeQuery();
  if (!rs.next())
  throw new PrincipalNotExistsException(address);
  prc = new Principal(rs.getString(1), rs.getInt(2), rs.getInt(3));
  rs.close();
  rs = null;
  } catch (AdminException exc) {
  throw exc;
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  return prc;
  }

  public void alterPrincipal(Principal prc, boolean altLists, boolean altElements)
  throws AdminException, PrincipalNotExistsException, ListsCountExceededException,
  MembersCountExceededForOwnerException
  {
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(CHECK_PRINCIPAL_SQL);
  stmt.setString(1, prc.getAddress());
  rs = stmt.executeQuery();
  if (!rs.next() || rs.getInt(1) == 0)
  throw new PrincipalNotExistsException(prc.getAddress());
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(GET_DLS_FOR_OWNER_SQL);
  stmt.setString(1, prc.getAddress());
  rs = stmt.executeQuery();
  int listsCount = rs.next() ? rs.getInt(1) : 0;
  rs.close();
  rs = null;
  stmt.close();

  if (listsCount > prc.getMaxLists())
  throw new ListsCountExceededException(prc.getAddress());

  stmt = connection.prepareStatement(GET_MAX_MEMBERS_COUNT_FOR_OWNER_SQL);
  stmt.setString(1, prc.getAddress());
  rs = stmt.executeQuery();
  int maxMembersCount = rs.next() ? rs.getInt(1) : 0;
  rs.close();
  rs = null;
  stmt.close();

  if (maxMembersCount > prc.getMaxElements())
  throw new MembersCountExceededForOwnerException(prc.getAddress(), maxMembersCount, prc.getMaxElements());

  if (altLists) {
  stmt = connection.prepareStatement(ALT_PRINCIPAL_LIST_SQL);
  stmt.setInt(1, prc.getMaxLists());
  stmt.setString(2, prc.getAddress());
  stmt.executeUpdate();
  stmt.close();
  }
  if (altElements) {
  stmt = connection.prepareStatement(ALT_PRINCIPAL_ELEM_SQL);
  stmt.setInt(1, prc.getMaxElements());
  stmt.setString(2, prc.getAddress());
  stmt.executeUpdate();
  stmt.close();
  }
  connection.commit();
  } catch (AdminException exc) {
  throw exc;
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  }

  public void addMember(String dlname, String address) throws AdminException, ListNotExistsException, MemberAlreadyExistsException
  {
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(GET_DL_SQL_FOR_UPDATE);
  stmt.setString(1, dlname);
  rs = stmt.executeQuery();
  if (!rs.next())
  throw new ListNotExistsException(dlname);
  int maxMembers = rs.getInt(1);
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(CHECK_MEMBER_SQL);
  stmt.setString(1, dlname);
  stmt.setString(2, address);
  rs = stmt.executeQuery();
  if (rs.next() && rs.getInt(1) != 0)
  throw new MemberAlreadyExistsException(address, dlname);
  rs.close();
  rs = null;
  stmt.close();


  stmt = connection.prepareStatement(GET_DLMEMBERS_SQL);
  stmt.setString(1, dlname);
  rs = stmt.executeQuery();
  int membersCount = rs.next() ? rs.getInt(1) : 0;
  rs.close();
  rs = null;
  stmt.close();
  if (membersCount >= maxMembers)
  throw new MembersCountExceededException(dlname, membersCount, maxMembers);

  stmt = connection.prepareStatement(ADD_MEMBER_SQL);
  stmt.setString(1, dlname);
  stmt.setString(2, address);
  stmt.executeUpdate();
  connection.commit();
  } catch (AdminException exc) {
  throw exc;
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  }

  public void deleteMember(String dlname, String address) throws AdminException, ListNotExistsException, MemberNotExistsException
  {
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(CHECK_DL_SQL);
  stmt.setString(1, dlname);
  rs = stmt.executeQuery();
  if (!rs.next() || rs.getInt(1) == 0)
  throw new ListNotExistsException(dlname);
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(CHECK_MEMBER_SQL);
  stmt.setString(1, dlname);
  stmt.setString(2, address);
  rs = stmt.executeQuery();
  if (!rs.next() || rs.getInt(1) == 0)
  throw new MemberNotExistsException(address, dlname);
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(DELETE_MEMBER_SQL);
  stmt.setString(1, dlname);
  stmt.setString(2, address);
  stmt.executeUpdate();
  connection.commit();
  } catch (AdminException exc) {
  throw exc;
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  }

  public List members(String dlname) throws AdminException, ListNotExistsException
  {
  List list = new ArrayList();
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(CHECK_DL_SQL);
  stmt.setString(1, dlname);
  rs = stmt.executeQuery();
  if (!rs.next() || rs.getInt(1) == 0)
  throw new ListNotExistsException(dlname);
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(GET_MEMBERS_SQL);
  stmt.setString(1, dlname);
  rs = stmt.executeQuery();
  while (rs.next()) list.add(rs.getString(1));
  rs.close();
  rs = null;
  } catch (AdminException exc) {
  throw exc;
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  return list;
  }

  public void grantPosting(String dlname, String submitter) throws AdminException, ListNotExistsException, PrincipalNotExistsException, SubmitterAlreadyExistsException
  {
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(CHECK_DL_SQL);
  stmt.setString(1, dlname);
  rs = stmt.executeQuery();
  if (!rs.next() || rs.getInt(1) == 0)
  throw new ListNotExistsException(dlname);
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(CHECK_PRINCIPAL_SQL);
  stmt.setString(1, submitter);
  rs = stmt.executeQuery();
  if (!rs.next() || rs.getInt(1) == 0)
  throw new PrincipalNotExistsException(submitter);
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(CHECK_SUBMITTER_SQL);
  stmt.setString(1, dlname);
  stmt.setString(2, submitter);
  rs = stmt.executeQuery();
  if (rs.next() && rs.getInt(1) != 0)
  throw new SubmitterAlreadyExistsException(submitter, dlname);
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(ADD_SUBMITTER_SQL);
  stmt.setString(1, dlname);
  stmt.setString(2, submitter);
  stmt.executeUpdate();

  connection.commit();
  } catch (AdminException exc) {
  throw exc;
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  }

  public void revokePosting(String dlname, String submitter) throws AdminException, ListNotExistsException, SubmitterNotExistsException
  {
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(CHECK_DL_SQL);
  stmt.setString(1, dlname);
  rs = stmt.executeQuery();
  if (!rs.next() || rs.getInt(1) == 0)
  throw new ListNotExistsException(dlname);
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(CHECK_SUBMITTER_SQL);
  stmt.setString(1, dlname);
  stmt.setString(2, submitter);
  rs = stmt.executeQuery();
  if (!rs.next() || rs.getInt(1) == 0)
  throw new SubmitterNotExistsException(submitter, dlname);
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(DELETE_SUBMITTER_SQL);
  stmt.setString(1, dlname);
  stmt.setString(2, submitter);
  stmt.executeUpdate();

  connection.commit();
  } catch (AdminException exc) {
  throw exc;
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  }

  public List submitters(String dlname) throws AdminException, ListNotExistsException
  {
  List list = new ArrayList();
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(CHECK_DL_SQL);
  stmt.setString(1, dlname);
  rs = stmt.executeQuery();
  if (!rs.next() || rs.getInt(1) == 0)
  throw new ListNotExistsException(dlname);
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(GET_LIST_PRINCIPALS_SQL);
  stmt.setString(1, dlname);
  rs = stmt.executeQuery();
  while (rs.next()) {
  list.add(new Principal(rs.getString(1), rs.getInt(2), rs.getInt(3)));
  }
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  return list;
  }

  public void addDistributionList(DistributionList dl) throws AdminException, ListAlreadyExistsException
  {
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(CHECK_DL_SQL);
  stmt.setString(1, dl.getName());
  rs = stmt.executeQuery();
  if (rs.next() && rs.getInt(1) != 0)
  throw new ListAlreadyExistsException(dl.getName());
  rs.close();
  rs = null;
  stmt.close();

  if (!dl.isSys()) {
  stmt = connection.prepareStatement(GET_PRINCIPAL_SQL); //ADDRESS, MAX_LST, MAX_EL
  stmt.setString(1, dl.getNormalizedOwner());
  rs = stmt.executeQuery();
  if (!rs.next())
  throw new OwnerNotExistsException(dl.getName(), dl.getOwner());
  int maxLst = rs.getInt(2);
  int maxMembers = rs.getInt(3);
  rs.close();
  rs = null;
  stmt.close();

  if (maxMembers < dl.getMaxElements())
  throw new DistListMaxMembersGreaterThanOwnerMaxMembers(dl.getName(), dl.getOwner(), dl.getMaxElements(), maxMembers);


  stmt = connection.prepareStatement(GET_DLS_FOR_OWNER_SQL); //ADDRESS, MAX_LST, MAX_EL
  stmt.setString(1, dl.getNormalizedOwner());
  rs = stmt.executeQuery();
  int lsts = rs.next() ? rs.getInt(1) : 0;
  rs.close();
  rs = null;
  stmt.close();
  if (lsts >= maxLst)
  throw new ListsCountExceededException(dl.getOwner());
  }

  stmt = connection.prepareStatement(ADD_DL_SQL);
  stmt.setString(1, dl.getName());
  stmt.setInt(2, dl.getMaxElements());
  final String normalizedOwner = dl.getNormalizedOwner();
  if (dl.isSys() || normalizedOwner == null || normalizedOwner.trim().length() == 0)
  stmt.setNull(3, java.sql.Types.VARCHAR);
  else
  stmt.setString(3, normalizedOwner);
  stmt.executeUpdate();

  connection.commit();
  } catch (AdminException exc) {
  throw exc;
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  }

  public void deleteDistributionList(String dlname) throws AdminException, ListNotExistsException
  {
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(CHECK_DL_SQL);
  stmt.setString(1, dlname);
  rs = stmt.executeQuery();
  if (!rs.next() || rs.getInt(1) == 0)
  throw new ListNotExistsException(dlname);
  rs.close();
  rs = null;
  stmt.close();

  stmt = connection.prepareStatement(DELETE_MEMBERS_SQL);
  stmt.setString(1, dlname);
  stmt.executeUpdate();
  stmt.close();

  stmt = connection.prepareStatement(DELETE_DL_SUB_SQL);
  stmt.setString(1, dlname);
  stmt.executeUpdate();
  stmt.close();

  stmt = connection.prepareStatement(DELETE_DL_SQL);
  stmt.setString(1, dlname);
  stmt.executeUpdate();

  connection.commit();
  } catch (AdminException exc) {
  throw exc;
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  }

  public DistributionList getDistributionList(String dlname) throws AdminException, ListNotExistsException
  {
  DistributionList dl = null;
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(GET_DL_SQL);
  stmt.setString(1, dlname);
  rs = stmt.executeQuery();
  if (!rs.next())
  throw new ListNotExistsException(dlname);
  String name = rs.getString(1);
  String owner = rs.getString(2);
  if (rs.wasNull()) owner = null;
  int maxElements = rs.getInt(3);
  dl = new DistributionList(name, owner, maxElements);
  rs.close();
  rs = null;
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  return dl;
  }

  private List internal_list(String sql_statement, DlFilter filter) throws AdminException
  {
  List list = new ArrayList();
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();
  stmt = connection.prepareStatement(sql_statement);
  rs = stmt.executeQuery();
  while (rs.next()) {
  String name = rs.getString(1);
  String owner = rs.getString(2);
  if (rs.wasNull()) owner = null;
  int maxElements = rs.getInt(3);
  final DistributionList dl = new DistributionList(name, owner, maxElements);
  if (filter == null || filter.isItemAllowed(dl))
  list.add(dl);
  }
  rs.close();
  rs = null;
  } catch (Exception exc) {
  exc.printStackTrace();
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  cexc.printStackTrace();
  throw new AdminException(cexc.getMessage());
  }
  }
  return list;
  }

  public List list() throws AdminException
  {
  return internal_list(LIST_DL_SQL, null);
  }

  public List list(DlFilter filter) throws AdminException
  {
  if (filter.isEmpty())
  return list();

  String where_clause = "";
  if (!filter.isNamesContainsRegexp() && !filter.isOwnersContainsRegexp()) {
  for (int i = 0; i < filter.getNames().length; i++) {
  String name = filter.getNames()[i];
  where_clause += (i == 0 ? "" : " or ") + "(list like '" + name + "%')";
  }
  for (int i = 0; i < filter.getOwners().length; i++) {
  String owner = filter.getOwners()[i];
  where_clause += (i == 0 ? "" : " or ") + "(owner like '" + owner + "%')";
  }
  if (where_clause.length() > 0)
  where_clause = " where " + where_clause;
  }

  return internal_list(LIST_DL_SQL + where_clause, filter);
  }

  public void alterDistributionList(String dlname, int maxElements) throws AdminException, ListNotExistsException
  {
  PreparedStatement stmt = null;
  Connection connection = null;
  ResultSet rs = null;
  try {
  connection = ds.getConnection();

  // get dl
  stmt = connection.prepareStatement(GET_DL_SQL_FOR_UPDATE);
  stmt.setString(1, dlname);
  rs = stmt.executeQuery();
  if (!rs.next())
  throw new ListNotExistsException(dlname);
  //int oldMaxElements = rs.getInt(1);
  String owner = rs.getString(2);
  if (rs.wasNull()) owner = null;
  rs.close();
  rs = null;
  stmt.close();

  //check members count
  stmt = connection.prepareStatement(GET_DLMEMBERS_SQL);
  stmt.setString(1, dlname);
  rs = stmt.executeQuery();
  if (!rs.next())
  throw new ListNotExistsException(dlname);
  int members = rs.getInt(1);
  rs.close();
  rs = null;
  stmt.close();

  if (members > maxElements)
  throw new MembersCountExceededException(dlname, members, maxElements);

  // check owner's max elements
  if (owner != null && owner.trim().length() > 0) {
  stmt = connection.prepareStatement(GET_PRINCIPAL_SQL);
  stmt.setString(1, owner);
  rs = stmt.executeQuery();
  if (!rs.next())
  throw new OwnerNotExistsException(dlname, owner);
  int ownerMaxMembers = rs.getInt(3);
  rs.close();
  rs = null;
  stmt.close();

  if (maxElements > ownerMaxMembers)
  throw new DistListMaxMembersGreaterThanOwnerMaxMembers(dlname, owner, maxElements, ownerMaxMembers);
  }

  ////////////////////////////////////////////////////
  // alter DL
  stmt = connection.prepareStatement(ALT_DL_SQL);
  stmt.setInt(1, maxElements);
  stmt.setString(2, dlname);
  if (stmt.executeUpdate() <= 0)
  throw new ListNotExistsException(dlname);

  stmt.close();

  connection.commit();
  } catch (AdminException exc) {
  try {
  connection.rollback();
  } catch (Throwable e) {
  logger.error("Could not rollback alter DL", e);
  }
  throw exc;
  } catch (Exception exc) {
  logger.error("Exception occured in alter dL", exc);
  try {
  connection.rollback();
  } catch (Throwable e) {
  logger.error("Could not rollback alter DL", e);
  }
  throw new AdminException(exc.getMessage());
  } finally {
  try {
  if (stmt != null) stmt.close();
  connection.close();
  } catch (Exception cexc) {
  logger.error("Could not close connection in alter DL", cexc);
  throw new AdminException(cexc.getMessage());
  }
  }
  }
  */
}
