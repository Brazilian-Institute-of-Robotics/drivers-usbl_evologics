#ifndef _DUMMYPROJECT_DRIVER_HPP_
#define _DUMMYPROJECT_DRIVER_HPP_

#include <iodrivers_base/Driver.hpp>
#include <vector>
#include <usbl_evologics/UsblParser.hpp>
namespace usbl_evologics
{
    class UsblParser;
    enum DeliveryStatus {
        PENDING,
        DELIVERED,
        FAILED
    };
    struct SendInstantMessage {
        int destination;
        bool deliveryReport;
        enum DeliveryStatus deliveryStatus;
        size_t len;
        uint8_t *buffer;
    };
    struct ReceiveInstantMessage {
        int destination;
        int source;
        size_t len;
        uint8_t *buffer;
    };
    enum InterfaceMode {
        BURST_MODE,
        CONFIG_MODE
    };
    enum Pending {
        PENDING_OK,
        PENDING_POSITION,
        PENDING_TIME,
        ERROR,
        NO_PENDING
    };
    
    struct Position {
        int time;
        float x;
        float y;
        float z;
    };
    struct InterfaceStatus{
        enum InterfaceMode interfaceMode;
        enum Pending pending;
        int time;
        struct Position position;
        std::vector<struct SendInstantMessage*> instantMessages;
    };
    
    class UsblDriverCallbacks
    {
        public:
            virtual void gotInstantMessage(struct ReceiveInstantMessage *im) =0;
            virtual void gotBurstData(uint8_t const *data, size_t data_size) =0;
    };
    class Driver : public iodrivers_base::Driver
    {
            int extractPacket (uint8_t const *buffer, size_t buffer_size) const;
            struct InterfaceStatus mInterfaceStatus;
            std::vector<uint8_t> buffer;
            UsblParser* mParser;
            UsblDriverCallbacks *mCallbacks;
            void waitSynchronousMessage();
            void setInterfaceToConfigMode();
        public: 
            Driver();
            void read();
            void sendInstantMessage(struct usbl_evologics::SendInstantMessage *instantMessage);
            void sendBurstData(uint8_t const *buffer, size_t buffer_size);
            void open(std::string const& uri);
            struct Position requestPosition(bool x);
            struct Position getPosition();
            void setInterfaceToBurstMode();
            void setDriverCallbacks(UsblDriverCallbacks *cb);
            int getSystemTime();
            void setSystemTime(int time);
    };
    

} 

#endif 
