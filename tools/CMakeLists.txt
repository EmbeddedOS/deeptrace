set(EXECUTABLES
    sbench
)

foreach(EXE_NAME IN LISTS EXECUTABLES)
    add_executable(${EXE_NAME} ${EXE_NAME}.cc)
endforeach()