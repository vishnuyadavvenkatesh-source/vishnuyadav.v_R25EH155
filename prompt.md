1.Why is the Shape struct designed with a union inside it instead of declaring separate variables for each shape type?
2.What does MAX_SHAPES control and what happens when the user tries to add a shape beyond that limit?
3.Why does main() declare canvas[HEIGHT][WIDTH] as a local variable instead of a global variable?.
4.What is the difference between add_shape_flow() passing &shape_count and modify_shape_flow() passing shape_count from main()?
5.In add_shape_flow(), the new shape is built locally as Shape new_shape and then copied into list[*count]. Why is this approach used instead of directly filling list[*count]?
6.The Shape struct uses a union for its parameters. What is the size of this union in memory and which shape type determines that size — and why?
7.What would happen if clear_input_buffer() was removed from the code entirely — give a step by step scenario?
8.Why does get_int() use an infinite while(1) loop instead of a fixed number of retries, and what are the advantages and disadvantages of this approach?
