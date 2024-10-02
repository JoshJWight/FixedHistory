# GameController Knowledge

- Central class managing game logic and state
- Handles ticking of game objects
- Manages time manipulation mechanics
- Coordinates interactions between different game objects
- Handles paradox checking and resolution
- Manages history buffers for time manipulation

Key methods:
- mainLoop(): Main game loop
- tick(): Advances game state
- checkParadoxes(): Checks for time paradoxes
- pushTimeline(): Creates a new timeline
- popTimeline(): Removes the current timeline

Note: Be cautious when modifying this class as it's central to the game's core mechanics.

