@IF %1_X == _X GOTO showparams
@IF %2_X == _X GOTO showparams
@IF %3_X == _X GOTO showparams

@IF %4_X == _X GOTO notimeout
java -classpath .\Export.jar;ExportTest.jar ExportTest %1 %2 %3 %4
@GOTO end

:notimeout
java -classpath .\Export.jar;ExportTest.jar ExportTest %1 %2 %3
@GOTO end

:showparams
@ECHO -----------------------------------------
@ECHO . Usage is
@ECHO .    ExportTest.bat inputPath outputPath configfile [timeout(ms)]
@ECHO .
@ECHO -----------------------------------------
:end