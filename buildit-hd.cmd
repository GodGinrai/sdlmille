@echo off

@rem change directory
cd src

@rem Set the device you want to build for to 1
set PRE=1
set PIXI=0
set DEBUG=0

@rem List your source files here
set SRC=Card.cpp Deck.cpp Player.cpp Main.cpp Game.cpp Game_Graphics.cpp Hand.cpp Surface.cpp Tableau.cpp Options.cpp Stats.cpp Dimensions.cpp

@rem List the libraries needed
set LIBS=-lSDL -lSDL_ttf -lSDL_image -lpdl

@rem Name your output executable
set OUTFILE=..\pkg-hd\SDLMilleHD.elf

if %PRE% equ 0 if %PIXI% equ 0 goto :END

if %DEBUG% equ 1 (
   set DEVICEOPTS=-g
) else (
   set DEVICEOPTS=
)

set DEVICEOPTS=%DEVICEOPTS% -fno-exceptions -s

if %PRE% equ 1 (
   set DEVICEOPTS=%DEVICEOPTS% -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp
)

if %PIXI% equ 1 (
   set DEVICEOPTS=%DEVICEOPTS% -mcpu=arm1136jf-s -mfpu=vfp -mfloat-abi=softfp
)

echo %DEVICEOPTS%

arm-none-linux-gnueabi-g++ %DEVICEOPTS% -o %OUTFILE% %SRC% "-I%PALMPDK%\include" "-I%PALMPDK%\include\SDL" "-L%PALMPDK%\device\lib" -Wl,--allow-shlib-undefined %LIBS%

cd ..

goto :EOF

:END
echo Please select the target device by editing the PRE/PIXI variable in this file.
exit /b 1

