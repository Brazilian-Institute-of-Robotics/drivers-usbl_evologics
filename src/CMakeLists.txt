rock_library(usbl_evologics
    SOURCES UsblParser.cpp Driver.cpp 
    HEADERS DriverTypes.hpp UsblParser.hpp Driver.hpp Exceptions.hpp 
    DEPS_PKGCONFIG base-types iodrivers_base)
