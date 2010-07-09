package ru.novosoft.smsc.web;

import java.io.Serializable;
import javax.faces.model.DataModel;

public class DynamicModel extends DataModel implements Serializable {


  public DynamicModel() {
  }

  @Override
  public int getRowCount() {
    return rowCount;
  }

  public void setRowCount(int count) {
    if ((count >= -1) && (count <= 400))
      rowCount = count;
  }

  public int getActualRowCount() {
    return rowCount;
  }

  public void setActualRowCount(int count) {
    if ((count >= 0) && (count <= 400))
      rowCount = count;
  }

  public int getBlockSize() {
    return blockSize;
  }

  public void setBlockSize(int blockSize) {
    if (blockSize >= 0) {
      this.blockSize = blockSize;
    }
  }

  @Override
  public boolean isRowAvailable() {
    return (index >= 0) && (index < rowCount);
  }

  @Override
  public Object getRowData() {
    System.out.println("Get row data with index: " + index);
    return isRowAvailable() ? new Abonent(index) : null;
  }

  @Override
  public int getRowIndex() {
    return index;
  }

  @Override
  public void setRowIndex(int index) {
    System.out.println("Set row index: " + index);
    this.index = index;
  }

  @Override
  public Object getWrappedData() {
    return this;
  }

  @Override
  public void setWrappedData(Object obj) {
    throw new UnsupportedOperationException();
  }

  private int rowCount = 25;
  private int index = -1;
  private int blockSize = 10;


  public static class Abonent implements Serializable {
    private Integer name;
    private Integer status;
    private Integer date;
    private Integer message;

    public Abonent() {
    }

    public Abonent(Integer name) {
      this.name = name;
      this.status = name;
      this.date = name;
      this.message = name;
    }

    public Integer getName() {
      return name;
    }

    public void setName(Integer name) {
      this.name = name;
    }

    public Integer getStatus() {
      return status;
    }

    public void setStatus(Integer status) {
      this.status = status;
    }

    public Integer getDate() {
      return date;
    }

    public void setDate(Integer date) {
      this.date = date;
    }

    public Integer getMessage() {
      return message;
    }

    public void setMessage(Integer message) {
      this.message = message;
    }
  }
}