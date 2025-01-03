##########################################################################
# Copyright (c) 2010, Oracle.  All rights reserved.
#
# NAME
#   outsidein/OIT_CVOB/samples/exjava/api/src/CMakeLists.txt
#
# DESCRIPTION
#   This file is used to build the ExJava API jar file.
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
	com/outsideinsdk/Export.java
	com/outsideinsdk/ExportFormat.java
	com/outsideinsdk/ExportProperties.java
	com/outsideinsdk/ExportResources.java
	com/outsideinsdk/ExportStatusCode.java
	com/outsideinsdk/ExportThread.java
	)

add_jar(Export
	SOURCES
	${BUILD_SOURCEFILES} 			
	OUTPUT_DIR			
	${CMAKE_JAVA_TARGET_OUTPUT_DIR}
	)

add_custom_command(TARGET Export
                   POST_BUILD
		   COMMAND ${CMAKE_COMMAND} -E copy_if_different Export.jar ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Export.jar
		   COMMENT "Moving Export.jar into runtime directory."
		   )
