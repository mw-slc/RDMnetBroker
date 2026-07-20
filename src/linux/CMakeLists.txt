find_package(Threads REQUIRED)

add_executable(RDMnetBrokerService
  broker_service.h
  broker_service.cpp
  linux_broker_os_interface.h
  linux_broker_os_interface.cpp
  main.cpp
)
set_target_properties(RDMnetBrokerService PROPERTIES CXX_STANDARD 17)
target_link_libraries(RDMnetBrokerService PRIVATE RDMnetBrokerServiceCore Threads::Threads)

include(GNUInstallDirs)
install(TARGETS RDMnetBrokerService
  RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR}
)
