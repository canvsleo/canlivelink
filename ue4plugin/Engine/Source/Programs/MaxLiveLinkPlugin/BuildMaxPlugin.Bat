@echo off

RMDIR %~dp0Staging /S /Q
..\..\..\Build\BatchFiles\RunUAT.bat  BuildGraph -Script=Engine/Source/Programs/MaxLiveLinkPlugin/BuildMaxPlugin.xml -Target="Stage Max Plugin Module"