#ifndef ___HANDLER_FOR_POINTERS___
#define ___HANDLER_FOR_POINTERS___

#include <stdexcept>

/*
 *  Утилитный класс Handler предназначен для замены простых указателей.
 *  Handler ведет подсчет ссылок на динамически распределеныый объект
 *  и автоматически освобождает память, когда таких ссылок не остается.
 */

namespace smsc {
	namespace test {
		namespace util {

			// null pointer exception
			struct NullPointerException : public std::runtime_error {
				NullPointerException() : std::runtime_error("Null Pointer Exception") {
				}
			};

			// checks nothing
			template <class P> class NoCheck {
			public:
				P* checkPointer(P* ptr) const {
					return ptr;
				}
			};

			// throws NullPointerException
			template <class P> class Exceptioner {
			public:
				P* checkPointer(P* ptr) const {
					if (ptr == 0) {
						throw NullPointerException();
					}
					return ptr;
				}
			};

			// usual dispose policy
			template <class P> class UsualDeleter {
			public:
			  void dispose(P* object) {
				delete object;
			  }
			};
			
			// handler
			template 
			<
			  typename T, 
			  template <typename> class CheckingPolicy = Exceptioner, 
			  template <typename> class DisposePolicy = UsualDeleter
			>
			class Handler;

			template 
			<
			  typename T, 
			  template <typename> class CheckingPolicy, 
			  template <typename> class DisposePolicy
			>
			class Handler : public CheckingPolicy<T>, public DisposePolicy<T> {
			private:
				T* objectPtr;
				int* referenceCount;
				void release() {
					if (referenceCount != 0) {
						--(*referenceCount);
						if ((*referenceCount) == 0 && objectPtr != 0) {
							dispose(objectPtr);
							delete referenceCount;
						}
					}
				}
			public:
				Handler() : objectPtr(0), referenceCount(0) {
				}

				Handler(T* objectPtr) {
					this->objectPtr = objectPtr;
					if (objectPtr != 0) {
						referenceCount = new int(1);
					} else {
						referenceCount = 0;
					}
				}

				Handler(const Handler<T, CheckingPolicy, DisposePolicy> &handler) {
					if (handler.objectPtr != 0) {
						objectPtr = handler.objectPtr;
						referenceCount = handler.referenceCount;
						++(*referenceCount);
					} else {
						objectPtr = 0;
						referenceCount = 0;
					}
				}

				template <typename T1>
				Handler(Handler<T1, CheckingPolicy, DisposePolicy> &handler) {
					if (handler.objectPtr != 0) {
						objectPtr = static_cast<T*>(handler.getObjectPtr());
						referenceCount = handler.getReferenceCountPtr();
						++(*referenceCount);
					} else {
						objectPtr = 0;
						referenceCount = 0;
					}
				}

				template <typename T1>
				Handler<T, CheckingPolicy, DisposePolicy>& operator = (Handler<T1, CheckingPolicy, DisposePolicy> &handler) {
					if (objectPtr != handler.getObjectPtr()) {
						release();
						if(handler != 0) {
						  objectPtr = static_cast<T*>(handler.getObjectPtr());
						  referenceCount = handler.getReferenceCountPtr();
						  ++(*referenceCount);
						} else {
						  objectPtr = 0;
						  referenceCount = 0;
						}
					}
					return *this;
				}

				Handler<T, CheckingPolicy, DisposePolicy>& operator = (const Handler<T, CheckingPolicy, DisposePolicy> &handler) {
					if (objectPtr != handler.objectPtr) {
						release();
						if(handler != 0) {
						  objectPtr = handler.objectPtr;
						  referenceCount = handler.referenceCount;
						  ++(*referenceCount);
						} else {
						  objectPtr = 0;
						  referenceCount = 0;
						}
					}
					return *this;
				}

				Handler<T, CheckingPolicy, DisposePolicy>& operator = (T* objectPtr) {
					if (this->objectPtr != objectPtr) {
						release();
						this->objectPtr = objectPtr;
						if (objectPtr != 0) {
							referenceCount = new int(1);
						} else {
							referenceCount = 0;
						}
					}
					return *this;
				}

				/*friend bool operator==(const Handler<T>& handler, const int addr) {
					return handler.objectPtr == addr;
				}

				friend bool operator==(const int addr, const Handler<T>& handler) {
					return handler.objectPtr == addr;
				}*/

				friend bool operator==(const Handler<T, CheckingPolicy, DisposePolicy>& handler, const T* objectPtr) {
					return handler.objectPtr == objectPtr;
				}

				friend bool operator==(const T* objectPtr, const Handler<T, CheckingPolicy, DisposePolicy>& handler) {
					return handler.objectPtr == objectPtr;
				}

				friend bool operator!=(const Handler<T, CheckingPolicy, DisposePolicy>& handler, const T* objectPtr) {
					return handler.objectPtr != objectPtr;
				}

				friend bool operator!=(const T* objectPtr, const Handler<T, CheckingPolicy, DisposePolicy>& handler) {
					return handler.objectPtr != objectPtr;
				}

				friend bool operator==(const Handler<T, CheckingPolicy, DisposePolicy>& handler1, const Handler<T, CheckingPolicy, DisposePolicy>& handler2) {
					return handler1.objectPtr == handler2.objectPtr;
				}

				friend bool operator!=(const Handler<T, CheckingPolicy, DisposePolicy>& handler1, const Handler<T, CheckingPolicy, DisposePolicy>& handler2) {
					return handler1.objectPtr != handler2.objectPtr;
				}

				/*T* operator -> () {
						return checkPointer(objectPtr);
				}*/

				T* operator -> () const {
					return checkPointer(objectPtr);
				}

				T& operator * () const {
					return *checkPointer(objectPtr);
				}

				/*operator T& () const {
					return *checkPointer(objectPtr);
				}*/

				T* getObjectPtr() {
				  return objectPtr;
				}

				int* getReferenceCountPtr() {
				  return referenceCount;
				}

				~Handler() {
					release();
				}
			};


		}
	}
}

#endif //___HANDLER_FOR_POINTERS___
