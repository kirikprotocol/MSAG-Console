#ifndef ___SMPP_SESSION_FOR_TESTS___
#define ___SMPP_SESSION_FOR_TESTS___

#include <sme/SmppBase.hpp>

namespace smsc {
	namespace test {
		namespace smpp {

			using namespace smsc::core::synchronization;
			using namespace smsc::core::network;
			using namespace smsc::smpp;
			using smsc::core::buffers::IntHash;
			using smsc::util::Exception;

			using smsc::sme::SmppPduEventListener;
			using smsc::sme::SmppTransmitter;
			using smsc::sme::SmeConfig;
			using smsc::sme::SmppInvalidBindState;
			using smsc::sme::BindType;
			using smsc::sme::SmppConnectException;
			using smsc::sme::SmppReader;
			using smsc::sme::SmppWriter;

			class SmppSession {
				// логгер
				static log4cpp::Category& log;
			protected:
				struct Lock {
					time_t timeOut;
					Event* event;
					SmppHeader *pdu;
					int error;
				};
				class InnerListener:public SmppPduEventListener {
				public:
					SmppSession& session;
					InnerListener(SmppSession& s):session(s)
					{
					}
					void handleError(int errorCode)
					{
						if (session.listener) {
							session.listener->handleError(errorCode);
						}
					}
					void handleEvent(SmppHeader* pdu)
					{
						session.processIncoming(pdu);
					}
				};
				class InnerSyncTransmitter:public SmppTransmitter {
				public:

					InnerSyncTransmitter(SmppSession& s):session(s)
					{
					}
					virtual ~InnerSyncTransmitter()
					{
					}
					SmppHeader* sendPdu(SmppHeader* pdu)
					{
						if (!session.checkOutgoingValidity(pdu)) {
							throw SmppInvalidBindState();
						}
						Event event;
						int seq=pdu->get_sequenceNumber();
						session.registerPdu(seq,&event);
						session.writer.enqueue(pdu);
						event.Wait(session.cfg.smppTimeOut*1000);
						return session.getPduResponse(seq);
					};

					void sendGenericNack(PduGenericNack& pdu)
					{
						pdu.get_header().set_commandId(SmppCommandSet::GENERIC_NACK);
						//pdu.get_header().set_sequenceNumber(session.getNextSeq());
						session.writer.enqueue((SmppHeader*)&pdu);
					};
					void sendDeliverySmResp(PduDeliverySmResp& pdu)
					{
						pdu.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
						//pdu.get_header().set_sequenceNumber(session.getNextSeq());
						session.writer.enqueue((SmppHeader*)&pdu);
					};
					void sendDataSmResp(PduDataSmResp& pdu)
					{
						pdu.get_header().set_commandId(SmppCommandSet::DATA_SM_RESP);
						//pdu.get_header().set_sequenceNumber(session.getNextSeq());
						session.writer.enqueue((SmppHeader*)&pdu);
					};
					PduSubmitSmResp* submit(PduSubmitSm& pdu)
					{
						pdu.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
						pdu.get_header().set_sequenceNumber(session.getNextSeq());
						return(PduSubmitSmResp*)sendPdu((SmppHeader*)&pdu);
					};
					PduMultiSmResp* submitm(PduMultiSm& pdu)
					{
						pdu.get_header().set_commandId(SmppCommandSet::SUBMIT_MULTI);
						pdu.get_header().set_sequenceNumber(session.getNextSeq());
						return(PduMultiSmResp*)sendPdu((SmppHeader*)&pdu);
					};
					PduDataSmResp* data(PduDataSm& pdu)
					{
						pdu.get_header().set_commandId(SmppCommandSet::DATA_SM);
						pdu.get_header().set_sequenceNumber(session.getNextSeq());
						return(PduDataSmResp*)sendPdu((SmppHeader*)&pdu);
					};
					PduQuerySmResp* query(PduQuerySm& pdu)
					{
						pdu.get_header().set_commandId(SmppCommandSet::QUERY_SM);
						pdu.get_header().set_sequenceNumber(session.getNextSeq());
						return(PduQuerySmResp*)sendPdu((SmppHeader*)&pdu);
					};
					PduCancelSmResp* cancel(PduCancelSm& pdu)
					{
						pdu.get_header().set_commandId(SmppCommandSet::CANCEL_SM);
						pdu.get_header().set_sequenceNumber(session.getNextSeq());
						return(PduCancelSmResp*)sendPdu((SmppHeader*)&pdu);
					};
					PduReplaceSmResp* replace(PduReplaceSm& pdu)
					{
						pdu.get_header().set_commandId(SmppCommandSet::REPLACE_SM);
						pdu.get_header().set_sequenceNumber(session.getNextSeq());
						return(PduReplaceSmResp*)sendPdu((SmppHeader*)&pdu);
					};
				protected:
					SmppSession& session;
				};
				class InnerAsyncTransmitter:public InnerSyncTransmitter {
				public:
					InnerAsyncTransmitter(SmppSession& s):InnerSyncTransmitter(s)
					{
					}
					virtual ~InnerAsyncTransmitter()
					{
					}

					SmppHeader* sendPdu(SmppHeader* pdu)
					{
						if (!session.checkOutgoingValidity(pdu)) {
							throw SmppInvalidBindState();
						}
						int seq=pdu->get_sequenceNumber();
						session.registerPdu(seq,NULL);
						session.writer.enqueue(pdu);
						return NULL;
					};
				};

#ifdef _WIN32
				friend InnerAsyncTransmitter;
				friend InnerSyncTransmitter;
				friend InnerListener;
#endif

			public:
				SmppSession(const SmeConfig& config,SmppPduEventListener* lst):
				cfg(config),
				listener(lst),
				innerListener(*this),
				reader(&innerListener,&socket),
				writer(&innerListener,&socket),
				seqCounter(0),
				strans(*this),
				atrans(*this),
				closed(true)
				{
					__trace2__("SmppSession: CREATE SESSION %p", this);
				}

				~SmppSession()
				{
					close();
				}

				void connect()throw(SmppConnectException)
				{
					__trace2__("SmppSession: CONNECT %p",this);
					if (!closed)return;
					if (socket.Init(cfg.host.c_str(),cfg.port,cfg.timeOut)==-1)
						throw SmppConnectException(SmppConnectException::Reason::networkResolve);
					if (socket.Connect()==-1)
						throw SmppConnectException(SmppConnectException::Reason::networkConnect);
					reader.Start();
					writer.Start();
					__trace2__("SmppSession: CONNECTED %p",this);
					closed=false;
				}

				uint32_t bind(int bindtype) {
					PduBindTRX pdu;
					switch (bindtype) {
					case BindType::Transceiver:
						pdu.get_header().set_commandId(SmppCommandSet::BIND_TRANCIEVER);
						break;
					case BindType::Transmitter:
						pdu.get_header().set_commandId(SmppCommandSet::BIND_TRANSMITTER);
						break;
					case BindType::Receiver:
						pdu.get_header().set_commandId(SmppCommandSet::BIND_RECIEVER);
						break;
					}
					pdu.set_systemId(cfg.sid.c_str());
					pdu.set_password(cfg.password.c_str());
					pdu.set_systemType(cfg.systemType.c_str());
					uint32_t seq=getNextSeq();
					pdu.get_header().set_sequenceNumber(seq);

					SmppHeader *resp=atrans.sendPdu((SmppHeader*)&pdu);

					if(resp != 0) {
						log.error("Error when binding, PduBindTRXResp != NULL");
						disposePdu((SmppHeader*)resp);
					}

					return seq;
				}

				void close()
				{
					__trace2__("SmppSession: CLOSING %p",this);
					if (closed)return;
					reader.Stop();
					writer.Stop();
					socket.Close();
					reader.WaitFor();
					writer.WaitFor();
					closed=true;
				}

				int getNextSeq()
				{
					MutexGuard g(cntMutex);
					return ++seqCounter;
				}

				SmppTransmitter* getSyncTransmitter()
				{
					return &strans;
				}
				SmppTransmitter* getAsyncTransmitter()
				{
					return &atrans;
				}


			protected:
				SmeConfig cfg;
				Socket socket;
				SmppPduEventListener *listener;
				InnerListener innerListener;
				SmppReader reader;
				SmppWriter writer;
				int seqCounter;
				InnerSyncTransmitter strans;
				InnerAsyncTransmitter atrans;
				Mutex cntMutex,lockMutex;
				bool closed;
				//int bindType;

				IntHash<Lock> lock;

				void registerPdu(int seq,Event* event)
				{
					MutexGuard g(lockMutex);
					IntHash<Lock>::Iterator i=lock.First();
					Lock l;
					int key;
					__trace2__("registerPdu seq=%d",seq);
					time_t now=time(NULL);
					__require__(!lock.Exist(seq));
					while (i.Next(key,l)) {
						if (l.timeOut<now) {
							if (!l.pdu)lock.Delete(key);
							if (l.event) {
								l.event->Signal();
							}
						}
					}
					l.timeOut=time(NULL)+cfg.smppTimeOut;
					l.event=event;
					l.pdu=NULL;
					l.error=0;
					lock.Insert(seq,l);
				}
				SmppHeader* getPduResponse(int seq)
				{
					MutexGuard g(lockMutex);
					if (!lock.Exist(seq))return NULL;
					Lock &l=lock.Get(seq);
					if (l.error) {
						lock.Delete(seq);
						throw Exception("Unknown error");
					}
					SmppHeader *retval=l.pdu;
					lock.Delete(seq);
					return retval;
				}
				/*bool checkIncomingValidity(SmppHeader* pdu)
				{
					using namespace SmppCommandSet;
					switch(pdu->get_commandId())
					{
						case BIND_TRANSMITTER_RESP:
						case BIND_RECIEVER_RESP:
						case BIND_TRANCIEVER_RESP:
							return true;
					}
					switch(bindType)
					{
						case BindType::Receiver:
							switch(pdu->get_commandId())
							{
								case DELIVERY_SM:
								case GENERIC_NACK:
								case UNBIND_RESP:
									return true;
								case SUBMIT_SM_RESP:
								case DELIVERY_SM_RESP:
								case SUBMIT_SM:
								case QUERY_SM:
								default:
									return false;
							}
						case BindType::Transmitter:
							switch(pdu->get_commandId())
							{
								case GENERIC_NACK:
								case SUBMIT_SM_RESP:
								case UNBIND_RESP:
									return true;
								case DELIVERY_SM:
								case DELIVERY_SM_RESP:
								case SUBMIT_SM:
								case QUERY_SM:
								default:
									return false;
							}
						case BindType::Transceiver:return true;
					}
					return false;
				}*/
				bool checkIncomingValidity(SmppHeader* pdu)
				{
					return true;
					/*using namespace SmppCommandSet;
					switch (pdu->get_commandId()) {
					case BIND_TRANSMITTER_RESP:
					case BIND_RECIEVER_RESP:
					case BIND_TRANCIEVER_RESP:
					case ENQUIRE_LINK:
					case ENQUIRE_LINK_RESP:
						return true;
					}
					switch (bindType) {
					case BindType::Receiver:
						switch (pdu->get_commandId()) {
						case DATA_SM:
						case DELIVERY_SM:
						case GENERIC_NACK:
						case UNBIND_RESP:
							return true;
						default:
							return false;
						}
					case BindType::Transmitter:
						switch (pdu->get_commandId()) {
						case DATA_SM:
						case DATA_SM_RESP:
						case GENERIC_NACK:
						case SUBMIT_SM_RESP:
						case SUBMIT_MULTI_RESP:
						case UNBIND_RESP:
						case CANCEL_SM_RESP:
						case QUERY_SM_RESP:
						case REPLACE_SM_RESP:
						case UNBIND:
							return true;
						default:
							return false;
						}
					case BindType::Transceiver:
						switch (pdu->get_commandId()) {
						case DATA_SM:
						case DATA_SM_RESP:
						case GENERIC_NACK:
						case SUBMIT_SM_RESP:
						case SUBMIT_MULTI_RESP:
						case UNBIND_RESP:
						case CANCEL_SM_RESP:
						case QUERY_SM_RESP:
						case REPLACE_SM_RESP:
						case UNBIND:
						case DELIVERY_SM:
							return true;
						default:
							return false;
						}
					}
					return false;*/
				}

				bool checkOutgoingValidity(SmppHeader* pdu)
				{
					return true;
					/*using namespace SmppCommandSet;
					switch (pdu->get_commandId()) {
					case BIND_TRANSMITTER:
					case BIND_RECIEVER:
					case BIND_TRANCIEVER:
					case ENQUIRE_LINK:
					case ENQUIRE_LINK_RESP:
						return true;
					}
					switch (bindType) {
					case BindType::Receiver:
						switch (pdu->get_commandId()) {
						case UNBIND:
						case UNBIND_RESP:
						case DELIVERY_SM_RESP:
						case GENERIC_NACK:
							return true;
						default:
							return false;
						}
					case BindType::Transmitter:
						switch (pdu->get_commandId()) {
						case DATA_SM:
						case DATA_SM_RESP:
						case SUBMIT_SM:
						case SUBMIT_MULTI:
						case GENERIC_NACK:
						case CANCEL_SM:
						case REPLACE_SM:
						case QUERY_SM:
						case UNBIND:
						case UNBIND_RESP:
							return true;
						default:
							return false;
						}
					case BindType::Transceiver:
						switch (pdu->get_commandId()) {
						case DATA_SM:
						case DATA_SM_RESP:
						case SUBMIT_SM:
						case SUBMIT_MULTI:
						case GENERIC_NACK:
						case CANCEL_SM:
						case REPLACE_SM:
						case QUERY_SM:
						case UNBIND:
						case UNBIND_RESP:
						case DELIVERY_SM_RESP:
							return true;
						default:
							return false;
						}
					}
					return false;*/
				}


				void processIncoming(SmppHeader* pdu)
				{
					log.debug("processIncoming for %p", this);
					using namespace smsc::smpp::SmppCommandSet;
					/*if (!checkIncomingValidity(pdu)) {
						__warning2__("processIncoming: received pdu in invalid bind state (%x,%d)",pdu->get_commandId(),bindType);
						PduGenericNack gnack;
						gnack.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
						atrans.sendGenericNack(gnack);
						return;
					}*/
					lockMutex.Lock();
					int seq=pdu->get_sequenceNumber();
					log.debug("SequenceNumber = %i", seq);
					uint32_t commandId = pdu->get_commandId();
					log.debug("CommandId = %X", commandId);
					if(commandId == ENQUIRE_LINK_RESP) {
						log.debug("EnquireLink response has been received !!!");
					}
					switch (pdu->get_commandId()) {
					//отдельно отработаем всякие разные респонсы.
					case GENERIC_NACK://облом однако.
						{
							if (lock.Exist(seq)) {
								Lock &l=lock.Get(seq);
								if (l.event) {
									lockMutex.Unlock();
									l.error=1;
									l.event->Signal();
									disposePdu(pdu);
								} else {
									lock.Delete(seq);
									lockMutex.Unlock();
									listener->handleEvent(pdu);
								}
							} else {
								disposePdu(pdu);
								lockMutex.Unlock();
							}
						}break;
						/*case ENQUIRE_LINK_RESP:
						{
							lockMutex.Unlock();
							disposePdu(pdu);
						}break;*/
					case ENQUIRE_LINK:
						{
							lockMutex.Unlock();
							PduEnquireLinkResp resp;
							resp.get_header().set_commandId(ENQUIRE_LINK_RESP);
							resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
							resp.get_header().set_commandStatus(0);
							writer.enqueue((SmppHeader*)&resp);
							disposePdu(pdu);
						}break;
					case SUBMIT_SM_RESP:
					case SUBMIT_MULTI_RESP:
					case DATA_SM_RESP:
					case QUERY_SM_RESP:
					case CANCEL_SM_RESP:
					case REPLACE_SM_RESP:
					case BIND_TRANCIEVER_RESP:
					case BIND_TRANSMITTER_RESP:
					case BIND_RECIEVER_RESP:
					case UNBIND_RESP:
					case ENQUIRE_LINK_RESP:
						{
							if (lock.Exist(seq)) {
								__trace2__("processIncoming: lock for %d found",seq);
								Lock &l=lock.Get(seq);
								if (l.event) {
									lockMutex.Unlock();
									l.pdu=pdu;
									l.event->Signal();
								} else {
									lock.Delete(seq);
									lockMutex.Unlock();
									listener->handleEvent(pdu);
								}
							} else {
								__trace2__("processIncoming: lock for %d not found!!!",seq);
								lockMutex.Unlock();
								disposePdu(pdu);
							}
						}break;
						// и отдельно собственно пришедшие smpp-шки.
					case DELIVERY_SM:
					case DATA_SM:
					case ALERT_NOTIFICATION:
						{
							lockMutex.Unlock();
							listener->handleEvent(pdu);
						}break;
					default:
						{
							lockMutex.Unlock();
						}
					}
				}
			};
		}
	}
}
#endif //___SMPP_SESSION_FOR_TESTS___
