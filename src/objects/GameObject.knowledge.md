# GameObject Knowledge

- Base class for all game objects
- Handles common properties like position, size, and collision
- Manages object state and history for time manipulation
- Supports multiple sprites for different states or animations

Key properties:
- id: Unique identifier for each object
- state: Current state of the object
- colliderType: Type of collision detection (NONE, CIRCLE, BOX)
- backwards: Flag indicating if the object is moving backwards in time

Important methods:
- isColliding(): Checks collision with another GameObject
- activeAt(): Checks if the object is active at a given tick

Note: When adding new game object types, inherit from this class and implement specific behavior.

## Interactive Objects

The game now supports interactive objects, starting with the Door class. These objects can change state based on player interaction or game events, adding depth to level design and gameplay mechanics. 

Key points for interactive objects:
- Inherit from GameObject
- Use multiple sprites to represent different states (e.g., open/closed for doors)
- May require new interaction methods in the GameController
- Can potentially affect game logic and time manipulation mechanics

When designing new interactive objects, consider:
- How they fit into the existing game mechanics
- Their impact on level design and puzzle creation
- Potential interactions with time manipulation features

## Object Relationships

The game now supports complex relationships between objects, such as doors connected to multiple switches. This introduces new gameplay mechanics and puzzle possibilities:

- Objects can be linked to affect each other's states (e.g., switches controlling doors)
- These relationships add complexity to level design and time manipulation puzzles
- Consider how object relationships interact with the game's time mechanics
- Implement these relationships carefully to maintain consistency during time manipulation

When implementing new object relationships:
- Ensure that the relationship logic is clear and maintainable
- Consider the impact on game performance, especially with many interlinked objects
- Design puzzles that leverage these relationships in interesting ways
- Test thoroughly to prevent paradoxes or inconsistencies during time manipulation

