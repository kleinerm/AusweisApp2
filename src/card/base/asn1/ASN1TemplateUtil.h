/*!
 * \brief Utility template functions for encoding and decoding of ASN.1 types
 *
 * \copyright Copyright (c) 2015-2018 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include <openssl/asn1t.h>
#include <openssl/err.h>

#include <QByteArray>
#include <QLoggingCategory>
#include <QSharedPointer>

Q_DECLARE_LOGGING_CATEGORY(card)

namespace governikus
{

QByteArray getOpenSslError();

/*!
 * Default template function for creating an OpenSSL type. This must be specialized for each ASN.1 type.
 */
template<typename T>
T* newAsn1Object()
{
	static_assert(std::is_void<T>::value, "Implement specialization of newAsn1Object");
	return 0;
}


/*!
 * Template function for creating a SharedPointer holding an ASN.1 OpenSSL type.
 */
template<typename T>
QSharedPointer<T> newObject(T* pObject = newAsn1Object<T>())
{
	static auto deleter = [](T* pObjectToDelete)
			{
				freeAsn1Object(pObjectToDelete);
			};
	return QSharedPointer<T>(pObject, deleter);
}


/*!
 * Default template function for encoding an OpenSSL type. This must be specialized for each ASN.1 type.
 */
template<typename T>
int encodeAsn1Object(T*, unsigned char**)
{
	static_assert(std::is_void<T>::value, "Implement specialization of encodeObject");
	return 0;
}


/*!
 * Template function for encoding an OpenSSL type as DER encoded QByteArray.
 */
template<typename T>
QByteArray encodeObject(T* pObject)
{
	ERR_clear_error();
	unsigned char* encoded = nullptr;
	const int length = encodeAsn1Object(pObject, &encoded);
	if (length < 0)
	{
		qCWarning(card) << "Cannot encode ASN.1 object:" << getOpenSslError();
		return QByteArray();
	}

	return QByteArray(reinterpret_cast<char*>(encoded), length);
}


/*!
 * Default template function for decoding an OpenSSL type. This must be specialized for each ASN.1 type.
 */
template<typename T>
T* decodeAsn1Object(T**, const unsigned char**, long)
{
	static_assert(std::is_void<T>::value, "Implement specialization of decodeObject");
	return 0;
}


/*!
 * Default template function for freeing an OpenSSL type. This must be specialized for each ASN.1 type.
 */
template<typename T>
void freeAsn1Object(T*)
{
	static_assert(std::is_void<T>::value, "Implement specialization of freeObject");
}


/*!
 * Template function for decoding an OpenSSL type from DER encoded QByteArray.
 */
template<typename T>
QSharedPointer<T> decodeObject(const QByteArray& pData, bool pLogging = true)
{
	ERR_clear_error();
	const char* tmp = pData.constData();
	const unsigned char** dataPointer = reinterpret_cast<unsigned const char**>(&tmp);

	T* object = nullptr;
	if (!decodeAsn1Object(&object, dataPointer, pData.length()) && pLogging)
	{
		qCWarning(card) << "Cannot decode ASN.1 object:" << getOpenSslError();
	}

	static auto deleter = [](T* pTypeObject)
			{
				freeAsn1Object(pTypeObject);
			};
	return QSharedPointer<T>(object, deleter);
}


/*!
 * Return value of ASN.1 callback functions indicating success.
 */
static const int CB_SUCCESS = 1;


/*!
 * Return value of ASN.1 callback functions indicating an error.
 */
static const int CB_ERROR = 0;


#define IMPLEMENT_ASN1_OBJECT(name)\
	template<>\
	name * newAsn1Object<name>()\
	{\
		return name##_new();\
	}\
\
	template<>\
	int encodeAsn1Object<name>(name * pObject, unsigned char** encoded)\
	{\
		return i2d_##name(pObject, encoded);\
	}\
\
	template<>\
	name * decodeAsn1Object<name>(name** pObject, const unsigned char** pData, long pDataLen)\
	{\
		return d2i_##name(pObject, pData, pDataLen);\
	}\
\
	template<>\
	void freeAsn1Object<name>(name * pObject)\
	{\
		name##_free(pObject);\
	}

#define DECLARE_ASN1_OBJECT(name)\
	template<> name * newAsn1Object<name>();\
	template<> int encodeAsn1Object<name>(name * pObject, unsigned char** encoded);\
	template<> name * decodeAsn1Object<name>(name** pObject, const unsigned char** pData, long pDataLen);\
	template<> void freeAsn1Object<name>(name * pObject);


} // namespace governikus
