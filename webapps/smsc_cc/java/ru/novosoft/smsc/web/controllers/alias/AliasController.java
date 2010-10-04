package ru.novosoft.smsc.web.controllers.alias;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.web.WebContext;
import ru.novosoft.smsc.web.components.data_table.model.DataTableModel;
import ru.novosoft.smsc.web.components.data_table.model.DataTableSortOrder;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableModel;
import ru.novosoft.smsc.web.components.dynamic_table.model.DynamicTableRow;
import ru.novosoft.smsc.web.controllers.SmscController;

import javax.faces.event.ActionEvent;
import javax.faces.model.SelectItem;
import java.util.*;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 01.10.2010
 * Time: 11:03:55
 */
public class AliasController extends SmscController {


  private DynamicTableModel aliasesFilter;
  private DynamicTableModel adressesFilter;
  private Boolean hideFilter;

  private boolean initError=false;
  private List selectedRows;


  public AliasController() {
    super();
    init();
  }

  private void init() {
//    try {
      //any init
      aliasesFilter  = new DynamicTableModel();
      adressesFilter = new DynamicTableModel();
      selectedRows = new ArrayList();
//    }
//    catch (AdminException e){
//      log.error("init error",e);
//      addError(e);
//      initError=true;
//    }
  }

  public String clearFilter() {
    aliasesFilter = new DynamicTableModel();
    adressesFilter = new DynamicTableModel();
    hideFilter = null;
    return null;
  }

  public void setInitError(boolean initError) {
    this.initError = initError;
  }

  public boolean isInitError() {
    return initError;
  }


  public DynamicTableModel getAliasesFilter() {
    return aliasesFilter;
  }

  public void setAliasesFilter(DynamicTableModel aliasesFilter) {
    this.aliasesFilter = aliasesFilter;
  }

  public void setAdressesFilter(DynamicTableModel adressesFilter) {
    this.adressesFilter = adressesFilter;
  }

  public DynamicTableModel getAdressesFilter() {
    return adressesFilter;
  }

  public String getHideFilter() {
    return hideFilter==null ? "all": hideFilter.toString();
  }

  public void setHideFilter(String hideFilter) {
    if("all".equals(hideFilter)) this.hideFilter=null;
    else {
      this.hideFilter = Boolean.valueOf(hideFilter);
    }
  }

  public List<SelectItem> getHideOptions() {
    List<SelectItem> ret = new ArrayList<SelectItem>();
    ret.add(new SelectItem("all",getLocalizedString("smsc.alias.options.all")));
    ret.add(new SelectItem("true",getLocalizedString("smsc.alias.options.hidden")));
    ret.add(new SelectItem("false",getLocalizedString("smsc.alias.options.nothidden")));
    return ret;
  }



  public DataTableModel getAliasesModel() throws AdminException {
    final List<Alias> aliases = new ArrayList<Alias>();
    final long start=System.currentTimeMillis();

      AliasSet aset=null;
      try {

        aset = WebContext.getInstance().getAliasManager().getAliases();

        while(aset.next()) {
          Alias alias = aset.get();
          if (hideFilter != null && hideFilter.booleanValue()!=(alias.isHide())) {
            continue;
          }
          if(aliasesFilter!=null && aliasesFilter.getRows().size()>0) {
            boolean found=false;
            for(DynamicTableRow row : aliasesFilter.getRows()) {
              String filterVal = ((String) row.getValue("alias")).trim();
              if(alias.getAlias().getSimpleAddress().indexOf(filterVal)>=0) {
                found=true;
                break;
              }
            }
            if(!found) continue;
          }
          if(adressesFilter!=null && adressesFilter.getRows().size()>0) {
            boolean found=false;
            for(DynamicTableRow row : adressesFilter.getRows()) {
              String filterVal = ((String) row.getValue("address")).trim();
              if(alias.getAddress().getSimpleAddress().indexOf(filterVal)>=0) {
                found=true;
                break;
              }
            }
            if(!found) continue;
          }
          aliases.add(alias);
        }
      }
      finally {
        if(aset!=null) aset.close();
      }

      return new DataTableModel() {
        public List getRows(int startPos, int count, final DataTableSortOrder sortOrder) {

        List<Alias> tmp = new LinkedList<Alias>(aliases);

        Collections.sort(tmp, new Comparator<Alias>() {
          public int compare(Alias o1, Alias o2) {
            if (sortOrder != null) {
              int mul = sortOrder.isAsc() ? 1 : -1;
              if ("alias".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getAlias().getSimpleAddress().compareTo(o2.getAlias().getSimpleAddress());
              }
              else if ("address".equals(sortOrder.getColumnId())) {
                return (mul) * o1.getAddress().getSimpleAddress().compareTo(o2.getAddress().getSimpleAddress());
              }
              else if ("hidden".equals(sortOrder.getColumnId())) {
                int cmp = (mul) * (Boolean.valueOf(o1.isHide())).compareTo(o2.isHide());
                if(cmp!=0) return cmp;
                return (mul) * o1.getAddress().getSimpleAddress().compareTo(o2.getAddress().getSimpleAddress());
              }
            }
            return o1.getAlias().getSimpleAddress().compareTo(o2.getAlias().getSimpleAddress());
          }
        });

        List<Alias> result = new LinkedList<Alias>();
        for (Iterator<Alias> i = tmp.iterator(); i.hasNext() && count > 0;) {
          Alias a = i.next();
          if (--startPos < 0) {
            result.add(a);
            count--;
          }
        }
        System.out.println(start-System.currentTimeMillis());
        return result;
      }

      public int getRowsCount() {
        return aliases.size();
      }
    };
    
  }


  public void setSelectedRows(List selectedRows) {
    this.selectedRows = selectedRows;
  }

  public List getSelectedRows() {
    return selectedRows;
  }

 

  public void removeSelected(ActionEvent e) throws AdminException {
    List<Alias> toDelete = new ArrayList<Alias>();
    if (selectedRows != null && !selectedRows.isEmpty()) {

      AliasSet aset=null;
      try {
          aset = WebContext.getInstance().getAliasManager().getAliases();
          while(aset.next()) {
            Alias alias = aset.get();
            String sa = alias.getAlias().getSimpleAddress();
            for(String rsa : (List<String>)selectedRows) {
              if(rsa.equals(sa)) {
                toDelete.add(alias);
                break;
              }
            }
          }
      }
      catch (AdminException e1) {
        addError(e1);
      }
      finally {
        if(aset!=null) {
          aset.close();
        }
      }
      for(Alias alias : toDelete) {
        WebContext.getInstance().getAliasManager().deleteAlias(alias.getAlias());
      }
    }
  }


  public String edit() {
    return "ALIAS_EDIT";
  }


}
