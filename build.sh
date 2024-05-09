g++ --std=c++23 -o game \
	main.cc \
	GameController.cc \
	GameObject.cc \
	Graphics.cc \
	TextureBank.cc \
	-lsfml-graphics -lsfml-window -lsfml-system
