echo off

:: строки с точки это комментарий и они не запускаются
:: original video -> https://youtu.be/eqKVuLcLduM
:: Special thanks to Yuri Popov

:: впиши место установки лаунчера эпиков, меняй только начало
set VersionSelector=U:\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe

:: впиши имя своего проекта
set ProjectName=MacroTest.uproject
set ProjectPath=%CD%\%ProjectName%

:: для выбора версии нужно указать адрес папки в которой установлен движок
:: for UE 5
set EnginePath=U:\UNREAL_ENGINE\UE_5.4
set UBTRelativePatch=Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe

:: for UE 4
::set EnginePath=U:\UNREAL_ENGINE\UE_4.27
::set UBTRelativePatch=Engine\Binaries\DotNET\UnrealBuildTool.exe

::Start version selector
"%VersionSelector%" -switchversionsilent %ProjectPath% %EnginePath%
"%EnginePath%\%UBTRelativePatch%" -projectfiles -progress -project=%ProjectPath% 

:: flags for builds from source: -game -engine

::echo %ProjectPath%
pause