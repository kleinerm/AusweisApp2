/*!
 * \brief Implementation of ReaderManagerPlugIn for Simulator.
 *
 * \copyright Copyright (c) 2021-2023 Governikus GmbH & Co. KG, Germany
 */

#pragma once

#include "ReaderManagerPlugIn.h"
#include "SimulatorReader.h"

#include <QScopedPointer>


namespace governikus
{

class SimulatorReaderManagerPlugIn
	: public ReaderManagerPlugIn
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "governikus.ReaderManagerPlugIn" FILE "metadata.json")
	Q_INTERFACES(governikus::ReaderManagerPlugIn)

	private:
		QScopedPointer<SimulatorReader> mSimulatorReader;

	public:
		SimulatorReaderManagerPlugIn();

		[[nodiscard]] QList<Reader*> getReaders() const override;

		void init() override;

		void startScan(bool pAutoConnect) override;
		void stopScan(const QString& pError = QString()) override;

		void insert(const QString& pReaderName, const QVariant& pData) override;

	private Q_SLOTS:
		void onSettingsChanged();
};

} // namespace governikus
