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

