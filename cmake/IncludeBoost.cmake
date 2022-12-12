find_package(
    Boost REQUIRED COMPONENTS
    log
    program_options
    unit_test_framework
    wave
    thread
    date_time
    locale
    chrono
    filesystem
    atomic
    system
)

# Workaround for linking failure of Boost.UUID
if (MSVC)
	list(APPEND Boost_LIBRARIES bcrypt.lib)
endif()