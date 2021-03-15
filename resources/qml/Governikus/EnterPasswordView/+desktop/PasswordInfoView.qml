/*
 * \copyright Copyright (c) 2019-2021 Governikus GmbH & Co. KG, Germany
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Governikus.Global 1.0
import Governikus.Style 1.0
import Governikus.TitleBar 1.0
import Governikus.View 1.0
import Governikus.Type.ApplicationModel 1.0
import Governikus.Type.NumberModel 1.0


SectionPage {
	id: root

	signal close()

	property int passwordType: NumberModel.passwordType
	property alias rootEnabled: titleBarAction.rootEnabled

	Accessible.name: qsTr("Password information")
	Accessible.description: qsTr("This is the password information section of the AusweisApp2.")
	Keys.onReturnPressed: close()
	Keys.onEnterPressed: close()
	Keys.onEscapePressed: close()

	titleBarAction: TitleBarAction {
		id: titleBarAction

		rootEnabled: ApplicationModel.currentWorkflow === ""
		showSettings: false
		text: headline.text
		showHelp: false
	}

	GText {
		id: headline

		anchors.top: parent.top
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.margins: Constants.pane_padding

		activeFocusOnTab: true
		Accessible.name: headline.text

		wrapMode: Text.WordWrap
		//: LABEL DESKTOP_QML
		text: passwordType === NumberModel.PASSWORD_CAN ? qsTr("CAN information")
			 //: LABEL DESKTOP_QML
		     : passwordType === NumberModel.PASSWORD_PUK ? qsTr("PUK information")
			 //: LABEL DESKTOP_QML
		     : passwordType === NumberModel.PASSWORD_REMOTE_PIN ? qsTr("Smartphone as card reader information")
			 //: LABEL DESKTOP_QML
			 : qsTr("PIN information")
		textStyle: Style.text.header_inverse
		FocusFrame {}
	}

	GText {
		anchors.top: headline.bottom
		anchors.left: parent.left
		anchors.right: infoImage.left
		anchors.margins: Constants.pane_padding

		activeFocusOnTab: true

		text: {
			if (passwordType === NumberModel.PASSWORD_CAN && NumberModel.isCanAllowedMode) {
				//: INFO DESKTOP_QML Description text of CAN-allowed authentication
				return qsTr("The Card Access Number (CAN) allows to access the imprinted data of the ID card. In order to allow on-site reading of the personal data the service provider needs to acquire governmental authorization to do so. On-site reading is usually employed to automatically fill forms and prevent spelling mistakes when transfering the personal data.")
			}
			if (passwordType === NumberModel.PASSWORD_CAN && !NumberModel.isCanAllowedMode) {
				//: INFO DESKTOP_QML Description text of CAN if required for third PIN attempt
				return qsTr("The Card Access Number (CAN) is required if the PIN has already been entered incorrectly twice. In order to prevent a third incorrect entry and thus the blocking of the PIN without your consent, the CAN is also requested at this point. The CAN is a six-digit number that can be found on the front of the ID card. It is located at the bottom right next to the validity date (marked in red).")
			}
			if (passwordType === NumberModel.PASSWORD_PUK) {
				//: INFO DESKTOP_QML Description text of PUK
				return qsTr("The Personal Unblocking Key (PUK) is required if the PIN has been entered three times. At this point the PIN is blocked. The PUK is a ten-digit number you received with the letter sent to you by your competent authority (marked in red). Please note that you can only use the PUK to unblock the PIN entry. If you have forgotten your PIN, you can have a new PIN set at your competent authority.")
			}
			if (passwordType === NumberModel.PASSWORD_REMOTE_PIN) {
				//: INFO DESKTOP_QML Description text of SaC pairing
				return qsTr("You may use your smartphone as a card reader with AusweisApp2. The smartphone needs to feature a supported NFC chipset and both devices, your smartphone and this machine, need to be connected to the same WiFi network.<br><br>To use your smartphone as a card reader you'll always need to activate the remote service in the AusweisApp2 on the smartphone. For the first time you'll also need to start the pairing mode on your smartphone, select the device from the list of available devices on this machine and then enter the pairing code shown on the phone.")
			}

			//: INFO DESKTOP_QML Description text of PIN
			return qsTr("The Personal Identification Number (PIN) is chosen by you and is required for every use of the online eID function. You can change it anytime and indefinitely if you know your valid PIN. For your six-digit PIN choose a combination of numbers, that is not easy to guess, neither \"123456\" nor your birth date, or any other numbers printed on the ID card. If you are no longer aware of your valid PIN, you will need to contact your responsible authority to renew your PIN.<br><br>When changing the PIN for the first time, please use the five-digit Transport PIN. You will find the Transport PIN in the letter you received from your responsible authority (marked in red) after you have applied for your identity card.<br><br>Please note that you can not use the online eID function with the five-digit Transport PIN. A change to a six-digit PIN is mandatory.")
		}
		textStyle: Style.text.header_inverse
		horizontalAlignment: Text.AlignJustify

		FocusFrame {}
	}

	PasswordInfoImage {
		id: infoImage

		anchors.top: headline.bottom
		anchors.right: parent.right
		anchors.margins: Constants.pane_padding

		passwordType: root.passwordType
		scaleFactorGeneral: 2.75 * ApplicationModel.scaleFactor
		scaleFactorCan: 2 * ApplicationModel.scaleFactor
		textStyle: Style.text.normal_inverse
	}

	NavigationButton {
		id: button

		anchors {
			margins: Constants.component_spacing
			bottom: parent.bottom
			horizontalCenter: parent.horizontalCenter
		}

		buttonType: NavigationButton.Type.Back
		onClicked: root.close()
	}
}
