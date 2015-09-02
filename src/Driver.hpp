#ifndef _DUMMYPROJECT_DRIVER_HPP_
#define _DUMMYPROJECT_DRIVER_HPP_

#include <iodrivers_base/Driver.hpp>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include "UsblParser.hpp"
#include "IMParser.hpp"
#include "IMDataTypes.hpp"
#include "DriverTypes.hpp"

namespace usbl_evologics
{
    
    class Driver : public iodrivers_base::Driver
    {

    	public:



    		Driver();
    		~Driver();

    		void setInterface(InterfaceType	deviceInterface);

            // send a data to device
            // Send both command and raw data;
    		// If a command is sent, set 'mailCommand' as false and wait for a response
            bool sendData(bool &mailCommand);


            // return a valid answer, as a response or as a notification.
            // resp=true means the unique and necessary response of a command is received.
            // queueCommand.pop() and waitCommand=True realized in this case.
            bool readAnswer(bool &resp, std::string &output_message, std::string &raw_data);



    		// List of Commands. They will be pushed to queueCommand
    		void getConnetionStatus(void);
    		void getCurrentSetting(void);
    		void getIMDeliveryStatus(void);
    		void goSurface(void);






    		// Guard Time Escape Sequence
    		// Switches to COMMAND mode
    		void GTES(void);


    		InterfaceType getInterface(void);

    		void sendInstantMessage(SendedIM const &im);
    		void receiveInstantMessage(ReceivedIM &im);

    		void sendRawData(std::string const& raw_data);

    		int getSizeQueueCommand(void);
    		void enqueueCommand(std::string & command);


        private:
    		UsblParser	usblParser;
    		IMParser    imParser;

			OperationMode	mode;
			InterfaceType	interface;

			Position		pose;
			SendedIM		sendedIM;
            ReceivedIM		receveidIM;
			ConnectionStatus	connection_state;

//			CommandResponse		response;
//			Notification		notification;

			VersionNumbers device;
			StatusRequest device_status;
			DeviceSettings device_settings;
			DataChannel channel;
			AcousticChannel acoustic_channel;



            std::queue<std::string> queueCommand;
            std::queue<std::string> queueRawData;

            bool mailCommand;
            static const int max_packet_size = 20000;

            // send a command to device
            // check queueCommand and set waitCommand=False
            bool sendCommand(bool &mailCommand);
            // send a command to device
            // check queueCommand and set waitCommand=False
            bool sendRawData(void);

    		// aux function used by readAnswer().
    		int readInternal(std::string& buffer);
//            int checkRawData(std::string & buffer) const ;
    		int checkNotificationCommandMode(std::string const& buffer) const;
            int checkParticularResponse(std::string const& buffer) const;
            int checkRegularResponse(std::string const& buffer) const;

    		bool isResponse(std::string const &buffer, CommandResponse &response);
    		bool validResponse(std::string const &buffer);

    		bool isNotification(std::string const &buffer, Notification &notification);
    		bool validNotification(std::string const &buffer);
    		bool fullValidation(std::string const &buffer, Notification const &notification, std::string &output_msg);

    		std::string interpretResponse(std::string const &buffer, std::string const &command, CommandResponse const &response);
    		std::string interpretNotification(std::string const &buffer, Notification const &notification);

            std::string fillCommand(std::string const &command);
            std::string addEndLine (std::string const &command);

    		void modeManager(std::string const &command);
    		void modeMsgManager(std::string const &command);

		protected:

			//Pure virtual function from Driver
			virtual int extractPacket(uint8_t const* buffer, size_t buffer_size) const;
    };
 
}
#endif 
