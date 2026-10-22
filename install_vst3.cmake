#
# Copy the built VST3 into the folder a host scans, and do not fail the build if it cannot.
#
# A DAW holds a plugin's binary open while a set using it is loaded, so a rebuild with the
# host running cannot replace the installed copy. JUCE's COPY_PLUGIN_AFTER_BUILD turns that
# into a failed build; this says plainly what happened instead. Invoked with -DSRC= -DDST=.
#
if(NOT EXISTS "${SRC}")
    message(STATUS "Mz7: nothing to install - ${SRC} does not exist")
    return()
endif()

get_filename_component(BUNDLE "${SRC}" NAME)

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_directory "${SRC}" "${DST}/${BUNDLE}"
    RESULT_VARIABLE COPY_RESULT
    OUTPUT_QUIET
    ERROR_QUIET)

if(COPY_RESULT EQUAL 0)
    message(STATUS "Mz7: installed to ${DST}/${BUNDLE}")
else()
    message(STATUS "")
    message(STATUS "  Mz7: the plugin BUILT but could not be INSTALLED.")
    message(STATUS "  Your host almost certainly has the old one loaded. Close it and build again.")
    message(STATUS "    wanted: ${DST}/${BUNDLE}")
    message(STATUS "")
endif()
