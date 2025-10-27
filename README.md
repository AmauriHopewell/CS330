# CS330
Computer Visualization and Graphic Design
Reflection on Software Design and Development:

When approaching the design of software, I typically begin by identifying the core requirements and objectives, sketching a high-level architecture to outline the system's structure. 
For my 3D scene project, this meant determining the key components—such as the clock model, lighting, and texture mapping—before diving into specifics. 

I rely heavily on breaking problems into modular, manageable parts, ensuring each element (e.g., the clock’s rim, face, and hands) can function independently yet integrate seamlessly. 
This modular approach allows me to visualize the workflow and anticipate potential challenges, like ensuring the clock face fits within the rim or managing multiple textures, 
which guided my initial design decisions.

Working on this project has honed several new design skills, particularly in hierarchical modeling and shader integration. 
Crafting a function to encapsulate the clock’s components with group transformations taught me to apply matrix composition effectively, ensuring rotations and scales propagate correctly across nested objects. 

Additionally, troubleshooting the texture issue introduced me to the importance of passing world-space uniforms (e.g., objectPosition and objectScale) to shaders, a technique I hadn’t deeply explored before. 
This experience sharpened my ability to design systems that balance flexibility (e.g., adjustable clock size) with precision (e.g., texture alignment), skills I can leverage in future complex graphical applications.

My design process for this project followed an iterative cycle of planning, prototyping, and refining. 
I started with a conceptual sketch of the clock’s geometry, then implemented a basic version using existing code, testing each part (rim, face, hands, bell) individually. 
Feedback from rendering issues—such as the clock face not filling the rim—prompted adjustments to parameters like torusMinorRadius and uniform settings. 

This trial-and-error method, combined with referencing resources like Stack Overflow, allowed me to refine the design incrementally. 
In future work, this tactic of prototyping early and iterating based on visual feedback can be applied to validate designs in real-time, especially in graphics or game development where visual accuracy is critical.

When developing programs, I typically adopt a step-by-step approach, writing small, testable code blocks before integrating them into a larger system. 
For this 3D scene, I began with the base plane and progressively added objects, using the OpenGL framework to test rendering at each stage. 
This methodical progression helped isolate issues, like lighting overexposure, which I addressed through C++ adjustments to material and light properties. 
The process reflects my preference for building a solid foundation before adding complexity, ensuring stability as features expand.New development strategies emerged during this project, notably the use of iteration to refine code. 
I experimented with different light configurations and texture mappings, reverting changes when needed (e.g., reverting the fragment shader to fix overexposure via C++ tweaks). 
This iterative approach, supported by frequent recompilation and visual debugging, allowed me to adapt to unexpected behaviors, such as the texture tiling issue, which I resolved by adjusting uniforms. 
Iteration was central, as each adjustment (e.g., tweaking clockFaceRadius or torusMinorRadius) built on the previous, gradually aligning the output with the desired clock appearance.My approach to developing code evolved significantly across the milestones. 
Initially, I focused on functional correctness, ensuring shapes rendered without errors. As challenges like lighting and texture alignment arose, I shifted toward a more experimental mindset, using trial-and-error to test hypotheses (e.g., adjusting material strengths) and leveraging online resources for inspiration. 

By the project’s completion, I embraced a more systematic iteration cycle—plan, implement, test, adjust—culminating in a polished clock model. 
This evolution reflects a growing confidence in adapting to graphical complexities, a skill I’ll carry forward.

Computer science is instrumental in reaching my goals, particularly in fields like game design or data visualization, where problem-solving and algorithmic thinking are paramount. 
The logical structuring of code and debugging skills I’ve honed can be applied to optimize performance or create interactive experiences, aligning with my aspiration to build engaging software. 
Computational graphics and visualizations enhance my educational pathway by providing hands-on experience with shaders and matrices, concepts I can explore further in advanced graphics courses or research, deepening my technical expertise.

Professionally, computational graphics and visualizations equip me with marketable skills in 3D modeling and real-time rendering, valuable in industries like gaming, film, or virtual reality. 
The ability to manipulate textures, lights, and transformations—gained through this project—translates to creating visually appealing assets or optimizing rendering pipelines. 
These skills, combined with my evolving development approach, position me to contribute to innovative projects, potentially leading roles in design or technical art, fulfilling my career ambitions in creative technology.



