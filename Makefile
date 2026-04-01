TARGET = GameDev_4.exe
CXX = g++

# Thêm -I include để trình biên dịch hiểu đường dẫn tới các file .hpp của bạn
CXXFLAGS = -O2 -std=c++17 -I include

# Đường dẫn thư viện giữ nguyên theo repo mẫu
INCLUDE_DIR = -I SDL/x86_64-w64-mingw32/include
LIB_DIR = -L SDL/x86_64-w64-mingw32/lib
LIBS = -lSDL3 -lSDL3_ttf

# Danh sách toàn bộ các file mã nguồn (.cpp) của dự án GameDev_4
SRCS = src/main.cpp \
       src/Core/Engine.cpp \
       src/Core/Resource.cpp \
       src/Entities/Character.cpp \
       src/Entities/Skill.cpp \
       src/Entities/Team.cpp \
       src/Map/Grid.cpp \
       src/Map/Tile.cpp \
       src/Logic/TurnManager.cpp \
       src/Logic/Combat.cpp \
       src/Logic/SaveSystem.cpp \
       src/AI/AIInterface.cpp \
       src/AI/AIRandom.cpp \
       src/AI/AIMinimax.cpp \
       src/AI/AIMCTS.cpp

all:
	$(CXX) $(SRCS) $(CXXFLAGS) $(INCLUDE_DIR) $(LIB_DIR) $(LIBS) -o $(TARGET)

clean:
	del $(TARGET)