##########################################################################
# Copyright (c) 2010, Oracle.  All rights reserved.
#
# NAME
#   outsidein/OIT_CVOB/samples/exjava/Examples/ExportTest/CMakeLists.txt
#
# DESCRIPTION
#   This file is used to build the ExJava ExportTest sample app.
#
# EXPORT FUNCTIONS
#
# INTERNAL FUNCTIONS
#
# NOTES
#
# MODIFIED  (MM/DD/YY)
#    mjryan  02/14/14 - Adding CMake build scripts.
#
############################################################################

set(CMAKE_JAVA_COMPILE_FLAGS "-source" "1.4" "-target" "1.4")
set(BUILD_SOURCEFILES
	ExportTest.java
	)

add_jar(ExportTest 
		SOURCES
		${BUILD_SOURCEFILES} 			
		OUTPUT_DIR			
		${CMAKE_JAVA_TARGET_OUTPUT_DIR}
		INCLUDE_JARS
		Export
		)

add_dependencies(ExportTest Export)

add_custom_command(TARGET ExportTest
	                   POST_BUILD
					   COMMAND ${CMAKE_COMMAND} -E copy_if_different ExportTest.jar ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ExportTest.jar					   
					   COMMENT "Moving ExportTest.jar into runtime directory."
					   )
