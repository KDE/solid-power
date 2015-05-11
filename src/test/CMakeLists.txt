# test binary
set(solidpower_console_SRCS
   main.cpp
)

add_executable(solidpower-console ${solidpower_console_SRCS})

target_link_libraries(solidpower-console
    Qt5::Core
    KF5::SolidPower
)
