@echo off
chcp 65001 >nul
echo ==========================================
echo   TAO CAU TRUC DU AN BTL4_BOARDGAME
echo ==========================================

echo.
echo [1/3] Dang tao cac thu muc...
mkdir include\Core 2>nul
mkdir include\Entities 2>nul
mkdir include\Map 2>nul
mkdir include\Logic 2>nul
mkdir include\AI 2>nul
mkdir src\Core 2>nul
mkdir src\Entities 2>nul
mkdir src\Map 2>nul
mkdir src\Logic 2>nul
mkdir src\AI 2>nul
mkdir assets\sprites 2>nul
mkdir assets\fonts 2>nul
mkdir assets\sounds 2>nul
mkdir data 2>nul

echo [2/3] Dang tao cac file header (.hpp)...
type nul > include\Core\Engine.hpp
type nul > include\Core\Resource.hpp
type nul > include\Core\Common.hpp
type nul > include\Entities\Character.hpp
type nul > include\Entities\Skill.hpp
type nul > include\Entities\Team.hpp
type nul > include\Map\Grid.hpp
type nul > include\Map\Tile.hpp
type nul > include\Logic\TurnManager.hpp
type nul > include\Logic\Combat.hpp
type nul > include\Logic\SaveSystem.hpp
type nul > include\AI\AIInterface.hpp
type nul > include\AI\AIRandom.hpp
type nul > include\AI\AIMinimax.hpp
type nul > include\AI\AIMCTS.hpp

echo [3/3] Dang tao cac file source (.cpp) va file phu...
type nul > src\Core\Engine.cpp
type nul > src\Core\Resource.cpp
type nul > src\Entities\Character.cpp
type nul > src\Entities\Skill.cpp
type nul > src\Entities\Team.cpp
type nul > src\Map\Grid.cpp
type nul > src\Map\Tile.cpp
type nul > src\Logic\TurnManager.cpp
type nul > src\Logic\Combat.cpp
type nul > src\Logic\SaveSystem.cpp
type nul > src\AI\AIRandom.cpp
type nul > src\AI\AIMinimax.cpp
type nul > src\AI\AIMCTS.cpp
type nul > src\main.cpp

echo {} > data\savegame.json
echo build/ > .gitignore
echo .vscode/ >> .gitignore
echo *.exe >> .gitignore

echo.
echo ==========================================
echo HOAN TAT! Cau truc da duoc tao thanh cong.
echo ==========================================
pause