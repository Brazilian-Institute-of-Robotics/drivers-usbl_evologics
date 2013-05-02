#ifndef _DUMMYPROJECT_DRIVER_HPP_
#define _DUMMYPROJECT_DRIVER_HPP_

#include <iodrivers_base/Driver.hpp>
#include <vector>
#include "UsblParser.hpp"
namespace usbl_evologics
{
    enum InterfaceMode {
        BURST_MODE,
        CONFIG_MODE
    };
    enum Pending {
        PENDING_OK,
        PENDING_POSITION,
        ERROR,
        NO_PENDING
    };
    enum DelivertyStatus {
        PENDING,
        DELIVERED,
        FAILED
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
        struct Position position;
        std::vector<struct InstantMessage*> instantMessages;
    };
    struct InstantMessage {
        enum DelivertyStatus delivertyStatus;
    };
    class Driver : public iodrivers_base::Driver
    {
            int extractPacket (uint8_t const *buffer, size_t buffer_size) const;
            struct InterfaceStatus mInterfaceStatus;
            std::vector<uint8_t> buffer;
            UsblParser* mParser;
            void setInterfaceToBurstMode();
            void waitSynchronousMessage();
            void setInterfaceToConfigMode();
        public: 
            Driver();
            void read();
            void sendInstantMessage(struct InstantMessage *instantMessage);
            void sendBurstData(uint8_t const *buffer, size_t buffer_size);
            void open(std::string const& uri);
            struct Position requestPosition(bool x);
            struct Position getPosition();
    };

} 

#endif 
