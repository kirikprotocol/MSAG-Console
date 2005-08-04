#ifndef	_SMSEvent_H_
#define	_SMSEvent_H_


#include <asn_application.h>

/* Including external dependencies */
#include <EventTypeSMS.h>
#include <MonitorMode.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SMSEvent */
typedef struct SMSEvent {
	EventTypeSMS_t	 eventTypeSMS;
	MonitorMode_t	 monitorMode;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SMSEvent_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SMSEvent;

#ifdef __cplusplus
}
#endif

#endif	/* _SMSEvent_H_ */
