//
// $Id$
//
// SMSC project
//  Sme Proxy
//  Copyright (c) Aurorisoft.com
//  Author Alexey Chen (hedgehog@aurorisoft.com)
//  Created (2003/07/21)
//

#if !defined PROXY_SME_MIXER_HEADER
#define PROXY_SME_MIXER_HEADER

#include "proxysme.h"
#include "ProxySmeQueue.h"
#include <deque>
#include <stdexcept>
#include <memory>
#include "../smpp/smpp.h"
#include "../sme/SmppBase.hpp"
#include "../core/synchronization/Mutex.hpp"

SMSC_SMEPROXY_BEGIN

using namespace smsc::smpp;
using namespace smsc::sme;
using namespace smsc::core::synchronization;

/// ���������� ������� �� SMPP ������������
class PduListener: public SmppPduEventListener
{
  log4cpp::Category&      log_;
  DIRECTION               incom_dirct_; /// ����������� ��� ������
  Queue&                  que_;         /// ������� ������
public:
  PduListener(DIRECTION,Queue&,log4cpp::Category&);
  virtual ~PduListener();
  virtual void handleEvent(SmppHeader *pdu);
  virtual void handleError(int errorCode);
};

/// ����� ����������� ����� �������� SMPP �������
/// � ����������� ��������������
class Mixer { 
  log4cpp::Category&      log_;
  Queue&                  que_;           /// ������ �������
  PduListener             listen_left_;   /// �������� ������ ������������
  PduListener             listen_right_;  /// �������� ������� ������������
  auto_ptr<SmppSession>   left_;          /// ����� �����������
  auto_ptr<SmppSession>   right_;         /// ������ �����������
public:
  Mixer(Queue& que);
  ~Mixer();
  bool Connect();
  bool Disconnect();
  bool Reconnect();
  /// �������� ����� , ���� ������� �� �������� ������ ������������ ������� 
  bool SendPdu(DIRECTION,SmppHeader** pdu);
private:
  Mixer(const Mixer&);
  Mixer& operator=(const Mixer&);
};

SMSC_SMEPROXY_END

#endif // PROXY_SME_MIXER_HEADER

