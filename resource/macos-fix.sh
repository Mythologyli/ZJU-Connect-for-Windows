#!/bin/bash
# macOS 提示已损坏修复

clear
BLACK="\033[0;30m"  
DARK_GRAY="\033[1;30m"  
BLUE="\033[0;34m"  
LIGHT_BLUE="\033[1;34m"  
GREEN="\033[0;32m"  
LIGHT_GREEN="\033[1;32m"  
CYAN="\033[0;36m"  
LIGHT_CYAN="\033[1;36m"  
RED="\033[0;31m"  
LIGHT_RED="\033[1;31m"  
PURPLE="\033[0;35m"  
LIGHT_PURPLE="\033[1;35m"  
BROWN="\033[0;33m"  
YELLOW="\033[0;33m"  
LIGHT_GRAY="\033[0;37m"  
WHITE="\033[1;37m" 
NC="\033[0m"

echo -e "${BLUE}『已损坏，无法打开/ 来自身份不明的开发者』等问题修复工具${NC}"
echo

parentPath=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd "$parentPath"
appPath=$( find "$parentPath" -name '*.app' -maxdepth 1)

if [[ -z "$appPath" ]]
then
    echo -e "${RED}❌ 错误：${NC}未找到应用"
    echo "${YELLOW}可关闭此窗口${NC}"
    exit 1
fi

echo -e "${GREEN}待修复应用：${NC}${appPath}"

appName=${appPath##*/}
appBashName=${appName// /\ }
appDIR="/Applications/${appBashName}"

if [[ ! -d "$appDIR" ]]
then
    echo -e "${RED}❌ 错误：${NC}尚未将应用移动到应用程序目录"
    echo "请将应用移动到应用程序目录后，再尝试修复"
    echo "可关闭此窗口"
    exit 1
fi

echo
echo -e "${YELLOW}⚠️ 注意事项：${NC}"
echo "1. 接下来会需要您输入系统的开机密码"
echo "2. 密码输入没有回显，请直接输入"
echo "3. 输入完密码后，按下回车，即可开始修复"
echo
echo -e "${LIGHT_CYAN}========== 开始修复 ==========${NC}"
sudo xattr -rd com.apple.quarantine /Applications/"$appBashName"
sudo xattr -rc /Applications/"$appBashName"
sudo codesign --sign - --force --deep /Applications/"$appBashName"
echo -e "${LIGHT_CYAN}========== 修复完成 ==========${NC}"
echo "${YELLOW}可关闭此窗口${NC}"
