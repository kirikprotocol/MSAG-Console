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

			// handler
			template <typename T, template <class> class CheckingPolicy=Exceptioner>
			class Handler;

			template <typename T, template <class> class CheckingPolicy>
			class Handler : public CheckingPolicy<T> {
			private:
				T* objectPtr;
				int* referenceCount;
				void release() {
					if (referenceCount != 0) {
						--(*referenceCount);
						if ((*referenceCount) == 0 && objectPtr != 0) {
							delete objectPtr;
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

				Handler(const Handler<T>& handler) {
					if (handler != 0) {
						objectPtr = handler.objectPtr;
						referenceCount = handler.referenceCount;
						++(*referenceCount);
					} else {
						objectPtr = 0;
						referenceCount = 0;
					}
				}

				Handler& operator = (const Handler<T>& handler) {
					if (objectPtr != handler.objectPtr) {
						release();
						objectPtr = handler.objectPtr;
						referenceCount = handler.referenceCount;
						++(*referenceCount);
					}
					return *this;
				}

				Handler& operator = (T* objectPtr) {
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

				friend bool operator==(const Handler<T>& handler, const T* objectPtr) {
					return handler.objectPtr == objectPtr;
				}

				friend bool operator==(const T* objectPtr, const Handler<T>& handler) {
					return handler.objectPtr == objectPtr;
				}

				friend bool operator!=(const Handler<T>& handler, const T* objectPtr) {
					return handler.objectPtr != objectPtr;
				}

				friend bool operator!=(const T* objectPtr, const Handler<T>& handler) {
					return handler.objectPtr != objectPtr;
				}

				friend bool operator==(const Handler<T>& handler1, const Handler<T>& handler2) {
					return handler1.objectPtr == handler2.objectPtr;
				}

				friend bool operator!=(const Handler<T>& handler1, const Handler<T>& handler2) {
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

				~Handler() {
					release();
				}
			};


		}
	}
}

#endif //___HANDLER_FOR_POINTERS___
