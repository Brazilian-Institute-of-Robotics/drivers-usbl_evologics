rock_library(usbl_evologics
    SOURCES UsblParser.cpp Driver.cpp 
    HEADERS DriverTypes.hpp UsblParser.hpp Driver.hpp Exceptions.hpp 
    DEPS_PKGCONFIG base-types iodrivers_base)


#rock_executable(usbl_evologics_bin Main.cpp
#    DEPS usbl_evologics)

#find_package(Boost REQUIRED COMPONENTS regex)
#include_directories(${Boost_INCLUDE_DIRS})

#rock_executable(usbl_evologics_bin Main.cpp LIBS ${Boost_LIBRARIES})

