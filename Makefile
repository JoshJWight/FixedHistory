CXX = g++
CXXFLAGS = --std=c++23
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

SRCS = main.cc GameController.cc GameObject.cc Graphics.cc TextureBank.cc Player.cc Bullet.cc Enemy.cc Level.cc Controls.cc
OBJS = $(SRCS:.cc=.o)
TARGET = game

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
