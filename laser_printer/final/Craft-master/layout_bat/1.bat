@echo off
reg add "hklm\system\currentcontrolset\control\keyboard layout" /v "ScanCode Map" /t REG_BINARY /d "0000000000000000050000003a00010001003a0052e037e037e052e000000000" /f
echo "��λ������޸ģ�����ϵͳ����Ч"
pause