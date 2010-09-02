package ru.novosoft.smsc.admin.cluster_controller;

/**
 * @author Artem Snopkov
 */
public class CCAclInfo {

  private int id;
  private String name;
  private String description;

  public CCAclInfo() {
  }

  public CCAclInfo(int id, String name, String description) {
    this.id = id;
    this.name = name;
    this.description = description;
  }

  public int getId() {
    return id;
  }

  public void setId(int id) {
    this.id = id;
  }

  public String getName() {
    return name;
  }

  public void setName(String name) {
    this.name = name;
  }

  public String getDescription() {
    return description;
  }

  public void setDescription(String description) {
    this.description = description;
  }
}
