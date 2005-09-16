package xml;

//{{{ Imports
import javax.swing.table.*;
import java.awt.Component;
import org.gjt.sp.jedit.*;
import org.gjt.sp.jedit.gui.KeyEventWorkaround;
import org.gjt.sp.jedit.textarea.JEditTextArea;
import xml.completion.ElementDecl;
import xml.completion.IDDecl;
import xml.completion.XmlAttributeCompletion;
import xml.completion.XmlAttributeCompletionPopup;

import javax.swing.*;
import java.util.*;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;

import sidekick.SideKickCompletion;
//}}}
/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 13.09.2005
 * Time: 17:22:11
 * To change this template use File | Settings | File Templates.
 */





class EditTag
{
 //{{{ EditTagDialog constructor
 /**
  * elementName might not equal element.name due to case insensitivity
  * in HTML files.
  */
 EditTag(View view, String elementName, ElementDecl element,
  Map attributeValues, Map entityHash,
  List ids, boolean html)
 {
  //super(view, jEdit.getProperty("xml-edit-tag.title"),true);

  this.elementName = elementName;
  this.element = element;
  this.entityHash = entityHash;
  this.html = html;
  this.view=view;


  //{{{ Attribute table
  attributeModel = createAttributeModel(element.attributes,
   attributeValues,ids);
  //{{{ Preview field

  preview = new JTextArea(5,5);
  preview.setLineWrap(true);
  preview.setWrapStyleWord(true);
  preview.setEditable(false);

  //{{{ Buttons

  names=prepareTag();
  //updateTag();

 } //}}}


 //{{{ cancel() method
 public void cancel()
 {
  isOK = false;
 } //}}}

 //{{{ getNewTag() method
 public String getNewTag()
 {
  return (isOK ? newTag : null);
 } //}}}


 //{{{ Private members

 //{{{ Instance variables
 private boolean html;
 private String elementName;
 private ElementDecl element;
 private Map entityHash;
 private List attributeModel;
 private JTextArea preview;
 private String newTag;
 private boolean isOK;
 private View view;
 private ArrayList names;
  //}}}

 //{{{ createAttributeModel() method
 private ArrayList createAttributeModel(List declaredAttributes,
  Map attributeValues, List ids)
 {
  ArrayList stringIDs = new ArrayList(ids.size());
  for(int i = 0; i < ids.size(); i++)
  {
   stringIDs.add(((IDDecl)ids.get(i)).id);
  }

  ArrayList attributeModel = new ArrayList();
  for(int i = 0; i < declaredAttributes.size(); i++)
  {
   ElementDecl.AttributeDecl attr =
    (ElementDecl.AttributeDecl)
    declaredAttributes.get(i);

   boolean set;
   String value = (String)attributeValues.get(attr.name);
   if(value == null)
   {
    set = false;
    value = attr.value;
   }
   else
    set = true;

   if(attr.required)
    set = true;

   ArrayList values;
   if(attr.type.equals("IDREF")
    && stringIDs.size() > 0)
   {
    values = stringIDs;
    if(value == null)
     value = (String)stringIDs.get(0);
   }
   else
   {
    values = attr.values;
    if(value == null && values != null
     && values.size() > 0)
     value = (String)values.get(0);
   }

   attributeModel.add(new Attribute(set,attr.name,
    value,values,attr.type,attr.required));
  }

  Collections.sort(attributeModel,new AttributeCompare());

  return attributeModel;
 } //}}}

  public List getAttributeModel()
  {
    return attributeModel;
  }

  public ArrayList getNames()
  {
    return names;
  }

  private static XmlAttributeCompletionPopup popup;
  //{{{ prepareTag() method
 private ArrayList prepareTag()
 {
  int tagNameCase = TextUtilities.getStringCase(elementName);
  StringBuffer buf = new StringBuffer("<");
  buf.append(elementName);
  ArrayList names=new ArrayList(5);
    for(int i = 0; i < attributeModel.size(); i++)
  {
   Attribute attr = (Attribute)attributeModel.get(i);
   if(!attr.set) { //nameSet.put(attr.name,new Boolean(attr.set));
     names.add(attr.name);
    System.out.println("name of attribute  = "+attr.name+" set= "+attr.set);
    continue;
   }
   String attrName = attr.name;
   if(html)
   {
    switch(tagNameCase)
    {
    case TextUtilities.UPPER_CASE:
     attrName = attr.name.toUpperCase();
     break;
    case TextUtilities.LOWER_CASE:
     attrName = attr.name.toLowerCase();
     break;
    case TextUtilities.TITLE_CASE:
     attrName = TextUtilities.toTitleCase(
      attr.name);
     break;
    }
   }

   buf.append(' ');
   buf.append(attrName);

   if(html && attr.name.equals(attr.value.value))
   {
    continue;
   }

   buf.append("=\"");
   if(attr.value.value != null)
   {
    buf.append(XmlActions.charactersToEntities(
     attr.value.value,entityHash));
   }
   buf.append("\" ");
  }
   System.out.println("EditTag buf= "+buf.toString());
  view.setNames(names);
  //buf.append(">");
  isOK=true;
  newTag = buf.toString();
  return names;
  //preview.setText(newTag);
 } //}}}

 //}}}

  public void updateTag()
  {
    JEditTextArea textArea = view.getTextArea();
/*    for(int i = 0; i < attributeModel.size(); i++)
 {
  Attribute attr = (Attribute)attributeModel.get(i);
   System.out.println("name of attribute 1 = "+attr.name+" set= "+attr.set);
   if(attr.set)
     continue;
  */
     // show the popup if
   // - complete has one element and user invoked with delay key
   // - or complete has multiple elements
   // and popup is not already shown because of explicit invocation
   // of the complete action during the trigger delay
   if(popup != null)
    return;
   SideKickCompletion complete=new XmlAttributeCompletion(view,"",names);

   popup = new XmlAttributeCompletionPopup(view,
           textArea.getCaretPosition(),complete )  //complete
   {
    /** forget reference to this popup when it is disposed */
    public void dispose()
    {

      super.dispose();
      popup = null;
      view.setKeyEventInterceptor(new KeyHandler());
      //
    }
   };

//   String add=popup.getUpdateTag();
 //  System.out.println("add= "+add);
  // if (add!=null) newTag = newTag+add;


  // }
    //newTag=newTag+">";
  }
 //{{{ Inner classes
//{{{ KeyHandler class
 class KeyHandler extends KeyAdapter
 {
  //{{{ keyPressed() method
  public void keyPressed(KeyEvent evt)
  {
    JEditTextArea textArea = view.getTextArea();
    evt = KeyEventWorkaround.processKeyEvent(evt);
   if(evt == null)
    return;

   switch(evt.getKeyCode())
   {
   case KeyEvent.VK_ENTER:
   evt=null;
    break;
   case KeyEvent.VK_TAB:
    break;
   case KeyEvent.VK_ESCAPE:
    view.setKeyEventInterceptor(null);
    evt.consume();
    break;
     case KeyEvent.VK_LEFT:
     int caret=textArea.getCaretPosition();
     textArea.setCaretPosition(caret-1);
     evt.consume();
         break;

    case KeyEvent.VK_RIGHT:
     caret=textArea.getCaretPosition();
     textArea.setCaretPosition(caret+1);
     evt.consume();
         break;
    case KeyEvent.VK_UP:
     view.setKeyEventInterceptor(null);
     evt.consume();
    break;
   case KeyEvent.VK_DOWN:
     view.setKeyEventInterceptor(null);
     evt.consume();
    break;
   case KeyEvent.VK_SPACE:
     keyTyped(' ');
     evt.consume();
     break;
   case KeyEvent.VK_BACK_SPACE:
   case KeyEvent.VK_DELETE:
     caret=textArea.getCaretPosition();
     textArea.setCaretPosition(caret-1);
     textArea.setText(" ");
     caret=textArea.getCaretPosition();
     textArea.setCaretPosition(caret-1);
     view.processKeyEvent(evt,true);
    break;
   default:
    // from DefaultInputHandler
    if(!(evt.isControlDown() || evt.isAltDown() || evt.isMetaDown()))
    {
     if(!evt.isActionKey())
     {
      break;
     }
    }

   // view.processKeyEvent(evt,true);
    break;
   }
  } //}}}

  //{{{ keyTyped() method
  public void keyTyped(KeyEvent evt)
  {
   evt = KeyEventWorkaround.processKeyEvent(evt);
   if(evt == null)
    return;

   char ch = evt.getKeyChar();
   if(ch == '\b')
    return;

   keyTyped(ch);

   evt.consume();
  } //}}}

  //{{{ keyTyped() method
  private void keyTyped(char ch)
  {
  JEditTextArea textArea = view.getTextArea();
   if(names.size() == 0)
   {
    textArea.userInput(ch);
    view.setKeyEventInterceptor(null);
   }
   else  if(ch==' ')
   {
     textArea.userInput(ch);
     SideKickCompletion complete=new XmlAttributeCompletion(view,"",view.getNames());
    popup = new XmlAttributeCompletionPopup(view,
           textArea.getCaretPosition(),complete )  //complete
   {
    /** forget reference to this popup when it is disposed */
    public void dispose()
    {

      super.dispose();
      view.setKeyEventInterceptor(new KeyHandler());
      popup = null;
    }
   };
   }
   else  textArea.userInput(ch);//view.setKeyEventInterceptor(null);

  } //}}}
 } //}}}
 //{{{ ActionHandler class
 class ActionHandler implements ActionListener
 {
  public void actionPerformed(ActionEvent evt)
  {
 /*  if(evt.getSource() == empty)
    prepareTag();
   else if(evt.getSource() == ok)
    ok();
   else if(evt.getSource() == cancel)
    cancel();  */
  }
 } //}}}

 //{{{ Attribute class
 static class Attribute
 {
  //{{{ Instance variables
  boolean set;

  String name;
  Value value;
  String type;
  boolean required;
  //}}}

  //{{{ Attribute constructor
  Attribute(boolean set, String name,
   String value, ArrayList values,
   String type, boolean required)
  {
   this.set = set;
   this.name = name;
   this.value = new Value(value,values);
   this.type = type;
   this.required = required;
  } //}}}

  //{{{ Value class
  static class Value
  {
   String value;
   ArrayList values;

   Value(String value, ArrayList values)
   {
    this.value = value;
    this.values = values;
   }

   public String toString()
   {
    return value;
   }
  } //}}}
 } //}}}

 //{{{ AttributeCompare class
 static class AttributeCompare implements MiscUtilities.Compare
 {
  public int compare(Object obj1, Object obj2)
  {
   Attribute attr1 = (Attribute)obj1;
   Attribute attr2 = (Attribute)obj2;

   // put required attributes at the top
   if(attr1.required && !attr2.required)
    return -1;
   else if(!attr1.required && attr2.required)
    return 1;
   else
   {
    return MiscUtilities.compareStrings(
     attr1.name,attr2.name,true);
   }
  }
 } //}}}

 static ComboValueRenderer comboRenderer = new ComboValueRenderer();

 //{{{ AttributeTable class
 class AttributeTable extends JTable
 {
  //{{{ getCellEditor() method
  public TableCellEditor getCellEditor(int row, int column)
  {
   Object value = getModel().getValueAt(row,column);
   if(value instanceof Attribute.Value)
    return comboRenderer;

   return super.getCellEditor(row,column);
  } //}}}

  //{{{ getCellRenderer() method
  public TableCellRenderer getCellRenderer(int row, int column)
  {
   Object value = getModel().getValueAt(row,column);
   if(value instanceof Attribute.Value)
    return comboRenderer;

   return super.getCellRenderer(row,column);
  } //}}}
 } //}}}

 //{{{ AttributeTableModel class
 class AttributeTableModel extends AbstractTableModel
 {
  //{{{ getColumnCount() method
  public int getColumnCount()
  {
   return 4;
  } //}}}

  //{{{ getRowCount() method
  public int getRowCount()
  {
   return attributeModel.size();
  } //}}}

  //{{{ getColumnClass() method
  public Class getColumnClass(int col)
  {
   if(col == 0)
    return Boolean.class;
   else
    return String.class;
  } //}}}

  //{{{ getColumnName() method
  public String getColumnName(int col)
  {
   switch(col)
   {
   case 0:
    return jEdit.getProperty("xml-edit-tag.set");
   case 1:
    return jEdit.getProperty("xml-edit-tag.attribute");
   case 2:
    return jEdit.getProperty("xml-edit-tag.type");
   case 3:
    return jEdit.getProperty("xml-edit-tag.value");
   default:
    throw new InternalError();
   }
  } //}}}

  //{{{ isCellEditable() method
  public boolean isCellEditable(int row, int col)
  {
   if(col != 1 && col != 2)
    return true;
   else
    return false;
  } //}}}

  //{{{ getValueAt() method
  public Object getValueAt(int row, int col)
  {
   Attribute attr = (Attribute)attributeModel.get(row);
   switch(col)
   {
   case 0:
    return new Boolean(attr.set);
   case 1:
    return attr.name;
   case 2:
    if(attr.required)
    {
     if(attr.type.startsWith("("))
      return jEdit.getProperty("xml-edit-tag.required");
     else
      return attr.type + ", " + jEdit.getProperty("xml-edit-tag.required");
    }
    else if(attr.type.startsWith("("))
     return "";
    else
     return attr.type;
   case 3:
    if(attr.value.values != null)
     return attr.value;
    else
     return attr.value.value;
   default:
    throw new InternalError();
   }
  } //}}}

  //{{{ setValueAt() method
  public void setValueAt(Object value, int row, int col)
  {
   Attribute attr = (Attribute)attributeModel.get(row);
   switch(col)
   {
   case 0:
    if(attr.required)
     return;

    attr.set = ((Boolean)value).booleanValue();
    break;
   case 3:
    String sValue;
    if(value instanceof IDDecl)
     sValue = ((IDDecl)value).id;
    else
     sValue = value.toString();
    if(equal(attr.value.value,sValue))
     return;

    attr.set = true;
    attr.value.value = sValue;
    break;
   }

   fireTableRowsUpdated(row,row);

   prepareTag();
  } //}}}

  //{{{ equal() method
  private boolean equal(String str1, String str2)
  {
   if(str1 == null || str1.length() == 0)
   {
    if(str2 == null || str2.length() == 0)
     return true;
    else
     return false;
   }
   else
   {
    if(str2 == null)
     return false;
    else
     return str1.equals(str2);
   }
  } //}}}
 } //}}}

 //{{{ ComboValueRenderer class
 static class ComboValueRenderer extends DefaultCellEditor
  implements TableCellRenderer
 {
  JComboBox editorCombo;
  JComboBox renderCombo;

  //{{{ ComboValueRenderer constructor
  ComboValueRenderer()
  {
   this(new JComboBox());
  } //}}}

  //{{{ ComboValueRenderer constructor
  // this is stupid. why can't you reference instance vars
  // in a super() invocation?
  ComboValueRenderer(JComboBox comboBox)
  {
   super(comboBox);
   this.editorCombo = comboBox;
   editorCombo.setEditable(true);
   this.renderCombo = new JComboBox();
   renderCombo.setEditable(true);
  } //}}}

  //{{{ getTableCellEditorComponent() method
  public Component getTableCellEditorComponent(JTable table,
   Object value, boolean isSelected, int row, int column)
  {
   Attribute.Value _value = (Attribute.Value)value;
   editorCombo.setModel(new DefaultComboBoxModel(
    _value.values.toArray()));
   return super.getTableCellEditorComponent(table,
    _value.value,isSelected,row,column);
  } //}}}

  //{{{ getTableCellRendererComponent() method
  public Component getTableCellRendererComponent(JTable table,
   Object value, boolean isSelected, boolean hasFocus,
   int row, int column)
  {
   Attribute.Value _value = (Attribute.Value)value;
   renderCombo.setModel(new DefaultComboBoxModel(
    _value.values.toArray()));
   renderCombo.setSelectedItem(_value.value);
   return renderCombo;
  } //}}}
 } //}}}

 //}}}
}

