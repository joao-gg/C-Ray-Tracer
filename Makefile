CC = gcc

OUT = main

CFLAGS = 					\
	-Wall 					\
	-Wextra 				\
	-std=c2x
	
OPTFLAGS = 					\
	-O3 					\
	-march=native 			\
	-fno-math-errno 		\
	-flto 					\
	-fopenmp

SANFLAGS = 
# SANFLAGS = 				\
# 	-fsanitize=address 		\
# 	-fsanitize=undefined

LDFLAGS = 					\
	-lm

SRCS = ./src/lib/vec3.c ./src/lib/sphere.c ./src/main.c

OBJS = $(SRCS:.c=.o)

all:
	@echo ""
	@echo "Options:"
	@echo "  . make ray_tracer"
	@echo ""
	@echo "Suggestions:"
	@echo "  . make ray_tracer; time ./main; pnmtopng render.ppm > output.png"
	@echo ""

.PHONY: ray_tracer
ray_tracer:
	$(CC) $(SRCS) $(CFLAGS) $(OPTFLAGS) $(SANFLAGS) $(LDFLAGS) -o $(OUT)

.PHONY: clean
clean:
	rm -f $(OUT) render.ppm output.png
