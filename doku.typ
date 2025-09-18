== symbol
- a symbol is defined as $a<x,y,z>$ where $x,y,z$ are args
- args are evaluated in reverse, so x can depend on y and z,
  and y can depend on z
- when a symbol is expanded trough a rule $x,y,z$ refere to the
  symbol that is expanded and can be used in other symbols: \
  $A<x,y,z> -> B<x>-<z>$
- A -> 
  - $A$ (all default)
  - $A<;;>$ (all default)
  - $A<x; y; z>$ (no change)
  - $A<x;;>$ (y,z default)
  - $A<x>$ (y,z default)
  - $A<{*, 2}>$ (default for x scaled)
  - $A<x{*, 2}>$ (x scaled)
- symbols
  - [,] (start / end branch)
  - +,- (rotate left / right) -> default_rotation_angle
  - \,/ (increase / decrease widht) -> "?? where to save this information??"
  - !,? (increase / decrease hue) -> "implement later, do the above first"
  - ABCD (draw branch) -> default_len
  - abcd (jump) -> default_len

$a<x{*,2}>$
$A<x{*,2}>$


== components
- the L-String is generated via button press or at leas seperatly and first
- Structures are generated and drawn over one or more frames
- they get regenerated if global vars or the viewport changes
  - "maybe they could exist in local coordinate systems, so that
     they dont need to be regenerated on viewport changes. Maybe
     this can be done with blending?"
- 
