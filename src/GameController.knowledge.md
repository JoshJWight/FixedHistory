# GameController Knowledge

## Door Ticking Mechanism

The GameController now includes a function for ticking doors, which introduces a complex interaction between doors, switches, and time manipulation:

- Doors are connected to multiple switches
- Door state (open/closed) is determined by the number of active switches:
  - Odd number of active switches: door is open
  - Even number of active switches: door is closed
- The ticking function handles both forward and backward time progression
- This mechanism adds depth to puzzle design and time manipulation gameplay

Key considerations:
- Ensure consistency of door states during time manipulation
- Be aware of potential paradoxes or unexpected behavior in complex switch-door setups
- Consider the impact on level design and puzzle complexity
- Test thoroughly with various switch-door configurations and time manipulation scenarios

This new mechanic opens up possibilities for intricate time-based puzzles and challenges players to think about cause-and-effect relationships across different timelines.
