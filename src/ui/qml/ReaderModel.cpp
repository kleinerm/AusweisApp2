/*!
 * \copyright Copyright (c) 2017-2023 Governikus GmbH & Co. KG, Germany
 */

#include "ReaderModel.h"

#include "Env.h"
#include "HelpAction.h"
#include "LanguageLoader.h"
#include "ReaderConfiguration.h"
#include "ReaderManager.h"

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
	#include "ReaderDetector.h"
#endif

#include <QQmlEngine>


using namespace governikus;


ReaderModel::ReaderModel(QObject* pParent)
	: QAbstractListModel(pParent)
	, mKnownDrivers()
	, mConnectedReaders()
	, mConnectedReadersUpdateTime()
	, mSortedModel()
{
	const ReaderManager* const readerManager = Env::getSingleton<ReaderManager>();
	connect(readerManager, &ReaderManager::fireReaderAdded, this, &ReaderModel::onUpdateContent);
	connect(readerManager, &ReaderManager::fireReaderRemoved, this, &ReaderModel::onUpdateContent);
	connect(readerManager, &ReaderManager::fireStatusChanged, this, &ReaderModel::onUpdateContent);
	connect(Env::getSingleton<ReaderConfiguration>(), &ReaderConfiguration::fireUpdated, this, &ReaderModel::onUpdateContent);
#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
	connect(Env::getSingleton<ReaderDetector>(), &ReaderDetector::fireReaderChangeDetected, this, &ReaderModel::onUpdateContent);
#endif

	mSortedModel.setSourceModel(this);
	QQmlEngine::setObjectOwnership(&mSortedModel, QQmlEngine::CppOwnership);
	connect(this, &ReaderModel::fireModelChanged, &mSortedModel, &SortedReaderModel::onDataChanged);

	onUpdateContent();
}


void ReaderModel::collectReaderData()
{
	mKnownDrivers.clear();
	mConnectedReaders.clear();

	const QVector<ReaderInfo> installedReaders = Env::getSingleton<ReaderManager>()->getReaderInfos(ReaderFilter({
				ReaderManagerPlugInType::PCSC, ReaderManagerPlugInType::NFC
			}));

	for (const auto& installedReader : installedReaders)
	{
		const auto& readerSettingsInfo = installedReader.getReaderConfigurationInfo();
		mKnownDrivers += readerSettingsInfo;
		mConnectedReaders += readerSettingsInfo;
	}

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
	QVector<ReaderConfigurationInfo> readersWithoutDriver;
	const auto& attachedSupportedDevices = Env::getSingleton<ReaderDetector>()->getAttachedSupportedDevices();
	for (const auto& info : attachedSupportedDevices)
	{
		if (!mConnectedReaders.contains(info))
		{
			readersWithoutDriver.append(info);
		}
	}
	mConnectedReaders += readersWithoutDriver;
#endif
}


bool ReaderModel::indexIsValid(const QModelIndex& pIndex) const
{
	if (!pIndex.isValid())
	{
		Q_ASSERT(false && "Invoked with an invalid QModelIndex.");
		return false;
	}

	if (pIndex.row() >= rowCount())
	{
		Q_ASSERT(false && "Invoked with a row which is out of bounds.");
		return false;
	}

	return true;
}


void ReaderModel::onUpdateContent()
{
	beginResetModel();

	collectReaderData();
	mConnectedReadersUpdateTime = QTime::currentTime();

	endResetModel();

	Q_EMIT fireModelChanged();
}


void ReaderModel::onTranslationChanged()
{
	onUpdateContent();
}


int ReaderModel::rowCount(const QModelIndex&) const
{
	return mConnectedReaders.size();
}


QVariant ReaderModel::data(const QModelIndex& pIndex, int pRole) const
{
	if (!indexIsValid(pIndex))
	{
		return QVariant();
	}

	const auto& reader = mConnectedReaders.at(pIndex.row());
	switch (pRole)
	{
		case READER_NAME:
			return reader.getName();

		case READER_IMAGE_PATH:
			return getReaderImageUrl(pIndex);

		case READER_HTML_DESCRIPTION:
			return getHTMLDescription(pIndex);

		case READER_DRIVER_URL:
			return mConnectedReaders.at(pIndex.row()).getUrl();

		case READER_SUPPORTED:
			return isSupportedReader(pIndex);

		case READER_INSTALLED:
			return isInstalledReader(pIndex);

		default:
			return QVariant();
	}
}


QHash<int, QByteArray> ReaderModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles.insert(READER_NAME, "readerName");
	roles.insert(READER_IMAGE_PATH, "readerImagePath");
	roles.insert(READER_HTML_DESCRIPTION, "readerHTMLDescription");
	roles.insert(READER_SUPPORTED, "readerSupported");
	roles.insert(READER_INSTALLED, "readerInstalled");
	return roles;
}


QUrl ReaderModel::getReaderImageUrl(const QModelIndex& pIndex) const
{
	return mConnectedReaders.at(pIndex.row()).getIcon()->lookupUrl();
}


QString ReaderModel::getHTMLDescription(const QModelIndex& pIndex) const
{
	if (!indexIsValid(pIndex))
	{
		return QString();
	}

	if (mConnectedReaders.isEmpty())
	{
		return QString();
	}

	const auto& readerSupported = isSupportedReader(pIndex);
	const auto& readerInstalled = isInstalledReader(pIndex);

	if (readerSupported)
	{
		if (readerInstalled)
		{
			//: LABEL ALL_PLATFORMS
			return tr("Driver installed");
		}

		if (!Env::getSingleton<ReaderManager>()->isScanRunning(ReaderManagerPlugInType::PCSC))
		{
			//: LABEL ALL_PLATFORMS
			return tr("The smartcard service of your system is not reachable.");
		}

		//: LABEL ALL_PLATFORMS
		const auto& driverInfo = tr("No driver installed");
		//: INFO ALL_PLATFORMS The driver for card reader needs to be installed, the download link is provided in the message.
		const auto& driverDownloadInfo = tr("Please download and install the driver you can find at: %1").
				arg(QStringLiteral("<a href=\"%1\">%1</a>").arg(mConnectedReaders.at(pIndex.row()).getUrl()));

		return driverInfo + QStringLiteral("<br>") + driverDownloadInfo;
	}

	//: INFO ALL_PLATFORMS
	return tr("This card reader is not officially supported and may not work as expected.");
}


QString ReaderModel::getEmptyListDescriptionString() const
{
	const QString& onlineHelpSection = QStringLiteral("settingsPcscReader");
	const QString& url = HelpAction::getOnlineUrl(onlineHelpSection);
	//: Is embedded in a sentence.
	const QString& hyperlink = QStringLiteral("<a href=\"%1\">%2</a>").arg(url, tr("online help"));
	//: INFO ALL_PLATFORMS No card reader was found, the message contains a link to the installation section of the manual.
	return tr("No connected card reader found. See %1 for installation of card readers.").arg(hyperlink);
}


bool ReaderModel::isSupportedReader(const QModelIndex& pIndex) const
{
	if (!indexIsValid(pIndex))
	{
		return false;
	}

	const auto& readerSettingsInfo = mConnectedReaders.at(pIndex.row());
	return readerSettingsInfo.isKnownReader();
}


bool ReaderModel::isInstalledReader(const QModelIndex& pIndex) const
{
	if (!indexIsValid(pIndex))
	{
		return false;
	}

	const auto& readerSettingsInfo = mConnectedReaders.at(pIndex.row());
	return mKnownDrivers.contains(readerSettingsInfo);
}


QString ReaderModel::getLastUpdatedInformation() const
{
	if (!mConnectedReadersUpdateTime.isValid())
	{
		return QString();
	}

	const auto& updateTime = LanguageLoader::getInstance().getUsedLocale().toString(mConnectedReadersUpdateTime, tr("hh:mm:ss AP"));
	//: LABEL ALL_PLATFORMS
	return tr("The list of card readers was last updated at %1.").arg(updateTime);
}


SortedReaderModel* ReaderModel::getSortedModel()
{
	return &mSortedModel;
}
