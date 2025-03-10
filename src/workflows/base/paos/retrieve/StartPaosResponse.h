/*!
 * \brief Class represents the retrieved PAOS StartPaosResponse
 *
 * \copyright Copyright (c) 2014-2023 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "paos/ElementDetector.h"
#include "paos/ResponseType.h"

#include <QByteArray>
#include <QString>

namespace governikus
{

class StartPaosResponse
	: public ResponseType
	, private ElementDetector
{
	private:
		QString mResultMajor;
		QString mResultMinor;
		QString mResultMessage;
		int mRemainingDays;
		int mRemainingAttempts;
		QString mBlockingCode;

	public:
		explicit StartPaosResponse(const QByteArray& pXmlData);

		[[nodiscard]] int getRemainingDays() const;
		[[nodiscard]] int getRemainingAttempts() const;
		[[nodiscard]] const QString& getBlockingCode() const;

	private:
		void parse();
		bool handleFoundElement(const QString& pElementName, const QString& pValue, const QXmlStreamAttributes& pAttributes) override;

		int valuetoInt(const QString& pValue) const;
};

} // namespace governikus
