g++ --std=c++23 -o game \
	main.cc \
	GameController.cc \
	GameObject.cc \
	Graphics.cc \
	TextureBank.cc \
	Player.cc \
	Bullet.cc \
	Enemy.cc \
	Level.cc \
	Controls.cc \
	-lsfml-graphics -lsfml-window -lsfml-system
