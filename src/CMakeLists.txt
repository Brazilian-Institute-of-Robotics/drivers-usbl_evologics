rock_library(usbl_evologics
    SOURCES Driver.cpp UsblParser.cpp
    HEADERS Driver.hpp UsblParser.hpp DriverTypes.hpp Exceptions.hpp
    DEPS_PKGCONFIG base-types iodrivers_base)


rock_executable(usbl_evologics_bin Main.cpp
    DEPS usbl_evologics)

