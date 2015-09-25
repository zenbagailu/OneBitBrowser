include_directories (${CMAKE_SOURCE_DIR}) # so it can find includes with <> relative paths

add_library (mdns STATIC
             mDNSNameResolution.cpp
             UdpSocket.cpp
)
