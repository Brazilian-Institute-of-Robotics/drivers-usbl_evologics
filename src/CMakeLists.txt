rock_library(usbl_evologics
    SOURCES Dummy.cpp
    HEADERS Dummy.hpp)

rock_executable(usbl_evologics_bin Main.cpp
    DEPS usbl_evologics)

