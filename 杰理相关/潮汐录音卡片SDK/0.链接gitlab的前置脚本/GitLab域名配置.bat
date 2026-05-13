@echo off
set "ip=192.168.1.212"
set "domain=tidex-gitlab.com"
set "hostsfile=%SystemRoot%\system32\drivers\etc\hosts"

:: 检查管理员权限
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo [错误] 请右键点击此脚本，选择“以管理员身份运行”！
    pause
    exit
)

echo 正在更新 GitLab 域名映射到 %domain%...

:: 写入新纪录（这里为了保险，先删除旧的，再加新的）
findstr /v "%domain%" "%hostsfile%" > "%temp%\hosts.tmp"
echo %ip%  %domain% >> "%temp%\hosts.tmp"
copy /y "%temp%\hosts.tmp" "%hostsfile%" >nul

echo.
echo ==========================================
echo 配置完成！
echo 现在请访问 http://%domain%
echo ==========================================
pause
