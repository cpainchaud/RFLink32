@echo off
setlocal

powershell -ep Bypass .\pulses_to_csv.ps1 %*
