== symbol
- a symbol is defined as $a<x,y,z>$ where $x,y,z$ are args
- args are evaluated in reverse, so x can depend on y and z,
  and y can depend on z

== components
- the L-String is generated via button press or at leas seperatly and first
- Structures are generated and drawn over one or more frames
- they get regenerated if global vars or the viewport changes
  - "maybe they could exist in local coordinate systems, so that
     they dont need to be regenerated on viewport changes. Maybe
     this can be done with blending?"
- 
