#ifndef IO_DISPATCHER_LINKSET_HPP_HEADER_INCLUDED_B87B6FEC
# define IO_DISPATCHER_LINKSET_HPP_HEADER_INCLUDED_B87B6FEC

# include <list>
# include <sua/communication/LinkId.hpp>
# include <sua/communication/Message.hpp>
# include <sua/communication/TP.hpp>

# include <sua/sua_layer/io_dispatcher/Connection.hpp>
# include <sua/sua_layer/io_dispatcher/Link.hpp>
# include <sua/utilx/Iterator.hpp>
# include <logger/Logger.h>

namespace io_dispatcher {

//##Documentation
//## ����� ��������� ������������� ��������� (��������� ������) �� �����������,
//## �������� � LinkSet, � ������������ � �������� ��� ���� ���������. �����
//## ����� ��������� ��������� �� ����� �� ����������, �������� � �����.
class LinkSet : public Connection {
public:
  typedef enum {UNSPECIFIED=0, OVERRIDE=1, LOADSHARE=2, BROADCAST=3} linkset_mode_t;

  LinkSet(linkset_mode_t mode, const communication::LinkId& linkSetId);

  void addConnection(Link* connect);

  void removeConnection(Link* connect);

  virtual void send(const communication::Message& message);

  virtual communication::TP* receive();

  virtual bool hasReadyTransportPacket();

  virtual void bufferInputTransportPackets();

  class ConnectionIterator;
  ConnectionIterator getIterator();

  static const char* getLinksetModeStringPresentation(linkset_mode_t mode);
private:
  linkset_mode_t _mode;
  typedef std::list<Connection*> connections_lst_t;
  connections_lst_t _connections;
  connections_lst_t::iterator _lastUsedConnIter;
  bool _wasConnIterInitilized;

  smsc::logger::Logger* _logger;

public:
  class ConnectionIterator : public utilx::Iterator<Connection*> {
  public:
    virtual bool hasElement() const;
    virtual void next();
    virtual const_ref getCurrentElement() const;
    virtual ref getCurrentElement();
    virtual void deleteCurrentElement();
  private:
    ConnectionIterator(connections_lst_t& container);
    friend class LinkSet;
    //friend LinkSetIterator LinkSet::getIterator();
    connections_lst_t& _container;
    connections_lst_t::iterator _iter, _endIter;
  };
};

} // namespace io_dispatcher


#endif /* IO_DISPATCHER_LINKSET_HPP_HEADER_INCLUDED_B87B6FEC */
