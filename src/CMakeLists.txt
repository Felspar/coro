add_library(felspar-coro
    exceptions.cpp
)
target_compile_features(felspar-coro PUBLIC cxx_std_20)
target_compile_options(felspar-coro PUBLIC ${coro_opt})
target_include_directories(felspar-coro PUBLIC ../include)
target_link_libraries(felspar-coro PUBLIC felspar-memory felspar-exceptions)

install(TARGETS felspar-coro LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(DIRECTORY ../include/felspar DESTINATION include)

